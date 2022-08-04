/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.imsstack.imsservice.mmtel.sms;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.PhoneNumberUtils;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.TelephonyManager;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.text.TextUtils;

import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.util.HexDump;
import com.android.telephony.Rlog;

import java.io.ByteArrayOutputStream;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;

/**
 * Provides the functionality of SMS Transfer Layer as per 3GPP TS 23.040
 **/
public class SmsTransferLayer {
    private final Object mLock = new Object();
    private static final String TAG = "[GII-SmsTL] ";
    private final ImsCallContext mCallContext;
    private SmsRelayLayer mSmsRL = null;
    protected SmsRLListenerProxy mSmsRLListener = new SmsRLListenerProxy();
    private SmsTransferLayer.Listener mListener = null;
    public Map<Integer, TpduParam> mTokenMessageMap = new ConcurrentHashMap<>();
    private static final int MAX_MESSAGE_COUNT = 255;
    private static final int MAX_CDMA_PDU_LENGTH = 255;
    private static final int MAX_TPDU_LENGTH = 234;
    public final ConcurrentLinkedQueue<Integer> mSendSmsQueue =
                 new ConcurrentLinkedQueue<Integer>();
    private MessageHandler mSmsHandler = null;
    private HandlerThread mSmsHandlerThread = new HandlerThread("ImsSmsHandlerThread");
    public static final int REQUEST_SEND_NEXT_SMS_TO_RL = 1;
    private static final boolean VDBG = true;
    private UsatBasedSms mUsatBasedSms = null;
    public Map<Usat.MoSmsControlCommand, TpduParam> mUsatCmdMessageMap = new ConcurrentHashMap<>();

    /**
     * Start Index of TP-DestinationAddress after MTI(0th Index), TP-MR(1st), TP-DA length(2nd)
     * and TON(3rd) bytes
     */
    private static final int TP_DA_START_INDEX = 4;
    private static final int TP_DA_LENGTH_INDEX = 2;
    private static final int TPDU_LENGTH_BEFORE_DESTINATION_ADDRESS = 3;

    /** TP-Failure-Cause (TP-FCS). See TS 23.040 9.2.3.22 */
    public static final int TP_FCS_NONE = 0x00;
    public static final int TP_FCS_SHORT_MESSAGE_TYPE_0_NOT_SUPPORTED = 0x81;
    public static final int TP_FCS_CANNOT_REPLACE_SHORT_MESSAGE = 0x82;
    public static final int TP_FCS_UNSPECIFIED_TP_PID_ERROR = 0x8F;
    public static final int TP_FCS_MESSAGE_CLASS_NOT_SUPPORTED = 0x91;
    public static final int TP_FCS_UNSPECIFIED_TP_DCS_ERROR = 0x9F;
    public static final int TP_FCS_TPDU_NOT_SUPPORTED = 0xB0;
    public static final int TP_FCS_USIM_SMS_STORAGE_FULL = 0xD0;
    public static final int TP_FCS_NO_SMS_STORAGE_CAPABILITY_IN_USIM = 0xD1;
    public static final int TP_FCS_ERROR_IN_MS = 0xD2;
    public static final int TP_FCS_MEMORY_CAPACITY_EXCEEDED = 0xD3;
    public static final int TP_FCS_USIM_APPLICATION_TOOLKIT_BUSY = 0xD4;
    public static final int TP_FCS_USIM_DATA_DOWNLOAD_ERROR = 0xD5;
    public static final int TP_FCS_UNSPECIFIED_ERROR_CAUSE = 0xFF;

    /**
     * This class contains parameters related to each SMS sent and is required for
     * maintaining the Message details in queue when there are simultaneous SMS requests
     */
    private class TpduParam {
        int mToken;
        byte[] mTpdu;
        String mSmsc;
        String mDestinationAddress;
        TpduParam(int token, byte[] pdu, String scAddress, String destinationAddr) {
            mToken = token;
            mTpdu = pdu;
            mSmsc = scAddress;
            mDestinationAddress = destinationAddr;
        }
    }

