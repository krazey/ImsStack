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

package com.android.imsstack.imsservice.mmtel;

import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.ims.stub.ImsSmsImplBase;

import androidx.annotation.NonNull;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.imsservice.mmtel.sms.SmsTransferLayer;
import com.android.imsstack.imsservice.mmtel.sms.SmsUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.internal.annotations.VisibleForTesting;

/**
 * Implements ImsSmsImplBase to provide Sms over Ims feature.
 */
public final class ImsSmsImpl extends ImsSmsImplBase {
    private static final String TAG = "[GII-ImsSmsImpl]";
    private final ImsCallContext mCallContext;
    private SmsTransferLayer mSmsTL = null;
    private String mScAddress = null;
    private final SmsTLListenerProxy mSmsTLListener = new SmsTLListenerProxy();
    private boolean mReady = false;
    private final Object mLock = new Object();

    public ImsSmsImpl(ImsCallContext callContext) {
        mCallContext = callContext;
        init();
    }

    @VisibleForTesting
    public ImsSmsImpl(ImsCallContext callContext, SmsTransferLayer smsTransferLayer,
                                String scAddress) {
        mCallContext = callContext;
        mScAddress = scAddress;
        mSmsTL = smsTransferLayer;

        if (mSmsTL != null) {
            mSmsTL.setListener(mSmsTLListener);
        }
    }

    /**
     * Disposes the Object
     */
    public void dispose() {
        clear();
    }

    /**
     * Initialises the objects created by this Class
     */
    public void init() {
        if (mSmsTL == null) {
            log("init");
            mSmsTL = new SmsTransferLayer(mCallContext);
            mSmsTL.setListener(mSmsTLListener);
        }
        onReady();
    }

    /**
     * clears the objects created by this class.
     */
    public void clear() {
        mReady = false;
        if (mSmsTL != null) {
            log("clear");
            mSmsTL.setListener(null);
            mSmsTL.clear();
            mSmsTL = null;
        }
    }

    @Override
    public void onMemoryAvailable(int token) {
        //send SMMA
        log("SMMA token = " + token);
        int result;
        if (!mReady) {
            throw new RuntimeException("Sms Not Ready!");
        }
        try {
            if (mScAddress == null) {
                SmsManagerProxy smp =
                        AppContext.getInstance().getSystemServiceProxy(SmsManagerProxy.class);
                mScAddress = smp.getSmscAddress();
            }
            log("SMMA smsc = " + mScAddress);
            result = mSmsTL.sendMemoryAvailabilityNotification(token, mScAddress);
        } catch (RuntimeException e) {
            loge("onMemoryAvailable :: Can not send smma: " + e.getMessage());
        }

    }

    @Override
    public void sendSms(
            int token, int messageRef, String format, String smsc, boolean isRetry, byte[] pdu) {
        log("sendSms: token = " + token + " format = " + format + " smsc = " + smsc
                                + " isRetry = " + isRetry);
        int smsFormat = SmsUtils.FORMAT_INT_INVALID;
        int result;
        if (!mReady) {
            throw new RuntimeException("Sms Not Ready!");
        }
        try {
            if (format.equals(SmsMessage.FORMAT_3GPP)) {
                smsFormat = SmsUtils.FORMAT_INT_3GPP;
            } else if (format.equals(SmsMessage.FORMAT_3GPP2)) {
                smsFormat = SmsUtils.FORMAT_INT_3GPP2;
            } else {
                loge("Invalid SMS Format");
                onSendSmsResultError(
                        token,
                        messageRef,
                        SEND_STATUS_ERROR,
                        SmsManager.RESULT_INVALID_SMS_FORMAT,
                        RESULT_NO_NETWORK_ERROR);
                return;
            }

            if (pdu == null) {
                loge("Pdu is null");
                onSendSmsResultError(
                        token,
                        messageRef,
                        SEND_STATUS_ERROR,
                        SmsManager.RESULT_ERROR_NULL_PDU,
                        RESULT_NO_NETWORK_ERROR);
                return;

            }

            if (smsc == null || smsc.length() == 0) {
                loge("Smsc is null");
                onSendSmsResultError(
                        token,
                        messageRef,
                        SEND_STATUS_ERROR,
                        SmsManager.RESULT_INVALID_SMSC_ADDRESS,
                        RESULT_NO_NETWORK_ERROR);
                return;
            }
            result = mSmsTL.sendMoTPdu(token, smsFormat, messageRef, smsc, pdu);
            if (result == SmsUtils.SMS_RESULT_INVALID_SMSC_ADDRESS) {
                loge("Can not send sms - Invalid smsc");
                onSendSmsResultError(
                            token,
                            messageRef,
                            SEND_STATUS_ERROR,
                            SmsManager.RESULT_INVALID_SMSC_ADDRESS,
                            RESULT_NO_NETWORK_ERROR);
            } else if (result == SmsUtils.SMSRL_RESULT_PDU_ENCODING_FAILED) {
                loge("Can not send sms - Encoding Failed");
                onSendSmsResultError(
                            token,
                            messageRef,
                            SEND_STATUS_ERROR,
                            SmsManager.RESULT_ENCODING_ERROR,
                            RESULT_NO_NETWORK_ERROR);
            } else if (result == SmsUtils.SMSRL_RESULT_MTS_CONTROLLER_FAILED) {
                //TODO: b/234531121 change the MtsController Failure codes
                loge("Can not send sms - Failure from MtsController");
                onSendSmsResultError(
                            token,
                            messageRef,
                            SEND_STATUS_ERROR,
                            SmsManager.RESULT_ERROR_GENERIC_FAILURE,
                            RESULT_NO_NETWORK_ERROR);
            }
        } catch (RuntimeException e) {
            loge("sendSms failed: " + e.getMessage());
        }
    }

