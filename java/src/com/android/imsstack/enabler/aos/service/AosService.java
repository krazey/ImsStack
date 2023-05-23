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

import static com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;

import android.content.Intent;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;
import android.os.PersistableBundle;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;
import android.util.ArraySet;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfoListener;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationState;
import com.android.imsstack.enabler.aos.IIAosService;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Arrays;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class provides an interface to manage and control the AoS related information.
 */
public class AosService implements IAosRegistration, IAosInfo, Sim.Listener, Sim.IsimListener {

    public static final int PCO_TARGET_ID = 0xff00;
    public static final int PCO_NONE_VALUE = 0;

    private Handler mHandler;

    @VisibleForTesting
    protected final Set<IAosRegistrationListener> mAosRegistationListeners =
            new CopyOnWriteArraySet<IAosRegistrationListener>();
    @VisibleForTesting
    protected final Set<IAosInfoListener> mAosInfoListeners =
            new CopyOnWriteArraySet<IAosInfoListener>();

    @VisibleForTesting
    protected final ImsServiceRegistryListener mListener = new ImsServiceRegistryListener();

    @VisibleForTesting
    protected CapabilityPairs mCapabilityPairs;

    private long mNativeObject = 0;
    @VisibleForTesting
    protected JNIImsListenerProxy mNativeListener = new JNIImsListenerProxy();
    private int mSlotId = MSimUtils.DEFAULT_SLOT_ID;

    @VisibleForTesting
    protected int mRegisteredNetworkType = NetworkType.NONE;

    private int mRegState = RegistrationState.DEREGISTERED;

    @VisibleForTesting
    protected int mFeatureTagBits = 0;
    @VisibleForTesting
    protected final Set<String> mFeatureTags = new CopyOnWriteArraySet<String>();
    @VisibleForTesting
    protected boolean mIsConnectedOverCrossSim = false;
    private NativeStateListener mNativeStateListener;

    public void init(int slotId) {
        mSlotId = slotId;

        mHandler = new Handler(Looper.myLooper());

        mNativeObject = JniImsProxy.getInterface(IUIMS.AOS_SERVICE, mSlotId);
        if (mNativeObject != 0) {
            JniImsProxy.setListener(mNativeObject, mNativeListener);
        }

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        if (nsi != null) {
            mNativeStateListener = new NativeStateListener();
            nsi.addListener(mNativeStateListener);
        }

        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        isr.addListener(mListener);
    }

    public void cleanup() {
        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        isr.removeListener(mListener);

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
    }

