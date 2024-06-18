/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.imsservice.mmtel.sms;

import android.annotation.IntRange;
import android.os.RemoteException;
import android.telephony.SmsManager;
import android.telephony.SmsMessage;
import android.telephony.ims.aidl.IImsMmTelFeature;
import android.telephony.ims.aidl.IImsSmsListener;
import android.telephony.ims.stub.ImsSmsImplBase;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.its.util.SingleLatch;
import com.android.imsstack.util.Log;

import java.util.Objects;

/**
 * IMS SMS interface wrapper.
 */
public final class ImsSmsWrapper {
    private IImsMmTelFeature mIImsMmTelFeature;
    private @NonNull final IImsSmsListenerProxy mIImsSmsListenerProxy;
    private @NonNull final SingleLatch mWaitSmsResultLatch = new SingleLatch("WaitSmsResult");
    private @NonNull final SingleLatch mWaitSmsStatusResultLatch =
            new SingleLatch("WaitSmsStatusResult");
    private @NonNull final SingleLatch mWaitIncomingSmsLatch =
            new SingleLatch("WaitIncomingSms");
    private int mLastReceivedToken;
    private @Nullable byte[] mLastReceivedPdu;

    /** Constructor. */
    public ImsSmsWrapper(@NonNull IImsMmTelFeature mmtelFeature) {
        Objects.requireNonNull(mmtelFeature, "mmtelFeature must not be null.");

        logi("ImsSmsWrapper");
        mIImsMmTelFeature = mmtelFeature;
        mIImsSmsListenerProxy = new IImsSmsListenerProxy();
        try {
            mIImsMmTelFeature.setSmsListener(mIImsSmsListenerProxy);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /** Destroys. */
    public void destroy() {
        if (mIImsMmTelFeature != null) {
            try {
                mIImsMmTelFeature.setSmsListener(null);
            } catch (RemoteException e) {
                loge(e.toString());
            }
            mIImsMmTelFeature = null;
        }
    }

    /**
     * Sends an SMS.
     *
     * @param token unique token that should be used when triggering callbacks for this specific
     *              message.
     * @param messageRef the message reference, which may be 1 byte if it is in
     *                   {@link SmsMessage#FORMAT_3GPP} format (see TS.123.040) or 2 bytes if it is
     *                   in {@link SmsMessage#FORMAT_3GPP2} format (see 3GPP2 C.S0015-B).
     * @param format the format of the message.
     * @param smsc the Short Message Service Center address.
     * @param retry whether it is a retry of an already attempted message or not.
     * @param pdu PDU representing the contents of the message.
     */
    public void sendSms(int token, @IntRange(from = 0, to = 65535) int messageRef,
            @NonNull @SmsMessage.Format String format, String smsc, boolean retry,
            byte[] pdu) {
        Objects.requireNonNull(format, "format must not be null.");

        try {
            mIImsMmTelFeature.sendSms(token, messageRef, format, smsc, retry, pdu);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Sends a delivery report after {@link #onSmsReceived(int, String, byte[])} has been called to
     * acknowledge an incoming SMS.
     *
     *
     * @param token token provided in {@link #onSmsReceived(int, String, byte[])}
     * @param messageRef the message reference, which may be 1 byte if it is in
     *                   {@link SmsMessage#FORMAT_3GPP} format (see TS.123.040) or 2 bytes if it is
     *                   in {@link SmsMessage#FORMAT_3GPP2} format (see 3GPP2 C.S0015-B).
     * @param result result {@link ImsSmsImplBase#DeliverStatusResult} of delivering the message.
     * @param pdu PDU representing the contents of the message (see 3GPP TS 23.040).
     */
    public void acknowledgeSms(int token, @IntRange(from = 0, to = 65535)  int messageRef,
            int result, @Nullable byte[] pdu) {
        try {
            if (pdu == null) {
                mIImsMmTelFeature.acknowledgeSms(token, messageRef, result);
            } else {
                mIImsMmTelFeature.acknowledgeSmsWithPdu(token, messageRef, result, pdu);
            }
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * This can be be triggered after
     * {@link #onSmsStatusReportReceived(int, int, String, byte[])} or
     * {@link #onSmsStatusReportReceived(int, String, byte[])} has been called to provide the
     * result to the IMS provider.
     *
     * @param token token provided in {@link #onSmsStatusReportReceived(int, int, String, byte[])}
     *              or {@link #onSmsStatusReportReceived(int, String, byte[])}
     * @param messageRef the message reference, which may be 1 byte if it is in
     *                   {@link SmsMessage#FORMAT_3GPP} format (see TS.123.040) or 2 bytes if it is
     *                   in {@link SmsMessage#FORMAT_3GPP2} format (see 3GPP2 C.S0015-B).
     * @param result result {@link ImsSmsImplBase#StatusReportResult} of delivering the message.
     */
    public void acknowledgeSmsReport(int token, @IntRange(from = 0, to = 65535) int messageRef,
            int result) {
        try {
            mIImsMmTelFeature.acknowledgeSmsReport(token, messageRef, result);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /** Waits for an SMS Result. */
    public void waitForSmsResult(int timeInMillis) {
        mWaitSmsResultLatch.await(timeInMillis);
    }

    /** Waits for an SMS-Status-Report. */
    public void waitForStatusReport(int timeInMillis) {
        mWaitSmsStatusResultLatch.await(timeInMillis);
    }

    /** Waits for an incoming SMS. */
    public void waitForIncomingSms(int timeInMillis) {
        mWaitIncomingSmsLatch.await(timeInMillis);
    }

    /** Returns the last received token. */
    public int getLastReceivedToken() {
        return mLastReceivedToken;
    }

    /** Returns the last received PDU. */
    public @Nullable byte[] getLastReceivedPdu() {
        return mLastReceivedPdu;
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "ImsSmsWrapper: " + s);
    }

    private static void loge(String s) {
        Log.e(Log.TAG, "ImsSmsWrapper: " + s);
    }

    private class IImsSmsListenerProxy extends IImsSmsListener.Stub {
        /**
         * This method is triggerd to pass the result of the sent message.
         *
         * @param token token provided in
         *              {@link #sendSms(int, int, String, String, boolean, byte[])}.
         * @param messageRef the message reference, which may be 1 byte if it is in
         *                   {@link SmsMessage#FORMAT_3GPP} format (see TS.123.040) or 2 bytes if
         *                   it is in {@link SmsMessage#FORMAT_3GPP2} format (see 3GPP2 C.S0015-B).
         * @param status result {@link ImsSmsImplBase#SendStatusResult} of sending the SMS.
         * @param reason reason in case status is failure.
         * @param networkErrorCode the error code reported by the carrier network if sending this
         *                         SMS has resulted in an error or
         *                         {@link #RESULT_NO_NETWORK_ERROR} if no network error was
         *                         generated. See 3GPP TS 24.011 Section 7.3.4 for valid error
         *                         codes and more information.
         */
        @Override
        public void onSendSmsResult(int token, @IntRange(from = 0, to = 65535) int messageRef,
                int status, @SmsManager.Result int reason, int networkErrorCode) {
            logi("onSendSmsResult");
            mWaitSmsResultLatch.countDownAndInit();
        }

        /**
         * This method is triggered when the status report of the sent message is received.
         *
         * @param token unique token generated by IMS Stack that the test platform will use to
         *              trigger callbacks for this message.
         * @param format the format of the message.
         * @param pdu PDU representing the content of the status report.
         */
        @Override
        public void onSmsStatusReportReceived(int token, @SmsMessage.Format String format,
                byte[] pdu) {
            logi("onSmsStatusReportReceived");
            mLastReceivedToken = token;
            mLastReceivedPdu = pdu;
            mWaitSmsStatusResultLatch.countDownAndInit();
        }

        /**
         * This method is triggered when there is an incoming message.
         *
         * @param token unique token generated by IMS Stack that the test platform will use to
         *              trigger callbacks for this message.
         * @param format the format of the message.
         * @param pdu PDU representing the contents of the message.
         */
        @Override
        public void onSmsReceived(int token, @SmsMessage.Format String format, byte[] pdu) {
            logi("onSmsReceived");
            mLastReceivedToken = token;
            mLastReceivedPdu = pdu;
            mWaitIncomingSmsLatch.countDownAndInit();
        }

        /**
         * This API is used to report the result of sending RP-SMMA to test framework based on
         * received network responses(RP-ACK, RP-ERROR or SIP error).
         *
         * @param token provided in {@link ImsSmsImplBase#onMemoryAvailable()}.
         * @param result {@link ImsSmsImplBase#SendStatusResult} based on RP-ACK or RP_ERROR.
         * @param networkErrorCode the error code reported by the carrier network if sending this
         *                         SMS has resulted in an error or {@link #RESULT_NO_NETWORK_ERROR}
         *                         if no network error was generated. See 3GPP TS 24.011 Section
         *                         7.3.4 for valid error codes and more information.
         */
        @Override
        public void onMemoryAvailableResult(int token, int result, int networkErrorCode) {
            logi("onMemoryAvailableResult");
        }
    }
}