    /**
     * Listener to handle the events sent from SmsTransferLayer
     */
    public interface Listener {
        /**
         * notifies the receipt of report for SMS-SUBMIT sent
         * @param token provided in {@link #sendMoTPdu(int, int, String, byte[])}
         * @param messageRef TPMR value of respective SMS_SUBMIT message
         * @param result result of sending the SMS.
         * @param reason reason in case status is failure.
         * @param cause cause for the Failure
         */
        void notifySmsResult(int token, int messageRef, int result, int reason, int cause);

        /**
         * notifies ImsSmsImpl about the Incoming SMS-DELIVER or SMS-STATUS-REPORT message
         * @param token unique token generated to trigger callbacks for this message.
         * @param format the format of the message.
         * @param messageType Message Type with respect to TPDU
         * @param pdu PDU representing the contents of the message.
         *
         * @return the result if handling incoming RP-Data received
         */
        int notifySmsReceived(int token, int format, int messageType, byte[] pdu);
    }

    public SmsTransferLayer(ImsCallContext callContext) {
        mCallContext = callContext;
        mSmsHandlerThread.start();
        mSmsHandler = new MessageHandler(mSmsHandlerThread.getLooper());
        mSmsRL = new SmsRelayLayer(mCallContext);
        if (mSmsRL != null) {
            mSmsRL.setListener(mSmsRLListener);
        }
    }

    @VisibleForTesting
    public SmsTransferLayer(ImsCallContext callContext, SmsRelayLayer smsRL) {
        mCallContext = callContext;
        mSmsHandlerThread.start();
        mSmsHandler = new MessageHandler(mSmsHandlerThread.getLooper());
        if (smsRL == null) {
            mSmsRL = new SmsRelayLayer(mCallContext);
        } else {
            mSmsRL = smsRL;
        }
        if (mSmsRL != null) {
            mSmsRL.setListener(mSmsRLListener);
        }
    }

     /**
     * clears the objects created by this class.
     */
    public void clear() {
        mSmsRL = null;
        mSmsHandler = null;
        mUsatBasedSms.dispose();
    }

    public void setListener(SmsTransferLayer.Listener listener) {
        mListener = listener;
    }

    /**
     * Handles SMS-SUBMIT Message at Transfer Layer and notifies Relay Layer to send RP-DATA
     * @param token sent from framework to track callback for each SMS-SUBMIT message
     * @param smsFormat format of the message
     * @param tpMessageRef the TP-MR passed for the SMS-SUBMIT message
     * @param smsc the Short Message Service Center address
     * @param pdu PDU representing the contents of the message.
     *
     * @return result of processing of outgoing SMS's TPDU
     */
    public int sendMoTPdu(int token, int smsFormat, int tpMessageRef, String smsc, byte[] pdu) {
        Rlog.d(TAG, "sendMoTPdu : token = " + token
                                + "tpMessageRef = " + tpMessageRef
                                + "smsc = " + smsc
                                + "pdu = " + IccUtils.bytesToHexString(pdu));
        try {
            /* Framework's TPdu Parser expects the TPdu be prepended with SC-Address.
             * else the parser will throw exception. So prepending TPdu with 0,
             * which indicates that there is no SC address and its length is 0.
             * This way Parser will skip parsing for SC-Address
             */
            ByteArrayOutputStream bo = new ByteArrayOutputStream(pdu.length + 1);
            bo.write(0x00);
            bo.write(pdu, 0, pdu.length);
            byte[] frameworkPdu = bo.toByteArray();
            SmsMessage message = SmsMessage.createFromPdu(frameworkPdu, SmsMessage.FORMAT_3GPP);
            String address = message.getRecipientAddress();
            TpduParam tpduParameters = new TpduParam(token, pdu, smsc, address);
            UsatInterface usat = mCallContext.getUsatInterface();
            mUsatBasedSms = new UsatBasedSms();
            if (usat != null && usat.isServiceAvailable(Usat.SERVICE_MO_SMS_CONTROL)) {
                Rlog.i(TAG, "usat service available");
                IDcNetWatcher dcnw = mCallContext.getDcNetWatcher();
                int networkType =  TelephonyManager.NETWORK_TYPE_UNKNOWN;
                byte[] smscAddrBytes = HexDump.hexStringToByteArray(smsc);
                int len = smscAddrBytes[0];
                String rpAddress = PhoneNumberUtils.calledPartyBCDToString(smscAddrBytes, 1,
                                    len, PhoneNumberUtils.BCD_EXTENDED_TYPE_CALLED_PARTY);
                Rlog.d(TAG, "sendMoTPdu: rpAddress = " + rpAddress);
                if (dcnw != null) networkType = dcnw.getNetworkType();
                mUsatBasedSms.mMoSmsCCmd = usat.createMoSmsControlCommand(
                    rpAddress, address, networkType, mUsatBasedSms);
                mUsatCmdMessageMap.put(mUsatBasedSms.mMoSmsCCmd, tpduParameters);
                usat.sendCommand(mUsatBasedSms.mMoSmsCCmd);
                return SmsUtils.RESULT_SUCCESS;
            }
            return enqueueAndSendMessageToRL(tpduParameters);
        } catch (RuntimeException e) {
            Rlog.e(TAG, "SendSms Failed at SmsTransferLayer: " + e.getMessage());
            return SmsUtils.SMSTL_RESULT_EXCEPTION;
        }
    }

