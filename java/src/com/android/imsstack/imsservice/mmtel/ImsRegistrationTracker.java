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

package com.android.imsstack.imsservice.mmtel;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.util.ArraySet;
import android.util.Pair;
import android.util.SparseArray;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.FeatureTagMask;
import com.android.imsstack.imsservice.mmtel.config.base.ConfigurationListener;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

/**
* Tracks IMS Registration status of IMS Service and the radio access
* technology in which it is registered on. Communicates the
* same to ImsRegistrationImpl for further notifying to framework.
* This Class also notifies the registered feature capability to ImsFeatureManager.
*/
public class ImsRegistrationTracker {
    public static interface CapabilityUpdateListener {
        /**
        * Update failure when registration failed for requested capability
        */
        public void onCapabilitiesUpdateFailed(int capabilities, int networkType, int reason);
    };

    private final IContext mContext;
    private final ImsRegistrationImpl mRegImpl;
    private IRegistrationFeatureListener mFeatureListener;
    private RegTracker mRegTracker;
    private int mFeatures = FeatureTagMask.NONE;
    private List<Pair<Integer, Integer>> mCapabilities;
    private ConfigListener mConfigListener = null;
    private ContentObserver mDataRoamingSettingObserver = null;

    @VisibleForTesting
    public MessageHandler mHandler = null;

    private class ConfigListener extends ConfigurationListener {
        @Override
        public void onImsConfigurationChanged(int item) {
            if ((item == ProvisioningManager.KEY_VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE)
                    || (item == ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE)) {
                logi("onImsConfigurationChanged:: changed item" + item);
                CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                if (capabilityPairs != null) {
                    mRegTracker.changeCapabilities(capabilityPairs);
                }
            }
        }
    };

    public ImsRegistrationTracker(IContext context, ImsRegistrationImpl regImpl) {
        mContext = context;
        mRegImpl = regImpl;
        mFeatureListener = null;
        mFeatures = FeatureTagMask.NONE;
        mRegTracker = new RegTracker();
        mCapabilities = new ArrayList<Pair<Integer, Integer>>();

        mRegImpl.setRegistrationTracker(this);

        if (isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming()) {
            mHandler = new MessageHandler();
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
            if (isr != null) {
                ImsConfigImpl configImpl = isr.getConfig();
                if (configImpl != null) {
                    mConfigListener = new ConfigListener();
                    configImpl.addListener(mConfigListener);
                }
            }
        }

        if (!isIgnoreDataEnabledChangedForVideoCalls()) {
            if (mHandler == null) {
                mHandler = new MessageHandler();
            }
            registerDataRoamingSettingObserver();
        }
    }

    public void dispose() {
        mRegTracker.notifyDeregistered(mRegTracker.getNetworkType(),
                IAosRegistrationListener.ReasonCode.CODE_UNSPECIFIED, null);
        mRegImpl.setRegistrationTracker(null);
        mRegTracker.clear();
        mCapabilities.clear();

        if (mHandler != null) {
            mHandler.clear();
        }
        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
        if (isr != null && mConfigListener != null) {
            ImsConfigImpl configImpl = isr.getConfig();
            if (configImpl != null) {
                configImpl.removeListener(mConfigListener);
            }
        }

        if (mDataRoamingSettingObserver != null) {
            mContext.getContext().getContentResolver().unregisterContentObserver(
                    mDataRoamingSettingObserver);
        }
    }

    public ImsRegistrationImpl getRegistration() {
        return mRegImpl;
    }

    public int getRegisteredNetworkType() {
        if (mRegTracker != null) {
            return mRegTracker.getNetworkType();
        } else {
            return IAosRegistrationListener.NetworkType.NONE;
        }
    }

    public void setCapabilityUpdateListener(CapabilityUpdateListener listener) {
        mRegTracker.setListener(listener);
    }

    public boolean isRegistered() {
        return (mFeatures != FeatureTagMask.NONE);
    }

    public int getRegisteredFeatures() {
        return mFeatures;
    }

    public boolean isCallRegistered() {
        return (isCallVoiceRegistered() || isCallVideoRegistered());
    }

    public boolean isCallVideoRegistered() {
        return ((mFeatures & FeatureTagMask.VIDEO) != 0);
    }

    public boolean isCallVoiceRegistered() {
        return ((mFeatures & FeatureTagMask.MMTEL) != 0);
    }

    public boolean isSmsRegistered() {
        return ((mFeatures & FeatureTagMask.SMSIP) != 0);
    }

