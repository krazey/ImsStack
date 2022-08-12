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

package com.android.imsstack.enabler.acs.impl;

import android.annotation.IntDef;
import android.annotation.NonNull;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.android.imsstack.util.ImsLog;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.HashMap;

/**
 * This class provides to send an HTTP GET request to the server and receives an HTTP response.
 * To receive provisioning data from the AC server by service provider, the client should set
 * required headers or set to send all transactions to a specific network before sending HTTP
 * request.
 * HTTP response include error will be passed through Callback.
 */
public class HttpTransaction {
    /** @hide */
    @IntDef(prefix = {"RESULT_TYPE_"}, value = {
            RESULT_TYPE_INTERNAL_ERROR,
            RESULT_TYPE_HTTP_UNREACHABLE,
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ErrorResult {}

    // Internal error occurred during ACS request.
    public static final int RESULT_TYPE_INTERNAL_ERROR = 1;
    // Unreachable error occurred during ACS request.
    public static final int RESULT_TYPE_HTTP_UNREACHABLE = 2;

    /** @hide */
    @IntDef(prefix = {"REQUEST_"}, value = {
            REQUEST_HTTPS,
            REQUEST_NON_CELLULAR,
            REQUEST_GBA,
            REQUEST_DONE,
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface RequestType {}

    // minimum msg
    public static final int MSG_MIN = 0;
    // start msg
    public static final int MSG_START = 1;
    // stop msg
    public static final int MSG_STOP = 2;
    // send HTTPS request
    public static final int REQUEST_HTTPS = 3;
    // send non-cellular HTTP request using OTP ACS Auth
    public static final int REQUEST_NON_CELLULAR = 4;
    // send GBA request.
    public static final int REQUEST_GBA = 5;
    // ACS is done.
    public static final int REQUEST_DONE = 6;
    // maximum msg
    public static final int MSG_MAX = 7;

    /**
     * Callback interface for receiving result from the AC HTTP request.
     */
    public interface Callback {

        /**
         * Receive HTTP response to AC HTTP Request.
         *
         * @param responseCode the status code in HTTP response
         * @param responseString the reason phrase in HTTP response
         * @param xmlData provisioning data or null
         */
        void receivedHttpResponse(int responseCode, String responseString, byte[] xmlData);

        /**
         * Receive non HTTP response to AC HTTP Request.
         *
         * @param result the result is {@link ErrorResult} for meaning of values.
         */
        void receivedNonHttpResponse(@ErrorResult int result);
    }

    private interface MessageFunction {
        // return 0 means handled message complete
        int handleMessage(Message msg);
    }

    // Thread
    private final class MessageHandler extends Handler {
        MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            if (MSG_MIN < what && what < MSG_MAX) {
                mMessageFunctionMap.get(what).handleMessage(msg);
            } else {
                ImsLog.e(mSlotId, "unknown message : " + what);
            }
        }
    }

    private final HashMap<Integer, MessageFunction> mMessageFunctionMap =
            new HashMap<Integer, MessageFunction>() {
                {
                    put(MSG_START, mMsgFuncStart);
                    put(MSG_STOP, mMsgFuncStop);
                    put(REQUEST_DONE, mMsgFuncRequestDone);
                }
            };

    private final MessageFunction mMsgFuncStart = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {

            return 0;
        }
    };

    private final MessageFunction mMsgFuncStop = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {

            return 0;
        }
    };

    private final MessageFunction mMsgFuncRequestDone = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            Callback cb = mCallback;

            // TODO : need to update
            int responseCode = msg.arg1;
            byte[] acData = msg.obj != null ? (byte[]) msg.obj : null;
            // retryAfter value also include callback param.
            int retryAfter = msg.arg2;
            if (100 <= responseCode && responseCode <= 600) {
                // received http response
                cb.receivedHttpResponse(responseCode, null, acData);
            } else {
                // internal error or unreachable error
                cb.receivedNonHttpResponse(responseCode);
            }

            return 0;
        }
    };

    private final Handler mHandler;
    private final int mSlotId;
    private final int mSubId;
    private final Context mContext;

    private Callback mCallback = null;

    public HttpTransaction(Context context, int slotId, int subId) {
        mContext = context;
        mSlotId = slotId;
        mSubId = subId;
        HandlerThread handlerThread = new HandlerThread(HttpTransaction.class.getName());
        handlerThread.start();

        mHandler = new MessageHandler(handlerThread.getLooper());
    }

    /**
     * Set the {@link Callback} that can be used to listen to result of ACS request
     *
     * @param callback The callback to receive result.
     */
    public void setCallback(@NonNull Callback callback) {
        mCallback = callback;
    }

    /**
     * Remove the {@link Callback} that can be used to listen to result of ACS request
     */
    public void removeCallback() {
        mCallback = null;
    }

    /**
     * Open a transaction to receive provisioning data from the service provider.
     */
    public void open() {

    }

    /**
     * Close a transaction, since the client no longer needs to send.
     */
    public void close() {

    }
}