    /**
     * Notifies Relay Layer to Send RP-Ack or RP-Error based on result
     * @param token same as that used when SMS-DELIVER Message is
     * @param tlMessageType indicates the Message Type to be sent in TPdu
     * @param messageRef indicates the TP-MR with respect to this token
     * @param result indicates the result of SMS-DELIVER receipt
     *
     * @return result indicating if TPDU is sent successfully to relay layer
     */
    public int sendReportTPdu(int token, int tlMessageType, int messageRef, int result) {
        Rlog.d(TAG, "sendReportTPdu");
        try {
            int messageType = SmsUtils.RP_ERROR;
            if (result == ImsSmsImplBase.DELIVER_STATUS_OK) {
                messageType = SmsUtils.RP_ACK;
            }
            byte[] deliverReportPdu = generateDeliverReportPdu(result);
            Rlog.i(TAG, "SMS-DELIVER-REPORT = " + HexDump.dumpHexString(deliverReportPdu));
            return mSmsRL.sendRPMessage(token, messageType, null, null, deliverReportPdu, result);
        } catch (RuntimeException e) {
            Rlog.e(TAG, "sendReportTPdu Failed: " + e.getMessage());
            return SmsUtils.SMSTL_RESULT_EXCEPTION;
        }
    }

    /**
     * Generates the SMS-DELIVER-REPORT TPDU
     * @param deliverResult indicates the result of SMS-DELIVER receipt
     *
     * @return SMS-DELIVER-REPORT TPDU in byte array
     */
    public byte[] generateDeliverReportPdu(int deliverResult) {
        try {
            ByteArrayOutputStream bo = new ByteArrayOutputStream(MAX_TPDU_LENGTH);
            byte mtiByte = 0x00; // TP-MTI for SMS-DELIVER-REPORT as per TS 23.040 section 9.2.3.1
            byte parameterIndByte = 0x00;
            bo.write(mtiByte);

            int cause = TP_FCS_UNSPECIFIED_ERROR_CAUSE;

            switch (deliverResult) {
                case ImsSmsImplBase.DELIVER_STATUS_OK:
                    cause = TP_FCS_NONE;
                    break;
                case ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC:
                    cause = TP_FCS_UNSPECIFIED_ERROR_CAUSE;
                    break;
                case ImsSmsImplBase.DELIVER_STATUS_ERROR_NO_MEMORY:
                    cause = TP_FCS_MEMORY_CAPACITY_EXCEEDED;
                    break;
                case ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED:
                    cause = TP_FCS_TPDU_NOT_SUPPORTED;
                    break;
                default:
                    cause = TP_FCS_UNSPECIFIED_ERROR_CAUSE;
            }
            if (cause != TP_FCS_NONE) {
                bo.write((byte) cause);
            }

            bo.write(parameterIndByte); //TP-PID not set
            return bo.toByteArray();
        } catch (RuntimeException ex) {
            Rlog.e(TAG, "Generating Deliver Report failed: " + ex.toString());
            return null;
        }
    }