    public boolean isCallVoiceAndVideoRegistered() {
        return (isCallVoiceRegistered() && isCallVideoRegistered());
    }

    public void refreshCallRegistrationState() {
        mRegTracker.clear();
        mRegTracker.init();
    }

    /**
     * This is invoked to set IRegistrationFeatureListener.
    */
    public void setRegistrationFeatureListener(IRegistrationFeatureListener listener) {
        synchronized (this) {
            if (listener != null) {
                mFeatureListener = listener;
            }
        }
    }

    private void updateFeatureCapabilities() {
        synchronized (this) {
            if (mFeatureListener != null) {
                mFeatureListener.onRegistrationFeatureChanged();
            }
        }
    }

    public void updateSipDelegateRegistration() {
        mRegTracker.updateSipDelegateRegistration();
    }

    public void triggerSipDelegateDeregistration() {
        mRegTracker.triggerSipDelegateDeregistration();
    }

    public void triggerFullNetworkRegistration(int sipCode, @Nullable String sipReason) {
        mRegTracker.triggerFullNetworkRegistration(sipCode, sipReason);
    }

    public void changeCapabilities(List<CapabilityPair> enabledCaps,
            List<CapabilityPair> disabledCaps) {
        logi("changeCapabilities::enabledCaps "
                + enabledCaps + " disabledCaps " + disabledCaps);

        if (disabledCaps != null) {
            for (int i = 0; i < disabledCaps.size(); ++i) {
                CapabilityPair capability = disabledCaps.get(i);
                for (int j = 0; j < mCapabilities.size(); j++) {
                    Pair<Integer, Integer> capabilityPair = mCapabilities.get(j);
                    if ((capabilityPair.first == capability.getRadioTech())
                            && (capabilityPair.second == capability.getCapability())) {
                        logi("Remove disabledCapabilities::NetworkType "
                                + capability.getRadioTech() + " Capability "
                                + capability.getCapability());
                        mCapabilities.remove(capabilityPair);
                        break;
                    }
                }
            }
        }
        if (enabledCaps != null) {
            for (int i = 0; i < enabledCaps.size(); ++i) {
                CapabilityPair capability = enabledCaps.get(i);
                boolean alreadyExist = false;

                for (int j = 0; j < mCapabilities.size(); j++) {
                    Pair<Integer, Integer> capabilityPair = mCapabilities.get(j);
                    if ((capabilityPair.first == capability.getRadioTech())
                            && (capabilityPair.second == capability.getCapability())) {
                        alreadyExist = true;
                    }
                }
                if (!alreadyExist) {
                    int radioTech = capability.getRadioTech();
                    int capabilityType = capability.getCapability();
                    mCapabilities.add(new Pair<>(radioTech, capabilityType));
                }
            }
        }

        CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
        if (capabilityPairs != null) {
            mRegTracker.changeCapabilities(capabilityPairs);
        } else {
            mRegTracker.changeCapabilities(new CapabilityPairs());
        }
    }

    @VisibleForTesting
    protected IAosRegistration getIAosRegistration(int slotId) {
        return AosFactory.getInstance().getAosRegistration(slotId);
    }

