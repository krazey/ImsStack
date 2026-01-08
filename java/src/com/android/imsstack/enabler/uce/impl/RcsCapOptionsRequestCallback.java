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

import android.telephony.ims.RcsContactUceCapability;
import android.telephony.ims.stub.CapabilityExchangeEventListener.OptionsRequestCallback;
import android.util.Log;

import com.android.imsstack.enabler.uce.interf.RemoteOptionsCallback;

import java.util.concurrent.Executor;

public class RcsCapOptionsRequestCallback implements OptionsRequestCallback {

    private static final String LOG_TAG = RcsCapOptionsRequestCallback.class.getSimpleName();
    private RemoteOptionsCallback mRemoteOptionsCallback = null;
    private final Executor mMessageExecutor;

    public RcsCapOptionsRequestCallback(Executor executor) {
        mMessageExecutor = executor;
    }

    /**
     * set callback of option request to send the response from
     * the network back to the framework.
     * @param remoteOptionsCallback The callback of option request
     */
    public void setCallback(RemoteOptionsCallback remoteOptionsCallback) {
        mRemoteOptionsCallback = remoteOptionsCallback;
    }

    /**
     * Respond to a remote capability request from the contact specified with the
     * capabilities of this device.
     * @param ownCapabilities The capabilities of this device.
     * @param isBlocked Whether or not the user has blocked the number requesting the
     *         capabilities of this device. If true, the device should respond to the OPTIONS
     *         request with a 200 OK response and no capabilities.
     */
    @Override
    public void onRespondToCapabilityRequest(RcsContactUceCapability ownCapabilities,
            boolean isBlocked) {
        if (mRemoteOptionsCallback == null) {
            Log.d(LOG_TAG,"mRemoteOptionsCallback is null");
            return;
        }
        postAndRunTask(()-> {
            mRemoteOptionsCallback.onRespondToCapabilityRequest(
                    ownCapabilities.getFeatureTags(), isBlocked);
            Log.d(LOG_TAG, "featureTag sent to UceOptionsResponseCallback");
        });
    }

    /**
     * Respond to a remote capability request from the contact specified with the
     * specified error.
     * @param code The SIP response code to respond with.
     * @param reason A non-null String containing the reason associated with the SIP code.
     */
    @Override
    public void onRespondToCapabilityRequestWithError(int code, String reason) {
        if (mRemoteOptionsCallback == null) {
            Log.d(LOG_TAG, "mRemoteOptionsCallback is null");
            return;
        }
        postAndRunTask(()->{
        mRemoteOptionsCallback.onRespondToCapabilityRequestWithError(code, reason);
        Log.d(LOG_TAG, "reason code is sent to UceOptionsResponseCallback code:" + code);
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