    private class UsatBasedSms implements Usat.Listener {
        public Usat.MoSmsControlCommand mMoSmsCCmd = null;

        UsatBasedSms() {
        }

        @Override
        public void onCommandResponse(Usat.CommandResponse response) {
            Usat.MoSmsControlCommandResponse cmdRes = (Usat.MoSmsControlCommandResponse) response;
            Usat.MoSmsControlCommand cmd = (Usat.MoSmsControlCommand) cmdRes.getCommand();

            synchronized (mLock) {
                if (!cmd.equals(mMoSmsCCmd)) {
                    Rlog.e(TAG, "Command mismatched - " + cmd);
                    return;
                }
            }

            Rlog.i(TAG, "onCommandResponse :: cmd=" + cmd + ", result=" + cmdRes.getResult()
                    + ", rpDestAddr=" + cmdRes.getRpDestinationAddress()
                    + ", rpDestAddr=" + cmdRes.getRpDestinationAddress());

            TpduParam tpduParameters = mUsatCmdMessageMap.get(cmd);
            if (cmdRes.getResult() == Usat.RESULT_NOT_ALLOWED) {
                mListener.notifySmsResult(tpduParameters.mToken,
                                             tpduParameters.mTpdu[1] & 0xff,
                                             ImsSmsImplBase.SEND_STATUS_ERROR,
                                             SmsManager.RESULT_OPERATION_NOT_ALLOWED,
                                             ImsSmsImplBase.RESULT_NO_NETWORK_ERROR);
                return;
            }

            String rpDestAddr = cmd.getRpDestinationAddress();
            String tpDestAddr = cmd.getTpDestinationAddress();
            byte[] usatTpdu = tpduParameters.mTpdu;
            String encodedSmsc = tpduParameters.mSmsc;

            if (cmdRes.getResult() == Usat.RESULT_ALLOWED_WITH_MODIFICATION) {
                rpDestAddr = cmdRes.getRpDestinationAddress();
                tpDestAddr = cmdRes.getTpDestinationAddress();

                if (!TextUtils.isEmpty(rpDestAddr)) {
                    byte[] encodedSmscBytes = PhoneNumberUtils
                                              .networkPortionToCalledPartyBCDWithLength(rpDestAddr);
                    encodedSmsc = IccUtils.bytesToHexString(encodedSmscBytes);
                }
                if (TextUtils.isEmpty(tpDestAddr)) {
                    // Use the original dialed string if this is not present.
                    tpDestAddr = cmd.getTpDestinationAddress();
                } else {
                    //The TP-Destination-Address byte in TPdu must be modified
                    byte[] tpDestAddrBytes;

                    tpDestAddrBytes = PhoneNumberUtils.networkPortionToCalledPartyBCD(tpDestAddr);

                    //The length byte of destination address is number of digits in the address
                    int destAddrLen = calculateTpDestAddrLengthByte(tpDestAddrBytes);

                    //offset after destination address in original Tpdu
                    int offsetAfterAddress =  calculateIndexAfterTpDestAddr(tpduParameters.mTpdu);

                    //length rest of the original Tpdu after Tp-DestinationAddress
                    int tpduLenAfterAddress = tpduParameters.mTpdu.length - offsetAfterAddress;

                    ByteArrayOutputStream bo = new ByteArrayOutputStream(
                                                            TPDU_LENGTH_BEFORE_DESTINATION_ADDRESS
                                                            + tpDestAddrBytes.length
                                                            + tpduLenAfterAddress);
                    //Copy 2 bytes from original TPDU - 1st byte is MTI and 2nd byte is TP-MR
                    bo.write(tpduParameters.mTpdu, 0, 2);

                    bo.write(destAddrLen);

                    //copy modified destination address in the Tpdu
                    bo.write(tpDestAddrBytes, 0, tpDestAddrBytes.length);

                    /**
                     * skip the earlier destination address in original Tpdu
                     * copy rest of the Tpdu as is after conpying the modified TP-DA
                     */
                    bo.write(tpduParameters.mTpdu,  offsetAfterAddress, tpduLenAfterAddress);
                    usatTpdu = bo.toByteArray();
                }

            }
            tpduParameters.mTpdu = usatTpdu;
            tpduParameters.mDestinationAddress = tpDestAddr;
            tpduParameters.mSmsc = encodedSmsc;

            enqueueAndSendMessageToRL(tpduParameters);

            synchronized (mLock) {
                mMoSmsCCmd = null;
            }
        }

