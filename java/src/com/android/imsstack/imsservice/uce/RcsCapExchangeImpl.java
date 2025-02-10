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

package com.android.imsstack.imsservice.uce;

import android.content.Context;
import android.net.Uri;
import android.telephony.ims.stub.CapabilityExchangeEventListener;
import android.telephony.ims.stub.RcsCapabilityExchangeImplBase;

import com.android.imsstack.enabler.uce.impl.RcsCapEventListenerCallback;
import com.android.imsstack.enabler.uce.impl.RcsCapOptionsResponseCallback;
import com.android.imsstack.enabler.uce.impl.RcsCapPublishResponseCallback;
import com.android.imsstack.enabler.uce.impl.RcsCapSubscribeResponseCallback;
import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.UceManager;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Collection;
import java.util.Set;
import java.util.concurrent.Executor;

public class RcsCapExchangeImpl extends RcsCapabilityExchangeImplBase {
    private final CapabilityExchangeEventListener mCapabilityExchangeEventListener;
    private final int mSlotId;
    private final Executor mCapExchangeExecutor;
    private final IUceApi mUceApi;
    private final Context mContext;
    private final RcsCapPublishResponseCallback mRcsCapPublishResponseCallback;
    private final RcsCapSubscribeResponseCallback mRcsCapSubscribeResponseCallback;
    private final RcsCapOptionsResponseCallback mRcsCapOptionsResponseCallback;

    /**
     * Create a new RcsCapabilityExchangeImplBase instance.
     *
     * @param listener used by the framework to listen to events from the vendor RCS stack
     * @param slotId The slot ID associated with the RcsFeature.
     * @param context The context that is used in the ImsService.
     * @param executor The executor that will handle request from framework to stack.
     * @param messageExecutor The executor on which the callback methods will be invoked.
     */
    public RcsCapExchangeImpl(
            CapabilityExchangeEventListener listener,
            int slotId,
            Context context,
            Executor executor, Executor messageExecutor) {
        mCapabilityExchangeEventListener = listener;
        mSlotId = slotId;
        mContext = context;
        mCapExchangeExecutor = executor;
        mUceApi = UceManager.create(mContext, mSlotId);
        RcsCapEventListenerCallback event =
                new RcsCapEventListenerCallback(
                        mCapabilityExchangeEventListener, messageExecutor, mCapExchangeExecutor);

        if (mUceApi != null) {
            mUceApi.setListener(event);
        }
        mRcsCapPublishResponseCallback = new RcsCapPublishResponseCallback(messageExecutor);
        mRcsCapSubscribeResponseCallback = new RcsCapSubscribeResponseCallback(messageExecutor);
        mRcsCapOptionsResponseCallback = new RcsCapOptionsResponseCallback(messageExecutor);
    }

    @VisibleForTesting
    public RcsCapExchangeImpl(
            CapabilityExchangeEventListener listener,
            int slotId,
            Context context,
            IUceApi uceApi,
            RcsCapSubscribeResponseCallback capSubscribeResponseCallback,
            RcsCapOptionsResponseCallback optionsResponse,
            RcsCapPublishResponseCallback rcsCapPublishResponseCallback,
            Executor capExchangeExecutor,
            Executor callbackExecutor) {
        mCapabilityExchangeEventListener = listener;
        mSlotId = slotId;
        mContext = context;
        mUceApi = uceApi;
        mCapExchangeExecutor = capExchangeExecutor;
        RcsCapEventListenerCallback event =
                new RcsCapEventListenerCallback(
                        mCapabilityExchangeEventListener, callbackExecutor, capExchangeExecutor);
        if (mUceApi != null) {
            mUceApi.setListener(event);
        }
        mRcsCapPublishResponseCallback = rcsCapPublishResponseCallback;
        mRcsCapSubscribeResponseCallback = capSubscribeResponseCallback;
        mRcsCapOptionsResponseCallback = optionsResponse;
    }

