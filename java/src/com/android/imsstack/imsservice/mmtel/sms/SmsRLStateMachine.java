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
import android.telephony.Rlog;
import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.util.Base64;

import com.android.imsstack.enabler.mts.MtsController;
import com.android.imsstack.imsservice.mmtel.ImsCallContext;
import com.android.internal.annotations.VisibleForTesting;

import java.io.ByteArrayOutputStream;

/**
 * Implements the State Machine of Sms Relay Layer as per 24.011
 */
public class SmsRLStateMachine {
    private static final String TAG = "[GII-SmsRLSM] ";
    private MtsController mMtsController = null;
    private SmsRelayLayer.Listener mListener = null;
    private SmsRLState mCurrentState;
    private final ImsCallContext mContext;
    private Handler mHandler = null;
    public int mToken;
    public int mMessageType;
    public SmsRPdu mRPduData;
    public String mDestinationAddress;
    private Runnable mTR1TimerHandler = null;
    private Runnable mTR2TimerHandler = null;

    public SmsRLStateMachine(int token, int messageType, MtsController mtsController,
                                    ImsCallContext context, SmsRelayLayer.Listener listener,
                                                        String destinationAddress) {
        mCurrentState = SmsRLState.IDLE;
        mMtsController  = mtsController;
        mMessageType = messageType;
        mContext = context;
        mHandler = mContext.getCallHandler();
        mListener = listener;
        mDestinationAddress = destinationAddress;
        mToken = token;
    }