        public void dispose() {
            synchronized (mLock) {
                if (mMoSmsCCmd != null) {
                    UsatInterface usat = mCallContext.getUsatInterface();

                    if (usat != null) {
                        usat.cancelCommand(mMoSmsCCmd);
                    }

                    mMoSmsCCmd = null;
                }
            }
        }

    }

    /**
     * Calculate the length byte of destination address in Tpdu from encoded Tp-DA
     * @param tpDestAddrBytes Destination address encoded in BCD
     * @return the destination address length in BCD digits
     */
    public int calculateTpDestAddrLengthByte(byte[] tpDestAddrBytes) {
        /**
         * The length Byte of Tp-DA is not the number of bytes in encoded format
         * It is the number of BCD digits in encoded Format minus the TON byte and pad
         */
        //minus TON Byte and since each byte holds 2 digits, multiply the no. of bytes by 2.
        int destAddrLen = (tpDestAddrBytes.length - 1) * 2;

        /**
         * if there are odd number of digits , the last byte which consists of only one digit
         * say 'x', is encoded in hex as fx. f is the padding here and should not be counted in
         * length byte.
         */
         //check if padding is there in the last byte, if yes, subtract the same
        if ((tpDestAddrBytes[tpDestAddrBytes.length - 1] & 0xf0) == 0xf0) {
            destAddrLen = destAddrLen - 1;
        }

        return destAddrLen;
    }

    /**
     * Calculates the index after the end of Tp-DestinationAddress
     * @param tpdu Tpdu in byte array
     * @return  returns the index after the end of Tp-DestinationAddress
     */
    public int calculateIndexAfterTpDestAddr(byte[] tpdu) {
        /**
         * calculate the number of bytes of TP_DA in Tpdu
         * the length byte of Tp-DA indiactes number of BCD digits in address and since 2 BCD
         * digits forms 1 byte in encoded TP-DA in TPDU, divide the length byte by 2
         */
        int tpDAbyteLen = (tpdu[TP_DA_LENGTH_INDEX] + 1) / 2;
        int indexAfterTpDA = TP_DA_START_INDEX + tpDAbyteLen;
        return indexAfterTpDA;
    }

    /**
     * Inserts Message parameters in queue and map and sends the message to Relay Layer
     * if queue is empty
     * @param tpduParameters parametrs related to TPdu which needs to be sent to Relay Layer
     *
     * @return result of processing of message in Relay Layer
     */
    private int enqueueAndSendMessageToRL(TpduParam tpduParameters) {
        synchronized (mLock) {
            if (mTokenMessageMap.size() > MAX_MESSAGE_COUNT) {
                Rlog.e(TAG, "sendMoTpdu:Too many messages in queue");
                return SmsUtils.SMSTL_RESULT_QUEUE_SIZE_EXCEEDED;
            }
            if (mSendSmsQueue.isEmpty()) {
                if (VDBG) {
                    Rlog.d(TAG, "sendMoTpdu:queue is empty  adding token "
                                    + tpduParameters.mToken
                                    + " encoded Smsc = " + tpduParameters.mSmsc
                                    + " tpDestinationAddress = "
                                    + tpduParameters.mDestinationAddress
                                    + " tpdu = " + IccUtils.bytesToHexString(tpduParameters.mTpdu));
                }
                mSendSmsQueue.add(tpduParameters.mToken);
                mTokenMessageMap.put(tpduParameters.mToken, tpduParameters);
                return mSmsRL.sendRPMessage(tpduParameters.mToken, SmsUtils.RP_DATA,
                        tpduParameters.mSmsc, tpduParameters.mDestinationAddress,
                        tpduParameters.mTpdu, 0);
            } else {
                if (mTokenMessageMap.containsKey(tpduParameters.mToken)) {
                    Rlog.e(TAG, "sendMoTpdu: duplicate token - discarding the request");
                    return SmsUtils.SMSTL_RESULT_DUPLICATE_TOKEN;
                }
                Rlog.i(TAG, "sendMoTpdu:queue is not  empty  adding token "
                                                     + tpduParameters.mToken);
                mTokenMessageMap.put(tpduParameters.mToken, tpduParameters);
                mSendSmsQueue.add(tpduParameters.mToken);
                return SmsUtils.RESULT_SUCCESS;
            }
        }
    }

