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

import android.annotation.NonNull;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.SparseArray;

import com.android.imsstack.enabler.acs.AcService;
import com.android.imsstack.enabler.acs.AcServiceClientInfo;
import com.android.imsstack.enabler.acs.IAcServiceImplCallback;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

/**
 * Main module for AC client. Caller can access this by AcService interface.
 * AC client sends HTTP/HTTPs request service provider's server and handles response. When client
 * received provisioning data in HTTPS 200 OK, stores it into DataContainer and notifies by
 * IAcServiceImplCallback. This class will be created and worked base on SIM.
 */
public class AcServiceImpl {
    private static final String TERMINAL_VENDOR = "Google";
    private static final String TERMINAL_VERSION = "aaa";
    private static final String TERMINAL_NAME = "Google";

    private static final int MSG_MIN = 0;
    // mmtel feature created, run background operation
    // when boot completed and mmtel feature created, validation expired, ... etc
    // start AC progress, called by App
    private static final int MSG_START = 2;
    // stop AC progress, called by App
    private static final int MSG_STOP = 3;
    // received intent
    private static final int MSG_INTENT = 4;
    // received subscription changed event
    private static final int MSG_SUBSCRIPTION_CHANGED = 5;

    // received HTTP/HTTPS final response
    private static final int MSG_HTTP_RESPONSE = 6;

    private static final int MSG_MAX = 7;

    private static final SparseArray<AcServiceImpl> INSTANCES = new SparseArray<AcServiceImpl>();

    private interface MessageFunction {
        // return 0 means handled message complete
        int handleMessage(Message msg);
    }

    private final class EventCallback implements EventReceiver.EventReceiverCallback {
        @Override
        public void onReceivedIntent() {
            // TODO : check slot or sub ID
            // TODO : send message
            sendMessage(MSG_INTENT, 0, 0, null);
        }

