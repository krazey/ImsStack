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
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

/**
 * Main module for AC client. Caller can access this by AcService interface.
 * AC client sends HTTP/HTTPs request service provider's server and handles response. When client
 * received provisioning data in HTTPS 200 OK, stores it into ProvisioningData and notifies by
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
    // receive provisioning data from App
    private static final int MSG_PROVISIONING_DATA_RECEIVED = 4;
    // reset provisioning data from App
    private static final int MSG_PROVISIONING_DATA_RESET = 5;

    // received HTTP/HTTPS final response
    private static final int MSG_HTTP_RESPONSE = 6;

    // received HTTP/HTTPS internal error response
    private static final int MSG_HTTP_INTERNAL_ERROR = 7;

    private static final int MSG_MAX = 8;

    private static final SparseArray<AcServiceImpl> INSTANCES = new SparseArray<AcServiceImpl>();

    private interface MessageFunction {
        // return 0 means handled message complete
        int handleMessage(Message msg);
    }

    private class TxzResponse {
        private final int mResponseCode;
        private final String mResponseString;
        private final byte[] mProvisioningData;

        TxzResponse(int responseCode, String responseString, byte[] provisioningData) {
            mResponseCode = responseCode;
            mResponseString = responseString;
            mProvisioningData = provisioningData.clone();
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
                ImsLog.e(mSlotId, "unknown message : " + what);
            }
        }
    }

    private final MessageFunction mMsgFuncStart = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            mRequestInfo = createRequestInfo(mAcServiceClientInfo, mConfigContainer);

            // TODO : HTTP request

            setState(AcService.STATE_TYPE_PROGRESS);

            return 0;
        }
    };

    private final MessageFunction mMsgFuncStop = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            if (getState() == AcService.STATE_TYPE_PROGRESS) {
                // TODO : de-init if we need, for example de-register http callback

                // clear message Q
                mHandler.removeCallbacksAndMessages(null);
                setState(AcService.STATE_TYPE_READY);
                return 0;
            }

            ImsLog.i(mSubId, "already stopped " + getState());
            return 0;
        }
    };

    private final MessageFunction mMsgFuncProvisioningDataReceived = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            if (msg == null || msg.obj == null) {
                ImsLog.i(mSlotId, "parameter is null");
                return 0;
            }

            // check compressing
            byte[] data = (byte[]) msg.obj;
            if (msg.arg1 == 1) {
                data = mProvisioningData.decompressGzip((byte[]) msg.obj);
            }

            // save provisioning data to file
            mProvisioningData.createXmlFileFromBytes(data);

            // reset ConfigContainer
            resetConfig();

            // stop timer for re-config
            stopTimer();

            return 0;
        }
    };

    private final MessageFunction mMsgFuncProvisioningDataReset = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            // delete xml file includes provisioning data
            mProvisioningData.deleteXmlFile();

            // reset ConfigContainer
            resetConfig();

            // stop timer for re-config
            stopTimer();

            return 0;
        }
    };

    private final MessageFunction mMsgFuncHttpResponse = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            if (msg.obj == null) {
                ImsLog.i(mSlotId, "message object is null");
                return 0;
            }

            TxzResponse txzResponse = (TxzResponse) msg.obj;
            if (txzResponse.mResponseCode >= 200 && txzResponse.mResponseCode < 300) {
                handleHttpSuccessResponse(txzResponse);
            } else {
                handleHttpFailResponse(txzResponse);
            }

            return 0;
        }
    };

    private final MessageFunction mMsgFuncHttpInternalError = new MessageFunction() {
        @Override
        public int handleMessage(Message msg) {
            return 0;
        }
    };

    private final HashMap<Integer, MessageFunction> mMessageFunctionMap =
            new HashMap<Integer, MessageFunction>() {
            {
                put(MSG_START, mMsgFuncStart);
                put(MSG_STOP, mMsgFuncStop);
                put(MSG_PROVISIONING_DATA_RECEIVED, mMsgFuncProvisioningDataReceived);
                put(MSG_PROVISIONING_DATA_RESET, mMsgFuncProvisioningDataReset);
                put(MSG_HTTP_RESPONSE, mMsgFuncHttpResponse);
                put(MSG_HTTP_INTERNAL_ERROR, mMsgFuncHttpInternalError);
            }
    };

    private final CallbackManager mCallbackManager;
    private final Context mContext;
    private final Handler mHandler;
    private final ConfigContainer mConfigContainer;
    private final int mSlotId;
    private final int mSubId;

    private final Object mObject = new Object();
    private ProvisioningData mProvisioningData;
    private AcServiceClientInfo mAcServiceClientInfo;
    private RequestInfo mRequestInfo;

    private int mState;

    @VisibleForTesting
    public AcServiceImpl(int slotId, int subId, Context context, Looper looper,
            ProvisioningData provisioningData,
            CallbackManager callbackManager,
            ConfigContainer configContainer) {
        mSlotId = slotId;
        mSubId = subId;
        mContext = context;

        mHandler = new MessageHandler(looper);
        mProvisioningData = provisioningData;
        mCallbackManager = callbackManager;
        mConfigContainer = configContainer;

        mState = AcService.STATE_TYPE_NONE;
    }

    /**
     * Release all resources used internal
     */
    @VisibleForTesting
    public void destroy() {
        mHandler.getLooper().quit();
        mCallbackManager.clear();
    }

    private AcServiceImpl(Context context, int slotId, int subId) {
        mSlotId = slotId;
        mSubId = subId;
        mContext = context;

        HandlerThread handlerThread = new HandlerThread(AcServiceImpl.class.getName());
        handlerThread.start();
        mHandler = new MessageHandler(handlerThread.getLooper());

        mProvisioningData = new ProvisioningData(mContext, mSubId);
        mCallbackManager = new CallbackManager(mSlotId, mSubId);
        mConfigContainer = new ConfigContainer(context, mSlotId, mSubId);

        mState = AcService.STATE_TYPE_NONE;
    }

    /**
     * Returns a AcService for phoneId specified.
     * @param context The context of the application.
     * @param slotId The ID of the Phone or SIM Slot that this AcService will use.
     * @return Instance of the AcServiceImpl
     */
    public static AcServiceImpl getInstance(Context context, int slotId) {
        AcServiceImpl acServiceImpl;
        int subId = MSimUtils.getSubId(slotId);

        synchronized (INSTANCES) {
            acServiceImpl = INSTANCES.get(slotId);
            if (acServiceImpl == null) {
                acServiceImpl = new AcServiceImpl(context, slotId, subId);
                INSTANCES.put(slotId, acServiceImpl);
            }
        }

        return acServiceImpl;
    }

    /**
     * Returns a AcService for phoneId specified and this is only for Testing.
     * @param context The context of the application.
     * @param slotId The ID of the Phone or SIM Slot that this AcService will use.
     * @param subId The ID of the Subscription associated with slot ID.
     * @return Instance of the AcServiceImpl
     */
    @VisibleForTesting
    public static AcServiceImpl getInstance(Context context, int slotId, int subId) {
        AcServiceImpl acServiceImpl;
        synchronized (INSTANCES) {
            acServiceImpl = INSTANCES.get(slotId);
            if (acServiceImpl == null) {
                acServiceImpl = new AcServiceImpl(context, slotId, subId);
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
        if (!clientInfo.isValid()) {
            ImsLog.i(mSlotId, "parameter is not valid");
            return false;
        }

        ImsLog.i(mSlotId, "update clientInfo : " + clientInfo.toString());

        synchronized (mObject) {
            mAcServiceClientInfo = new AcServiceClientInfo(clientInfo);
        }

        mConfigContainer.updateClientInfo(clientInfo);

        return true;
    }

    /**
     * Reconfiguration triggered by the caller
     * @param reason Description of reason.
     * @return true if the AcService module can to request provisioning and the result will be
     * notified by callback, or false otherwise.
     */
    public boolean start(int reason) {
        ImsLog.i(mSlotId, "start reason : " + reason);

        // check clientInfo
        synchronized (mObject) {
            if (mAcServiceClientInfo == null) {
                mAcServiceClientInfo = mConfigContainer.getClientInfo();
                if (mAcServiceClientInfo == null) {
                    ImsLog.i(mSlotId, "AcServiceClientInfo is not available");
                    return false;
                }
            }
        }

        // send message to start
        return sendMessage(MSG_START, 0, 0, null);
    }

    /**
     * Stop the configuration process, it is only available when the provisioning process is
     * running. (before called the registered callback)
     */
    public void stop() {
        if (getState() == AcService.STATE_TYPE_NONE) {
            ImsLog.i(mSlotId, "State is None");
            return;
        }

        if (mHandler == null) {
            ImsLog.i(mSlotId, "handler is not ready");
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
        int arg1 = isCompressed ? 1 : 0;
        sendMessage(MSG_PROVISIONING_DATA_RECEIVED, arg1, 0, (Object) data.clone());
    }

    /**
     * The provisioning xml through notifyConfigDataReceived() is not available anymore.
     */
    public void notifyProvisioningRemoved() {
        sendMessage(MSG_PROVISIONING_DATA_RESET, 0, 0, null);
    }

    /**
     * Get the state of AcService module
     * @return The state is defined in AcService#AcServiceSateFlag
     */
    public int getState() {
        return mState;
    }

    private boolean sendMessage(int what, int arg1, int arg2, Object obj) {
        if (mHandler == null) {
            ImsLog.e(mSlotId, "handler is not ready, msg : " + what + " could not be handled");
            return false;
        }
        Message msg = mHandler.obtainMessage(what, arg1, arg2, obj);
        mHandler.sendMessage(msg);

        return true;
    }

    private RequestInfo createRequestInfo(@NonNull AcServiceClientInfo acServiceClientInfo,
            ConfigContainer configContainer) {
        String version = "0";
        if (configContainer != null) {
            version = Integer.toString(configContainer.getAcVersion(0));
        }
        RequestInfo requestInfo = new RequestInfo.RequestInfoBuilder(
                mSlotId, mSubId, acServiceClientInfo)
                .setAcVersion(version)
                .build();

        return requestInfo;
    }

    private void setState(int state) {
        ImsLog.d(mSlotId, "old : " + mState + " new : " + state);
        mState = state;
    }

    private void resetConfig() {
        if (mConfigContainer == null) {
            ImsLog.i(mSlotId, "Config object is null");
            return;
        }

        mConfigContainer.resetAcValue();
        mConfigContainer.resetClientInfo();
    }

    private void stopTimer() {
        // TODO : call ReconfigAgent
    }

    private void handleHttpSuccessResponse(TxzResponse txzResponse) {
        if (txzResponse.mProvisioningData == null) {
            ImsLog.i(mSlotId, "provisioning data is null");
        }
        // TODO : check pre-provisioning?
        // TODO : partial provisioning
        // TODO : full provisioning

    }

    private void handleHttpFailResponse(TxzResponse txzResponse) {

    }
}
