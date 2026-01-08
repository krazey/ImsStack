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
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase.OptionsResponseCallback;
import android.util.Log;

import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;

import java.util.List;
import java.util.concurrent.Executor;

public class RcsCapOptionsResponseCallback implements OptionsResponse {

    private static final String LOG_TAG = RcsCapOptionsResponseCallback.class.getSimpleName();
    private OptionsResponseCallback mOptionsResponseCallBack;
    private final Executor mMessageExecutor;

    public RcsCapOptionsResponseCallback(Executor messageExecutor) {
        mMessageExecutor = messageExecutor;
    }

    /**
     * set the callback OptionsResponseCallback of options requests
     * to send the response from the network back to the framework.
     *
     * @param optionsCallback Tha callback of Options callback
     */
    public void setCallback(OptionsResponseCallback optionsCallback) {
        mOptionsResponseCallBack = optionsCallback;
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
        if (mOptionsResponseCallBack == null) {
            Log.d(LOG_TAG, "OptionsResponseCallBack object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mOptionsResponseCallBack.onCommandError(code);
                Log.d(LOG_TAG, "onCommandError OptionsResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onCommandError Exception:" + e.toString());
            }
        });
    }

    /**
     * Send the response of a SIP OPTIONS capability exchange to the framework.
     *
     * @param sipCode The SIP response code that was sent by the network in response
     * to the request sent by {@link IUceApi#sendOptionsCapabilityRequest}.
     * @param reason The optional SIP response reason sent by the network.
     * If none was sent, this should be an empty string.
     * @param theirCaps the contact's UCE capabilities associated with the
     * capability request.
     */
    @Override
    public void onNetworkResponse(int sipCode, String reason, List<String> theirCaps) {
        if (mOptionsResponseCallBack == null) {
            Log.d(LOG_TAG, "OptionsResponseCallBack object is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mOptionsResponseCallBack.onNetworkResponse(sipCode, reason, theirCaps);
                Log.d(LOG_TAG, "onNetworkResponse OptionsResponse sent to framework");
            } catch (ImsException e) {
                Log.e(LOG_TAG, "onNetworkResponse Exception:" + e.toString());
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
