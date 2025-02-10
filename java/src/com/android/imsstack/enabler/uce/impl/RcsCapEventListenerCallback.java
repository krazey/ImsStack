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

import android.net.Uri;
import android.telephony.ims.ImsException;
import android.telephony.ims.stub.CapabilityExchangeEventListener;

import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.RemoteOptionsCallback;
import com.android.imsstack.enabler.uce.interf.UceEventListener;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;
import java.util.concurrent.Executor;

public class RcsCapEventListenerCallback implements UceEventListener {
    private final CapabilityExchangeEventListener mEventListener;
    private final Executor mMessageExecutor;
    private final Executor mRequestExecutor;
    private final RcsCapOptionsRequestCallback mRcsOptionsRequestCallback;

    public RcsCapEventListenerCallback(CapabilityExchangeEventListener listener,
            Executor messageExecutor, Executor requestExecutor) {
        mEventListener = listener;
        mMessageExecutor = messageExecutor;
        mRequestExecutor = requestExecutor;
        mRcsOptionsRequestCallback =
                new RcsCapOptionsRequestCallback(mRequestExecutor);
    }

    @VisibleForTesting
    public RcsCapEventListenerCallback(CapabilityExchangeEventListener listener,
            Executor messageExecutor, Executor requestExecutor,
            RcsCapOptionsRequestCallback rcsOptionsRequestCallback) {
        mEventListener = listener;
        mMessageExecutor = messageExecutor;
        mRequestExecutor = requestExecutor;
        mRcsOptionsRequestCallback = rcsOptionsRequestCallback;
    }

    /**
     * Trigger the framework to provide a capability update using
     * {@link IUceApi#publishCapabilities}.
     *
     * CAPABILITY_UPDATE_TRIGGER_UNKNOWN = 0 (The reason for the request is unknown)
     * CAPABILITY_UPDATE_TRIGGER_ETAG_EXPIRED = 1 (When the Entity Tag (ETag) is  expiring)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED = 2
     * (requested due to moving to LTE with VoPS disabled)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED = 3
     * (requested due to moving to LTE with VoPS enabled)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD = 4 (requested due to moving to eHRPD)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_HSPAPLUS = 5 (requested due to moving to HSPA+)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_3G = 6 (requested due to moving to 3G)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_2G = 7 (requested due to moving to 2G)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_WLAN = 8 (requested due to moving to WLAN)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_IWLAN = 9 (requested due to moving to IWAL)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_DISABLED = 10
     * (due to moving to 5G NR with VoPS disabled)
     * CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_ENABLED = 11
     * (due to moving to 5G NR with VoPS enabled)
     *
     * This is typically used when trying to generate an initial PUBLISH for a new subscription to
     * the network. The device will cache all presence publications after boot until this method is
     * called the first time.
     *
     * @param publishTriggerType The reason for the capability update request.
     */
    public void onRequestPublishCapabilities(int publishTriggerType) {
        if (mEventListener == null) {
            Log.d(this, "EventListener is null");
            return;
        }
        postAndRunTask(() -> {
            try {
                mEventListener.onRequestPublishCapabilities(publishTriggerType);
                Log.d(this, "onRequestPublishCapabilities is sent to framework");
            } catch (ImsException e) {
                Log.e(this, "onRequestPublishCapabilities Exception = " + e.getMessage());
            }
        });
    }

    /**
     * Notify the framework that the ImsService has refreshed the PUBLISH
     * internally, which has resulted in a new PUBLISH result.
     * This method must return both SUCCESS (200 OK) and FAILURE (300+) codes in order to
     * keep the AOSP stack up to date.
     *
     * @param reasonCode : The SIP response code sent from the network
     * @param reasonPharse : The optional reason response from the network. If the
     * network provided no reason with the sip code, the string should be empty.
     * @param reasonHeaderCause : The “cause” parameter of the “reason” header
     * included in the SIP message.
     * @param reasonHeadertext : reasonHeaderText The “text” parameter of the “reason” header
     * included in the SIP message.
     */
    @Override
    public void onPublishUpdated(int reasonCode, String reasonPharse,
            int reasonHeaderCause, String reasonHeadertext) {
        Log.i(this, "onPublishUpdated ");
        if (mEventListener == null) {
            Log.d(this, "EventListener is null ");
            return;
        }
        postAndRunTask(() -> {
            try {
                mEventListener.onPublishUpdated(reasonCode, reasonPharse,
                        reasonHeaderCause, reasonHeadertext);
                Log.d(this, "onPublishUpdated sent to framework");
            } catch (ImsException e) {
                Log.e(this, "onPublishUpdated Exception = " + e.getMessage());
            }
        });
    }

    /**
     * Notify the framework that the device's capabilities have been unpublished
     * from the network.
     */
    @Override
    public void onUnPublish() {
        if (mEventListener == null) {
            Log.d(this, "EventListener is null ");
            return;
        }
        postAndRunTask(() -> {
            try {
                mEventListener.onUnpublish();
                Log.d(this, "onUnPublish  sent to framework");
            } catch (ImsException e) {
                Log.e(this, "onUnPublish Exception = " + e.getMessage());
            }
        });
    }

    /**
     * Notify the framework that the ImsService has received an Options request from the Remote.
     * {@link IUceApi#sendOptionsCapabilityRequest}.
     *
     * @param contactUri The URI associated with the remote contact that is
     * requesting capabilities.
     * @param remoteCapabilities The remote contact's capability information. The capability
     * information is in the format defined in RCC.07 section 2.6.1.3.
     * @param remoteOptionsCallback The callback of Options request which is received from
     * Remote user.
     */
    @Override
    public void onRemoteCapabilityRequest(Uri contactUri, Set<String> remoteCapabilities,
            RemoteOptionsCallback remoteOptionsCallback) {
        if (mEventListener == null) {
            Log.d(this, "EventListener is null");
            return;
        }
        mRcsOptionsRequestCallback.setCallback(remoteOptionsCallback);
        postAndRunTask(() -> {
            try {
                mEventListener.onRemoteCapabilityRequest(
                        contactUri, remoteCapabilities, mRcsOptionsRequestCallback);
                Log.d(this, "onRemoteCapabilityRequest is sent to framework");
            } catch (ImsException e) {
                Log.e(this, "onRemoteCapabilityRequest Exception = " + e.getMessage());
            }
        });
    }

    private void postAndRunTask(Runnable task) {
        if (mMessageExecutor != null) {
            mMessageExecutor.execute(task);
        } else {
            Log.d(this, "MessageExecutor object is null");
        }
    }
}