    private class MessageHandler extends Handler {
        MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Rlog.i(TAG, "MessageHandler - what=" + msg.what);

            if (mListener == null) {
                Rlog.e(TAG, "mListener is null");
                return;
            }

            switch (msg.what) {
                case REQUEST_SEND_NEXT_SMS_TO_RL:
                    TpduParam tpduParameters = (TpduParam) msg.obj;
                    if (VDBG) {
                        Rlog.d(TAG, "sending Message in queue to RL: "
                                    + tpduParameters.mToken
                                    + " encoded Smsc = " + tpduParameters.mSmsc
                                    + " tpDestinationAddress = "
                                    + tpduParameters.mDestinationAddress
                                    + " tpdu = " + IccUtils.bytesToHexString(tpduParameters.mTpdu));
                    }
                    mSmsRL.sendRPMessage(tpduParameters.mToken, SmsUtils.RP_DATA,
                                         tpduParameters.mSmsc, tpduParameters.mDestinationAddress,
                                         tpduParameters.mTpdu, 0);
                    break;

                default :
                    Rlog.e(TAG, "Inavlid message to handler");
            }
        }
    }

    /**
     * generates CDMA TPDU to be sent to framework
     * @param pdu CDMA PDU received from network
     *
     * @return returns the 3GPP2 TPDU that has to be sent to framework
     */
    protected byte[] generateCdmaPdu(byte[] pdu) {
        com.android.internal.telephony.cdma.SmsMessage cdmaMsg = null;

        if (pdu == null) {
            Rlog.e(TAG, "pdu is null");
            return new byte[0];
        }

        int pduLength = pdu.length;
        Rlog.i(TAG, "original pdu length = " + pduLength);
        if (pduLength <= 0 || pduLength > MAX_CDMA_PDU_LENGTH) {
            Rlog.e(TAG, "Invalid pdu length");
            return new byte[0];
        }

        byte[] cdmaPdu = new byte[pduLength + 2];
        //TODO: BugId: b/240369384, Need to check the behaviour at modem side
        /* The first byte should be interpreted as per 3GPP2 C.S0023 3.4.27
         * It is a status field, we have hardcoded as
         * ‘001’ Message received by MS from network,
         * which means message read
         */
        cdmaPdu[0] = 0x01;
        cdmaPdu[1] = (byte) pduLength;
        System.arraycopy(pdu, 0, cdmaPdu, 2, pduLength);

        Rlog.i(TAG, "EfRecord Pdu = " + IccUtils.bytesToHexString(cdmaPdu));

        if (cdmaPdu != null && cdmaPdu.length > 0) {
            cdmaMsg =  com.android.internal.telephony.cdma.SmsMessage
                                            .createFromEfRecord(0, cdmaPdu);

            if (cdmaMsg != null) {
                if (VDBG) {
                    Rlog.d(TAG, "Originating Address = " + cdmaMsg.getOriginatingAddress());
                    Rlog.d(TAG, "Message Body = " + cdmaMsg.getMessageBody());
                }
                /* generates framework compatible CDMA PDU */
                cdmaMsg.createPdu();
                Rlog.i(TAG, "cdmaMsg.mPdu = " + IccUtils.bytesToHexString(cdmaMsg.getPdu()));
            } else {
                Rlog.e(TAG, "cdmaMsg is null");
            }
        }

        return ((cdmaMsg != null) ? cdmaMsg.getPdu() : new byte[0]);

    }

    class SmsRLListenerProxy implements SmsRelayLayer.Listener {

        public int notifyRLDataIndication(
                int token, int smsFormat, int rpMessageType, byte[] pdu) {
            Rlog.d(TAG, "notifyRLDataIndication: token = " + token
                                         + " smsFormat = " + smsFormat
                                         + " rpMessageType = " + rpMessageType);
            int result = SmsUtils.SMSTL_RESULT_FAILURE;
            try {
                SmsTransferLayer.Listener listener = mListener;
                int newToken = token;
                byte[] cdmaPdu = null;
                if (listener == null) {
                    Rlog.d(TAG, "Listener is null");
                    return result;
                }
                if (smsFormat == SmsUtils.FORMAT_INT_3GPP2) {
                    cdmaPdu = generateCdmaPdu(pdu);
                    if (cdmaPdu == null || cdmaPdu.length == 0) {
                        Rlog.i(TAG, "getCdmaPdu returned error");
                        return SmsUtils.SMSTL_RESULT_GENERATE_CDMA_PDU_FAILED;
                    }
                    synchronized (mLock) {
                        Rlog.d(TAG, "notifyRLDataIndication: calling notifySmsReceived");
                        return listener.notifySmsReceived(token, smsFormat,
                                                SmsUtils.TP_SMS_DELIVER, cdmaPdu);
                    }
                }
                int tpMessageType = SmsUtils.TP_SMS_DELIVER;
                int originAddressLength = pdu[SmsUtils.MODIFIED_TPDU_ORIGIN_ADDR_LENGTH_INDEX];
                int modifiedTpduStartIndex = 1 + originAddressLength; //length byte + Value length
                int tpMti = pdu[modifiedTpduStartIndex + SmsUtils.TPDU_MTI_INDEX] & 0x3;
                if (tpMti == SmsUtils.TPDU_MTI_SMS_STATUS_REPORT) {
                    Rlog.i(TAG, "notifyRLDataIndication: received SMS_STATUS_REPORT");
                    tpMessageType = SmsUtils.TP_SMS_STATUS_REPORT;
                }
                synchronized (mLock) {
                    return listener.notifySmsReceived(token, smsFormat, tpMessageType, pdu);
                }
            } catch (RuntimeException e) {
                Rlog.e(TAG, "notifyRLDataIndication Failed: " + e.getMessage());
                return SmsUtils.SMSTL_RESULT_EXCEPTION;
            }
        }

        public void notifyRLReportIndication(int token, int messageRef, int result,
                                             int reason, int causeCode) {
            try {
                Rlog.d(TAG, "notifyRLReportIndication");
                TpduParam tpduParameters;
                int nextToken;
                SmsTransferLayer.Listener listener = mListener;
                // TODO: map result and reason
                if (listener == null) {
                    return;
                }
                synchronized (mLock) {
                    listener.notifySmsResult(token, messageRef, result, reason, causeCode);
                    if (mTokenMessageMap.containsKey(token)) {
                        mSendSmsQueue.remove(token);
                        mTokenMessageMap.remove(token);
                    }
                    if (!mSendSmsQueue.isEmpty()) {
                        nextToken = mSendSmsQueue.peek();
                        if (!mTokenMessageMap.containsKey(nextToken)) {
                            Rlog.d(TAG, "notifyRLReportIndication: next token not in Map");
                            return;
                        }
                        tpduParameters = mTokenMessageMap.get(nextToken);
                        Message msg = Message.obtain();
                        msg.what = REQUEST_SEND_NEXT_SMS_TO_RL;
                        msg.obj = tpduParameters;
                        mSmsHandler.sendMessage(msg);
                    }
                }
            } catch (RuntimeException e) {
                Rlog.e(TAG, "notifyRLReportIndication Failed: " + e.getMessage());
            }
        }
    }
}
