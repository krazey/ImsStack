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

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

public class MtcEmergencyServiceManager {

    private static final int MSG_SEND_REQUEST = 101;
    private static final int DYNAMIC_ROUTING_NUMBER_CONFIG_COUNTRY_ISO_INDEX = 0;
    private static final int DYNAMIC_ROUTING_NUMBER_CONFIG_MNC_INDEX = 1;
    private static final int DYNAMIC_ROUTING_NUMBER_CONFIG_NUMBER_INDEX = 2;

    private final IBaseContext mContext;
    private MessageHandler mMessageHandler = null;
    private MtcCall mCall = null;
    private long mNativeObject = 0;
    private MtcJniProxy mMtcJniProxy;
    private ECallStateListener mECallStateListener = null;
    private final ICallStateTracker mCallStateTracker;
    private int mEmergencyType;
    private boolean mIsStopEmergencyServiceRequired;

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
     * @param callee The dialed string.
     */
    public void openEmergencyService(@EmergencyCallRouting int emergencyRouting,
            String callee, IServiceStateTracker serviceStateTracker) {
        emergencyRouting = maybeUpdateEmergencyRouting(
                emergencyRouting, callee, serviceStateTracker);
        mEmergencyType = emergencyRouting == EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL
                ? IUMtcCall.EMERGENCYTYPE_NORMAL_ROUTING
                : IUMtcCall.EMERGENCYTYPE_EMERGENCY_ROUTING;

        if (mCall.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false)
                && !CallFeature.isWiFiEmcOverEmergencyPdn(mContext.getSlotId())) {
            emergencyRouting = EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL;
        }

        log("openEmergencyService :: emergencyRouting=" + emergencyRouting);

        if (serviceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY)) {
            onEsOpened(emergencyRouting == EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL
                    ? IUMtcCall.SERVICETYPE_NORMAL : IUMtcCall.SERVICETYPE_EMERGENCY);
            return;
        }

        mCallStateTracker.addListener(mECallStateListener);
        mIsStopEmergencyServiceRequired = true;

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.OPEN_EMERGENCY_SERVICE);
        parcel.writeInt(convertEmergencyRoutingToServiceType(emergencyRouting));
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
        mIsStopEmergencyServiceRequired = false;
    }

    private void onEsOpening() {
        log("onEsOpening");
    }

    private void onEsOpened(int serviceType) {
        log("onEsOpened :: serviceType=" + serviceType);
        mIsStopEmergencyServiceRequired = false;

        if (mCall == null) {
            return;
        }

        mCall.createNativeCallObject();
        mCall.open(serviceType, mEmergencyType, false, false);
    }

    private void onEsUnavailable() {
        log("onEsUnavailable");
        mIsStopEmergencyServiceRequired = false;
    }

    private int convertEmergencyRoutingToServiceType(
            @EmergencyCallRouting int emergencyRoutingIn) {
        switch (emergencyRoutingIn) {
            case EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN:
                return IUMtcCall.SERVICETYPE_EMERGENCY;
            case EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL:
                return IUMtcCall.SERVICETYPE_NORMAL;
            default: // EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY
                return IUMtcCall.SERVICETYPE_EMERGENCY;
        }
    }

    private @EmergencyCallRouting int maybeUpdateEmergencyRouting(
            @EmergencyCallRouting int emergencyRouting, String callee,
            IServiceStateTracker serviceStateTracker) {
        if (emergencyRouting != EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN) {
            return emergencyRouting;
        }

        if (isFromNetworkOrSim(callee)) {
            return emergencyRouting;
        }

        if (!isDynamicRoutingNumber(callee)) {
            return emergencyRouting;
        }

        if (serviceStateTracker.isServiceRegistered(IUMtcService.SERVICE_VOIP)) {
            log("update to EMERGENCY_CALL_ROUTING_NORMAL");
            emergencyRouting = EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL;
        }

        return emergencyRouting;
    }

    private boolean isFromNetworkOrSim(String callee) {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mContext.getSlotId());
        if (telephony == null) {
            return false;
        }
        for (EmergencyNumber number : telephony.getEmergencyNumberList()) {
            if (number.getNumber().equals(callee)) {
                if (number.isFromSources(EmergencyNumber.EMERGENCY_NUMBER_SOURCE_NETWORK_SIGNALING)
                        || number.isFromSources(EmergencyNumber.EMERGENCY_NUMBER_SOURCE_SIM)) {
                    log("SIM or NETWORK emergency number");
                    return true;
                }
            }
        }

        return false;
    }

    private boolean isDynamicRoutingNumber(String callee) {
        String[] configs = AgentFactory.getInstance().getAgent(ConfigInterface.class,
                mContext.getSlotId()).getCarrierConfig().getStringArray(CarrierConfig.ImsEmergency
                        .KEY_DYNAMIC_ROUTING_NUMBER_PER_PLMN_STRING_ARRAY);
        if (configs == null) {
            return false;
        }

        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mContext.getSlotId());
        if (telephony == null) {
            return false;
        }
        String countryIso = telephony.getNetworkCountryIso();
        String mnc = telephony.getNetworkMnc();
        log("isDynamicRoutingNumber :: countryIso=" + countryIso + ", mnc=" + mnc);

        for (String config : configs) {
            String[] fields = config.split(",");
            // Format: "iso,mnc,number1,number2,..."
            if (fields == null || fields.length < 3) {
                continue;
            }
            if (!countryIso.equals(fields[DYNAMIC_ROUTING_NUMBER_CONFIG_COUNTRY_ISO_INDEX])) {
                continue;
            }
            if (!fields[DYNAMIC_ROUTING_NUMBER_CONFIG_MNC_INDEX].isEmpty() && mnc != null
                    && !mnc.equals(fields[DYNAMIC_ROUTING_NUMBER_CONFIG_MNC_INDEX])) {
                continue;
            }
            for (int i = DYNAMIC_ROUTING_NUMBER_CONFIG_NUMBER_INDEX; i < fields.length; i++) {
                if (callee.equals(fields[i])) {
                    return true;
                }
            }
        }

        return false;
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

            IServiceStateTracker sst = mContext.getServiceStateTracker();
            sst.handleEmergencyCallDestroyed();

            releaseCall();
            if (mIsStopEmergencyServiceRequired) {
                mIsStopEmergencyServiceRequired = false;
                stopEmergencyService();
            }
        }
    }
}
