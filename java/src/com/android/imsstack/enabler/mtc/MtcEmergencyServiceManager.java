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

import android.annotation.NonNull;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.telephony.TelephonyManager;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;
import android.text.TextUtils;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.PhoneConstants;

public class MtcEmergencyServiceManager {

    private static final int MSG_SEND_REQUEST = 101;

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
        IntentFilter filter = new IntentFilter(TelephonyManager.ACTION_NETWORK_COUNTRY_CHANGED);
        AppContext.getInstance().getBroadcastReceiverProxy().registerReceiver(
                mIntentReceiver, filter);
    }

    public void clear() {
        log("clear");

        mMessageHandler = null;
        mCall = null;
        mNativeObject = 0;
        mCallStateTracker.removeListener(mECallStateListener);
        AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(mIntentReceiver);
    }

    public void setNativeObject(long nativeObject) {
        mNativeObject = nativeObject;
    }

    public void setCall(MtcCall call) {
        log("setCall");

        mCall = call;
    }

    /**
     * Retrieves the cached network country ISO.
     *
     * This method returns the cached value of the network country ISO.
     * The ISO value is initially populated and updated whenever the
     * {@link android.content.Intent#ACTION_NETWORK_COUNTRY_CHANGED} intent is received.
     * This ensures that the returned value is the most recently known country ISO,
     * even if the device's radio is temporarily off (e.g., in airplane mode).
     * If there is no valid country ISO received via ACTION_NETWORK_COUNTRY_CHANGED,
     * it returns the most recent valid country ISO obtained from ImsPrivateProperties.
     * For devices with multiple phone IDs, this method prioritizes returning the last known valid
     * country ISO for its own phone ID.
     * If that is not valid, it then returns the most recently recognized country ISO from any
     * other phone ID.
     *
     * @return A non-null string representing the cached network country ISO.
     * Returns an empty string if no country ISO has been cached.
     */
    @NonNull
    public String getNetworkCountryIso() {
        String countryIsoFromProperties = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_NETWORK_COUNTRY_ISO, mContext.getPhoneId());
        String countryIsoFromTelephony = "";
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mContext.getSlotId());
        if (telephony != null) {
            countryIsoFromTelephony = telephony.getNetworkCountryIso();
        }

        if (!TextUtils.isEmpty(countryIsoFromTelephony)) {
            if (!countryIsoFromTelephony.equals(countryIsoFromProperties)) {
                updatePersistentProperty(ImsPrivateProperties.Persistent.KEY_NETWORK_COUNTRY_ISO,
                        countryIsoFromTelephony, mContext.getSlotId());
            }
            log("Country ISO from telephony : " + countryIsoFromTelephony);
            return countryIsoFromTelephony;
        }

        if (!TextUtils.isEmpty(countryIsoFromProperties)) {
            log("Country ISO from KEY_NETWORK_COUNTRY_ISO : " + countryIsoFromProperties);
            return countryIsoFromProperties;
        }

        countryIsoFromProperties = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_OVERALL_LAST_NETWORK_COUNTRY_ISO,
                MSimUtils.DEFAULT_PHONE_ID);

        if (!TextUtils.isEmpty(countryIsoFromProperties)) {
            log("Country ISO from KEY_OVERALL_LAST_NETWORK_COUNTRY_ISO : "
                    + countryIsoFromProperties);
            return countryIsoFromProperties;
        }

        log("No valid country ISO");
        return "";
    }

    /**
     * Notifies the Native to do registration for emergency call.
     *
     * @param emergencyRouting The routing information for emergency call.
     */
    public void openEmergencyService(
            @EmergencyCallRouting int emergencyRouting, IServiceStateTracker serviceStateTracker) {
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
                    ? IUMtcCall.SERVICETYPE_NORMAL : IUMtcCall.SERVICETYPE_EMERGENCY, true);
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
                onEsOpened(serviceType, false);
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

    private void onEsOpened(int serviceType, boolean usingAlreadyOpenedEmergencyService) {
        log("onEsOpened :: serviceType=" + serviceType + ", usingAlreadyOpenedEmergencyService="
                + usingAlreadyOpenedEmergencyService);
        mIsStopEmergencyServiceRequired = false;

        if (mCall == null) {
            return;
        }

        mCall.createNativeCallObject();
        mCall.open(serviceType, mEmergencyType, false, false, usingAlreadyOpenedEmergencyService);
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

    private static void updatePersistentProperty(String key, String value, int slotId) {
        ImsPrivateProperties.Persistent.set(key, value, slotId);
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
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

    @VisibleForTesting
    BroadcastReceiver getBroadcastReceiver() {
        return mIntentReceiver;
    }

    private BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyManager.ACTION_NETWORK_COUNTRY_CHANGED.equals(intent.getAction())) {
                String countryIso = intent.getStringExtra(
                        TelephonyManager.EXTRA_NETWORK_COUNTRY);
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY, -1);
                log("ACTION_NETWORK_COUNTRY_CHANGED [" + phoneId + "] : " + countryIso);

                if (TextUtils.isEmpty(countryIso)) {
                    return;
                }
                updatePersistentProperty(ImsPrivateProperties.Persistent.KEY_NETWORK_COUNTRY_ISO,
                        countryIso, phoneId);
                updatePersistentProperty(
                        ImsPrivateProperties.Persistent.KEY_OVERALL_LAST_NETWORK_COUNTRY_ISO,
                        countryIso, MSimUtils.DEFAULT_PHONE_ID);
            }
        }
    };
}