    @Override
    public void acknowledgeSms(int token, int messageRef, int result) {
        acknowledgeSms(token, messageRef, result, null);
    }

    @Override
    public void acknowledgeSms(int token, int messageRef,
            int result, byte[] pdu) {
        log("acknowledgeSms: token = " + token + " messageRef = " + messageRef
                + " result = " + result);
        if (!mReady) {
            throw new RuntimeException("Sms Not Ready!");
        }
        try {
            int tlResult = mSmsTL.sendReportTPdu(token, messageRef, result, pdu);
            if (tlResult != SmsUtils.RESULT_SUCCESS) {
                loge("Sending Acknowledge Failed");
            }
        } catch (RuntimeException e) {
            loge("acknowledgeSmsWithPdu Failed: " + e.getMessage());
        }
    }

    @Override
    public void acknowledgeSmsReport(int token, int messageRef, int result) {
        log("acknowledgeSmsReport: token = " + token + " messageRef = " + messageRef
                + " result = " + result);
        if (!mReady) {
            throw new RuntimeException("Sms Not Ready!");
        }
        try {
            int tlResult = mSmsTL.sendReportTPdu(token, messageRef, result, null);
            if (tlResult != SmsUtils.RESULT_SUCCESS) {
                loge("Sending Acknowledge Failed");
            }
        } catch (RuntimeException e) {
            loge("acknowledgeSmsReport Failed: " + e.getMessage());
        }
    }

    @Override
    public void onReady() {
        synchronized (mLock) {
            mReady = true;
            log("Sms Ready");
        }
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        pw.println("Sms:");
        pw.increaseIndent();

        pw.decreaseIndent();
    }

    private static void log(String s) {
        ImsLog.d(TAG + s);
    }

    private static void loge(String s) {
        ImsLog.e(TAG + s);
    }

    private class SmsTLListenerProxy implements SmsTransferLayer.Listener {

        @Override
        public void notifySmsResult(int token, int messageRef, int result, int reason, int cause) {
            log("notifySmsResult: token = " + token + " TP message Reference = " + messageRef
                    + " result = " + result + " reason = " + " cause = " + cause);
            try {
                if (result == SEND_STATUS_OK) {
                    onSendSmsResultSuccess(token, messageRef);
                } else {
                    onSendSmsResultError(token, messageRef, result, reason, cause);
                }
            } catch (RuntimeException e) {
                loge("notifySmsResult Failed: " + e.getMessage());
            }
        }

        @Override
        public int notifySmsReceived(int token, int format, int messageType, byte[] pdu) {
            log("notifySmsReceived: format = " + format
                                        + " token = " + token
                                        + " messageType = " + messageType);
            try {
                int messageRef = pdu[2] & 0xff;
                if (messageType == SmsUtils.TP_SMS_DELIVER) {
                    onSmsReceived(token, SmsUtils.getFormatString(format), pdu);
                    return SmsUtils.RESULT_SUCCESS;
                } else if (messageType == SmsUtils.TP_SMS_STATUS_REPORT) {
                    onSmsStatusReportReceived(token, SmsUtils.getFormatString(format), pdu);
                    return SmsUtils.RESULT_SUCCESS;
                } else {
                    loge("Invalid Message Type");
                    return mSmsTL.sendReportTPdu(token, messageRef,
                                        ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED,
                                        null);
                }
            } catch (RuntimeException e) {
                loge("notifySmsReceived Failed : " + e.getMessage());
                return SmsUtils.RESULT_FAILURE;
            }
        }
    }
}