    /**
     * Stop the necessary information that was previously initialized for AoS service.
     */
    public void stop() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim != null) {
            sim.removeListener(this);
            sim.removeIsimListener(this);
        }
    }

    @Override
    public void addListener(IAosRegistrationListener listener) {
        mAosRegistationListeners.add(listener);
    }

    @Override
    public void removeListener(IAosRegistrationListener listener) {
        mAosRegistationListeners.remove(listener);
    }

    @Override
    public void addListener(IAosInfoListener listener) {
        mAosInfoListeners.add(listener);
    }

    @Override
    public void removeListener(IAosInfoListener listener) {
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
    public void triggerFullNetworkRegistration(int sipCode, String sipReason) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_REQUEST_FULL_REGISTRATION);
        parcel.writeInt(sipCode);
        parcel.writeString((sipReason == null) ? "" : sipReason);

        sendRequest(parcel);
    }

    @Override
    public void changeCapabilities(CapabilityPairs pairs) {
        if (pairs == null) {
            return;
        }

        adjustCapabilities(pairs);

        if (mCapabilityPairs != null && mCapabilityPairs.equals(pairs)) {
            return;
        }

        mCapabilityPairs = pairs;

        notifyCapabilitiesChanged();
    }

    @Override
    public void controlRegistration(int requestType, int pcscfOrder, int cause) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_REQUEST_CONTROL_REGISTRATION);
        parcel.writeInt(requestType);
        parcel.writeInt(pcscfOrder);
        parcel.writeInt(cause);

        sendRequest(parcel);
    }

    @Override
    public int getRegisteredNetworkType() {
        return mRegisteredNetworkType;
    }

    @Override
    public int getRegistrationState() {
        return mRegState;
    }

    @Override
    public void notifyAirplaneSetting(boolean isOn) {
        sendRequest(IIAosService.J2N_NOTIFY_AIRPLANE_SETTING, isOn);
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
    public void notifyRoamingPreferredVoiceNetwork(int state) {
        sendRequest(IIAosService.J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK, state);
    }

    @Override
    public void notifyServiceSetting(int state, int serviceBits) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_SERVICE_SETTING);
        parcel.writeInt(state);
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
    public void notifyIpcanHandoverFailure(int targetNetwork, int causeCode) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_IPCAN_HANDOVER_FAILURE);
        parcel.writeInt(targetNetwork);
        parcel.writeInt(causeCode);
        sendRequest(parcel);
    }

    @Override
    public void notifyIsimState(int state) {
        sendRequest(IIAosService.J2N_NOTIFY_ISIM_STATE, state);
    }

    @Override
    public void notifyLocationInfo(int state) {
        sendRequest(IIAosService.J2N_NOTIFY_LOCATION_INFO, state);
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
    public void notifyPhoneNumberState(boolean isRefresh, int state) {
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(IIAosService.J2N_NOTIFY_PHONE_NUMBER_STATE);
        parcel.writeInt(isRefresh ? 1 : 0);
        parcel.writeInt(state);
        sendRequest(parcel);
    }

    @Override
    public void notifyPlmnChanged() {
        sendRequest(IIAosService.J2N_NOTIFY_PLMN_CHANGED);
    }

    @Override
    public void notifyPowerOff() {
        sendRequest(IIAosService.J2N_NOTIFY_POWER_OFF);
    }

    @Override
    public void notifyPreciseCallState(int state) {
        sendRequest(IIAosService.J2N_NOTIFY_PRECISE_CALL_STATE, state);
    }

    @Override
    public void notifyCarrierSignalPcoValueChanged(Intent intent) {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                handleCarrierSignalPcoValueChanged(intent);
            }
        });
    }

    @Override
    public void notifyCrossSimStatus(boolean isConnectedOverCrossSim) {
        ImsLog.d(mSlotId, "AosService: notifyCrossSimStatus");
        if (mIsConnectedOverCrossSim == isConnectedOverCrossSim) {
            return;
        }

        mIsConnectedOverCrossSim = isConnectedOverCrossSim;
        if ((mRegisteredNetworkType == NetworkType.CROSS_SIM) && !mIsConnectedOverCrossSim) {
            updateRegisteredNetworkType(NetworkType.IWLAN);
        } else if ((mRegisteredNetworkType == NetworkType.IWLAN) && mIsConnectedOverCrossSim) {
            updateRegisteredNetworkType(NetworkType.CROSS_SIM);
        }
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
            int isimState = sim.getIsimState();

            switch (isimState) {
                case Sim.ISIM_STATE_NOT_PRESENT:
                    notifyIsimState(IsimState.NOT_PRESENT);
                    break;
                case Sim.ISIM_STATE_NOT_READY:
                    notifyIsimState(IsimState.NOT_READY);
                    break;
                case Sim.ISIM_STATE_LOADED:
                    notifyIsimState(IsimState.LOADED);
                    break;
                case Sim.ISIM_STATE_REFRESH_STARTED:
                    notifyIsimState(IsimState.REFRESH_STARTED);
                    break;
                default:
                    break;
            }
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

        for (Map.Entry<Integer, Integer> entry : mCapabilityPairs.getCapabilities().entrySet()) {
            parcel.writeInt(entry.getKey());
            parcel.writeInt(entry.getValue());
        }

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
            case 1:
                value = values[0];
                break;
            case 4:
                value = values[3];
                break;
            default:
                ImsLog.i("Invalid PCO values length : " + values.length);
                break;
        }
        ImsLog.d("Returns PCO value : " + value);
        return value;
    }

    private void notifyImsServiceChanged() {
        controlRegistration((isImsEnabled()) ? IAosRegistration.RequestType.START :
                IAosRegistration.RequestType.STOP, IAosRegistration.Pcscf.CURRENT,
                IAosRegistration.Cause.IMS_SERVICE);
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

    private void updateRegisteredNetworkType(int networkType) {
        ImsLog.d(mSlotId, "updateRegisteredNetworkType :: networkType(" + networkType + ")");
        mRegisteredNetworkType = networkType;
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyRegistered(mRegisteredNetworkType, mFeatureTagBits, mFeatureTags);
        }
    }

    private int adjustedNetworkType(int networkType, int featureTagBits, Set<String> featureTags) {
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

    private void onRegistered(int networkType, int featureTagBits, Set<String> featureTags) {
        mRegState = IAosRegistrationListener.RegistrationState.REGISTERED;
        mRegisteredNetworkType = adjustedNetworkType(networkType, featureTagBits, featureTags);
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyRegistered(mRegisteredNetworkType, featureTagBits, featureTags);
        }
    }

    private void onRegistering(int networkType, int featureTagBits, Set<String> featureTags) {
        int adjustedNetworkType = adjustedNetworkType(networkType, featureTagBits, featureTags);
        mRegState = IAosRegistrationListener.RegistrationState.REGISTERING;
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyRegistering(adjustedNetworkType, featureTagBits, featureTags);
        }
    }

    private void onDeregistered(int networkType, int reason) {
        if (mRegState == IAosRegistrationListener.RegistrationState.DEREGISTERED
                && reason == ReasonCode.CODE_UNSPECIFIED) {
            return;
        }

        mRegState = IAosRegistrationListener.RegistrationState.DEREGISTERED;
        mRegisteredNetworkType = NetworkType.NONE;
        mFeatureTagBits = 0;
        mFeatureTags.clear();
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyDeregistered(networkType, reason, getErrorMessage(reason));
        }
    }

    private void onTechnologyChangeFailed(int networkType, int causeCode) {
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyTechnologyChangeFailed(networkType, causeCode, null);
        }
    }

    private void onAssociatedUriChanged(Uri[] uris) {
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyAssociatedUriChanged(uris);
        }
    }

    private void onCapabilitiesUpdateFailed(int capabilities, int networkType, int reason) {
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyCapabilitiesUpdateFailed(capabilities, networkType, reason);
        }
    }

    private void onAosIsimStateChanged(int state) {
        for (IAosInfoListener l : mAosInfoListeners) {
            l.notifyAosIsimStateChanged(state);
        }
    }

    @SuppressWarnings("unused")
    private void onRegEventStateChanged(int state) {
        // TODO : FIX
    }

    @SuppressWarnings("unused")
    private void onPhoneNumberRetryRequest(int command) {
        // TODO : FIX
    }

    @SuppressWarnings("unused")
    private void onWifiServiceRequest(int command) {
        // TODO : FIX
    }

    private void updateRegistered(int networkType, int featureTagBits,
            Set<String> featureTags) {
        ImsLog.d(mSlotId, "updateRegistered :: networkType(" + networkType +
                "), featureTagBits (" + featureTagBits + "), featureTags : " +
                featureTags.toString());

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onRegistered(networkType, featureTagBits, featureTags);
            }
        });
    }

    private void updateRegistering(int networkType, int featureTagBits,
            Set<String> featureTags) {
        ImsLog.d(mSlotId, "updateRegistering :: networkType(" + networkType +
        "), featureTagBits (" + featureTagBits + "), featureTags : " +
        featureTags.toString());

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onRegistering(networkType, featureTagBits, featureTags);
            }
        });
    }

    private void updateDeregistered(int networkType, int reason) {
        ImsLog.d(mSlotId, "updateDeregistered :: networkType(" + networkType + "), reason("
                + reason + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onDeregistered(networkType, reason);
            }
        });
    }

    private void updateTechnologyChangeFailed(int networkType, int reason) {
        ImsLog.d(mSlotId, "updateTechnologyChangeFailed :: networkType(" + networkType +
                "), reason(" + reason + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onTechnologyChangeFailed(networkType, reason);
            }
        });
    }

    private void updateAssociatedUriChanged(Uri[] uris) {
        ImsLog.d(mSlotId, "updateAssociatedUriChanged :: URIs : " +
                java.util.Arrays.toString(uris));
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onAssociatedUriChanged(uris);
            }
        });
    }

    private void updateCapabilitiesUpdateFailed(int capabilities, int networkType, int reason) {
        ImsLog.d(mSlotId, "updateCapabilitiesUpdateFailed :: capabilities(" + capabilities +
                "), networkType(" + networkType + "), reason(" + reason + ")");

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onCapabilitiesUpdateFailed(capabilities, networkType, reason);
            }
        });
    }

    private void NotifyAosIsimStateChanged(int state) {
        ImsLog.d(mSlotId, "NotifyAosIsimStateChanged :: state(" + state + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onAosIsimStateChanged(state);
            }
        });
    }

    private void NotifyRegEventStateChanged(int state) {
        ImsLog.d(mSlotId, "NotifyRegEventStateChanged :: state(" + state + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onRegEventStateChanged(state);
            }
        });
    }

    private void RequestPhoneNumberRetry(int command) {
        ImsLog.d(mSlotId, "RequestPhoneNumberRetry :: command(" + command + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onPhoneNumberRetryRequest(command);
            }
        });
    }

    private void RequestWifiService(int command) {
        ImsLog.d(mSlotId, "RequestWifiService :: command(" + command + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onWifiServiceRequest(command);
            }
        });
    }

    private String getErrorMessage(int reason) {
        String key = convertReasonToKey(reason);
        return (key == null) ? null :
                getStringFromBundle(CarrierConfig.Assets.KEY_WFC_ERR_MESSAGE_BUNDLE, key);
    }

    private String convertReasonToKey(int reason) {
        switch (reason) {
            case ReasonCode.CODE_REGISTRATION_ERROR_WFC_REG_403:
                return CarrierConfig.Assets.KEY_WFC_ERR_REG_403_STRING;
            case ReasonCode.CODE_REGISTRATION_ERROR_WFC_REG_500:
                return CarrierConfig.Assets.KEY_WFC_ERR_REG_500_STRING;
            case ReasonCode.CODE_REGISTRATION_ERROR_WFC_NOT_SUPPORTED_COUNTRY:
                return CarrierConfig.Assets.KEY_WFC_ERR_NOT_SUPPORTED_COUNTRY_STRING;
            case ReasonCode.CODE_REGISTRATION_ERROR_WFC_SUB_403:
                return CarrierConfig.Assets.KEY_WFC_ERR_SUB_403_STRING;
            case ReasonCode.CODE_REGISTRATION_ERROR_WFC_NOTIFY_TERMINATED:
                return CarrierConfig.Assets.KEY_WFC_ERR_NOTIFY_TERMINATED_STRING;
            case ReasonCode.CODE_REGISTRATION_ERROR_WFC_OTHER_FAILURES:
                return CarrierConfig.Assets.KEY_WFC_ERR_OTHER_FAILURES_STRING;
            default:
                return null;
        }
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

                int isimState = sim.getIsimState();

                switch (isimState) {
                    case Sim.ISIM_STATE_NOT_PRESENT:
                        notifyIsimState(IsimState.NOT_PRESENT);
                        break;
                    case Sim.ISIM_STATE_NOT_READY:
                        notifyIsimState(IsimState.NOT_READY);
                        break;
                    case Sim.ISIM_STATE_LOADED:
                        notifyIsimState(IsimState.LOADED);
                        break;
                    case Sim.ISIM_STATE_REFRESH_STARTED:
                        notifyIsimState(IsimState.REFRESH_STARTED);
                        break;
                    default:
                        break;
                }
            }

            notifyAosStart();
        }
    }

    private class ImsServiceRegistryListener implements ImsServiceRegistry.Listener {
        @Override
        public void onImsOnOffChanged() {
            notifyImsServiceChanged();
        }
    }

    private class JNIImsListenerProxy implements JniImsListener {

        @Override
        public void onMessage(Parcel parcel) {
            int message = parcel.readInt();
            ImsLog.i("ListenerProxy.onMessage() :: message=" + message);

            switch (message) {
                case IIAosService.N2J_NOTIFY_REGISTERED: {
                    int networkType = parcel.readInt();
                    int featureTagBits = parcel.readInt();

                    Set<String> featureTags = new ArraySet<String>();
                    int count = parcel.readInt();
                    for (int i = 0; i < count; i++) {
                        featureTags.add(parcel.readString());
                    }

                    updateRegistered(networkType, featureTagBits, featureTags);
                    break;
                }

                case IIAosService.N2J_NOTIFY_REGISTERING: {
                    int networkType = parcel.readInt();
                    int featureTagBits = parcel.readInt();

                    Set<String> featureTags = new ArraySet<String>();
                    int count = parcel.readInt();
                    for (int i = 0; i < count; i++) {
                        featureTags.add(parcel.readString());
                    }

                    updateRegistering(networkType, featureTagBits, featureTags);
                    break;
                }

                case IIAosService.N2J_NOTIFY_DEREGISTERED: {
                    int networkType = parcel.readInt();
                    int reason = parcel.readInt();

                    updateDeregistered(networkType, reason);
                    break;
                }

                case IIAosService.N2J_NOTIFY_TECHNOLOGY_CHANGE_FAILED: {
                    int networkType = parcel.readInt();
                    int reason = parcel.readInt();

                    updateTechnologyChangeFailed(networkType, reason);
                    break;
                }

                case IIAosService.N2J_NOTIFY_ASSOCIATED_URI_CHANGED: {
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
                    break;
                }

                case IIAosService.N2J_NOTIFY_CAPABILITIES_UPDATE_FAILED: {
                    int capabilities = parcel.readInt();
                    int networkType = parcel.readInt();
                    int reason = parcel.readInt();

                    updateCapabilitiesUpdateFailed(capabilities, networkType, reason);
                    break;
                }

                case IIAosService.N2J_NOTIFY_AOS_ISIM_STATE: {
                    int state = parcel.readInt();
                    NotifyAosIsimStateChanged(state);
                    break;
                }

                case IIAosService.N2J_NOTIFY_REG_EVENT_STATE: {
                    int state = parcel.readInt();
                    NotifyRegEventStateChanged(state);
                    break;
                }

                case IIAosService.N2J_REQUEST_PHONE_NUMBER_RETRY: {
                    int command = parcel.readInt();
                    RequestPhoneNumberRetry(command);
                    break;
                }

                case IIAosService.N2J_REQUEST_WIFI_SERVICE: {
                    int command = parcel.readInt();
                    RequestWifiService(command);
                    break;
                }

                default:
                    ImsLog.d("Not handled");
                    break;
            }
        }
    }
}
