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
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.imsstack.imsservice.mmtel.ImsRegistrationTracker;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.io.ByteArrayOutputStream;
import java.util.Arrays;

/**
 * Implements the State Machine of Sms Relay Layer as per 24.011
 */
public class SmsRLStateMachine {
    private static final String TAG = "[GII-SmsRLSM] ";
    //TODO: b/245837957: To be changed to False or ImsLog.isDebuggable()
    private static final boolean DBG = true;
    private MtsController mMtsController = null;
    private SmsRelayLayer.Listener mListener = null;
    private SmsRLState mCurrentState;
    private final ImsCallContext mContext;
    private Handler mHandler = null;
    public int mToken;
    public int mMessageType;
    public int mRetryCount = 0;
    public SmsRPdu mRPduData;
    public String mPSISmsc;
    public String mDestinationAddress;
    public int mTpMr;
    public int mTimerTr1;
    public int mTimerTr2;

    protected Runnable mTR1TimerHandler = new Runnable() {
        @Override
        public void run() {
            onTR1TimerExpired();
        }
    };

    protected Runnable mTR2TimerHandler = new Runnable() {
        @Override
        public void run() {
            onTR2TimerExpired();
        }
    };


    public SmsRLStateMachine(int token, int messageType, MtsController mtsController,
                                    ImsCallContext context, SmsRelayLayer.Listener listener,
                                    String psiSmsc, String destinationAddress) {
        mCurrentState = SmsRLState.IDLE;
        mMtsController  = mtsController;
        mContext = context;
        mHandler = mContext.getCallHandler();
        mMessageType = messageType;
        mListener = listener;
        mPSISmsc = psiSmsc;
        mDestinationAddress = destinationAddress;
        mToken = token;
        mTimerTr1 = getTimerTR1();
        mTimerTr2 = getTimerTR2();
    }

