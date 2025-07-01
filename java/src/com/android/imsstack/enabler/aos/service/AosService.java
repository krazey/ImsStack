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
package com.android.imsstack.enabler.aos.service;

import android.annotation.IntRange;
import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;
import android.os.PersistableBundle;
import android.telephony.DataFailCause;
import android.telephony.PreciseCallState;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.util.ArraySet;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;
import com.android.imsstack.core.agents.LocationInterface;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.core.agents.WifiInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDcApn;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfoListener;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationType;
import com.android.imsstack.enabler.aos.IIAosService;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.JniObjectId;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class provides an interface to manage and control the AoS related information.
 */
public class AosService implements IAosRegistration, IAosInfo, Sim.Listener, Sim.IsimListener,
         IDcNetWatcher.Listener, IApn.Listener, ImsPhoneStateListener {

    /** public constants */
    public static final int PCO_TARGET_ID = 0xff00;
    public static final int PCO_NONE_VALUE = 0;

    /** private members */
    private long mNativeObject = 0;
    private int mSlotId = MSimUtils.DEFAULT_SLOT_ID;
    private Handler mHandler;
    private RegistrationState mRegState = RegistrationState.DEREGISTERED;
    private NativeStateListener mNativeStateListener;
    private IPhoneStateNotifier mNotifier;

    /** package-private members */
    NetworkType mRegisteredNetworkType = NetworkType.NONE;
    int mFeatureTagBits = 0;
    int mPreciseCallState = PreciseCallState.PRECISE_CALL_STATE_IDLE;
    boolean mIsConnectedOverCrossSim = false;
    CapabilityPairs mCapabilityPairs;

    /** Final collections */
    final Set<IAosRegistrationListener> mAosRegistrationListeners = new CopyOnWriteArraySet<>();
    final Set<IAosInfoListener> mAosInfoListeners = new CopyOnWriteArraySet<>();
    final Set<String> mFeatureTags = new CopyOnWriteArraySet<>();

    /** Final listeners */
    final ImsServiceRegistryListener mServiceRegistryListener = new ImsServiceRegistryListener();
    final JniImsListenerProxy mJniImsListenerProxy = new JniImsListenerProxy();
    final LocationInterface.Listener mLocationListener = new LocationInterface.Listener() {
        @Override
        public void onLastKnownCountryUpdated() {
            sendRequest(IIAosService.J2N_NOTIFY_LOCATION_INFO,
                    LocationInfo.COUNTRY_CHANGED.getValue());
        }

        @Override
        public void onInstantRequestedLocationUpdated() {
            sendRequest(IIAosService.J2N_NOTIFY_LOCATION_INFO,
                    LocationInfo.FIXED.getValue());
        }
    };
    final WifiInterface.Listener mWifiListener = new WifiInterface.Listener() {
        @Override
        public void onWifiStateChanged() {
            WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
            sendRequest(IIAosService.J2N_NOTIFY_WIFI_SETTING,
                    (wifi != null) ? wifi.isWifiEnabled() : false);
        }
    };

    public void init(int slotId) {
        mSlotId = slotId;

        mHandler = new Handler(Looper.myLooper());

        mNativeObject = JniImsProxy.getInterface(JniObjectId.AOS, mSlotId);
        if (mNativeObject != 0) {
            JniImsProxy.setListener(mNativeObject, mJniImsListenerProxy);
        }

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        if (nsi != null) {
            mNativeStateListener = new NativeStateListener();
            nsi.addListener(mNativeStateListener);
        }

        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        isr.addListener(mServiceRegistryListener);
    }

    public void cleanup() {
        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        isr.removeListener(mServiceRegistryListener);

        if (mNativeStateListener != null) {
            NativeStateInterface nsi =
                    AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }

        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }

        if (mNativeObject == 0) {
            return;
        }

        JniImsProxy.removeListener(mNativeObject);
        JniImsProxy.releaseInterface(mNativeObject);
        mNativeObject = 0;
        mSlotId = MSimUtils.DEFAULT_SLOT_ID;
    }

    /**
     * Start any necessary information for AoS service.
     */
    public void start() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(this);
            sim.addIsimListener(this);
        }

        PhoneStateInterface phoneState =
                AgentFactory.getInstance().getAgent(PhoneStateInterface.class, mSlotId);
        if (phoneState != null) {
            mNotifier = phoneState.createNotifier(this, mHandler.getLooper());
            mNotifier.setEvents(LISTEN_PRECISE_CALL_STATE);

            phoneState.addNotifier(mNotifier);
        }

        LocationInterface location = AgentFactory.getInstance().getAgent(
                LocationInterface.class, mSlotId);
        if (location != null) {
            location.addListener(mLocationListener);
        }

        IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
        if (dnw != null) {
            dnw.addListener(this);
        }

        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        IApn apn = (dcApn != null) ? dcApn.getApnControl(EApnType.IMS.getType()) : null;
        if (apn != null) {
            apn.addListener(this);
        }

        WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
        if (wifi != null) {
            wifi.addListener(mWifiListener);
        }
    }

    /**
     * Stop the necessary information that was previously initialized for AoS service.
     */
    public void stop() {
        WifiInterface wifi = AgentFactory.getInstance().getAgent(WifiInterface.class);
        if (wifi != null) {
            wifi.removeListener(mWifiListener);
        }

        IDcApn dcApn = DcFactory.getDcAgent(IDcApn.class, mSlotId);
        IApn apn = (dcApn != null) ? dcApn.getApnControl(EApnType.IMS.getType()) : null;
        if (apn != null) {
            apn.removeListener(this);
        }

        IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
        if (dnw != null) {
            dnw.removeListener(this);
        }

        LocationInterface location = AgentFactory.getInstance().getAgent(
                LocationInterface.class, mSlotId);
        if (location != null) {
            location.removeListener(mLocationListener);
        }

        if (mNotifier != null) {
            PhoneStateInterface phoneState =
                    AgentFactory.getInstance().getAgent(PhoneStateInterface.class, mSlotId);
            if (phoneState != null) {
                phoneState.removeNotifier(mNotifier);
            }
            mNotifier.setListener(null);
            mNotifier = null;
        }

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.removeListener(this);
            sim.removeIsimListener(this);
        }
    }

    @Override
    public void addListener(@NonNull IAosRegistrationListener listener) {
        Objects.requireNonNull(listener, "listener must not be null");
        mAosRegistrationListeners.add(listener);
    }

    @Override
    public void removeListener(@NonNull IAosRegistrationListener listener) {
        Objects.requireNonNull(listener, "listener must not be null");
        mAosRegistrationListeners.remove(listener);
    }

    @Override
    public void addListener(@NonNull IAosInfoListener listener) {
        Objects.requireNonNull(listener, "listener must not be null");
        mAosInfoListeners.add(listener);
    }

    @Override
    public void removeListener(@NonNull IAosInfoListener listener) {
        Objects.requireNonNull(listener, "listener must not be null");
        mAosInfoListeners.remove(listener);
    }

    @Override
    public void updateSipDelegateRegistration() {
        sendRequest(IIAosService.J2N_REQUEST_REGISTRATION);
    }

    @Override
    public void triggerSipDelegateDeregistration() {
        sendRequest(IIAosService.J2N_REQUEST_DEREGISTRATION);
    }

    @Override
    public void triggerFullNetworkRegistration(@IntRange(from = 100, to = 699) int sipCode,
            @Nullable String sipReason) {
        if (sipCode < 100 || sipCode > 699) {
            throw new IllegalArgumentException("Invalid sipCode. Must be between 100 and 699.");
        }

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_REQUEST_FULL_REGISTRATION);
        parcel.writeInt(sipCode);
        parcel.writeString((sipReason != null) ? sipReason : "");

        sendRequest(parcel);
    }

    @Override
    public void changeCapabilities(@NonNull CapabilityPairs pairs) {
        Objects.requireNonNull(pairs, "listener must not be null");

        adjustCapabilities(pairs);

        if (mCapabilityPairs != null && mCapabilityPairs.equals(pairs)) {
            return;
        }

        mCapabilityPairs = pairs;

        notifyCapabilitiesChanged();
    }

    @Override
    public void controlRegistration(RequestType requestType, Pcscf pcscfOrder, Cause cause) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION);
        parcel.writeInt(requestType.getValue());
        parcel.writeInt(pcscfOrder.getValue());
        parcel.writeInt(cause.getValue());

        sendRequest(parcel);
    }

    @Override
    public NetworkType getRegisteredNetworkType() {
        return mRegisteredNetworkType;
    }

    @Override
    public RegistrationState getRegistrationState() {
        return mRegState;
    }

    @Override
    public void notifyDataRoamingSetting(boolean isAllowed) {
        sendRequest(IIAosService.J2N_NOTIFY_DATA_ROAMING_SETTING, isAllowed);
    }

    @Override
    public void notifyMobileDataSetting(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_MOBILE_DATA_SETTING, isOn);
    }

    @Override
    public void notifyRoamingPreferredVoiceNetwork(RoamingPreferredVoiceNetwork state) {
        sendRequest(IIAosService.J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK, state.getValue());
    }

    @Override
    public void notifyServiceSetting(ServiceSetting state, int serviceBits) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_SERVICE_SETTING);
        parcel.writeInt(state.getValue());
        parcel.writeInt(serviceBits);
        sendRequest(parcel);
    }

    @Override
    public void notifyTtySetting(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_TTY_SETTING, isOn);
    }

    @Override
    public void notifyVideoSetting(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_VIDEO_SETTING, isOn);
    }

    @Override
    public void notifyVolteSetting(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_VOLTE_SETTING, isOn);
    }

    @Override
    public void notifyWfcSetting(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_WFC_SETTING, isOn);
    }

    @Override
    public void notifyAosStart() {
        sendRequest(IIAosService.J2N_NOTIFY_AOS_START);
    }

    @Override
    public void notifyIsimState(@Sim.IsimState int state) {
        sendRequest(IIAosService.J2N_NOTIFY_ISIM_STATE, state);
    }

    @Override
    public void notifyMobileDataLimit(boolean isLimited) {
        sendRequest(IIAosService.J2N_NOTIFY_MOBILE_DATA_LIMIT, isLimited);
    }

    @Override
    public void notifyNetworkVideoCapability(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY, isOn);
    }

    @Override
    public void notifyPhoneNumberState(boolean isRefresh, PhoneNumberState state) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE);
        parcel.writeInt(isRefresh ? 1 : 0);
        parcel.writeInt(state.getValue());
        sendRequest(parcel);
    }

    @Override
    public void notifyPowerOff() {
        sendRequest(IIAosService.J2N_NOTIFY_POWER_OFF);
    }

    @Override
    public void notifyCarrierSignalPcoValueChanged(@NonNull Intent intent) {
        Objects.requireNonNull(intent, "intent must not be null");
        mHandler.post(() -> handleCarrierSignalPcoValueChanged(intent));
    }

    @Override
    public void notifyEmergencyCallbackModeChanged(
            EmergencyCallbackModeType type, EmergencyCallbackModeState state, long duration) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_EMERGENCY_CALLBACK_MODE_CHANGED);
        parcel.writeInt(type.getValue());
        parcel.writeInt(state.getValue());
        parcel.writeLong(duration);
        sendRequest(parcel);
    }

    @Override
    public void notifyNasSecurityAlgorithmChanged(boolean isNullAlgo) {
        ImsLog.d(mSlotId, "AosService: notifyNasSecurityAlgorithmChanged - null=" + isNullAlgo);
        sendRequest(IIAosService.J2N_NOTIFY_NAS_ALGORITHM_CHANGED, isNullAlgo);
    }

    @Override
    public void notifyAllowedNetworkTypesChanged(long networkTypesBitMask) {
        ImsLog.d(mSlotId, "AosService: notifyAllowedNetworkTypesChanged - network types="
                + networkTypesBitMask);
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_ALLOWED_NETWORK_TYPES_CHANGED);
        parcel.writeLong(networkTypesBitMask);
        sendRequest(parcel);
    }

    @Override
    public void onSimStateChanged() {
        ImsLog.d(mSlotId, "AosService: onSimStateChanged");
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim != null) {
            if (sim.isSimLoaded()) {
                notifyPhoneNumberState(false, PhoneNumberState.SIM_LOADED);
            }
        }
    }

    @Override
    public void onIsimStateChanged() {
        ImsLog.d(mSlotId, "AosService: onIsimStateChanged");
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim != null) {
            notifyIsimState(sim.getIsimState());
        } else {
            ImsLog.e(mSlotId, "SimInterface is null for slot: " + mSlotId);
        }
    }

    @Override
    public void onPreciseCallStateChanged(PreciseCallState callState) {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);
        int csCallState = (telephony != null)
                ? telephony.getCsCallState() : TelephonyManager.CALL_STATE_IDLE;

        if (csCallState == TelephonyManager.CALL_STATE_IDLE) {
            if (mPreciseCallState != PreciseCallState.PRECISE_CALL_STATE_IDLE) {
                mPreciseCallState = PreciseCallState.PRECISE_CALL_STATE_IDLE;
                sendRequest(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE, mPreciseCallState);
            }
            return;
        }

        // For CS Call, both Foreground and Background Call state check is required
        int foregroundCallState = callState.getForegroundCallState();
        int backgroundCallState = callState.getBackgroundCallState();

        ImsLog.i(mSlotId, "ForegroundCallState = " + foregroundCallState
                + ", BackgroundCallState = " + backgroundCallState);

        if ((backgroundCallState == PreciseCallState.PRECISE_CALL_STATE_HOLDING)
                || (backgroundCallState == PreciseCallState.PRECISE_CALL_STATE_ACTIVE)) {
            mPreciseCallState = backgroundCallState;
            sendRequest(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE, mPreciseCallState);
        } else {
            // If Background Call is not present, post ForegroundCallState to AoSCallTracker
            if (mPreciseCallState != foregroundCallState) {
                mPreciseCallState = foregroundCallState;
                sendRequest(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE, mPreciseCallState);
            }
        }
    }

    @Override
    public void onNetworkOperatorChanged(String networkOperator) {
        ImsLog.d(mSlotId, "AosService: onNetworkOperatorChanged");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.J2N_NOTIFY_PLMN_CHANGED);
        parcel.writeString(networkOperator);
        sendRequest(parcel);
    }

    @Override
    public void onAirplaneModeChanged(boolean airplaneMode) {
        ImsLog.d(mSlotId, "AosService: onAirplaneModeChanged");
        sendRequest(IIAosService.J2N_NOTIFY_AIRPLANE_SETTING, airplaneMode);
    }

    @Override
    public void onVopsStateChanged(int state, String plmn) {
        ImsLog.d(mSlotId, "AosService: onVopsStateChanged");

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IIAosService.J2N_NOTIFY_VOPS_STATE_CHANGED);
        parcel.writeInt(state);
        parcel.writeString(plmn);
        sendRequest(parcel);
    }

    @Override
    public void onHandoverStateChanged(int handoverState, int networkType, int failCause) {
        ImsLog.d(mSlotId, "AosService: onHandoverStateChanged");
        if (handoverState == IApn.HANDOVER_FAILURE) {
            if (failCause == DataFailCause.IWLAN_IKEV2_AUTH_FAILURE
                    && isIkeAuthFailureNotificationRequired()) {
                mHandler.post(() -> onTechnologyChangeFailed(RegistrationType.NORMAL,
                        getRegistrationNetworkType(networkType),
                        ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE));
            }
            int targetNetwork = (networkType == TelephonyManager.NETWORK_TYPE_IWLAN)
                    ? IApn.IPCAN_CATEGORY_MOBILE : IApn.IPCAN_CATEGORY_WLAN;

            Parcel parcel = Parcel.obtain();
            parcel.writeInt(IIAosService.J2N_NOTIFY_IPCAN_HANDOVER_FAILURE);
            parcel.writeInt(targetNetwork);
            parcel.writeInt(failCause);
            sendRequest(parcel);
        }
    }

    @Override
    public void onCrossSimStatusChanged(boolean connectedOverCrossSim) {
        ImsLog.d(mSlotId, "AosService: onCrossSimStatusChanged");
        if (mIsConnectedOverCrossSim == connectedOverCrossSim) {
            return;
        }

        mIsConnectedOverCrossSim = connectedOverCrossSim;
        if ((mRegisteredNetworkType == NetworkType.CROSS_SIM) && !mIsConnectedOverCrossSim) {
            updateRegisteredNetworkType(NetworkType.IWLAN);
        } else if ((mRegisteredNetworkType == NetworkType.IWLAN) && mIsConnectedOverCrossSim) {
            updateRegisteredNetworkType(NetworkType.CROSS_SIM);
        }
    }

    @Override
    public void onPreciseDataConnectionStateChanged(int apnType, int state, int failCause,
            int networkType) {
        if (apnType != EApnType.IMS.getType()) {
            return;
        }

        ImsLog.d(mSlotId, "AosService: onPreciseDataConnectionStateChanged");
        switch (state) {
            case TelephonyManager.DATA_CONNECTING -> {
                controlRegistration(RequestType.START_IMS_EST_TIMER,
                        Pcscf.CURRENT, Cause.DATA_CONNECTING);
            }
            case TelephonyManager.DATA_DISCONNECTING -> {
                if (getRegisteredNetworkType() != NetworkType.NONE) {
                    updateDataFailureReason(failCause);
                    controlRegistration(RequestType.STOP,
                            Pcscf.CURRENT, Cause.DATA);
                }
            }
            case TelephonyManager.DATA_DISCONNECTED -> {
                if (failCause == DataFailCause.IWLAN_IKEV2_AUTH_FAILURE
                        && isIkeAuthFailureNotificationRequired()) {
                    mHandler.post(() -> onDeregistered(
                            getRegistrationNetworkType(networkType),
                            ReasonCode.DATA_EPDG_TUNNEL_IKEV2_AUTH_FAILURE));
                }
                updateDataFailureReason(failCause);
            }
            default -> { }
        }
    }

    private boolean isImsEnabled() {
        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        return isr.isImsEnabled();
    }

    private void sendRequest(int message, boolean isTrue) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(message);
        parcel.writeInt(isTrue ? 1 : 0);
        sendRequest(parcel);
    }

    private void sendRequest(int message, int state) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(message);
        parcel.writeInt(state);
        sendRequest(parcel);
    }

    private void sendRequest(int message) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(message);
        sendRequest(parcel);
    }

    private void sendRequest(Parcel parcel) {
        if (mNativeObject == 0) {
            parcel.recycle();
            parcel = null;
            return;
        }

        byte[] data = parcel.marshall();
        parcel.recycle();
        parcel = null;

        JniImsProxy.sendData(mNativeObject, data);
    }

    private void notifyCapabilitiesChanged() {
        if (mCapabilityPairs == null) {
            return;
        }

        ImsLog.d(mSlotId, mCapabilityPairs.toString());

        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_REQUEST_CAPABILITIES_CHANGED);
        parcel.writeInt(mCapabilityPairs.getCapabilities().size());

        for (Map.Entry<NetworkType, Integer> entry
                    : mCapabilityPairs.getCapabilities().entrySet()) {
            parcel.writeInt(entry.getKey().getValue());
            parcel.writeInt(entry.getValue());
        }

        sendRequest(parcel);

        notifyCapabilitiesUpdated(mCapabilityPairs);
    }

    private void updateDataFailureReason(int reason) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_UPDATE_DATA_FAILURE_REASON);
        parcel.writeInt(reason);

        sendRequest(parcel);
    }

    private void handleCarrierSignalPcoValueChanged(Intent intent) {
        String apn = ApnSetting.getApnTypeString(
                intent.getIntExtra(TelephonyManager.EXTRA_APN_TYPE, -1));
        if (!ApnSetting.TYPE_IMS_STRING.equals(apn)) {
            return;
        }

        if (PCO_TARGET_ID != intent.getIntExtra(TelephonyManager.EXTRA_PCO_ID, 0)) {
            return;
        }

        sendRequest(IIAosService.J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED,
                getPcoValue(intent.getByteArrayExtra(TelephonyManager.EXTRA_PCO_VALUE)));
    }

    private int getPcoValue(byte[] values) {
        int value = PCO_NONE_VALUE;
        if (values == null) {
            ImsLog.d("PCO Values are null");
            return value;
        }
        ImsLog.d("PCO values : " + Arrays.toString(values));
        switch (values.length) {
            case 1 -> value = values[0];
            case 4 -> value = values[3];
            default -> ImsLog.i("Invalid PCO values length : " + values.length);
        }
        ImsLog.d("Returns PCO value : " + value);
        return value;
    }

    private void notifyImsServiceChanged() {
        controlRegistration((isImsEnabled()) ? IAosRegistration.RequestType.START :
                IAosRegistration.RequestType.STOP,
                IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
    }

    private void notifyCrossSimStatus(NetworkType networkType) {
        sendRequest(IIAosService.J2N_NOTIFY_CROSS_SIM_STATUS,
                (networkType == NetworkType.CROSS_SIM
                ? CrossSimStatus.DATA_CONNECTED : CrossSimStatus.DATA_DISCONNECTED).getValue());
    }

    private void adjustCapabilities(CapabilityPairs pairs) {
        /*
         * Note : If LTE/NR-VIDEO capability is enabled, add IWLAN-VIDEO capability.
         */
        if (pairs.hasCapability(IAosRegistrationListener.NetworkType.LTE,
                IAosRegistrationListener.Capability.VIDEO) ||
                pairs.hasCapability(IAosRegistrationListener.NetworkType.NR,
                IAosRegistrationListener.Capability.VIDEO)) {
            ImsLog.d("Add Capability - IWLAN-VIDEO");
            pairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                    IAosRegistrationListener.Capability.VIDEO);
        }
    }

    private void updateRegisteredNetworkType(NetworkType networkType) {
        // This function is for considering CROSS_SIM.
        // Hence, it is used only for RegistrationType.NORMAL type.

        ImsLog.d(mSlotId, "updateRegisteredNetworkType :: networkType(" + networkType + ")");
        mRegisteredNetworkType = networkType;
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyRegistered(RegistrationType.NORMAL, mRegisteredNetworkType, mFeatureTagBits,
                    mFeatureTags);
        }

        notifyCrossSimStatus(networkType);
    }

    private NetworkType adjustedNetworkType(
            NetworkType networkType, int featureTagBits, Set<String> featureTags) {
        mFeatureTagBits = featureTagBits;
        mFeatureTags.clear();
        mFeatureTags.addAll(featureTags);

        if (mIsConnectedOverCrossSim) {
            if (networkType == NetworkType.IWLAN) {
                return NetworkType.CROSS_SIM;
            }
            ImsLog.d(mSlotId, "Even though it connected over CrossSim, networkType is not IWLAN");
        }
        return networkType;
    }

    private void onRegistered(NetworkType networkType, int featureTagBits,
            Set<String> featureTags) {
        mRegState = IAosRegistrationListener.RegistrationState.REGISTERED;
        mRegisteredNetworkType = adjustedNetworkType(networkType, featureTagBits, featureTags);
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyRegistered(RegistrationType.NORMAL, mRegisteredNetworkType, featureTagBits,
                    featureTags);
        }
    }

    private void onRegisteredForEmergency(int regType, NetworkType networkType, int featureTagBits,
            Set<String> featureTags) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyRegistered(regType, networkType, featureTagBits, featureTags);
        }
    }

    private void onRegistering(NetworkType networkType, int featureTagBits,
            Set<String> featureTags) {
        NetworkType adjustedNetworkType = adjustedNetworkType(networkType, featureTagBits,
                featureTags);
        mRegState = IAosRegistrationListener.RegistrationState.REGISTERING;
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyRegistering(RegistrationType.NORMAL, adjustedNetworkType, featureTagBits,
                    featureTags);
        }
    }

    private void onRegisteringForEmergency(int regType, NetworkType networkType, int featureTagBits,
            Set<String> featureTags) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyRegistering(regType, networkType, featureTagBits, featureTags);
        }
    }

    private void onDeregistered(NetworkType networkType, ReasonCode reason) {
        if (mRegState == IAosRegistrationListener.RegistrationState.DEREGISTERED
                && reason == ReasonCode.UNSPECIFIED) {
            return;
        }

        mRegState = IAosRegistrationListener.RegistrationState.DEREGISTERED;
        mRegisteredNetworkType = NetworkType.NONE;
        mFeatureTagBits = 0;
        mFeatureTags.clear();

        if (networkType == NetworkType.NONE) {
            if (reason == ReasonCode.PLMN_BLOCK
                    || reason == ReasonCode.PLMN_BLOCK_WITH_TIMEOUT
                    || reason == ReasonCode.PLMN_BLOCK_WITH_TIMEOUT_BY_VOPS_NOT_SUPPORTED
                    || reason == ReasonCode.PLMN_BLOCK_WITH_TIMEOUT_BY_SSAC_BARRED) {
                IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mSlotId);
                if (dnw != null) {
                    networkType = getRegistrationNetworkType(dnw.getNetworkType());
                    ImsLog.d(mSlotId, "update actual networkType : " + networkType);
                }
            }
        }

        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyDeregistered(RegistrationType.NORMAL, networkType, reason,
                    getErrorMessage(reason));
        }
    }

    private void onDeregisteredForEmergency(
            int regType, NetworkType networkType, ReasonCode reason) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyDeregistered(regType, networkType, reason, getErrorMessage(reason));
        }
    }

    private void onDeregistering(int regType) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyDeregistering(regType);
        }
    }

    private void onTechnologyChangeFailed(
            int regType, NetworkType networkType, ReasonCode reason) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyTechnologyChangeFailed(regType, networkType, reason, null);
        }
    }

    private void onAssociatedUriChanged(Uri[] uris) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyAssociatedUriChanged(uris);
        }
    }

    private void onCapabilitiesUpdateFailed(int capabilities, NetworkType networkType, int reason) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyCapabilitiesUpdateFailed(capabilities, networkType, reason);
        }
    }

    private void onCapabilitiesUpdated(IAosRegistration.CapabilityPairs pairs) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyCapabilitiesUpdated(pairs);
        }
    }

    private void onRegEventStateChanged(int statusCode, Set<Uri> impus) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyRegEventStateChanged(statusCode, impus);
        }
    }

    private void onAosIsimStateChanged(int state) {
        for (IAosInfoListener l : mAosInfoListeners) {
            l.notifyAosIsimStateChanged(state);
        }
    }

    private void onImsFeatureChanged(int regType, NetworkType networkType, int featureTagBits) {
        for (IAosRegistrationListener l : mAosRegistrationListeners) {
            l.notifyImsFeatureChanged(regType, networkType, featureTagBits);
        }
    }

    @SuppressWarnings("unused")
    private void onPhoneNumberRetryRequest(int command) {
        // TODO : FIX
    }

    @SuppressWarnings("unused")
    private void onWifiServiceRequest(int command) {
        // TODO : FIX
    }

    private void updateRegistered(int regType, NetworkType networkType, int featureTagBits,
            Set<String> featureTags) {
        ImsLog.d(mSlotId, "updateRegistered :: regType(" + regType + "), networkType("
                + networkType.toString() + "), featureTagBits (" + featureTagBits
                + "), featureTags : " + featureTags.toString());

        if (regType == RegistrationType.EMERGENCY || regType == RegistrationType.FAKE) {
            mHandler.post(() -> onRegisteredForEmergency(regType, networkType, featureTagBits,
                    featureTags));
        } else {
            mHandler.post(() -> onRegistered(networkType, featureTagBits, featureTags));
        }
    }

    private void updateRegistering(int regType, NetworkType networkType, int featureTagBits,
            Set<String> featureTags) {
        ImsLog.d(mSlotId, "updateRegistering :: regType(" + regType + "), networkType("
                + networkType.toString() + "), featureTagBits (" + featureTagBits
                + "), featureTags : " + featureTags.toString());

        if (regType == RegistrationType.EMERGENCY || regType == RegistrationType.FAKE) {
            mHandler.post(() -> onRegisteringForEmergency(regType, networkType, featureTagBits,
                    featureTags));
        } else {
            mHandler.post(() -> onRegistering(networkType, featureTagBits, featureTags));
        }
    }

    private void updateDeregistered(int regType, NetworkType networkType, ReasonCode reason) {
        ImsLog.d(mSlotId, "updateDeregistered :: regType(" + regType + "), networkType("
                + networkType.toString() + "), reason(" + reason.toString() + ")");

        if (regType == RegistrationType.EMERGENCY || regType == RegistrationType.FAKE) {
            mHandler.post(() -> onDeregisteredForEmergency(regType, networkType, reason));
        } else {
            mHandler.post(() -> onDeregistered(networkType, reason));
        }
    }

    private void updateDeregistering(int regType) {
        ImsLog.d(mSlotId, "updateDeregistering");
        mHandler.post(() -> onDeregistering(regType));
    }

    private void updateTechnologyChangeFailed(
            int regType, NetworkType networkType, ReasonCode reason) {
        ImsLog.d(mSlotId, "updateTechnologyChangeFailed :: regType(" + regType + "), networkType("
                + networkType.toString() + "), reason(" + reason.toString() + ")");

        mHandler.post(() -> onTechnologyChangeFailed(regType, networkType, reason));
    }

    private void updateAssociatedUriChanged(Uri[] uris) {
        ImsLog.d(mSlotId, "updateAssociatedUriChanged :: URIs : " +
                java.util.Arrays.toString(uris));
        mHandler.post(() -> onAssociatedUriChanged(uris));
    }

    private void updateCapabilitiesUpdateFailed(
            int capabilities, NetworkType networkType, int reason) {
        ImsLog.d(mSlotId, "updateCapabilitiesUpdateFailed :: capabilities(" + capabilities +
                "), networkType(" + networkType + "), reason(" + reason + ")");

        mHandler.post(() -> onCapabilitiesUpdateFailed(capabilities, networkType, reason));
    }

    private void notifyCapabilitiesUpdated(IAosRegistration.CapabilityPairs pairs) {
        ImsLog.d(mSlotId, "notifyCapabilitiesUpdated :: pairs(" + pairs.toString() + ")");
        mHandler.post(() -> onCapabilitiesUpdated(pairs));
    }

    private void notifyAosIsimStateChanged(int state) {
        ImsLog.d(mSlotId, "notifyAosIsimStateChanged :: state(" + state + ")");
        mHandler.post(() -> onAosIsimStateChanged(state));
    }

    private void notifyRegEventStateChanged(int statusCode, Set<Uri> impus) {
        ImsLog.d(mSlotId, "notifyRegEventStateChanged :: statusCode(" + statusCode + "), IMPU: "
                + impus.toString());
        mHandler.post(() -> onRegEventStateChanged(statusCode, impus));
    }

    private void updateImsFeatureChanged(int regType, NetworkType networkType, int featureTagBits) {
        ImsLog.d(mSlotId, "updateImsFeatureChanged :: regType(" + regType + "), networkType("
                + networkType.toString() + "), featureTagBits (" + featureTagBits + ")");

        mHandler.post(() -> onImsFeatureChanged(regType, networkType, featureTagBits));
    }

    private void requestPhoneNumberRetry(int command) {
        ImsLog.d(mSlotId, "RequestPhoneNumberRetry :: command(" + command + ")");
        mHandler.post(() -> onPhoneNumberRetryRequest(command));
    }

    private void requestWifiService(int command) {
        ImsLog.d(mSlotId, "RequestWifiService :: command(" + command + ")");
        mHandler.post(() -> onWifiServiceRequest(command));
    }

    private String getErrorMessage(ReasonCode reason) {
        String key = convertReasonToKey(reason);
        return (key == null) ? null :
                getStringFromBundle(CarrierConfig.ImsWfc.KEY_WFC_ERR_MESSAGE_BUNDLE, key);
    }

    private boolean isIkeAuthFailureNotificationRequired() {
        ConfigInterface configInterface =
                AgentFactory.getInstance().getAgent(ConfigInterface.class, mSlotId);
        CarrierConfig carrierConfig =
                (configInterface == null) ? null : configInterface.getCarrierConfig();
        return (carrierConfig == null) ? false : carrierConfig.getBoolean(
                CarrierConfig.ImsWfc.KEY_NOTIFY_IKE_AUTH_FAILURE_FOR_WFC_ACTIVATION_BOOL);
    }

    private static NetworkType getRegistrationNetworkType(int telephonyNetworkType) {
        return switch (telephonyNetworkType) {
            case TelephonyManager.NETWORK_TYPE_IWLAN -> NetworkType.IWLAN;
            case TelephonyManager.NETWORK_TYPE_UMTS,
                    TelephonyManager.NETWORK_TYPE_HSDPA,
                    TelephonyManager.NETWORK_TYPE_HSUPA,
                    TelephonyManager.NETWORK_TYPE_HSPA,
                    TelephonyManager.NETWORK_TYPE_HSPAP,
                    TelephonyManager.NETWORK_TYPE_TD_SCDMA -> NetworkType.UTRAN;
            case TelephonyManager.NETWORK_TYPE_LTE -> NetworkType.LTE;
            case TelephonyManager.NETWORK_TYPE_NR -> NetworkType.NR;
            default -> NetworkType.NONE;
        };
    }

    private static String convertReasonToKey(ReasonCode reason) {
        return switch (reason) {
            case WFC_REG_RESP_403 ->
                    CarrierConfig.ImsWfc.KEY_WFC_ERR_REG_403_STRING;
            case WFC_REG_RESP_403_NOT_SUPPORTED_COUNTRY ->
                    CarrierConfig.ImsWfc.KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING;
            case WFC_REG_RESP_500 ->
                    CarrierConfig.ImsWfc.KEY_WFC_ERR_REG_500_STRING;
            case WFC_REG_RESP_OTHER_FAILURES ->
                    CarrierConfig.ImsWfc.KEY_WFC_ERR_OTHER_FAILURES_STRING;
            case WFC_SUB_RESP_403 ->
                    CarrierConfig.ImsWfc.KEY_WFC_ERR_SUB_403_STRING;
            case WFC_SUB_NOTIFY_TERMINATED ->
                    CarrierConfig.ImsWfc.KEY_WFC_ERR_NOTIFY_TERMINATED_STRING;
            default -> null;
        };
    }

    private String getStringFromBundle(String bundleKey, String key) {
        ConfigInterface ci = AgentFactory.getInstance().getAgent(ConfigInterface.class, mSlotId);
        CarrierConfig cc = (ci == null) ? null : ci.getCarrierConfig();
        PersistableBundle pb = (cc == null) ? null : cc.getBundle(bundleKey);
        return (pb == null) ? null : pb.getString(key, null);
    }

    private final class NativeStateListener implements NativeStateInterface.Listener {
        @Override
        public void onNativeServiceReady() {
            ImsLog.d(mSlotId, "NativeState: service ready.");
            notifyCapabilitiesChanged();
            notifyImsServiceChanged();

            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

            if (sim != null) {
                if (sim.isSimLoaded()) {
                    notifyPhoneNumberState(false, PhoneNumberState.SIM_LOADED);
                }

                notifyIsimState(sim.getIsimState());
            } else {
                ImsLog.e(mSlotId, "SimInterface is null for slot: " + mSlotId);
            }

            notifyAosStart();
        }
    }

    @VisibleForTesting
    ImsServiceRegistryListener getServiceRegistryListener() {
        return mServiceRegistryListener;
    }

    private class ImsServiceRegistryListener implements ImsServiceRegistry.Listener {
        @Override
        public void onImsOnOffChanged() {
            notifyImsServiceChanged();
        }
    }


    @VisibleForTesting
    JniImsListenerProxy getJniImsListenerProxy() {
        return mJniImsListenerProxy;
    }

    private class JniImsListenerProxy implements JniImsListener {

        @Override
        public void onMessage(Parcel parcel) {
            int message = parcel.readInt();
            ImsLog.i("ListenerProxy.onMessage() :: message=" + message);

            switch (message) {
                case IIAosService.N2J_NOTIFY_REGISTERED -> {
                    int regType = parcel.readInt();
                    int networkType = parcel.readInt();
                    int featureTagBits = parcel.readInt();

                    Set<String> featureTags = new ArraySet<String>();
                    int count = parcel.readInt();
                    for (int i = 0; i < count; i++) {
                        featureTags.add(parcel.readString());
                    }

                    updateRegistered(regType, NetworkType.of(networkType), featureTagBits,
                            featureTags);
                }
                case IIAosService.N2J_NOTIFY_REGISTERING -> {
                    int regType = parcel.readInt();
                    int networkType = parcel.readInt();
                    int featureTagBits = parcel.readInt();

                    Set<String> featureTags = new ArraySet<String>();
                    int count = parcel.readInt();
                    for (int i = 0; i < count; i++) {
                        featureTags.add(parcel.readString());
                    }

                    updateRegistering(regType, NetworkType.of(networkType), featureTagBits,
                            featureTags);
                }
                case IIAosService.N2J_NOTIFY_DEREGISTERED -> {
                    int regType = parcel.readInt();
                    int networkType = parcel.readInt();
                    int reason = parcel.readInt();

                    updateDeregistered(regType, NetworkType.of(networkType), ReasonCode.of(reason));
                }
                case IIAosService.N2J_NOTIFY_DEREGISTERING -> {
                    int regType = parcel.readInt();
                    updateDeregistering(regType);
                }
                case IIAosService.N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED -> {
                    int regType = parcel.readInt();
                    int networkType = parcel.readInt();
                    int reason = parcel.readInt();

                    updateTechnologyChangeFailed(
                            regType, NetworkType.of(networkType), ReasonCode.of(reason));
                }
                case IIAosService.N2J_NOTIFY_ASSOCIATED_URI_CHANGED -> {
                    int count = parcel.readInt();
                    if (count <= 0) {
                        ImsLog.d("No URIs");
                        updateAssociatedUriChanged(null);
                        break;
                    }

                    Uri[] uris = new Uri[count];
                    for (int i = 0; i < count; ++i) {
                        uris[i] = Uri.parse(parcel.readString());
                    }

                    updateAssociatedUriChanged(uris);
                }
                case IIAosService.N2J_NOTIFY_CAPABILITIES_UPDATE_FAILED -> {
                    int capabilities = parcel.readInt();
                    int networkType = parcel.readInt();
                    int reason = parcel.readInt();

                    updateCapabilitiesUpdateFailed(
                            capabilities, NetworkType.of(networkType), reason);
                }
                case IIAosService.N2J_NOTIFY_REG_EVENT_STATE -> {
                    int statusCode = parcel.readInt();

                    Set<Uri> impus = new ArraySet<>();
                    int count = parcel.readInt();
                    for (int i = 0; i < count; i++) {
                        impus.add(Uri.parse(parcel.readString()));
                    }

                    notifyRegEventStateChanged(statusCode, impus);
                }
                case IIAosService.N2J_NOTIFY_AOS_ISIM_STATE -> {
                    int state = parcel.readInt();
                    notifyAosIsimStateChanged(state);
                }
                case IIAosService.N2J_NOTIFY_IMS_FEATURE_CHANGED -> {
                    int regType = parcel.readInt();
                    int networkType = parcel.readInt();
                    int featureTagBits = parcel.readInt();

                    updateImsFeatureChanged(regType, NetworkType.of(networkType), featureTagBits);
                }
                case IIAosService.N2J_REQUEST_PHONE_NUMBER_RETRY -> {
                    int command = parcel.readInt();
                    requestPhoneNumberRetry(command);
                }
                case IIAosService.N2J_REQUEST_WIFI_SERVICE -> {
                    int command = parcel.readInt();
                    requestWifiService(command);
                }
                default -> ImsLog.d("Not handled");
            }
        }
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        pw.println("Aos:");
        pw.increaseIndent();
        pw.println("state=" + mRegState);
        pw.println("networkType=" + mRegisteredNetworkType);
        pw.decreaseIndent();
    }
}