        @Override
        public void onSubscriptionChanged() {
            sendMessage(MSG_SUBSCRIPTION_CHANGED, 0, 0, null);
        }
    }

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
                ImsLog.e("unknown message : " + what);
            }
        }
    }

    private final MessageFunction mMessageFunctionStart = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            mRequestInfo = createRequestInfo(mAcServiceClientInfo, mDataContainer);

            setState(AcService.STATE_TYPE_PROGRESS);
            return 0;
        }
    };

    private final MessageFunction mMessageFunctionStop = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {

/*
            if (getState() == AcService.STATE_TYPE_PROGRESS) {

            }
*/

            mHandler.removeCallbacksAndMessages(null);

            setState(AcService.STATE_TYPE_READY);
            return 0;
        }
    };

    private final MessageFunction mMessageFunctionIntent = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            return 0;
        }
    };

    private final MessageFunction mMessageFunctionSubscriptionChanged = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            return 0;
        }
    };

    private final MessageFunction mMessageFunctionHttpResponse = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            return 0;
        }
    };

    private final HashMap<Integer, MessageFunction> mMessageFunctionMap =
            new HashMap<Integer, MessageFunction>() {
            {
                put(MSG_START, mMessageFunctionStart);
                put(MSG_STOP, mMessageFunctionStop);
                put(MSG_INTENT, mMessageFunctionIntent);
                put(MSG_SUBSCRIPTION_CHANGED, mMessageFunctionSubscriptionChanged);
                put(MSG_HTTP_RESPONSE, mMessageFunctionHttpResponse);
            }
    };

    private final CallbackManager mCallbackManager;
    private final Context mContext;
    private final EventReceiver mEventReceiver;
    private final EventReceiver.EventReceiverCallback mCallback;
    private final Handler mHandler;
    private final ConfigContainer mConfigContainer;
    private final int mSlotId;

    private AcServiceClientInfo mAcServiceClientInfo;
    private RequestInfo mRequestInfo;
    private DataContainer mDataContainer;

    private int mSubId;
    private int mState;

    @VisibleForTesting
    public AcServiceImpl(int slotId, int subId, Context context, Looper looper,
            EventReceiver eventReceiver, ConfigContainer configContainer,
            DataContainer dataContainer) {
        mSlotId = slotId;
        mSubId = subId;
        mCallbackManager = new CallbackManager(slotId);

        mHandler = new MessageHandler(looper);
        mContext = context;

        mCallback = new EventCallback();
        mEventReceiver = eventReceiver;
        mEventReceiver.registerCallback(mCallback);

        mConfigContainer = configContainer;
        mDataContainer = dataContainer;

        mState = AcService.STATE_TYPE_NONE;
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

    private AcServiceImpl(Context context, int slotId) {
        mSlotId = slotId;
        mSubId = MSimUtils.getSubId(slotId);
        mCallbackManager = new CallbackManager(slotId);

        HandlerThread handlerThread = new HandlerThread(AcServiceImpl.class.getName());
        handlerThread.start();

        mHandler = new MessageHandler(handlerThread.getLooper());
        mContext = context;

        mCallback = new EventCallback();
        mEventReceiver = EventReceiver.getInstance(context);
        mEventReceiver.registerCallback(mCallback);

        mConfigContainer = ConfigContainer.getInstance(context);
        mDataContainer = new DataContainer(context, slotId, mSubId);

        mState = AcService.STATE_TYPE_NONE;
    }

    /**
     * Returns a AcService for phoneId specified and IllegalArgumentException will be
     * thrown if the phoneId is not valid.
     * @param context The context of the application.
     * @param slotId The ID of the Phone or SIM Slot that this AcService will use.
     * @return Instance of the AcServiceImpl
     */
    public static AcServiceImpl getInstance(Context context, int slotId) {
        AcServiceImpl acServiceImpl;
        synchronized (INSTANCES) {
            acServiceImpl = INSTANCES.get(slotId);
            if (acServiceImpl == null) {
                acServiceImpl = new AcServiceImpl(context, slotId);
                INSTANCES.put(slotId, acServiceImpl);
            }
        }

        return acServiceImpl;
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
        ImsLog.i("update clientInfo old : " + mAcServiceClientInfo.toString()
                + " new : " + clientInfo.toString());

        synchronized (mAcServiceClientInfo) {
            mAcServiceClientInfo = new AcServiceClientInfo(clientInfo.getRcsVersion(),
                    clientInfo.getRcsProfile(), clientInfo.getClientVendor(),
                    clientInfo.getClientVersion(), clientInfo.isRcsEnabledByUser());
        }
        mConfigContainer.updateClientInfo(mSubId, clientInfo);

        return true;
    }

    /**
     * Reconfiguration triggered by the caller
     * @return true if the AcService module can to request provisioning and the result will be
     * notified by callback, or false otherwise.
     */
    public boolean start() {
        // TODO : check clientInfo
        if (mAcServiceClientInfo == null) {
            ImsLog.i("clientInfo is not available");
            return false;
        }

        // TODO : send message to start
        return sendMessage(MSG_START, 0, 0, null);
    }

    /**
     * Stop the configuration process, it is only available when the provisioning process is
     * running. (before called the registered callback)
     */
    public void stop() {
        if (getState() == AcService.STATE_TYPE_NONE) {
            ImsLog.i("State is None");
            return;
        }

        if (mHandler == null) {
            ImsLog.i("handler is not ready");
            return;
        }

        Message msg = mHandler.obtainMessage(MSG_STOP);
        mHandler.sendMessageAtFrontOfQueue(msg);
    }

    /**
     * Notify the provisioning xml has been received
     * @param data Provisioning xml
     * @param isCompressed The xml file is compressed in gzip format
     */
    public void notifyProvisioningReceived(byte[] data, boolean isCompressed) {
        // mAcServiceImpl.notifyConfigDataReceived(data, isCompressed);
    }

    /**
     * The provisioning xml through notifyConfigDataReceived() is not available anymore.
     */
    public void notifyProvisioningRemoved() {
        // mAcServiceImpl.notifyConfigDataRemoved();
    }

    /**
     * Get the state of AcService module
     * @return The state is defined in AcService#AcServiceSateFlag
     */
    public int getState() {
        return mState;
    }

    /**
     * Start background operation and will be called when boot completed, mmtel feature created,
     * validation expired and so on.
     * @param reason Description of reason.
     */
    public void readyForService(int reason) {
        ImsLog.i("start background reason : " + reason);

        synchronized (mAcServiceClientInfo) {
            mAcServiceClientInfo = mConfigContainer.getClientInfo(mSubId);
        }

        if (mAcServiceClientInfo == null) {
            ImsLog.i("AcServiceClientInfo is invalid");
            return;
        }

        sendMessage(MSG_START, 0, 0, null);
    }

    private boolean sendMessage(int what, int arg1, int arg2, Object obj) {
        if (mHandler == null) {
            ImsLog.e("handler is not ready, msg : " + what + " could not be handled");
            return false;
        }
        Message msg = mHandler.obtainMessage(what, arg1, arg2, obj);
        mHandler.sendMessage(msg);

        return true;
    }

    private RequestInfo createRequestInfo(@NonNull AcServiceClientInfo acServiceClientInfo,
            DataContainer dataContainer) {
        String version = "0";
        if (dataContainer != null) {
            version = Integer.toString(dataContainer.getVersion());
        }
        RequestInfo requestInfo = new RequestInfo.RequestInfoBuilder(
                mSlotId, mSubId, acServiceClientInfo)
                .setAcVersion(version)
                .build();

        return requestInfo;
    }

    private void setState(int state) {
        ImsLog.d("old : " + mState + " new : " + state);
        mState = state;
    }
}