    /**
     * Interface which provides Event handler Apis for Relay Layer State Machine
     */
    public interface StateEventListener {
        /**
         * Handles RP-Data received from network and sends back to Transfer Layer
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         * @param mtRPData includes Encoded RPDU and other details of incoming SMS-RPDU
         * @return the result  if incoming RPDU is processed successfully
         */
        default int onRPDataFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPData) {
            loge(smsRLStateMachine.mCurrentState + " State: onRPDataFromNetwork :: Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }

        /**
         * Handles RP-Data received from TransferLayer and sends to MtsCOntroller
         * @param smsRLStateMachine  the state machine object which holds details of
         * this Sms Session
         * @param moRPData includes Encoded RPDU and other details of MO SMS-RPDU
         * @return the result if MO RPDU is sent successfully from RelayLayer
         */
        default int onRPDataFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPData) {
            loge(smsRLStateMachine.mCurrentState + " State: onRPDataFromTL :: Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }
        /**
         * Handles RP-Ack received from network and sends back to Transfer Layer
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         * @param moRPAck includes Encoded RPDU and other details of incoming RP-Ack
         */
        default void onRPAckFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPAck) {
            loge(smsRLStateMachine.mCurrentState + " State: onRPAckFromNetwork :: Invalid Event");
        }

        /**
         * Handles RP-Ack received from TransferLayer and sends to MtsCOntroller
         * @param smsRLStateMachine  the state machine object which holds details of
         * this Sms Session
         * @param mtRPAck includes Encoded RPDU and other details of RP-Ack
         * @return the result if RP-Ack is sent successfully from Relay Layer
         */
        default int onRPAckFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPAck) {
            loge(smsRLStateMachine.mCurrentState + " State: onRPAckFromTL :: Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }

        /**
         * Handles RP-Error received from network and sends back to Transfer Layer
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         * @param moRPError includes Encoded RPDU and other details of incoming RP-Ack
         */
        default void onRPErrorFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPError) {
            loge(smsRLStateMachine.mCurrentState + " State: onRPErrorFromNetwork :: Invalid Event");
        }

        /**
         * Handles RP-Error received from TransferLayer and sends to MtsCOntroller
         * @param smsRLStateMachine  the state machine object which holds details of
         * this Sms Session
         * @param mtRPError includes Encoded RPDU and other details of RP-Ack
         * @return the result if MO RP-Error is sent successfully from Relay Layer
         */
        default int onRPErrorFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPError) {
            loge(smsRLStateMachine.mCurrentState + " State: onRPErrorFromTL :: Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }

        /**
         * Handles the TR1 Timer Expiry, sets the appropriate state and Request retry
         * Sms to telephony
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         */
        default void onTR1TimerExpired(SmsRLStateMachine smsRLStateMachine) {
            loge(smsRLStateMachine.mCurrentState + " State: onTR1TimerExpired :: Invalid Event");
        }

        /**
         * Handles the TR2 Timer Expiry, sets the appropriate state and Request retry
         * Sms to telephony
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         */
        default void onTR2TimerExpired(SmsRLStateMachine smsRLStateMachine) {
            loge(smsRLStateMachine.mCurrentState + " State: onTR2TimerExpired :: Invalid Event");
        }

        /**
         * Handles the state machine when there is SIP Error response for the SIP
         * Message that is sent to network
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms session
         * @param isSuccess the boolean value based on SIP Error response
         * @param status the retryAfter value in SIP Response
         * @return the result of handling Sip response for RP Message
         */
        default int onSipResponseForRPMessage(SmsRLStateMachine smsRLStateMachine,
                                              boolean isSuccess, int status) {
            loge(smsRLStateMachine.mCurrentState
                    + " State: onSipResponseForRPMessage :: Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }
    }

    enum SmsRLState implements StateEventListener {
        IDLE {
            @Override
            public int onRPDataFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPData) {
                logi("IDLE State: onRPDataFromNetwork");
                byte[] tpdu = mtRPData.getUserData();
                byte[] rpdu = mtRPData.getRpduByteArray();
                if (smsRLStateMachine.mListener == null) {
                    loge("Listener is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }

                /** Framework's TPdu Parser expects the TPdu be prepended with SC-Address.
                 * else the parser will throw exception. So prepending TPdu with
                 * RP-OriginatingAdrress.
                 */
                int originAddressLength  = rpdu[SmsUtils.RPDU_ORIGIN_ADDR_LENGTH_INDEX];
                ByteArrayOutputStream bo = new ByteArrayOutputStream(originAddressLength
                                                                  + tpdu.length + 1);
                bo.write(originAddressLength);
                bo.write(rpdu, SmsUtils.RPDU_ORIGIN_ADDR_VALUE_INDEX, originAddressLength);
                bo.write(tpdu, 0, tpdu.length);
                byte[] frameworkPdu = bo.toByteArray();
                smsRLStateMachine.mTpMr = tpdu[SmsUtils.TPDU_MR_INDEX] & 0xff;
                smsRLStateMachine.mDestinationAddress = mtRPData.getOrigAddr();
                int result = smsRLStateMachine.mListener
                                     .notifyRLDataIndication(smsRLStateMachine.mToken,
                                     SmsUtils.FORMAT_INT_3GPP, SmsUtils.RP_DATA,
                                     frameworkPdu);
                if (result != SmsUtils.RESULT_SUCCESS) {
                    loge("Failed to process Sms");
                    return SmsUtils.SMSTL_RESULT_FAILURE;
                }
                smsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
                //set TR2 timer
                if (smsRLStateMachine.mHandler != null) {
                    smsRLStateMachine.mHandler.postDelayed(smsRLStateMachine.mTR2TimerHandler,
                                                       smsRLStateMachine.mTimerTr2);
                } else {
                    smsRLStateMachine.mTR2TimerHandler = null;
                    loge("Handler is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                return SmsUtils.RESULT_SUCCESS;
            }
            @Override
            public int onRPDataFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPData) {
                logi("IDLE State: onRPDataFromTL");
                byte[] tpdu = moRPData.getUserData();
                byte[] encodedPdu = moRPData.getRpduByteArray();
                if (tpdu != null) {
                    smsRLStateMachine.mTpMr = tpdu[SmsUtils.TPDU_MR_INDEX] & 0xff;
                }
                if (encodedPdu == null) {
                    loge("Encoding Failed");
                    return SmsUtils.SMSRL_RESULT_PDU_ENCODING_FAILED;
                }
                if (smsRLStateMachine.mMtsController == null) {
                    loge("mMtsController is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                if (DBG) {
                    log(" Target Address = "
                                    + ImsLog.hiddenString(smsRLStateMachine.mPSISmsc)
                                    + " Dialled Number = "
                                    + ImsLog.hiddenString(smsRLStateMachine.mDestinationAddress)
                                    + " messageRef = "
                                    + moRPData.getMessageRef());
                }
                boolean result = smsRLStateMachine.mMtsController.sendMessage(
                                                SmsUtils.FORMAT_INT_3GPP, encodedPdu,
                                                smsRLStateMachine.mPSISmsc,
                                                smsRLStateMachine.mDestinationAddress,
                                                moRPData.getMessageRef(),
                                                moRPData.isRetry());

                if (!result) {
                    loge("Failed to send Sms");
                    return SmsUtils.SMSRL_RESULT_MTS_CONTROLLER_FAILED;
                }
                smsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);

                if (smsRLStateMachine.mHandler != null) {
                    //set TR1 timer
                    smsRLStateMachine.mHandler.postDelayed(smsRLStateMachine.mTR1TimerHandler,
                                                        smsRLStateMachine.mTimerTr1);
                } else {
                    smsRLStateMachine.mTR1TimerHandler = null;
                    loge("Handler is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                return SmsUtils.RESULT_SUCCESS;
            }
        },
        WAIT_FOR_RPACK_FROM_NW {
            @Override
            public void onRPAckFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPAck) {
                log("WAIT_FOR_RPACK_FROM_NW State: onRPAckFromNetwork");
                if (smsRLStateMachine.mListener == null) {
                    loge("Listener is null");
                    return;
                }
                //stop TR1
                int token = smsRLStateMachine.mToken;
                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR1TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine.mTR1TimerHandler);
                    smsRLStateMachine.mTR1TimerHandler = null;
                } else {
                    loge("Handler is null");
                    smsRLStateMachine.mTR1TimerHandler = null;
                }

                int tpMR = smsRLStateMachine.mTpMr;
                smsRLStateMachine.mListener.notifyRLReportIndication(token, tpMR,
                                                                    ImsSmsImplBase.SEND_STATUS_OK,
                                                                    SmsManager.RESULT_ERROR_NONE,
                                                                    0);
                smsRLStateMachine.setState(IDLE);
            }
            @Override
            public void onRPErrorFromNetwork(SmsRLStateMachine smsRLStateMachine,
                                                                SmsRPdu moRPError) {
                log("WAIT_FOR_RPACK_FROM_NW State: onRPErrorFromNetwork");

                if (smsRLStateMachine.mListener == null) {
                    loge("Listener is null");
                    return;
                }

                int causeCode = moRPError.getRPCause();
                int token = smsRLStateMachine.mToken;
                int tpMR = smsRLStateMachine.mTpMr;
                int sendStatus = smsRLStateMachine.getSendStatus(causeCode);

                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR1TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine.mTR1TimerHandler);
                    smsRLStateMachine.mTR1TimerHandler = null;
                } else {
                    loge("Handler is null");
                    smsRLStateMachine.mTR1TimerHandler = null;
                }

                smsRLStateMachine.mListener.notifyRLReportIndication(token, tpMR, sendStatus,
                                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(causeCode),
                                causeCode);
                smsRLStateMachine.setState(IDLE);
            }

            @Override
            public void onTR1TimerExpired(SmsRLStateMachine smsRLStateMachine) {
                log("WAIT_FOR_RPACK_FROM_NW State: onTR1TimerExpired");
                if (smsRLStateMachine.mListener == null) {
                    loge("Listener is null");
                    return;
                }
                log("handleMessage: token = " + smsRLStateMachine.mToken
                                                    + " Tp-MessageReference = "
                                                    + smsRLStateMachine.mTpMr);
                if (smsRLStateMachine.mMtsController != null) {
                    smsRLStateMachine.mMtsController.notifyMoSmsTimedOut();
                }
                smsRLStateMachine.mListener.notifyRLReportIndication(smsRLStateMachine.mToken,
                                                smsRLStateMachine.mTpMr,
                                                ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                                                SmsManager.RESULT_ERROR_GENERIC_FAILURE, 0);
                smsRLStateMachine.setState(IDLE);
            }
            @Override
            public int onSipResponseForRPMessage(SmsRLStateMachine smsRLStateMachine,
                                                  boolean isSuccess, int status) {
                logi("WAIT_FOR_RPACK_FROM_NW State: onSipResponseForRPMessage");
                if (smsRLStateMachine.mListener == null) {
                    loge("Listener is  null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }

                if (isSuccess) {
                    logi("onSipResponseForRPMessage: 2xx received");
                    return SmsUtils.RESULT_SUCCESS;
                }

                smsRLStateMachine.mListener.notifyRLReportIndication(smsRLStateMachine.mToken,
                                                    smsRLStateMachine.mTpMr,
                                                    status,
                                                    SmsManager.RESULT_NETWORK_ERROR, 0);
                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR1TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine
                                                                        .mTR1TimerHandler);
                    smsRLStateMachine.mTR1TimerHandler = null;
                } else {
                    smsRLStateMachine.mTR1TimerHandler = null;
                }

                smsRLStateMachine.setState(IDLE);
                return SmsUtils.RESULT_SUCCESS;
            }
        },

        WAIT_TO_SEND_RPACK_TO_NW {
            @Override
            public int onRPAckFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPAck) {
                log("WAIT_TO_SEND_RPACK_TO_NW: onRPAckFromTL");
                byte[] encodedPdu = encodedPdu = mtRPAck.getRpduByteArray();
                if (encodedPdu == null) {
                    loge("onRPAckFromTL: Encoding Failed");
                    return SmsUtils.SMSRL_RESULT_INVALID_STATE;
                }
                if (smsRLStateMachine.mMtsController == null) {
                    loge("onRPAckFromTL: mMtsController is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                if (DBG) {
                    log("onRPAckFromTL: Encoded RPDU = "
                            + ImsLog.hiddenString(ImsUtils.bytesToHexString(encodedPdu))
                            + " dialled number = " + ImsLog.hiddenString(smsRLStateMachine
                                                    .mDestinationAddress)
                            + " PSI/SMSC = " + ImsLog.hiddenString(smsRLStateMachine.mPSISmsc));
                }
                boolean result = smsRLStateMachine.mMtsController
                                            .sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu,
                                            smsRLStateMachine.mPSISmsc,
                                            smsRLStateMachine.mDestinationAddress,
                                                         mtRPAck.getMessageRef());
                if (!result) {
                    logi("onRPAckFromTL: failed to send RpAck");
                    return SmsUtils.SMSRL_RESULT_INVALID_STATE;
                }
                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR2TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine.mTR2TimerHandler);
                    smsRLStateMachine.mTR2TimerHandler = null;
                } else {
                    logi("onRPAckFromTL: handler is null");
                    smsRLStateMachine.mTR2TimerHandler = null;
                }
                return SmsUtils.RESULT_SUCCESS;
            }
            @Override
            public int onRPErrorFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPError) {
                log("WAIT_TO_SEND_RPACK TO NW : onRPErrorFromTL");
                byte[] encodedPdu = mtRPError.getRpduByteArray();
                if (encodedPdu == null) {
                    log("onRPErrorFromTL: Encoding Failed");
                    return SmsUtils.SMSRL_RESULT_PDU_ENCODING_FAILED;
                }
                if (DBG) {
                    log("onRPErrorFromTL: Encoded RPDU = "
                            + ImsLog.hiddenString(ImsUtils.bytesToHexString(encodedPdu))
                            + " dialled number = "
                            + ImsLog.hiddenString(smsRLStateMachine.mDestinationAddress)
                            + " PSI/SMSC = " + ImsLog.hiddenString(smsRLStateMachine.mPSISmsc));
                }
                boolean result = smsRLStateMachine.mMtsController
                                                  .sendMessage(SmsUtils.FORMAT_INT_3GPP, encodedPdu,
                                                             smsRLStateMachine.mPSISmsc,
                                                             smsRLStateMachine.mDestinationAddress,
                                                             mtRPError.getMessageRef());
                if (!result) {
                    loge("onRPErrorFromTL: failed to send RpError");
                    return SmsUtils.SMSRL_RESULT_MTS_CONTROLLER_FAILED;
                }
                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR2TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine.mTR2TimerHandler);
                    smsRLStateMachine.mTR2TimerHandler = null;
                } else {
                    if (DBG) log("onRPErrorFromTL: handler is null");
                    smsRLStateMachine.mTR2TimerHandler = null;
                }
                smsRLStateMachine.setState(IDLE);
                return SmsUtils.RESULT_SUCCESS;
            }
            @Override
            public void onTR2TimerExpired(SmsRLStateMachine smsRLStateMachine) {
                logi("onTR2TimerExpired");
                smsRLStateMachine.setState(IDLE);
                //TODO: Send an RP-Error back to network
            }
        }
    }
    /**
     * Sets the state  of  state machine
     * @param state the state to be set as current state of state machine
     */
    public void setState(SmsRLState state) {
        mCurrentState = state;
    }

    /**
     * Gets the SEND_STATUS param from carrier config if any
     * to indicate framework either to retry over IMS or Fallback
     *
     * @param causeCode the RP-Cause in RP-Error from network
     * @return the sendStatus to be sent to framework indicating the retry mechanism
     */
    public int getSendStatus(int causeCode) {
        int sendStatus = SmsRPErrorCause.getSendSmsStatusByRPCauseCode(causeCode);
        if (needToCheckNetworkStatus(causeCode)) {
            if (isImsRegisteredOnWifi() && (isRoaming() || !isCellularNetworkAvailable())) {
                return ImsSmsImplBase.SEND_STATUS_ERROR;
            }
        }
        int[] causeValues = getConfigInterface(mContext.getSlotId()).getCarrierConfig()
                .getIntArray(CarrierConfigManager.ImsSms
                .KEY_SMS_RP_CAUSE_VALUES_TO_RETRY_OVER_IMS_INT_ARRAY);
        if (causeValues != null
                && Arrays.stream(causeValues).anyMatch(value -> value == causeCode)) {
            sendStatus = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
        }
        causeValues = getConfigInterface(mContext.getSlotId()).getCarrierConfig()
                .getIntArray(CarrierConfigManager.ImsSms
                .KEY_SMS_RP_CAUSE_VALUES_TO_FALLBACK_INT_ARRAY);
        if (causeValues != null
                && Arrays.stream(causeValues).anyMatch(value -> value == causeCode)) {
            sendStatus = ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK;
        }
        return sendStatus;
    }

    @VisibleForTesting
    protected boolean isImsRegisteredOnWifi() {
        ImsServiceRecord serviceRecord = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
        if (serviceRecord == null) {
            return false;
        }
        ImsRegistrationTracker regTracker = serviceRecord.getRegistrationTracker();
        if (regTracker == null) {
            return false;
        }
        return regTracker.getRegisteredNetworkType() == IAosRegistrationListener.NetworkType.IWLAN;
    }

    @VisibleForTesting
    protected boolean isRoaming() {
        IDcNetWatcher dcnw = getNetworkWatcher();
        return (dcnw != null) && dcnw.isRoaming();
    }

    @VisibleForTesting
    protected boolean isCellularNetworkAvailable() {
        IDcNetWatcher dcnw = getNetworkWatcher();
        return (dcnw != null)
                && (dcnw.getCellularDataServiceState() == ServiceState.STATE_IN_SERVICE);
    }

    @VisibleForTesting
    protected boolean needToCheckNetworkStatus(int causeCode) {
        ConfigInterface config = getConfigInterface(mContext.getSlotId());
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        if (cc == null) {
            return false;
        }
        int[] causeCodes = cc.getIntArray(
                CarrierConfig.ImsSms.KEY_SMS_EVALUATE_RADIO_STATUS_FOR_RP_ERROR_CAUSES_INT_ARRAY);
        if (causeCodes != null && Arrays.stream(causeCodes).anyMatch(value -> value == causeCode)) {
            return true;
        }
        return false;
    }

    private IDcNetWatcher getNetworkWatcher() {
        return DcFactory.getDcAgent(IDcNetWatcher.class, mContext.getSlotId());
    }

    /**
     * gets the state of state machine
     * @return the state of state machine
     */
    @VisibleForTesting
    public SmsRLState getState() {
        return mCurrentState;
    }

    /**
     * gets the TR1 Timer from carrier Config
     * @return the TR1 Timer in milliseconds
     */
    public int getTimerTR1() {
        return getConfigInterface(mContext.getSlotId()).getCarrierConfig()
                .getInt(CarrierConfigManager.ImsSms.KEY_SMS_TR1_TIMER_MILLIS_INT);

    }

    /**
     * gets the TR2 Timer from carrier Config
     * @return the TR2 Timer in milliseconds
     */
    public int getTimerTR2() {
        return getConfigInterface(mContext.getSlotId()).getCarrierConfig()
                .getInt(CarrierConfigManager.ImsSms.KEY_SMS_TR2_TIMER_MILLIS_INT);
    }

    /**
     * Handles RP-Data received from network and sends back to Transfer Layer
     * @param mtRPData includes Encoded RPDU and other details of incoming SMS-RPDU
     * @return the result  if incoming RPDU is processed successfully
     */
    public int onRPDataFromNetwork(SmsRPdu mtRPData) {
        return mCurrentState.onRPDataFromNetwork(this, mtRPData);
    }

    /**
     * Handles RP-Data received from TransferLayer and sends to MtsCOntroller
     * @param moRPData includes Encoded RPDU and other details of MO SMS-RPDU
     * @return the result if MO RPDU is sent successfully from RelayLayer
     */
    public int onRPDataFromTL(SmsRPdu moRPData) {
        return mCurrentState.onRPDataFromTL(this, moRPData);
    }

    /**
     * Handles RP-Ack received from network and sends back to Transfer Layer
     * @param moRPAck includes Encoded RPDU and other details of incoming RP-Ack
     */
    public void onRPAckFromNetwork(SmsRPdu moRPAck) {
        mCurrentState.onRPAckFromNetwork(this, moRPAck);
    }

    /**
     * Handles RP-Ack received from TransferLayer and sends to MtsCOntroller
     * @param mtRPAck includes Encoded RPDU and other details of RP-Ack
     * @return the result if RP-Ack is sent successfully from Relay Layer
     */
    public int onRPAckFromTL(SmsRPdu mtRPAck) {
        return mCurrentState.onRPAckFromTL(this, mtRPAck);
    }

    /**
     * Handles RP-Error received from network and sends back to Transfer Layer
     * @param moRPError includes Encoded RPDU and other details of incoming RP-Ack
     */
    public void onRPErrorFromNetwork(SmsRPdu moRPError) {
        mCurrentState.onRPErrorFromNetwork(this, moRPError);
    }

    /**
     * Handler for processing the RP-Error and send to MtsController
     * @param mtRPError the SmsRPdu object which includes RP-Error PDU
     * @return the result if RP-Error was successfully sent from Relay Layer
     */
    public int onRPErrorFromTL(SmsRPdu mtRPError) {
        return mCurrentState.onRPErrorFromTL(this, mtRPError);
    }

    /**
     * Handler for TR1 Timer Expiry
     */
    public void onTR1TimerExpired() {
        mCurrentState.onTR1TimerExpired(this);
    }

    /**
     * Handler for TR2 Timer Expiry
     */
    public void onTR2TimerExpired() {
        mCurrentState.onTR2TimerExpired(this);
    }

    /**
     * Handles the state machine if there is any SIP-Error Response
     * for the RPDU sent
     * @param isSuccess indicates whether sip response is 2xx or not
     * @param status the value of retryAfter header if any in response
     * @return the result of processing the SIP response
     */
    public int onSipResponseForRPMessage(boolean isSuccess, int status) {
        return mCurrentState.onSipResponseForRPMessage(this, isSuccess, status);
    }

    /**
     * Returns the configuration interface.
     *
     * @param slotId The slot-id to be retrieved.
     * @return A ConfigInterface instance.
     */
    public static ConfigInterface getConfigInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
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
}

