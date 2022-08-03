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

import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.os.Parcel;
import android.util.ArraySet;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfoListener;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IIAosService;
import com.android.imsstack.jni.JniIms;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This class provides an interface to manage and control the AoS related information.
 */
public class AosService implements IAosRegistration, IAosInfo, Sim.Listener, Sim.IsimListener {

    public static final int EVENT_NATIVE_BOOT_COMPLETED = 1000;
    private AosServiceHandler mHandler;

    private final Set<IAosRegistrationListener> mAosRegistationListeners =
            new CopyOnWriteArraySet<IAosRegistrationListener>();
    private final Set<IAosInfoListener> mAosInfoListeners =
            new CopyOnWriteArraySet<IAosInfoListener>();

    private CapabilityPairs mCapabilityPairs;

    private long mNativeObject = 0;
    private JNIImsListenerProxy mNativeListener = new JNIImsListenerProxy();
    private int mSlotId = MSimUtils.DEFAULT_SLOT_ID;

    private int mRegisteredNetworkType = NetworkType.NONE;

    public void start(int slotId) {
        mSlotId = slotId;

        mHandler = new AosServiceHandler();

        mNativeObject = JniIms.getInterface(IUIMS.AOS_SERVICE, mSlotId);
        if (mNativeObject != 0) {
            JniIms.setListener(mNativeObject, mNativeListener);
        }

        IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
        if (jniEvt != null) {
            jniEvt.registerForNativeBootComplete(mHandler, EVENT_NATIVE_BOOT_COMPLETED, null);
        }
    }

    public void stop() {
        if (mHandler != null) {
            IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
            if (jniEvt != null) {
                jniEvt.unregisterForNativeBootComplete(mHandler);
            }

            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }

        if (mNativeObject == 0) {
            return;
        }

        JniIms.removeListener(mNativeObject);
        JniIms.releaseInterface(mNativeObject);
        mNativeObject = 0;
        mSlotId = MSimUtils.DEFAULT_SLOT_ID;
    }

    /**
     * Initialize any necessasry information for AoS service.
     */
    public void init() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);

        if (sim != null) {
            sim.addListener(this);
            sim.addIsimListener(this);
        }
    }

    /**
     * Clean up the necessary information that was previously initialized for AoS service.
     */
    public void cleanup() {
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

        JniIms.sendData(mNativeObject, data);
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

    private void onRegistered(int networkType, int featureTagBits,
            Set<String> featureTags) {
        mRegisteredNetworkType = networkType;
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyRegistered(networkType, featureTagBits, featureTags);
        }
    }

    private void onRegistering(int networkType, int featureTagBits,
            Set<String> featureTags) {
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyRegistering(networkType, featureTagBits, featureTags);
        }
    }

    private void onDeregistered(int reason) {
        mRegisteredNetworkType = NetworkType.NONE;
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyDeregistered(reason);
        }
    }

    private void onTechnologyChangeFailed(int networkType, int causeCode) {
        for (IAosRegistrationListener l : mAosRegistationListeners) {
            l.notifyTechnologyChangeFailed(networkType, causeCode);
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

    private void updateDeregistered(int reason) {
        ImsLog.d(mSlotId, "updateDeregistered :: reason(" + reason + ")");
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                onDeregistered(reason);
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

    private final class AosServiceHandler extends Handler {

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                ImsLog.d(mSlotId, "msg is null");
                return;
            }

            ImsLog.i(mSlotId, "handleMessage :: msg= " + msg.what);

            switch (msg.what) {
                case EVENT_NATIVE_BOOT_COMPLETED:
                    handleNativeBootCompleted();
                    break;

                default:
                    break;
            }
        }

        private void handleNativeBootCompleted() {
            ImsLog.d(mSlotId, "");
            notifyCapabilitiesChanged();

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

                    int count = parcel.readInt();
                    if (count <= 0) {
                        ImsLog.d("No featureTags");
                        updateRegistered(networkType, featureTagBits, new ArraySet<String>());
                        break;
                    }

                    Set<String> featureTags = new ArraySet<String>();
                    for (int i = 0; i < count; i++) {
                        featureTags.add(parcel.readString());
                    }

                    updateRegistered(networkType, featureTagBits, featureTags);
                    break;
                }

                case IIAosService.N2J_NOTIFY_REGISTERING: {
                    int networkType = parcel.readInt();
                    int featureTagBits = parcel.readInt();

                    int count = parcel.readInt();
                    if (count <= 0) {
                        ImsLog.d("No featureTags");
                        updateRegistering(networkType, featureTagBits, new ArraySet<String>());
                        break;
                    }

                    Set<String> featureTags = new ArraySet<String>();
                    for (int i = 0; i < count; i++) {
                        featureTags.add(parcel.readString());
                    }

                    updateRegistering(networkType, featureTagBits, featureTags);
                    break;
                }

                case IIAosService.N2J_NOTIFY_DEREGISTERED: {
                    int reason = parcel.readInt();

                    updateDeregistered(reason);
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
