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

package com.android.imsstack.enabler.mtc;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public class MtcEmergencyServiceManager {

    private static final int MSG_SEND_REQUEST = 101;

    private final IBaseContext mContext;
    private MessageHandler mMessageHandler = null;
    private MtcCall mCall = null;
    private long mNativeObject = 0;
    private MtcJniProxy mMtcJniProxy;
    private ECallStateListener mECallStateListener = null;
    private final ICallStateTracker mCallStateTracker;

    public MtcEmergencyServiceManager(IBaseContext context, ICallStateTracker callStateTracker) {
        mContext = context;
        mCallStateTracker = callStateTracker;
        mMtcJniProxy = MtcJniProxy.getInstance();
    }

    @VisibleForTesting
    public MtcEmergencyServiceManager(IBaseContext context,
            ICallStateTracker callStateTracker, MtcJniProxy mtcJniProxy) {
        mContext = context;
        mCallStateTracker = callStateTracker;
        mMtcJniProxy = mtcJniProxy;
    }

    public void init() {
        log("init");

        mMessageHandler = new MessageHandler(mContext.getCallLooper());
        mCall = null;
        mNativeObject = 0;
        mECallStateListener = new ECallStateListener();
    }

    public void clear() {
        log("clear");

        mMessageHandler = null;
        mCall = null;
        mNativeObject = 0;
        mCallStateTracker.removeListener(mECallStateListener);
    }

    public void setNativeObject(long nativeObject) {
        mNativeObject = nativeObject;
    }

    public void setCall(MtcCall call) {
        log("setCall");

        mCall = call;
    }

    /**
     * Notifies the Native to do registration for emergency call.
     *
     * @param emergencyRouting The routing information for emergency call.
     */
    public void openEmergencyService(
            @EmergencyCallRouting int emergencyRouting, IServiceStateTracker serviceStateTracker) {
        log("openEmergencyService");

        if (serviceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY)) {
            onEsOpened(IUMtcCall.SERVICETYPE_EMERGENCY);
            return;
        }

        mCallStateTracker.addListener(mECallStateListener);

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.OPEN_EMERGENCY_SERVICE);
        parcel.writeInt(convertEmergencyRouting(emergencyRouting));
        sendRequest(parcel);
    }

    private void stopEmergencyService() {
        log("stopEmergencyService");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.STOP_EMERGENCY_SERVICE);
        sendRequest(parcel);
    }

    public void onEmergencyServiceStateChanged(int state, int reason, int serviceType) {
        logi("onEmergencyServiceStateChanged :: state=" + state + ", reason=" + reason +
                ", serviceType=" + serviceType);

        switch (state) {
            case IUMtcService.ES_IDLE:
                onEsIdle();
                break;
            case IUMtcService.ES_OPENING:
                onEsOpening();
                break;
            case IUMtcService.ES_OPENED:
                onEsOpened(serviceType);
                break;
            case IUMtcService.ES_UNAVAILABLE:
                onEsUnavailable();
                break;
            default:
                break;
        }
    }

    private void onEsIdle() {
        log("onEsIdle");

        releaseCall();
    }

    private void onEsOpening() {
        log("onEsOpening");
    }

    private void onEsOpened(int serviceType) {
        log("onEsOpened :: serviceType=" + serviceType);

        if (mCall == null) {
            return;
        }

        mCall.createNativeCallObject();
        mCall.open(serviceType, true, false, false);
        releaseCall();
    }

    private void onEsUnavailable() {
        log("onEsUnavailable");
        releaseCall();
    }

    private int convertEmergencyRouting(@EmergencyCallRouting int emergencyRoutingIn) {
        switch (emergencyRoutingIn) {
            case EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN:
                return IUMtcService.EMERGENCY_CALL_ROUTING_UNKNOWN;
            case EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL:
                return IUMtcService.EMERGENCY_CALL_ROUTING_NORMAL;
            default: // EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY
                return IUMtcService.EMERGENCY_CALL_ROUTING_EMERGENCY;
        }
    }

    private void sendRequest(Parcel parcel) {
        Message.obtain(mMessageHandler, MSG_SEND_REQUEST, parcel).sendToTarget();
    }

    private long getNativeObject() {
        return mNativeObject;
    }

    private void releaseCall() {
        mCall = null;
        mCallStateTracker.removeListener(mECallStateListener);
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-MTC] " + s);
    }

    private class MessageHandler extends Handler {
        public MessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SEND_REQUEST: {
                    log("MtcEmergencyServiceManager :: sendRequest");

                    Parcel parcel = (Parcel)msg.obj;

                    mMtcJniProxy.sendDataToNative(getNativeObject(), parcel);
                    break;
                }
                default:
                    break;
            }
        }
    }

    @VisibleForTesting
    class ECallStateListener extends CallStateListener {
        @Override
        public void onCallDestroyed(Call call) {
            if (mCall != call) {
                return;
            }

            releaseCall();
            stopEmergencyService();
        }
    }
}
