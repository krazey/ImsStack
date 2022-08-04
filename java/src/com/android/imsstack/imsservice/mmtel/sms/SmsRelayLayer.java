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

import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.util.Base64;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.AppContext;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.util.HexDump;
import com.android.telephony.Rlog;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Provides the functionality of SMS Relay Layer as per 3GPP TS 24.011
 **/
public class SmsRelayLayer {
    private static final String TAG = "[GII-SmsRL] ";
    private final ImsCallContext mCallContext;
    private SmsTransferLayer mSmsTL = null;
    private MtsController mMtsController = null;
    SmsRLStateMachine mSmsRLStateMachine = null;
    private final MtsControllerListenerProxy mMtsControllerListener =
                                                  new MtsControllerListenerProxy();
    private final Object mLock = new Object();
    //token-RPMessageReference synchronisation
    private AtomicInteger mRPMR = new AtomicInteger();
    private AtomicInteger mToken = new AtomicInteger();
    public Map<Integer, Integer> mMoMRTokenMap = new ConcurrentHashMap<>();
    public Map<Integer, Integer> mMTTokenMRMap = new ConcurrentHashMap<>();
    private static final boolean VDBG = true;
    public Map<Integer, SmsRLStateMachine> mTokenStateTrackerMap = new ConcurrentHashMap<>();

    //TODO: to be included in ImsSmsImplBase
    public static final int DELIVER_STATUS_ERROR_INVALID_MR_VALUE = 5;
    public static final int GENERIC_ERROR_CAUSE_VALUE = 0x6f;
    Map<Integer , Integer> mDeliverCause = new ConcurrentHashMap<>();

    /**
     * Listener to handle the events sent from SmsRelayLayer
     */
    public interface Listener {
        /**
         * notifies Transfer Layer when RP-Data is received
         * @param token unique token generated to trigger callbacks for this message
         * @param smsFormat the format of the message
         * @param messageType the message type of RPDU
         * @param pdu TPdu representing the contents of the message
         *
         * @return the result of handling Data at transfer layer
         */
        int notifyRLDataIndication(int token, int smsFormat,
                                             int messageType, byte[] pdu);

        /**
         * notifies Transfer Layer when RP-Ack/RP-Error is received
         * @param token provided in {@link #sendRPMessage(int, int, String, String, byte[])}
         * @param tpMessageRef TPNR value of respective MS_SUBMIT message
         * @param result result of sending the SMS.
         * @param reason reason in case status is failure.
         * @param cause cause code if sending Sms is failed
         */
        void notifyRLReportIndication(int token, int tpMessageRef, int result,
                                      int reason, int cause);
    }

