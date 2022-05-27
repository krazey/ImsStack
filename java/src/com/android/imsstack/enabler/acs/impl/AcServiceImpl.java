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

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import com.android.imsstack.enabler.acs.AcService;
import com.android.imsstack.enabler.acs.AcServiceClientInfo;
import com.android.imsstack.enabler.acs.IAcServiceImplCallback;
import com.android.internal.annotations.VisibleForTesting;

/**
 * Main module for AC client. Caller can access this by AcService interface.
 * AC client sends HTTP/HTTPs request service provider's server and handles response. When client
 * received provisioning data in HTTPS 200 OK, stores it into DataContainer and notifies by
 * IAcServiceImplCallback. This class will be created and worked base on SIM.
 */
public class AcServiceImpl {
    private final class EventCallback implements EventReceiver.EventReceiverCallback {
        @Override
        public void onReceivedIntent() {

        }

        @Override
        public void onSubscriptionChanged() {

        }
    }

    private final class MessageHandler extends Handler {
        MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
        }
    }

    private final CallbackManager mCallbackManager;
    private final Context mContext;
    private final EventReceiver mEventReceiver;
    private final EventReceiver.EventReceiverCallback mCallback;
    private final Handler mHandler;

    private final int mSlotId;

    private int mStatus;

    @VisibleForTesting
    public AcServiceImpl(int slotId, Context context, Looper looper, EventReceiver eventReceiver) {
        mSlotId = slotId;
        mCallbackManager = new CallbackManager(slotId);

        mHandler = new MessageHandler(looper);
        mContext = context;

        mCallback = new EventCallback();
        mEventReceiver = eventReceiver;
        mEventReceiver.registerCallback(mCallback);
        mStatus = AcService.STATE_TYPE_NONE;
    }

    /**
     * Release all resources used internal
     */
    @VisibleForTesting
    public void destroy() {
        mHandler.getLooper().quit();
        mCallbackManager.clear();
        mEventReceiver.unregisterCallback(mCallback);
    }

    /**
     * Create AcServiceImpl instance
     * @param context The context of the application.
     * @param slotId SIM slot ID which will be used for trace.
     */
    public AcServiceImpl(Context context, int slotId) {
        mSlotId = slotId;
        mCallbackManager = new CallbackManager(slotId);

        HandlerThread handlerThread = new HandlerThread(AcServiceImpl.class.getName());
        handlerThread.start();

        mHandler = new MessageHandler(handlerThread.getLooper());
        mContext = context;

        mCallback = new EventCallback();
        mEventReceiver = EventReceiver.getInstance(context);
        mEventReceiver.registerCallback(mCallback);

        mStatus = AcService.STATE_TYPE_NONE;
    }

    /**
     * Register a new AcServiceCallback to listen to changes of provisioning
     * @param callback The callback to be registered.
     * @return true if the registration callback process is success, or false otherwise.
     */
    public boolean setCallback(IAcServiceImplCallback callback) {
        return mCallbackManager.registerCallback(callback);
    }

    /**
     * Unregister an existing callback. When the subscription associated with this callback is
     * removed, this callback will automatically be removed.
     * @param callback The callback to be removed.
     */
    public void removeCallback(IAcServiceImplCallback callback) {
        mCallbackManager.unregisterCallback(callback);
    }

    /**
     * Provide the client configuration parameters which to be included the RCS auto configuration
     * request.
     * @param clientInfo RCS client configuration
     * @return true if the operation is success, or false otherwise.
     */
    public boolean setClientInfo(AcServiceClientInfo clientInfo) {
        return true;
    }

    /**
     * Reconfiguration triggered by the caller
     * @return true if the AcService module can to request provisioning and the result will be
     * notified by callback, or false otherwise.
     */
    public boolean start() {
        // TODO : check clientInfo

        // TODO : send message to start
        return false;
    }

    /**
     * Stop the configuration process, it is only available when the provisioning process is
     * running. (before called the registered callback)
     */
    public void stop() {
        // TODO : check current status

        // TODO :send message to stop
    }
}
