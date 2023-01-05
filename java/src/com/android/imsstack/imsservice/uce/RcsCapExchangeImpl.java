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

import com.android.imsstack.enabler.uce.impl.RcsCapEventListenerCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapOptionsResponseCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapPublishResponseCallBack;
import com.android.imsstack.enabler.uce.impl.RcsCapSubscribeResponseCallBack;
import com.android.imsstack.enabler.uce.interf.IUceApi;
import com.android.imsstack.enabler.uce.interf.UceManager;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Collection;
import java.util.Set;

public class RcsCapExchangeImpl extends RcsCapabilityExchangeImplBase {
    private CapabilityExchangeEventListener mCapabilityExchangeEventListener;
    private int mSlotId = -1;
    private final MessageExecutor mCapExchangeExecutor =
        new MessageExecutor("RcsCapExchangeExecutor");
    private final MessageExecutor mCallBackExecutor =
            new MessageExecutor("CallBackExecutor");
    private IUceApi mUceApi = null;
    private Context mContext = null;
    private RcsCapPublishResponseCallBack mRcsCapPublishResponseCallBack;
    private RcsCapSubscribeResponseCallBack mRcsCapSubscribeResponseCallBack;
    private RcsCapOptionsResponseCallBack mRcsCapOptionsResponseCallBack;

    /**
     * Create a new RcsCapabilityExchangeImplBase instance.
     * @param listener used by the framework to listen to events from the vendor RCS stack
     * @param slotId  The slot ID associated with the RcsFeature.
     * @param context The context that is used in the ImsService.
     */
    public RcsCapExchangeImpl(CapabilityExchangeEventListener listener, int slotId,
        Context context) {
        mCapabilityExchangeEventListener = listener;
        mSlotId = slotId;
        mContext = context;
        mUceApi = UceManager.create(mContext, mSlotId);
        RcsCapEventListenerCallBack event = new RcsCapEventListenerCallBack(
                mCapabilityExchangeEventListener, mCallBackExecutor, mCapExchangeExecutor);
        if (mUceApi != null)
            mUceApi.setListener(event);
        mRcsCapPublishResponseCallBack = new RcsCapPublishResponseCallBack(mCallBackExecutor);
        mRcsCapSubscribeResponseCallBack = new RcsCapSubscribeResponseCallBack(mCallBackExecutor);
        mRcsCapOptionsResponseCallBack = new RcsCapOptionsResponseCallBack(mCallBackExecutor);
    }

    @VisibleForTesting
    public RcsCapExchangeImpl(CapabilityExchangeEventListener listener, int slotId,
            Context context, IUceApi uceApi,
            RcsCapSubscribeResponseCallBack capSubscribeResponseCallBack,
            RcsCapOptionsResponseCallBack optionsResponse,
            RcsCapPublishResponseCallBack rcsCapPublishResponseCallBack) {
        mCapabilityExchangeEventListener = listener;
        mSlotId = slotId;
        mContext = context;
        mUceApi = uceApi;
        mRcsCapPublishResponseCallBack = rcsCapPublishResponseCallBack;
        mRcsCapSubscribeResponseCallBack = capSubscribeResponseCallBack;
        mRcsCapOptionsResponseCallBack = optionsResponse;
    }

    /**
     * The user capabilities of one or multiple contacts have been requested by the framework.
     * <p>
     * The implementer must follow up this call with an
     * {@link SubscribeResponseCallback#onCommandError} call to indicate this operation has failed
     * The response from the network to the SUBSCRIBE request must be sent back to the framework
     * using {@link SubscribeResponseCallback#onNetworkResponse(int, String)}.
     * As NOTIFY requests come in from the network, the requested contact’s capabilities should be
     * sent back to the framework using
     * {@link SubscribeResponseCallback#onNotifyCapabilitiesUpdate{@code (List<String>)}} and
     * {@link SubscribeResponseCallback#onResourceTerminated{@code(List<Pair< Uri ,String>>)}}
     * should be called with the presence information for the contacts specified.
     * <p>
     * Once the subscription is terminated,
     * {@link SubscribeResponseCallback#onTerminated(String, long)} must be called for the
     * framework to finish listening for NOTIFY responses.
     *
     * @param uris A {@link Collection } of the {@link Uri}s that the framework is requesting the
     *             UCE capabilities for.
     * @param subscribeCallback   The callback of the subscribe request.
     */
    @Override
    public void subscribeForCapabilities(Collection<Uri> uris,
            SubscribeResponseCallback subscribeCallback) {
        if (mUceApi != null) {
            postAndRunTask(() -> {
                mRcsCapSubscribeResponseCallBack.setCallBack(subscribeCallback);
                mUceApi.subscribeCapabilities(uris, mRcsCapSubscribeResponseCallBack);
                ImsLog.d("subscribeForCapabilities request");
            });
        } else {
            ImsLog.i("mUceApi is null for slot " + mSlotId);
        }
    }

    /**
     * The capabilities of this device have been updated and should be published to the network.
     * <p>
     * If this operation succeeds, network response updates should be sent to the framework using
     * {@link PublishResponseCallback#onNetworkResponse(int, String)}.
     *
     * @param pidfXml The XML PIDF document containing the capabilities of this device to be sent
     *                to the carrier’s presence server.
     * @param publishCallback      The callback of the publish request
     */
    @Override
    public void publishCapabilities(String pidfXml, PublishResponseCallback publishCallback) {
        if(mUceApi != null) {
            postAndRunTask(()-> {
                mRcsCapPublishResponseCallBack.setCallBack(publishCallback);
                mUceApi.publishCapabilities(pidfXml, mRcsCapPublishResponseCallBack);
                ImsLog.d("publishCapabilities request with pidfxml");
            });
        } else {
            ImsLog.i("mUceApi is null for slot "+ mSlotId);
        }
    }

    /**
     * Push one's own capabilities to a remote user via the SIP OPTIONS presence exchange mechanism
     * in order to receive the capabilities of the remote user in response.
     * <p>
     * The implementer must use {@link OptionsResponseCallback} to send the response of
     * this query from the network back to the framework.
     *
     * @param contactUri     The URI of the remote user that we wish to get the capabilities of.
     * @param myCapabilities The capabilities of this device to send to the remote user.
     * @param optionsCallback       The callback of this request which is sent from the remote user.
     */
    @Override
    public void sendOptionsCapabilityRequest(Uri contactUri, Set<String> myCapabilities,
            OptionsResponseCallback optionsCallback) {
        if(mUceApi != null) {
            postAndRunTask(()-> {
                mRcsCapOptionsResponseCallBack.setCallBack(optionsCallback);
                mUceApi.sendOptionsCapabilityRequest(contactUri, myCapabilities,
                        mRcsCapOptionsResponseCallBack);
                ImsLog.d("sendOptionsCapabilityRequest request");
            });
        } else {
            ImsLog.i("mUceApi is null for slot "+ mSlotId);
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