    public SmsRelayLayer(ImsCallContext callContext) {
        mCallContext = callContext;
        mMtsController = new MtsController(mCallContext);
        if (mMtsController != null) {
            mMtsController.setListener(mMtsControllerListener);
        }
        mMtsController.startNativeConnection();
        //Initialise Cause values as per TS 24.011 section 8,.2.5.4
        mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_OK, 0);
        mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC, 0x6f);
        mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_ERROR_NO_MEMORY, 0x16);
        mDeliverCause.put(ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED, 0x61);
        mDeliverCause.put(DELIVER_STATUS_ERROR_INVALID_MR_VALUE, 0x51);
    }

    @VisibleForTesting
    public SmsRelayLayer(ImsCallContext callContext, MtsController mtsController) {
        mCallContext = callContext;
        if (mtsController == null) {
            mMtsController = new MtsController(mCallContext);
        } else {
            mMtsController = mtsController;
        }

        if (mMtsController != null) {
            mMtsController.setListener(mMtsControllerListener);
        }
        mMtsController.startNativeConnection();
    }

    private SmsRelayLayer.Listener mListener = null;

    public void setListener(SmsRelayLayer.Listener listener) {
        mListener = listener;
    }

    /**
     * Creates RPDU and requests MtsController to send RPDU to native
     * @param token unique token generated by platform used for
     *              callbacks for this specific message
     * @param rpType the Message Type with respect to RPDU
     * @param smsc the Short Message Service Center address.
     * @param destinationAddress the Target Address of the message
     * @param tpdu PDU representing the contents of the message.
     * @param statusResult status result sent by framework for Incoming SMS
     *
     * @return the result of sending RPDU to SMS Enabler
     */
    public int sendRPMessage(
            int token, int rpType, String smsc, String destinationAddress,
            byte[] tpdu, int statusResult) {
        try {
            Rlog.d(TAG, "sendRPMessage: token = " + token
                    + " RP Message Type = " + rpType
                    + " Smsc = " + smsc
                    + " destinationAddress = " +  destinationAddress
                    + " statusResult = " + statusResult);
            byte[] encodedPdu = null;
            TelephonyManager tm = null;
            String targetAddress = null;
            int rpCause = GENERIC_ERROR_CAUSE_VALUE;
            if (mDeliverCause.containsKey(statusResult)) {
                rpCause = mDeliverCause.get(statusResult);
            }
            int result = SmsUtils.SMSRL_RESULT_INVALID_STATE;
            /* In Case of RP-Ack, Target address is already fetched when RP-Data is received
             * and  saved with corresponding StateMachine object
             */
            if (rpType == SmsUtils.RP_DATA || rpType == SmsUtils.RP_SMMA) {
                //fetch PSI Value
                int subId = mCallContext.getSubId();
                tm = AppContext.getTelephonyManager(subId);
                if (tm != null) {
                    //As per b/232048441 targetAddress must be set to PSI value,
                    targetAddress = tm.getSmscIdentity(TelephonyManager.APPTYPE_ISIM);
                    Rlog.d(TAG, "sendRPMessage PSI in ISIM " + targetAddress);
                    if (targetAddress == null || targetAddress.length() == 0) {
                        targetAddress = tm.getSmscIdentity(TelephonyManager.APPTYPE_USIM);
                        Rlog.d(TAG, "sendRPMessage PSI in USIM" + targetAddress);
                    }
                }
                //return if smsc is null as it is a must to construct RP-Destination address
                if (smsc == null || smsc.length() == 0) {
                    Rlog.e(TAG, "Smsc is null");
                    return SmsUtils.SMS_RESULT_INVALID_SMSC_ADDRESS;
                }
                //As per b/232048441 if PSI is null, targetAddress is set to smsc
                if (targetAddress == null || targetAddress.length() == 0) {
                    Rlog.i(TAG, "PSI is null");
                    /* smsc address is passed in encoded format. Extracting the address
                     * string in decoded format and assigning it to TargetAddress
                     */
                    targetAddress = decodeSmsc(smsc);
                    Rlog.d(TAG, "sendRPMEssage targetAddress sets to SMsc" + targetAddress);
                }
                /* As per b/232048441, if PSI & Smsc is null,
                 * targetAddress is set to destination Address
                */
                if (targetAddress == null || targetAddress.length() == 0) {
                    Rlog.i(TAG, "sending As SMSCaddr as Target Address to MtsController: "
                             + destinationAddress);
                    targetAddress = destinationAddress;
                }

                if (destinationAddress == null || destinationAddress.length() == 0) {
                    destinationAddress = targetAddress;
                }
            }
            int rpMR = -1;
            synchronized (mLock) {
                if (rpType == SmsUtils.RP_DATA) {
                    rpMR = getRPMessageReference();
                    mMoMRTokenMap.put(rpMR, token);
                    SmsRPdu moRPDataPdu = new SmsRPdu(rpMR, rpType, smsc,
                                                      rpCause, tpdu);
                    mSmsRLStateMachine = new SmsRLStateMachine(token, SmsUtils.RP_DATA,
                                                mMtsController, mCallContext, mListener,
                                                targetAddress, destinationAddress);
                    mTokenStateTrackerMap.put(token, mSmsRLStateMachine);
                    result  = mSmsRLStateMachine.onRPDataFromTL(moRPDataPdu);
                } else if (rpType == SmsUtils.RP_ACK) {
                    if (mMTTokenMRMap.containsKey(token)) {
                        rpMR = mMTTokenMRMap.get(token);
                        mMTTokenMRMap.remove(token);
                    } else {
                        Rlog.e(TAG, "No Corresponding Rp-Data for the RP-Ack");
                        return SmsUtils.SMSRL_RESULT_TOKEN_DOES_NOT_EXIST;
                    }
                    mSmsRLStateMachine = mTokenStateTrackerMap.get(token);
                    SmsRPdu mtRPAckPdu = new SmsRPdu(rpMR, rpType, smsc,
                                                     rpCause, tpdu);
                    result = mSmsRLStateMachine.onRPAckFromTL(mtRPAckPdu);
                    mTokenStateTrackerMap.remove(token);
                } else if (rpType == SmsUtils.RP_ERROR) {
                    if (mMTTokenMRMap.containsKey(token)) {
                        rpMR = mMTTokenMRMap.get(token);
                        mMTTokenMRMap.remove(token);
                    } else {
                        Rlog.e(TAG, "No Corresponding Rp-Data for the RP-Error");
                        return SmsUtils.SMSRL_RESULT_TOKEN_DOES_NOT_EXIST;
                    }
                    mSmsRLStateMachine = mTokenStateTrackerMap.get(token);
                    SmsRPdu mtRPErrorPdu = new SmsRPdu(rpMR, rpType, smsc,
                                                       rpCause, tpdu);
                    result = mSmsRLStateMachine.onRPErrorFromTL(mtRPErrorPdu);
                    mTokenStateTrackerMap.remove(token);
                } else if (rpType == SmsUtils.RP_SMMA) {
                    rpMR = getRPMessageReference();
                    mMoMRTokenMap.put(rpMR, token);
                    SmsRPdu moRPSmmaPdu = new SmsRPdu(rpMR, rpType, smsc,
                                                      rpCause, tpdu);
                    mSmsRLStateMachine = new SmsRLStateMachine(token, SmsUtils.RP_SMMA,
                                    mMtsController, mCallContext, mListener,
                                    targetAddress, destinationAddress);
                    mTokenStateTrackerMap.put(token, mSmsRLStateMachine);
                    result = mSmsRLStateMachine.onRPDataFromTL(moRPSmmaPdu);
                } else {
                    Rlog.e(TAG, "Invalid message type");
                    return SmsUtils.SMSRL_RESULT_INVALID_RP_MESSAGE_TYPE;
                }
            }
            return result;
        } catch (RuntimeException e) {
            Rlog.e(TAG, "Can not send sms: " + e.getMessage());
            return SmsUtils.SMSRL_RESULT_EXCEPTION;
        }
    }

    private String decodeSmsc(String smsc) {
        byte[] targetAddrBytes = HexDump.hexStringToByteArray(smsc);
        int len = targetAddrBytes[0];
        return PhoneNumberUtils.calledPartyBCDToString(targetAddrBytes, 1,
                            len, PhoneNumberUtils.BCD_EXTENDED_TYPE_CALLED_PARTY);
    }
    /*
     * Increments and returns RP-MR Value
     * @return next RP-MR value
     */
    private int getRPMessageReference() {
        return (mRPMR.incrementAndGet() % SmsUtils.MAX_RPMR_VALUE);
    }

    class MtsControllerListenerProxy extends MtsController.Listener {
        @Override
        public void notifyStatusForOutgoingMessage(
                int statusResult, int format, int retryAfter, int messageReference) {
            try {
                Rlog.i(TAG, "notifyStatusForOutgoingMessage");
                int token = -1;
                int result = SmsUtils.RESULT_SUCCESS;
                Listener listener = mListener;
                int status = ImsSmsImplBase.SEND_STATUS_ERROR;
                if (statusResult == MtsController.MO_SUCCESS) {
                    Rlog.i(TAG, "SIP 2xx response");
                    return;
                }
                if (listener == null) {
                    Rlog.e(TAG, "listener is null");
                    return;
                }
                synchronized (mLock) {
                    if (mMoMRTokenMap.containsKey(messageReference)) {
                        token = mMoMRTokenMap.get(messageReference);
                        mMoMRTokenMap.remove(messageReference);
                    } else {
                        Rlog.e(TAG, "MessageReference does not match any MO RP_Data:"
                                + messageReference);
                        return;
                    }
                    mSmsRLStateMachine = mTokenStateTrackerMap.get(token);

                    if (statusResult == MtsController.MO_IMS_TEMP_FAILURE) {
                        status = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
                    } else if (statusResult == MtsController.MO_IMS_PERM_FAILURE) {
                        status = ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK;
                    }
                    mTokenStateTrackerMap.remove(token);
                    result = mSmsRLStateMachine.onSipResponseForRPMessage(false, status);
                    if (result != SmsUtils.RESULT_SUCCESS) {
                        Rlog.e(TAG, "notifyStatusForOutgoingMessage Failed: ");
                    }
                }
            } catch (RuntimeException e) {
                Rlog.e(TAG, "notifyStatusForOutgoingMessage Failed: " + e.getMessage());
            }
        }

        @Override
        public int notifyIncomingMessage(int smsFormat, String encodedData) {
            try {
                Rlog.d(TAG, "notifyIncomingMessage: smsFormat = " + smsFormat);
                int token = 0;
                int result;
                Listener listener = mListener;
                if (listener == null) {
                    Rlog.e(TAG, "Listener is null");
                    return mMtsController.MT_FAILURE;
                }
                encodedData = encodedData.replaceAll("\\s", "");
                byte[] pduData = Base64.decode(encodedData, Base64.NO_PADDING);
                if (VDBG) {
                    StringBuilder sb = new StringBuilder();
                    for (int i = 0; i < pduData.length; i++) {
                        sb.append(String.format("%02X ", pduData[i]));
                    }
                    Rlog.i(TAG, "RPDU = " + sb.toString());
                }
                if (smsFormat == SmsUtils.FORMAT_INT_3GPP2) {
                    token = mToken.incrementAndGet();
                    synchronized (mLock) {
                        Rlog.i(TAG, "calling notifyRLDataIndication with token = " + token);
                        result = listener.notifyRLDataIndication(token, smsFormat,
                                                                 SmsUtils.RP_DATA, pduData);
                    }
                    if (result != SmsUtils.RESULT_SUCCESS) {
                        return MtsController.MT_FAILURE;
                    }
                    return MtsController.MT_SUCCESS;
                }
                byte[] tpdu = null;
                SmsRPdu mtData = new SmsRPdu(pduData);
                Rlog.i(TAG, "rpdu fields new: message type ="
                                + mtData.getMessageType()
                                + " messageRef ="
                                + mtData.getMessageRef()
                                + " Originating Address = "
                                + mtData.getOrigAddr());
                int messageRef = mtData.getMessageRef();
                TelephonyManager tm = null;
                String targetAddress = null;
                //fetch PSI Value
                int subId = mCallContext.getSubId();
                tm = AppContext.getTelephonyManager(subId);
                if (tm != null) {
                    targetAddress = tm.getSmscIdentity(TelephonyManager.APPTYPE_ISIM);
                    Rlog.d(TAG, "notifyIncomingMessage PSI in ISIM " + targetAddress);
                    if (targetAddress == null || targetAddress.length() == 0) {
                        targetAddress = tm.getSmscIdentity(TelephonyManager.APPTYPE_USIM);
                        Rlog.d(TAG, "notifyIncomingMessage PSI in USIM" + targetAddress);
                    }
                }
                if (targetAddress == null || targetAddress.length() == 0) {
                    targetAddress = mtData.getOrigAddr();
                    Rlog.d(TAG, "PSI null so setting to  orig address" + targetAddress);
                }
                if (mtData.getMessageType() == SmsUtils.RP_ACK) {
                    synchronized (mLock) {
                        if (mMoMRTokenMap.containsKey(messageRef)) {
                            token = mMoMRTokenMap.get(messageRef);
                            mMoMRTokenMap.remove(messageRef);
                        } else {
                            //send RP-Error as per TS 24.011, section 9.3.3
                            Rlog.e(TAG, "No Matching RP-Data for Rp-Ack");
                            int rpCause = mDeliverCause.get(DELIVER_STATUS_ERROR_INVALID_MR_VALUE);
                            SmsRPdu rpErrorPdu = new SmsRPdu(mtData.getMessageRef(),
                                                     SmsUtils.RP_ERROR,
                                                     mtData.getOrigAddr(), rpCause,
                                                     null);
                            byte[] encodedPdu = rpErrorPdu.getRpduByteArray();
                            String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
                            Rlog.i(TAG, "base 64 Encoded RPDU: " + pdu64);
                            boolean res = false;
                            res =  mMtsController.sendMessage(SmsUtils.FORMAT_INT_3GPP,
                                                    pdu64, mtData.getOrigAddr(),
                                                    targetAddress, mtData.getMessageRef());

                            if (!res) {
                                Rlog.e(TAG, "failed to send RP-Error");
                            }
                            return MtsController.MT_FAILURE;
                        }
                        mSmsRLStateMachine = mTokenStateTrackerMap.get(token);
                        mSmsRLStateMachine.mRPduData = mtData;
                        mSmsRLStateMachine.onRPAckFromNetwork(mtData);
                        mTokenStateTrackerMap.remove(token);
                    }
                } else if (mtData.getMessageType() == SmsUtils.RP_DATA) {
                    token = mToken.incrementAndGet();
                    synchronized (mLock) {
                        mMTTokenMRMap.put(token, messageRef);
                        mSmsRLStateMachine = new SmsRLStateMachine(token, SmsUtils.RP_DATA,
                                                mMtsController, mCallContext, mListener,
                                                targetAddress,
                                                mtData.getOrigAddr());
                        mTokenStateTrackerMap.put(token, mSmsRLStateMachine);
                        result = mSmsRLStateMachine.onRPDataFromNetwork(mtData);
                        if (result != SmsUtils.RESULT_SUCCESS) {
                            return MtsController.MT_FAILURE;
                        }
                    }
                } else if (mtData.getMessageType() == SmsUtils.RP_ERROR) {
                    if (mMoMRTokenMap.containsKey(messageRef)) {
                        token = mMoMRTokenMap.get(messageRef);
                        mMoMRTokenMap.remove(messageRef);
                    } else {
                        Rlog.e(TAG, "No Matching RP-Data for Rp-Error");
                        return MtsController.MT_FAILURE;
                    }
                    mSmsRLStateMachine = mTokenStateTrackerMap.get(token);
                    mSmsRLStateMachine.mRPduData = mtData;
                    mSmsRLStateMachine.onRPErrorFromNetwork(mtData);
                    mTokenStateTrackerMap.remove(token);
                } else {
                    //send RP-Error as per TS 24.011, section 9.3.3
                    int rpCause = mDeliverCause.get(ImsSmsImplBase
                                               .DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED);
                    SmsRPdu rpErrorPdu = new SmsRPdu(mtData.getMessageRef(), SmsUtils.RP_ERROR,
                                                   mtData.getOrigAddr(), rpCause,
                                                   null);
                    byte[] encodedPdu = rpErrorPdu.getRpduByteArray();
                    String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
                    Rlog.i(TAG, "base 64 Encoded RPDU: " + pdu64);
                    boolean res = false;
                    res =  mMtsController.sendMessage(SmsUtils.FORMAT_INT_3GPP,
                                                    pdu64, mtData.getOrigAddr(),
                                                    targetAddress,
                                                    mtData.getMessageRef());

                    if (!res) Rlog.e(TAG, "failed to send RP-ERROR");

                    return MtsController.MT_FAILURE;
                }
                return MtsController.MT_SUCCESS;
            } catch (RuntimeException e) {
                Rlog.e(TAG, "notifyIncomingMessage Failed " + e.getMessage());
                return MtsController.MT_FAILURE;
            }
        }
    }
}