    /**
     * The user capabilities of one or multiple contacts have been requested by the framework.
     *
     * <p>The implementer must follow up this call with an {@link
     * SubscribeResponseCallback#onCommandError} call to indicate this operation has failed The
     * response from the network to the SUBSCRIBE request must be sent back to the framework using
     * {@link SubscribeResponseCallback#onNetworkResponse(int, String)}. As NOTIFY requests come in
     * from the network, the requested contact’s capabilities should be sent back to the framework
     * using {@link SubscribeResponseCallback#onNotifyCapabilitiesUpdate(List<String>)} and
     * {@link SubscribeResponseCallback#onResourceTerminated(List<Pair<Uri, String>>)} should be
     * called with the presence information for the contacts specified.
     *
     * <p>Once the subscription is terminated, {@link SubscribeResponseCallback#onTerminated(String,
     * long)} must be called for the framework to finish listening for NOTIFY responses.
     *
     * @param uris A {@link Collection } of the {@link Uri}s that the framework is requesting the
     *     UCE capabilities for.
     * @param subscribeCallback The callback of the subscribe request.
     */
    @Override
    public void subscribeForCapabilities(
            Collection<Uri> uris, SubscribeResponseCallback subscribeCallback) {
        if (mUceApi != null) {
            postAndRunTask(
                    () -> {
                        mRcsCapSubscribeResponseCallback.setCallback(subscribeCallback);
                        mUceApi.subscribeCapabilities(uris, mRcsCapSubscribeResponseCallback);
                        ImsLog.d("subscribeForCapabilities request");
                    });
        } else {
            ImsLog.i("mUceApi is null for slot " + mSlotId);
        }
    }

    /**
     * The capabilities of this device have been updated and should be published to the network.
     *
     * <p>If this operation succeeds, network response updates should be sent to the framework using
     * {@link PublishResponseCallback#onNetworkResponse(int, String)}.
     *
     * @param pidfXml The XML PIDF document containing the capabilities of this device to be sent to
     *     the carrier’s presence server.
     * @param publishCallback The callback of the publish request
     */
    @Override
    public void publishCapabilities(String pidfXml, PublishResponseCallback publishCallback) {
        if (mUceApi != null) {
            postAndRunTask(
                    () -> {
                        mRcsCapPublishResponseCallback.setCallback(publishCallback);
                        mUceApi.publishCapabilities(pidfXml, mRcsCapPublishResponseCallback);
                        ImsLog.d("publishCapabilities request with pidfxml");
                    });
        } else {
            ImsLog.i("mUceApi is null for slot " + mSlotId);
        }
    }

    /**
     * Push one's own capabilities to a remote user via the SIP OPTIONS presence exchange mechanism
     * in order to receive the capabilities of the remote user in response.
     *
     * <p>The implementer must use {@link OptionsResponseCallback} to send the response of this
     * query from the network back to the framework.
     *
     * @param contactUri The URI of the remote user that we wish to get the capabilities of.
     * @param myCapabilities The capabilities of this device to send to the remote user.
     * @param optionsCallback The callback of this request which is sent from the remote user.
     */
    @Override
    public void sendOptionsCapabilityRequest(
            Uri contactUri, Set<String> myCapabilities, OptionsResponseCallback optionsCallback) {
        if (mUceApi != null) {
            postAndRunTask(
                    () -> {
                        mRcsCapOptionsResponseCallback.setCallback(optionsCallback);
                        mUceApi.sendOptionsCapabilityRequest(
                                contactUri, myCapabilities, mRcsCapOptionsResponseCallback);
                        ImsLog.d("sendOptionsCapabilityRequest request");
                    });
        } else {
            ImsLog.i("mUceApi is null for slot " + mSlotId);
        }
    }

    private void postAndRunTask(Runnable task) {
        if (mCapExchangeExecutor != null) {
            mCapExchangeExecutor.execute(task);
        } else {
            ImsLog.d("MessageExecutor object is null");
        }
    }
}
