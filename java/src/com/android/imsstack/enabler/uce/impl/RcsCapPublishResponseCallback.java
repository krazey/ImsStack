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

package com.android.imsstack.enabler.uce.impl;

import android.telephony.ims.ImsException;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase.PublishResponseCallback;
import android.util.Log;

import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.PublishResponse;

import java.util.concurrent.Executor;

public class RcsCapPublishResponseCallback implements PublishResponse {

    private static final String LOG_TAG = RcsCapPublishResponseCallback.class.getSimpleName();
    private PublishResponseCallback mPublishResponseCallBack;
    private final Executor mMessageExecutor;

    public RcsCapPublishResponseCallback(Executor messageExecutor) {
        mMessageExecutor = messageExecutor;
    }

    /**
     * set callback PublishResponseCallback of publish request
     * to send the response from the network back to the framework.
     *
     * @param publishCallback The callback of publish request
     */
    public void setCallback(PublishResponseCallback publishCallback) {
        mPublishResponseCallBack = publishCallback;
    }

    /**
     * Provide the framework with a subsequent network response update to
     * {@link IUceApi#publishCapabilities(String, PublishResponse)}.
     *
     * If this network response also contains a “Reason” header, then the
     * {@link #onNetworkResponse(int, String, int, String)} method should be used instead.
     *
     * @param sipCode The SIP response code sent from the network for the operation
     * token specified.
     * @param reason The optional reason response from the network. If there is a reason header
     * included in the response, that should take precedence over the reason provided in the
     * status line. If the network provided no reason with the sip code, the string should be empty.
     */
    @Override
    public void onNetworkResponse(int sipCode, String reason) {
        if (mPublishResponseCallBack == null) {
            Log.d(LOG_TAG, "PublishResponseCallBack object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mPublishResponseCallBack.onNetworkResponse(sipCode, reason);
                Log.d(LOG_TAG, "onNetworkResponse PublishResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onNetworkResponse Exception:" + e.toString());
            }
        });
    }

    /**
     * Provide the framework with a subsequent network response update to
     * {@link IUceApi#publishCapabilities(String, PublishResponse)} that also
     * includes a reason provided in the “reason” header. See RFC3326 for more
     * information.
     *
     * @param sipCode The SIP response code sent from the network.
     * @param reasonPhrase The optional reason response from the network. If the
     * network provided no reason with the sip code, the string should be empty.
     * @param reasonHeaderCause The “cause” parameter of the “reason” header
     * included in the SIP message.
     * @param reasonHeaderText The “text” parameter of the “reason” header
     * included in the SIP message.
     */
    @Override
    public void onNetworkResponse(int sipCode, String reasonPhrase, int reasonHeaderCause,
            String reasonHeaderText) {
        if (mPublishResponseCallBack == null) {
            Log.d(LOG_TAG, "PublishResponseCallBack object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mPublishResponseCallBack.onNetworkResponse(sipCode, reasonPhrase, reasonHeaderCause,
                        reasonHeaderText);
                Log.d(LOG_TAG, "onNetworkResponse PublishResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onNetworkResponse Exception:" + e.toString());
            }
        });
    }

    /**
     * Notify the framework that the command associated with this callback has failed.
     *
     * @param code The reason why the associated command has failed.
     * COMMAND_CODE_SERVICE_UNKNOWN = 0 (Service is unknown)
     * COMMAND_CODE_GENERIC_FAILURE = 1 (The command failed with an unknown error)
     * COMMAND_CODE_INVALID_PARAM = 2 (Invalid parameter(s))
     * COMMAND_CODE_FETCH_ERROR = 3 (Fetch error)
     * COMMAND_CODE_REQUEST_TIMEOUT = 4 (Request timed out)
     * COMMAND_CODE_INSUFFICIENT_MEMORY = 5 (Failure due to insufficient memory available)
     * COMMAND_CODE_LOST_NETWORK_CONNECTION = 6 (Network connection is lost)
     * COMMAND_CODE_NOT_SUPPORTED = 7 (Requested feature/resource is not supported)
     * COMMAND_CODE_NOT_FOUND = 8 (Contact or resource is not found)
     * COMMAND_CODE_SERVICE_UNAVAILABLE = 9 (Service is not available)
     * COMMAND_CODE_NO_CHANGE = 10 (Command resulted in no change in state, ignoring)
     */
    @Override
    public void onCommandError(int code) {
        if (mPublishResponseCallBack == null) {
            Log.d(LOG_TAG, "PublishResponseCallBack object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mPublishResponseCallBack.onCommandError(code);
                Log.d(LOG_TAG, "onCommandError PublishResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onCommandError Exception:" + e.toString());
            }
        });
    }

    private void postAndRunTask(Runnable task) {
        if (mMessageExecutor != null) {
            mMessageExecutor.execute(task);
        } else {
            Log.d(LOG_TAG, "MessageExecutor object is null");
        }
    }
}