    @VisibleForTesting
    protected ConfigInterface getConfigInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
    }

    @VisibleForTesting
    protected IDcNetWatcher getDcNetWatcher(int slotId) {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, slotId);
    }

    @VisibleForTesting
    protected Handler getMessageHandler() {
        return mHandler;
    }

    @VisibleForTesting
    protected ImsStackRegistry.ImsServiceListener getImsServiceListener() {
        return mRegTracker;
    }

    protected CapabilityPairs createCapabilityPairsFromCapabilities() {

        if (mCapabilities.isEmpty()) {
            return null;
        }

        CapabilityPairs capabilityPairs = new CapabilityPairs();
        for (int i = 0; i < mCapabilities.size(); i++) {
            Pair<Integer, Integer> finalcapability = mCapabilities.get(i);
            int networkType = convertToAosNetworkType(finalcapability.first);
            int capability = convertToAosCapability(finalcapability.second);

            if (isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming()) {
                if ((networkType == IAosRegistrationListener.NetworkType.IWLAN)
                        && (capability == IAosRegistrationListener.Capability.VOICE)) {
                    if (isVoiceRoaming() && isCellularPreferredMode()) {
                        continue;
                    }
                }
            }

            if (!isIgnoreDataEnabledChangedForVideoCalls()) {
                if (!isMobileDataEnabled()) {
                    if (capability == IAosRegistrationListener.Capability.VIDEO) {
                        logi("Mobile data is off :: ignoring video capability");
                        continue;
                    }
                }
            }

            capabilityPairs.addCapability(networkType, capability);
            logi("changeCapabilities::finalCaps networkType"
                    + networkType + " Capability " + capability);

            if (((networkType == IAosRegistrationListener.NetworkType.LTE)
                    && (capability == IAosRegistrationListener.Capability.VIDEO))
                    || ((networkType == IAosRegistrationListener.NetworkType.NR)
                    && (capability == IAosRegistrationListener.Capability.VIDEO))) {
                ImsLog.d("Add Capability - IWLAN-VIDEO");
                capabilityPairs.addCapability(IAosRegistrationListener.NetworkType.IWLAN,
                        IAosRegistrationListener.Capability.VIDEO);
            }
        }
        return capabilityPairs;
    }

    private boolean isCellularPreferredMode() {
        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
        ImsConfigImpl configImpl = isr.getConfig();

        if ((configImpl.getConfigInt(
                ProvisioningManager.KEY_VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE)
                == ProvisioningManager.PROVISIONING_VALUE_DISABLED)
                || (configImpl.getConfigInt(ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE)
                == ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED)) {
            return true;
        }
        return false;
    }

    private boolean isMobileDataEnabled() {
        TelephonyManager tm = AppContext.getTelephonyManager(mContext.getSubId());
        if (isRoaming()) {
            return tm.isDataRoamingEnabled() && tm.isDataEnabled();
        }
        return tm.isDataEnabled();
    }

    private boolean isVoiceRoaming() {
        IDcNetWatcher dcnw = getDcNetWatcher(mContext.getSlotId());
        return (dcnw != null) ? dcnw.isVoiceRoaming() : false;
    }

    private boolean isRoaming() {
        IDcNetWatcher dcnw = getDcNetWatcher(mContext.getSlotId());
        return (dcnw != null) ? dcnw.isRoaming() : false;
    }

    private boolean isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming() {
        ConfigInterface config = getConfigInterface(mContext.getSlotId());
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return cc != null && cc.getBoolean(
                CarrierConfig.Assets
                .KEY_SUPPORT_VOWIFI_CAPABILITY_WHEN_WIFI_ONLY_OR_PREFERRED_IN_ROAMING_BOOL);
    }

    private boolean isIgnoreDataEnabledChangedForVideoCalls() {
        ConfigInterface config = getConfigInterface(mContext.getSlotId());
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        if (cc != null && cc.getBoolean(
                CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
            return true;
        }
        return false;
    }

    private void registerDataRoamingSettingObserver() {
        if (mDataRoamingSettingObserver != null) {
            return;
        }

        mDataRoamingSettingObserver = new ContentObserver(mContext.getDefaultHandler()) {
            @Override
            public void onChange(boolean bChange) {
                CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                if (capabilityPairs != null) {
                    mRegTracker.changeCapabilities(capabilityPairs);
                }
            }
        };

        mContext.getContext().getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.DATA_ROAMING), true,
                mDataRoamingSettingObserver);
    }
    /**
     * Call aos to send deregistration
     *
     */
    public void onDeregistrationTriggered(int reason) {
        logi("DeregistrationTriggered :: request from framework" + reason);
        mRegTracker.controlRegistration(reason);
    }

    private int convertToAosNetworkType(int radioTech) {
        switch (radioTech) {
            case ImsRegistrationImpl.REGISTRATION_TECH_LTE:
                return IAosRegistrationListener.NetworkType.LTE;
            case ImsRegistrationImpl.REGISTRATION_TECH_IWLAN:
                return IAosRegistrationListener.NetworkType.IWLAN;
            case ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM:
                return IAosRegistrationListener.NetworkType.CROSS_SIM;
            case ImsRegistrationImpl.REGISTRATION_TECH_NR:
                return IAosRegistrationListener.NetworkType.NR;
            case ImsRegistrationImpl.REGISTRATION_TECH_NONE: // FALL-THROUGH
            default:
                return IAosRegistrationListener.NetworkType.NONE;
        }
    }

    private int convertToAosCapability(int capability) {
        switch (capability) {
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                return IAosRegistrationListener.Capability.VOICE;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                return IAosRegistrationListener.Capability.VIDEO;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT:
                return IAosRegistrationListener.Capability.UT;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS:
                return IAosRegistrationListener.Capability.SMS;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER:
                return IAosRegistrationListener.Capability.CALL_COMPOSER;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE: // FALL-THROUGH
            default:
                return IAosRegistrationListener.Capability.NONE;
        }
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class MessageHandler extends Handler {
        private boolean mInitCompleted = false;
        public static final int EVENT_ROAMING_STATE_CHANGED = 1;

        MessageHandler() {
            super(mContext.getDefaultLooper());
            init();
        }

        public void init() {
            if (!mInitCompleted) {
                IDcNetWatcher dcnw = getDcNetWatcher(mContext.getSlotId());
                if (dcnw != null) {
                    dcnw.registerForRoamingStateChanged(this,
                            EVENT_ROAMING_STATE_CHANGED, null);
                }
                mInitCompleted = true;
            }
        }

        public void clear() {
            IDcNetWatcher dcnw = getDcNetWatcher(mContext.getSlotId());
            if (dcnw != null) {
                dcnw.unregisterForRoamingStateChanged(this);
            }
            mInitCompleted = false;
        }

        @Override
        public void handleMessage(Message msg) {
            logi("MessageHandler :: msg=" + msg.what);

            switch (msg.what) {
                case EVENT_ROAMING_STATE_CHANGED: {
                    CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                    if (capabilityPairs != null) {
                        mRegTracker.changeCapabilities(capabilityPairs);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    private class RegTracker implements IAosRegistrationListener,
            ImsStackRegistry.ImsServiceListener {
        private SparseArray<String> mFeatureTags;
        private int mNetworkType;
        private CapabilityUpdateListener mListener = null;
        private IAosRegistration mAosReg = null;

        public RegTracker() {
            init();
        }

        public void clear() {
            mNetworkType = IAosRegistrationListener.NetworkType.LTE;

            if (mAosReg != null) {
                mAosReg.removeListener(this);
            }
            ImsStackRegistry.removeImsServiceListener(this);
        }

        public void init() {
            mNetworkType = IAosRegistrationListener.NetworkType.LTE;
            mAosReg = getIAosRegistration(mContext.getSlotId());
            ImsStackRegistry.addImsServiceListener(this);

            initFeatureTags();

            if (mAosReg == null) {
                logi("ImsRegistrationTracker AosReg null");
                return;
            }
            mAosReg.addListener(this);
        }

        public void setListener(CapabilityUpdateListener listener) {
            mListener = listener;
        }

        public int getNetworkType() {
            return mNetworkType;
        }

        public boolean updateNetworkType(int networkType) {
            if (mNetworkType != networkType) {
                logi("RegTracker :: networkType - " + mNetworkType
                        + " >> " + networkType + "; phoneId=" + mContext.getPhoneId());

                mNetworkType = networkType;
                return true;
            }
            return false;
        }

        public boolean updateFeatures(int featureTagBits) {
            if (mFeatures != featureTagBits) {
                logi("RegTracker :: Features - " + mFeatures
                        + String.format("0x%08X", featureTagBits)
                        + "; phoneId=" + mContext.getPhoneId());

                mFeatures = featureTagBits;
                return true;
            }
            return false;
        }

        public void updateSipDelegateRegistration() {
            if (mAosReg != null) {
                mAosReg.updateSipDelegateRegistration();
            }
        }

        public void triggerSipDelegateDeregistration() {
            if (mAosReg != null) {
                mAosReg.triggerSipDelegateDeregistration();
            }
        }

        public void triggerFullNetworkRegistration(int sipCode, @Nullable String sipReason) {
            if (mAosReg != null) {
                mAosReg.triggerFullNetworkRegistration(sipCode, sipReason);
            }
        }

        public void changeCapabilities(CapabilityPairs capabilities) {
            if (mAosReg != null) {
                mAosReg.changeCapabilities(capabilities);
            }
        }

        public void controlRegistration(int reason) {
            mAosReg.controlRegistration(IAosRegistration.RequestType.STOP,
                    IAosRegistration.Pcscf.CURRENT, reason);
        }

        @Override
        public void notifyRegistered(int networkType, int featureTagBits,
                Set<String> featureTags) {
            logi("ImsRegistrationTracker notifyRegistered");

            int radioTech = convertToTelephonyNetworkType(networkType);

            if (featureTags.isEmpty()) {
                mRegImpl.notifyRegistered(radioTech, makeFeatureTags(featureTagBits));
            } else {
                mRegImpl.notifyRegistered(radioTech, featureTags);
            }

            boolean networkTypeChanged = updateNetworkType(networkType);

            boolean featureChanged = updateFeatures(featureTagBits);

            if (networkTypeChanged || featureChanged) {
                updateFeatureCapabilities();
            }
        }

        @Override
        public void notifyRegistering(int networkType, int featureTagBits,
                Set<String> featureTags) {
            logi("ImsRegistrationTracker notifyRegistering");

            int radioTech = convertToTelephonyNetworkType(networkType);

            if (featureTags.isEmpty()) {
                mRegImpl.notifyRegistering(radioTech, makeFeatureTags(featureTagBits));
            } else {
                mRegImpl.notifyRegistering(radioTech, featureTags);
            }
        }

        @Override
        public void notifyDeregistered(int networkType, int reason, String message) {
            logi("ImsRegistrationTracker notifyDeregistered - network=" + networkType
                    + ", reason =" + reason + ", message =" + message);
            int radioTech = convertToTelephonyNetworkType(networkType);
            mRegImpl.notifyDeregistered(radioTech, reason, message);
            boolean networkTypeChanged = updateNetworkType(
                    IAosRegistrationListener.NetworkType.NONE);
            boolean featureChanged = updateFeatures(FeatureTagMask.NONE);

            if (networkTypeChanged || featureChanged) {
                updateFeatureCapabilities();
            }
        }

        @Override
        public void notifyTechnologyChangeFailed(int networkType, int reason, String message) {
            int radioTech = convertToTelephonyNetworkType(networkType);

            mRegImpl.notifyTechnologyChangeFailed(radioTech, reason, message);
        }

        @Override
        public void notifyAssociatedUriChanged(Uri[] uris) {
            mRegImpl.notifyAssociatedUriChanged(uris);
        }

        @Override
        public void notifyCapabilitiesUpdateFailed(int capabilities, int networkType, int reason) {
            if (mListener != null) {
                int radioTech = convertToTelephonyNetworkType(networkType);
                int capability = convertToTelephonyCapability(capabilities);

                mListener.onCapabilitiesUpdateFailed(capability, radioTech,
                        ImsFeature.CAPABILITY_ERROR_GENERIC);
            }
        }

        @Override
        public void onImsServiceStarted(int slotId) {
            logi("onImsServiceStarted: slotId=" + slotId + ", mySlotId=" + mContext.getSlotId());

            if (slotId != mContext.getSlotId()) {
                return;
            }

            if (isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming()) {
                if (mHandler == null) {
                    mHandler = new MessageHandler();
                } else {
                    mHandler.init();
                }
                ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
                if (isr != null) {
                    ImsConfigImpl configImpl = isr.getConfig();
                    if (configImpl != null) {
                        if (mConfigListener == null) {
                            mConfigListener = new ConfigListener();
                        }
                        configImpl.addListener(mConfigListener);
                    }
                }
            }

            if (!isIgnoreDataEnabledChangedForVideoCalls()) {
                if (mHandler == null) {
                    mHandler = new MessageHandler();
                } else {
                    mHandler.init();
                }
                registerDataRoamingSettingObserver();
            }

            if (mAosReg == null) {
                mAosReg = getIAosRegistration(mContext.getSlotId());
                if (mAosReg != null) {
                    mAosReg.addListener(this);
                    CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                    if (capabilityPairs != null) {
                        changeCapabilities(capabilityPairs);
                    }
                }
            }
        }

        @Override
        public void onImsServiceStopped(int slotId) {
            logi("onImsServiceStopped: slotId=" + slotId);

            if (slotId != mContext.getSlotId()) {
                return;
            }

            if (getIAosRegistration(mContext.getSlotId()) != null) {
                mAosReg.removeListener(this);
            }
            mAosReg = null;

            if (mHandler != null) {
                mHandler.clear();
            }
        }

        private int convertToTelephonyCapability(int capability) {
            switch (capability) {
                case IAosRegistrationListener.Capability.VOICE:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
                case IAosRegistrationListener.Capability.VIDEO:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;
                case IAosRegistrationListener.Capability.UT:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;
                case IAosRegistrationListener.Capability.SMS:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
                case IAosRegistrationListener.Capability.CALL_COMPOSER:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER;
                case IAosRegistrationListener.Capability.NONE: // FALL-THROUGH
                default:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE;
            }
        }

        private int convertToTelephonyNetworkType(int networkType) {
            switch (networkType) {
                case IAosRegistrationListener.NetworkType.LTE:
                    return ImsRegistrationImpl.REGISTRATION_TECH_LTE;
                case IAosRegistrationListener.NetworkType.IWLAN:
                    return ImsRegistrationImpl.REGISTRATION_TECH_IWLAN;
                case IAosRegistrationListener.NetworkType.CROSS_SIM:
                    return ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM;
                case IAosRegistrationListener.NetworkType.NR:
                    return ImsRegistrationImpl.REGISTRATION_TECH_NR;
                case IAosRegistrationListener.NetworkType.UTRAN: // FALL-THROUGH
                default:
                    return ImsRegistrationImpl.REGISTRATION_TECH_NONE;
            }
        }

        private @NonNull Set<String> makeFeatureTags(int featureTagBits) {
            logi("ImsRegistrationTracker makeFeatureTags: "
                    + String.format("0x%08X", featureTagBits));
            Set<String> featureTags = new ArraySet<String>();

            for (int i = 0; i < mFeatureTags.size(); i++) {
                if ((featureTagBits & mFeatureTags.keyAt(i)) != 0) {
                    featureTags.add(mFeatureTags.valueAt(i));
                }
            }
            return featureTags;
        }

        private void initFeatureTags() {
            mFeatureTags = new SparseArray<>();

            mFeatureTags.put(FeatureTagMask.MMTEL,
                    "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"");
            mFeatureTags.put(FeatureTagMask.VIDEO, "video");
            mFeatureTags.put(FeatureTagMask.TEXT, "text");
            mFeatureTags.put(FeatureTagMask.USSI, "+g.3gpp.nw-init-ussi");
            mFeatureTags.put(FeatureTagMask.VERSTAT, "+g.3gpp.verstat");
            mFeatureTags.put(FeatureTagMask.SMSIP, "+g.3gpp.smsip");
            mFeatureTags.put(FeatureTagMask.STANDALONE_MSG,
                    "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-"
                    + "service.ims.icsi.oma.cpm.msg,urn%3Aurn-7%3A3gpp-"
                    + "service.ims.icsi.oma.cpm.largemsg,urn%3Aurn-7%3A3gpp-"
                    + "service.ims.icsi.oma.cpm.deferred\";+g.gsma.rcs.cpm.pager-large");
            mFeatureTags.put(FeatureTagMask.CHAT_IM,
                    "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im\"");
            mFeatureTags.put(FeatureTagMask.CHAT_SESSION, "+g.3gpp.icsi-ref="
                    + "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"");
            mFeatureTags.put(FeatureTagMask.FILE_TRANSFER, "+g.3gpp.iari-ref="
                    + "\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp\"");
            mFeatureTags.put(FeatureTagMask.FILE_TRANSFER_VIA_SMS,
                    "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms\"");
            mFeatureTags.put(FeatureTagMask.CALL_COMPOSER_ENRICHED_CALLING, "+g.3gpp.icsi-ref="
                    + "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer\"");
            mFeatureTags.put(FeatureTagMask.CALL_COMPOSER_VIA_TELEPHONY, "+g.gsma.callcomposer");
            mFeatureTags.put(FeatureTagMask.POST_CALL, "+g.3gpp.icsi-ref="
                    + "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered\"");
            mFeatureTags.put(FeatureTagMask.SHARED_MAP, "+g.3gpp.icsi-ref="
                    + "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap\"");
            mFeatureTags.put(FeatureTagMask.SHARED_SKETCH, "+g.3gpp.icsi-ref="
                    + "\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch\"");
            mFeatureTags.put(FeatureTagMask.GEO_PUSH, "+g.3gpp.iari-ref="
                    + "\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush\"");
            mFeatureTags.put(FeatureTagMask.GEO_PUSH_VIA_SMS, "+g.3gpp.iari-ref="
                    + "\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms\"");
            mFeatureTags.put(FeatureTagMask.CHATBOT_COMMUNICATION_USING_SESSION,
                    "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot\"");
            mFeatureTags.put(FeatureTagMask.CHATBOT_COMMUNICATION_USING_STANDALONE_MSG,
                    "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa\"");
            mFeatureTags.put(FeatureTagMask.CHATBOT_VERSION_SUPPORTED,
                    "+g.gsma.rcs.botversion=\"#=1\"");
            mFeatureTags.put(FeatureTagMask.CHATBOT_VERSION_V2_SUPPORTED,
                    "+g.gsma.rcs.botversion=\"#=1,#=2\"");
            mFeatureTags.put(FeatureTagMask.CHATBOT_ROLE, "+g.gsma.rcs.isbot");
            mFeatureTags.put(FeatureTagMask.PRESENCE,
                    "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp\"");
        }
    }
}
