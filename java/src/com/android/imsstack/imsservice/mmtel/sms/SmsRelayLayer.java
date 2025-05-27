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

import android.net.Uri;
import android.telephony.PhoneNumberUtils;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Provides the functionality of SMS Relay Layer as per 3GPP TS 24.011
 **/
public class SmsRelayLayer {
    private static final String TAG = "[GII-SmsRL] ";
    private final ImsCallContext mCallContext;
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
    //TODO: b/245837957: To be changed to False or ImsLog.isDebuggable()
    private static final boolean DBG = true;
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
        mMtsController = mtsController;
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
     * @deprecated use {@code sendRPMessage(int, int, String, String, byte[], int)} instead
     *
     * @return the result of sending RPDU to SMS Enabler
     */
    public int sendRPMessage(
            int token, int rpType, String smsc, String destinationAddress,
            byte[] tpdu, int statusResult) {
        return sendRPMessage(token, rpType, smsc, destinationAddress, tpdu, statusResult, false);
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
     * @param isRetry true if it's a retry attempt, otherwise false.
     *
     * @return the result of sending RPDU to SMS Enabler
     */
    public int sendRPMessage(
            int token, int rpType, String smsc, String destinationAddress,
            byte[] tpdu, int statusResult, boolean isRetry) {
        try {
            logi("sendRPMessage");
            if (DBG) {
                log("token = " + token
                        + " RP Message Type = " + rpType
                        + " Smsc = " + smsc
                        + " destinationAddress = " +  ImsLog.hiddenString(destinationAddress)
                        + " statusResult = " + statusResult);
            }
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
                targetAddress = getPSIValue();
                if (DBG) log("PSI = " + ImsLog.hiddenString(targetAddress));
                //return if smsc is null as it is a must to construct RP-Destination address
                if (smsc == null || smsc.length() == 0) {
                    loge("Smsc is null");
                    return SmsUtils.SMS_RESULT_INVALID_SMSC_ADDRESS;
                }
                //As per b/232048441 if PSI is null, targetAddress is set to smsc
                if (targetAddress == null || targetAddress.length() == 0) {
                    logi("PSI is null");
                    /* smsc address is passed in encoded format. Extracting the address
                     * string in decoded format and assigning it to TargetAddress
                     */
                    if (rpType == SmsUtils.RP_SMMA) {
                        targetAddress = smsc;
                    } else {
                        targetAddress = decodeSmsc(smsc);
                    }
                    if (DBG) log("targetAddress set to Smsc:" + ImsLog.hiddenString(targetAddress));
                }
                /* As per b/232048441, if PSI & Smsc is null,
                 * targetAddress is set to destination Address
                */
                if (targetAddress == null || targetAddress.length() == 0) {
                    if (DBG) {
                        log("sending SMSCaddress as Target Address: "
                                + ImsLog.hiddenString(destinationAddress));
                    }
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
                                                      rpCause, tpdu, isRetry);
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
                        loge("No Corresponding Rp-Data for the RP-Ack");
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
                        loge("No Corresponding Rp-Data for the RP-Error");
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
                                                      rpCause, tpdu, isRetry);
                    mSmsRLStateMachine = new SmsRLStateMachine(token, SmsUtils.RP_SMMA,
                                    mMtsController, mCallContext, mListener,
                                    targetAddress, destinationAddress);
                    mTokenStateTrackerMap.put(token, mSmsRLStateMachine);
                    result = mSmsRLStateMachine.onRPDataFromTL(moRPSmmaPdu);
                } else {
                    loge("Invalid message type");
                    return SmsUtils.SMSRL_RESULT_INVALID_RP_MESSAGE_TYPE;
                }
            }
            return result;
        } catch (RuntimeException e) {
            loge("SendRPMessage Failed: " + e.getMessage());
            return SmsUtils.SMSRL_RESULT_EXCEPTION;
        }
    }

    private String decodeSmsc(String smsc) {
        byte[] targetAddrBytes = ImsUtils.hexStringToBytes(smsc);
        int len = targetAddrBytes[0];
        return PhoneNumberUtils.calledPartyBCDToString(targetAddrBytes, 1,
                            len, PhoneNumberUtils.BCD_EXTENDED_TYPE_CALLED_PARTY);
    }

    /**
     * Fetches PSI Value from SmsManager using SystemApi
     * @return PSI Value
     */
    protected String getPSIValue() {
        try {
            synchronized (mLock) {
                SmsManagerProxy smp =
                        AppContext.getInstance().getSystemServiceProxy(SmsManagerProxy.class);
                Uri psi = smp.getSmscIdentity();
                return psi.toString();
            }
        } catch (RuntimeException e) {
            loge("getPSIValue: " + e.getMessage());
            return null;
        }
    }
    /*
     * Increments and returns RP-MR Value
     * @return next RP-MR value
     */
    private int getRPMessageReference() {
        return (mRPMR.incrementAndGet() % SmsUtils.MAX_RPMR_VALUE);
    }

    private void sendRPError(int deliverStatusResult, SmsRPdu mtData, String targetAddress) {
        try{
            int rpCause = mDeliverCause.get(deliverStatusResult);
            SmsRPdu rpErrorPdu = new SmsRPdu(mtData.getMessageRef(), SmsUtils.RP_ERROR,
                                               mtData.getOrigAddr(), rpCause,
                                               null);
            byte[] encodedPdu = rpErrorPdu.getRpduByteArray();
            if (DBG) {
                log("Sending Encoded RP-Error: "
                            + ImsLog.hiddenString(ImsUtils.bytesToHexString(encodedPdu)));
            }
            boolean res =  mMtsController.sendMessage(SmsUtils.FORMAT_INT_3GPP,
                                            encodedPdu,
                                            targetAddress,
                                            mtData.getOrigAddr(),
                                            mtData.getMessageRef());
            if (!res) loge("Failed to send RP-ERROR");
        } catch (RuntimeException e) {
            loge("sendRPError Failed: " + e.getMessage());
        }
    }

    private static void log(String s) {
        ImsLog.d(TAG + s);
    }

    private static void loge(String s) {
        ImsLog.e(TAG + s);
    }

    private static void logi(String s) {
        ImsLog.i(TAG + s);
    }

    class MtsControllerListenerProxy extends MtsController.Listener {
        @Override
        public void notifyStatusForOutgoingMessage(
                int statusResult, int format, int messageReference) {
            try {
                log("notifyStatusForOutgoingMessage :: statusResult = " + statusResult
                        + " format = " + format + " messageReference = " + messageReference);
                int token = -1;
                int result = SmsUtils.RESULT_SUCCESS;
                Listener listener = mListener;
                int status = ImsSmsImplBase.SEND_STATUS_ERROR;
                if (statusResult == MtsController.MO_SUCCESS) {
                    logi("SIP 2xx response");
                    return;
                }
                if (listener == null) {
                    loge("listener is null");
                    return;
                }
                synchronized (mLock) {
                    if (mMoMRTokenMap.containsKey(messageReference)) {
                        token = mMoMRTokenMap.get(messageReference);
                        mMoMRTokenMap.remove(messageReference);
                    } else {
                        loge("MessageReference does not exist");
                        return;
                    }
                    mSmsRLStateMachine = mTokenStateTrackerMap.get(token);

                    if (statusResult == MtsController.MO_ERROR_GENERIC) {
                        status = ImsSmsImplBase.SEND_STATUS_ERROR;
                    } else if (statusResult == MtsController.MO_ERROR_RETRY) {
                        status = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
                    } else if (statusResult == MtsController.MO_ERROR_FALLBACK) {
                        status = ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK;
                    }
                    mTokenStateTrackerMap.remove(token);
                    result = mSmsRLStateMachine.onSipResponseForRPMessage(false, status);
                    if (result != SmsUtils.RESULT_SUCCESS) {
                        loge("onSipResponseForRPMessage Failed");
                    }
                }
            } catch (RuntimeException e) {
                loge("notifyStatusForOutgoingMessage Failed: " + e.getMessage());
            }
        }

        @Override
        public void notifyIncomingMessage(int smsFormat, byte[] pduData) {
            try {
                logi("notifyIncomingMessage SmsFormat = " + smsFormat);
                int token = 0;
                int result;
                Listener listener = mListener;
                if (smsFormat == SmsUtils.FORMAT_INT_3GPP2) {
                    token = mToken.incrementAndGet();
                    if(listener == null) {
                        loge("Listener is null");
                        return;
                    }
                    synchronized (mLock) {
                        result = listener.notifyRLDataIndication(token, smsFormat,
                                                                 SmsUtils.RP_DATA, pduData);
                    }
                    if (result != SmsUtils.RESULT_SUCCESS) {
                        loge("notifyRLDataIndication Failed");
                    }
                    return;
                }
                SmsRPdu mtData = new SmsRPdu(pduData);
                if (DBG) {
                    log("Rpdu fields new: message type ="
                                    + mtData.getMessageType()
                                    + " messageRef ="
                                    + mtData.getMessageRef()
                                    + " Originating Address = "
                                    + ImsLog.hiddenString(mtData.getOrigAddr()));
                }
                int messageRef = mtData.getMessageRef();
                String targetAddress = getPSIValue();
                if (targetAddress == null || targetAddress.length() == 0) {
                    targetAddress = mtData.getOrigAddr();
                    if (DBG) {
                        log("PSI is null so setting to  orig address"
                                + ImsLog.hiddenString(targetAddress));
                    }
                }
                if (listener == null) {
                    loge("Listener is null");
                    sendRPError(ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC, mtData, targetAddress);
                    return;
                }
                if (mtData.getMessageType() == SmsUtils.RP_ACK) {
                    synchronized (mLock) {
                        if (mMoMRTokenMap.containsKey(messageRef)) {
                            token = mMoMRTokenMap.get(messageRef);
                            mMoMRTokenMap.remove(messageRef);
                        } else {
                            //send RP-Error as per TS 24.011, section 9.3.3
                            loge("No Matching RP-Data for Rp-Ack");
                            sendRPError(DELIVER_STATUS_ERROR_INVALID_MR_VALUE, mtData,
                                    targetAddress);
                            return;
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
                            loge("onRPDataFromNetwork Failed");
                            sendRPError(ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC, mtData,
                                    targetAddress);
                            mMTTokenMRMap.remove(token);
                            mTokenStateTrackerMap.remove(token);
                            return;
                        }
                    }
                } else if (mtData.getMessageType() == SmsUtils.RP_ERROR) {
                    if (mMoMRTokenMap.containsKey(messageRef)) {
                        token = mMoMRTokenMap.get(messageRef);
                        mMoMRTokenMap.remove(messageRef);
                    } else {
                        loge("No Matching RP-Data for Rp-Error");
                        return;
                    }
                    mSmsRLStateMachine = mTokenStateTrackerMap.get(token);
                    mSmsRLStateMachine.mRPduData = mtData;
                    mSmsRLStateMachine.onRPErrorFromNetwork(mtData);
                    mTokenStateTrackerMap.remove(token);
                } else {
                    //send RP-Error as per TS 24.011, section 9.3.3
                    sendRPError(ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED, mtData,
                            targetAddress);
                }
            } catch (RuntimeException e) {
                loge("notifyIncomingMessage Failed: " + e.getMessage());
            }
        }
    }
}