    @VisibleForTesting
    public SmsRLStateMachine(SmsRLState state, int token, int messageType,
                                MtsController mtsController, ImsCallContext context,
                                SmsRelayLayer.Listener listener,
                                String destinationAddress, Handler handler) {

        mCurrentState = SmsRLState.IDLE;
        mMtsController  = mtsController;
        mMessageType = messageType;
        mContext = context;
        mHandler = handler;
        mListener = listener;
        mDestinationAddress = destinationAddress;
        mToken = token;
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
            Rlog.e(TAG, "Invalid Event");
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
            Rlog.e(TAG, "Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }
        /**
         * Handles RP-Ack received from network and sends back to Transfer Layer
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         * @param moRPAck includes Encoded RPDU and other details of incoming RP-Ack
         */
        default void onRPAckFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPAck) {
            Rlog.e(TAG, "Invalid Event");
        }

        /**
         * Handles RP-Ack received from TransferLayer and sends to MtsCOntroller
         * @param smsRLStateMachine  the state machine object which holds details of
         * this Sms Session
         * @param mtRPAck includes Encoded RPDU and other details of RP-Ack
         * @return the result if RP-Ack is sent successfully from Relay Layer
         */
        default int onRPAckFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPAck) {
            Rlog.e(TAG, "Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }

        /**
         * Handles RP-Error received from network and sends back to Transfer Layer
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         * @param moRPError includes Encoded RPDU and other details of incoming RP-Ack
         */
        default void onRPErrorFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPError) {
            Rlog.e(TAG, "Invalid Event");
        }

        /**
         * Handles RP-Error received from TransferLayer and sends to MtsCOntroller
         * @param smsRLStateMachine  the state machine object which holds details of
         * this Sms Session
         * @param mtRPError includes Encoded RPDU and other details of RP-Ack
         * @return the result if MO RP-Error is sent successfully from Relay Layer
         */
        default int onRPErrorFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPError) {
            Rlog.e(TAG, "Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }

        /**
         * Handles the TR1 Timer Expiry, sets the appropriate state and Request retry
         * Sms to telephony
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         */
        default void onTR1TimerExpired(SmsRLStateMachine smsRLStateMachine) {
            Rlog.e(TAG, "Invalid Event");
        }

        /**
         * Handles the TR2 Timer Expiry, sets the appropriate state and Request retry
         * Sms to telephony
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms Session
         */
        default void onTR2TimerExpired(SmsRLStateMachine smsRLStateMachine) {
            Rlog.e(TAG, "Invalid Event");
        }

        /**
         * Handles the state machine when there is SIP Error response for the SIP
         * Message that is sent to network
         * @param smsRLStateMachine the state machine object which holds details of
         * this Sms session
         * @param isSuccess the boolean value based on SIP Error response
         * @param retryAfter the retryAfter value in SIP Response
         * @return the result of handling Sip response for RP Message
         */
        default int onSipResponseForRPMessage(SmsRLStateMachine smsRLStateMachine,
                                              boolean isSuccess, int retryAfter) {
            Rlog.e(TAG, "Invalid Event");
            return SmsUtils.SMSRL_RESULT_INVALID_STATE;
        }
    }

    enum SmsRLState implements StateEventListener {
        IDLE {
            @Override
            public int onRPDataFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPData) {
                Rlog.i(TAG, "IDLE State: onRPDataFromNetwork");
                byte[] tpdu = mtRPData.getUserData();
                byte[] rpdu = mtRPData.getRpduByteArray();
                if (smsRLStateMachine.mListener == null) {
                    Rlog.e(TAG, "Listener is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                smsRLStateMachine.mTR2TimerHandler = new Runnable() {
                    @Override
                    public void run() {
                        smsRLStateMachine.onTR2TimerExpired();
                    }
                };
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
                int result = smsRLStateMachine.mListener
                                     .notifyRLDataIndication(smsRLStateMachine.mToken,
                                     SmsUtils.FORMAT_INT_3GPP, SmsUtils.RP_DATA,
                                     frameworkPdu);
                if (result != SmsUtils.RESULT_SUCCESS) {
                    Rlog.i(TAG, "failed to process Sms");
                    return SmsUtils.SMSTL_RESULT_FAILURE;
                }
                smsRLStateMachine.setState(WAIT_TO_SEND_RPACK_TO_NW);
                //set TR2 timer
                if (smsRLStateMachine.mHandler != null) {
                    smsRLStateMachine.mHandler.postDelayed(smsRLStateMachine.mTR2TimerHandler,
                                                       SmsUtils.TIMER_TR2);
                } else {
                    smsRLStateMachine.mTR2TimerHandler = null;
                    Rlog.e(TAG, "Handler is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                return SmsUtils.RESULT_SUCCESS;
            }
            @Override
            public int onRPDataFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPData) {
                Rlog.i(TAG, "IDLE State: onRPDataFromTL");
                byte[] encodedPdu = null;
                encodedPdu = moRPData.getRpduByteArray();
                if (encodedPdu == null) {
                    Rlog.i(TAG, "Encoding Failed");
                    return SmsUtils.SMSRL_RESULT_PDU_ENCODING_FAILED;
                }
                if (smsRLStateMachine.mMtsController == null) {
                    Rlog.e(TAG, "mMtsController is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
                Rlog.i(TAG, "base 64 Encoded RPDU: " + pdu64);
                boolean result = smsRLStateMachine.mMtsController.sendMessage(
                                                    SmsUtils.FORMAT_INT_3GPP,
                                                    pdu64,
                                                    smsRLStateMachine.mDestinationAddress,
                                                    moRPData.getMessageRef());

                if (!result) {
                    Rlog.i(TAG, "failed to send Sms");
                    return SmsUtils.SMSRL_RESULT_MTS_CONTROLLER_FAILED;
                }
                smsRLStateMachine.setState(WAIT_FOR_RPACK_FROM_NW);
                smsRLStateMachine.mTR1TimerHandler = new Runnable() {
                    @Override
                    public void run() {
                        smsRLStateMachine.onTR1TimerExpired();
                    }
                };
                if (smsRLStateMachine.mHandler != null) {
                    //set TR1 timer
                    smsRLStateMachine.mHandler.postDelayed(smsRLStateMachine.mTR1TimerHandler,
                                                        SmsUtils.TIMER_TR1M);
                } else {
                    smsRLStateMachine.mTR1TimerHandler = null;
                    Rlog.e(TAG, "Handler is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }
                return SmsUtils.RESULT_SUCCESS;
            }
        },
        WAIT_FOR_RPACK_FROM_NW {
            @Override
            public void onRPAckFromNetwork(SmsRLStateMachine smsRLStateMachine, SmsRPdu moRPAck) {
                Rlog.i(TAG, "WAIT_FOR_RPACK_FROM_NW State: onRPAckFromNetwork");
                if (smsRLStateMachine.mListener == null) {
                    Rlog.e(TAG, "Listener is null");
                    return;
                }
                //stop TR1
                int token = smsRLStateMachine.mToken;
                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR1TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine.mTR1TimerHandler);
                    smsRLStateMachine.mTR1TimerHandler = null;
                } else {
                    Rlog.e(TAG, "Handler is null");
                    smsRLStateMachine.mTR1TimerHandler = null;
                }

                smsRLStateMachine.mListener.notifyRLReportIndication(token,
                                            ImsSmsImplBase.SEND_STATUS_OK,
                                            SmsManager.RESULT_ERROR_NONE);
                smsRLStateMachine.setState(IDLE);

            }
            @Override
            public void onTR1TimerExpired(SmsRLStateMachine smsRLStateMachine) {
                if (smsRLStateMachine.mListener == null) {
                    Rlog.e(TAG, "Listener is null");
                    return;
                }
                Rlog.i(TAG, "WAIT_FOR_RPACK_FROM_NW State: onTR1TimerExpired");
                Rlog.i(TAG, "handleMessage: token = " + smsRLStateMachine.mToken);
                smsRLStateMachine.mListener.notifyRLReportIndication(smsRLStateMachine.mToken,
                                            ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                                            SmsManager.RESULT_ERROR_GENERIC_FAILURE);
                smsRLStateMachine.setState(IDLE);
            }
            @Override
            public int onSipResponseForRPMessage(SmsRLStateMachine smsRLStateMachine,
                                                  boolean isSuccess, int retryAfter) {
                //TODO: on Sip error response, need to stop timer and set to IDLE State
                return SmsUtils.SMSRL_RESULT_INVALID_STATE;
            }
        },

        WAIT_TO_SEND_RPACK_TO_NW {
            @Override
            public int onRPAckFromTL(SmsRLStateMachine smsRLStateMachine, SmsRPdu mtRPAck) {
                Rlog.i(TAG, "WAIT_TO_SEND_RPACK_TO_NW: onRPAckFromTL");
                byte[] encodedPdu = encodedPdu = mtRPAck.getRpduByteArray();
                if (encodedPdu == null) {
                    Rlog.i(TAG, "Encoding Failed");
                    return SmsUtils.SMSRL_RESULT_INVALID_STATE;
                }
                if (smsRLStateMachine.mMtsController == null) {
                    Rlog.e(TAG, "mMtsController is null");
                    return SmsUtils.SMSRL_RESULT_FAILURE;
                }


                String pdu64 = Base64.encodeToString(encodedPdu, Base64.DEFAULT);
                Rlog.i(TAG, "base 64 Encoded RPDU: " + pdu64);
                boolean result = smsRLStateMachine.mMtsController
                                            .sendMessage(SmsUtils.FORMAT_INT_3GPP, pdu64,
                                            mtRPAck.getDestAddr(), mtRPAck.getMessageRef());
                if (!result) {
                    Rlog.i(TAG, "failed to send RpAck");
                    return SmsUtils.SMSRL_RESULT_INVALID_STATE;
                }
                if (smsRLStateMachine.mHandler != null
                        && smsRLStateMachine.mTR2TimerHandler != null) {
                    smsRLStateMachine.mHandler.removeCallbacks(smsRLStateMachine.mTR2TimerHandler);
                    smsRLStateMachine.mTR2TimerHandler = null;
                } else {
                    Rlog.i(TAG, "onRPAckFromTL handler is null");
                    smsRLStateMachine.mTR2TimerHandler = null;
                }
                return SmsUtils.RESULT_SUCCESS;
            }
            @Override
            public void onTR2TimerExpired(SmsRLStateMachine smsRLStateMachine) {
                smsRLStateMachine.setState(IDLE);
                Rlog.i(TAG, "WAIT_TO_SEND_RPACK_TO_NW: onTR2TimerExpired");
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
     * gets the state of state machine
     * @return the state of state machine
     */
    @VisibleForTesting
    public SmsRLState getState() {
        return mCurrentState;
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
     * @param retryAfter the value of retryAfter header if any in response
     * @return the result of processing the SIP response
     */
    public int onSipResponseForRPMessage(boolean isSuccess, int retryAfter) {
        return mCurrentState.onSipResponseForRPMessage(this, isSuccess, retryAfter);
    }
}
