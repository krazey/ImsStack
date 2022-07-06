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
import android.telephony.SmsMessage;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.uicc.IccUtils;
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
    public final ConcurrentLinkedQueue<Integer> mSendSmsQueue =
                 new ConcurrentLinkedQueue<Integer>();
    private MessageHandler mSmsHandler = null;
    private HandlerThread mSmsHandlerThread = new HandlerThread("ImsSmsHandlerThread");
    public static final int REQUEST_SEND_NEXT_SMS_TO_RL = 1;
    private static final boolean VDBG = true;

    /**
     * This class contains parameters related to each SMS sent and is required for
     * maintaining the Message details in queue when there are simultaneous SMS requests
     */
    private class TpduParam{
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
         * @param result result of sending the SMS.
         * @param reason reason in case status is failure.
         */
        void notifySmsResult(int token, int result, int reason);

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

    public void setListener(SmsTransferLayer.Listener listener) {
        mListener = listener;
    }

    /**
     * Handles SMS-SUBMIT Message at Transfer Layer and notifies Relay Layer to send RP-DATA
     * @param token sent from framework to track callback for each SMS-SUBMIT message
     * @param smsFormat format of the message
     * @parm tpMessageRef the TP-MR passed for the SMS-SUBMIT message
     * @param smsc the Short Message Service Center address
     * @param pdu PDU representing the contents of the message.
     *
     * @return result of processing of outgoing SMS's TPDU
     */
    public int sendMoTPdu(int token, int smsFormat, int tpMessageRef, String smsc, byte[] pdu) {
        Rlog.d(TAG, "sendMoTPdu");
        try {
            int result;
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

            synchronized (mLock) {
                if (mTokenMessageMap.size() > MAX_MESSAGE_COUNT) {
                    Rlog.e(TAG, "sendMoTpdu:Too many messages in queue");
                    return SmsUtils.SMSTL_RESULT_QUEUE_SIZE_EXCEEDED;
                }
                if (mSendSmsQueue.isEmpty()) {
                    Rlog.i(TAG, "sendMoTpdu:queue is empty  adding token " + token);
                    mSendSmsQueue.add(token);
                    mTokenMessageMap.put(token, tpduParameters);
                    return mSmsRL.sendRPMessage(token, SmsUtils.RP_DATA,
                            smsc, address, pdu);
                } else {
                    if (mTokenMessageMap.containsKey(token)) {
                        Rlog.e(TAG, "sendMoTpdu: duplicate token - discarding the request");
                        return SmsUtils.SMSTL_RESULT_DUPLICATE_TOKEN;
                    }
                    Rlog.i(TAG, "sendMoTpdu:queue is not  empty  adding token " + token);
                    mTokenMessageMap.put(token, tpduParameters);
                    mSendSmsQueue.add(token);
                    return SmsUtils.RESULT_SUCCESS;
                }
            }
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
            //int smsSubmitToken;
            if (result == ImsSmsImplBase.DELIVER_STATUS_OK) {
                messageType = SmsUtils.RP_ACK;
            }
            return mSmsRL.sendRPMessage(token, messageType, null, null, null);
        } catch (RuntimeException e) {
            Rlog.e(TAG, "sendReportTPdu Failed: " + e.getMessage());
            return SmsUtils.SMSTL_RESULT_EXCEPTION;
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
                    Rlog.i(TAG, "sending token " + tpduParameters.mToken);
                    mSmsRL.sendRPMessage(tpduParameters.mToken, SmsUtils.RP_DATA,
                                         tpduParameters.mSmsc, tpduParameters.mDestinationAddress,
                                         tpduParameters.mTpdu);
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

        public void notifyRLReportIndication(int token, int result, int reason) {
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
                    listener.notifySmsResult(token, result, reason);
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
