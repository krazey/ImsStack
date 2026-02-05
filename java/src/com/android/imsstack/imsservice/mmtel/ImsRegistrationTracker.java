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

import static android.provider.Settings.Global.DATA_ROAMING;

import android.database.ContentObserver;
import android.net.Uri;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;
import android.util.ArraySet;
import android.util.Pair;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.Capability;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.FeatureTagMask;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.NetworkType;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.RegistrationType;
import com.android.imsstack.imsservice.mmtel.config.base.ConfigUtils;
import com.android.imsstack.imsservice.mmtel.config.base.ConfigurationListener;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.LocalLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
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

    private static final int MAX_LOG_LINES = 50;
    private final LocalLog mLocalLog = new LocalLog(MAX_LOG_LINES);

    private final IContext mContext;
    private final ImsRegistrationImpl mRegImpl;
    private IRegistrationFeatureListener mFeatureListener;
    private RegTracker mRegTracker;
    private int mFeatures = FeatureTagMask.NONE;
    private List<Pair<Integer, Integer>> mCapabilities;
    private ConfigListener mConfigListener = null;
    private ContentObserver mDataRoamingSettingObserver = null;
    private IDcNetWatcher.Listener mNetWatcherListener;

    private class ConfigListener extends ConfigurationListener {
        @Override
        public void onImsConfigurationChanged(int item) {
            if ((item == ProvisioningManager.KEY_VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE)
                    || (item == ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE)
                    || (item == ProvisioningManager.KEY_RTT_ENABLED)) {
                if ((item == ProvisioningManager.KEY_RTT_ENABLED)
                        || isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming()) {
                    logi("onImsConfigurationChanged:: changed item" + item);

                    mLocalLog.log("ImsConfiguration Changed="
                            + ConfigUtils.getProvisioningItem(item));

                    CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                    if (capabilityPairs != null) {
                        mRegTracker.changeCapabilities(capabilityPairs);
                    }
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

        initialize();
    }

    public void dispose() {
        mRegTracker.notifyDeregistered(RegistrationType.NORMAL, mRegTracker.getNetworkType(),
                ReasonCode.UNSPECIFIED, null, 0);
        mRegImpl.setRegistrationTracker(null);
        mRegTracker.clear();
        mCapabilities.clear();

        cleanup();
    }

    public ImsRegistrationImpl getRegistration() {
        return mRegImpl;
    }

    /**
     * Returns the network type on which IMS is currently registered.
     *
     * @return The registered network type as a {@link NetworkType}, or
     *         {@code NetworkType.NONE} if IMS is not registered or the tracker is null.
     */
    public NetworkType getRegisteredNetworkType() {
        if (mRegTracker != null) {
            return mRegTracker.getNetworkType();
        } else {
            return NetworkType.NONE;
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
        cleanup();
        initialize();
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

    private void updateAvailableCapabilities(int featureTagBits) {
        int enabledFeatures = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE;
        int disabledFeatures = MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE;

        if (isCallVoiceRegistered()) {
            if ((featureTagBits & FeatureTagMask.MMTEL) != 0) {
                enabledFeatures |= MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
            } else {
                disabledFeatures |= MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
            }
        }

        if (isCallVideoRegistered()) {
            if ((featureTagBits & FeatureTagMask.VIDEO) != 0) {
                enabledFeatures |= MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;
            } else {
                disabledFeatures |= MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;
            }
        }

        if (isSmsRegistered()) {
            if ((featureTagBits & FeatureTagMask.SMSIP) != 0) {
                enabledFeatures |= MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
            } else {
                disabledFeatures |= MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
            }
        }

        if (enabledFeatures == MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE
                && disabledFeatures == MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE) {
            return;
        }

        synchronized (this) {
            if (mFeatureListener != null) {
                mFeatureListener.onAvailableFeatureChanged(enabledFeatures, disabledFeatures);
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
        logi("changeCapabilities::enabledCaps " + enabledCaps + " disabledCaps " + disabledCaps);

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

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull IndentingPrintWriter pw) {
        pw.println("Registration:");
        pw.increaseIndent();
        pw.println("registered=" + FeatureTagMask.toString(mFeatures));
        pw.println("networkType=" + getRegisteredNetworkType());
        pw.println("Most recent logs:");
        pw.increaseIndent();
        mLocalLog.dump(pw);
        pw.decreaseIndent();

        AosFactory.getInstance().dump(mContext.getSlotId(), pw);
        pw.decreaseIndent();
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
        return DcFactory.getDcAgent(IDcNetWatcher.class, slotId);
    }

    @VisibleForTesting
    protected ImsStackRegistry.ImsServiceListener getImsServiceListener() {
        return mRegTracker;
    }

    @VisibleForTesting
    protected void updateFeatures(int featureTagBits) {
        mRegTracker.updateFeatures(featureTagBits);
    }

    @VisibleForTesting
    protected void updateNetworkType(NetworkType networkType) {
        mRegTracker.updateNetworkType(networkType);
    }

    protected CapabilityPairs createCapabilityPairsFromCapabilities() {
        CapabilityPairs capabilityPairs = createSmsCapabilityPairs();

        if (mCapabilities.isEmpty() && capabilityPairs == null) {
            return null;
        }

        if (capabilityPairs == null) {
            capabilityPairs = new CapabilityPairs();
        }

        for (int i = 0; i < mCapabilities.size(); i++) {
            Pair<Integer, Integer> finalcapability = mCapabilities.get(i);
            NetworkType networkType = convertToAosNetworkType(finalcapability.first);
            int capability = convertToAosCapability(finalcapability.second);

            if (isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming()) {
                if ((networkType == NetworkType.IWLAN) && (capability == Capability.VOICE)) {
                    if (isVoiceRoaming() && isCellularPreferredMode()) {
                        continue;
                    }
                }
            }
            /*
             * Note : To support data roaming case, until framework supports.
             */
            if (!getBooleanCarrierConfig(
                    CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
                if (!isMobileDataEnabled()) {
                    if ((capability == Capability.VIDEO) && (networkType != NetworkType.IWLAN)) {
                        logi("Mobile data is off :: ignoring video capability");
                        continue;
                    }
                }
            }

            capabilityPairs.addCapability(networkType, capability);

            if ((capability == Capability.VOICE) && isRttSupported()) {
                ImsLog.d("Add text Capability");
                capabilityPairs.addCapability(networkType, Capability.TEXT);
            }
        }
        /*
         * Note : Add IWLAN-VIDEO capability when WFC-OFF.
         */
        if (getBooleanCarrierConfig(
                    CarrierConfig.ImsWfc.KEY_VIDEO_OVER_WIFI_SUPPORTED_WITHOUT_VOICE_BOOL)
                    && isVtEnabled()) {
            ImsLog.d("Add Capability - IWLAN-VIDEO when WFC-OFF");
            capabilityPairs.addCapability(NetworkType.IWLAN, Capability.VIDEO);
        }

        logi("Final Capabilities" + capabilityPairs);
        return capabilityPairs;
    }

    protected void initialize() {
        if (mConfigListener == null) {
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
            ImsConfigImpl configImpl = (isr != null) ? isr.getConfig() : null;
            if (configImpl != null) {
                mConfigListener = new ConfigListener();
                configImpl.addListener(mConfigListener);
            }
        }

        addNetWatcherListener();
        if (!getBooleanCarrierConfig(
                CarrierConfigManager.KEY_IGNORE_DATA_ENABLED_CHANGED_FOR_VIDEO_CALLS)) {
            registerDataRoamingSettingObserver();
        }
    }

    protected void cleanup() {
        if (mConfigListener != null) {
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
            ImsConfigImpl config = (isr != null) ? isr.getConfig() : null;
            if (config != null) {
                config.removeListener(mConfigListener);
            }
            mConfigListener = null;
        }

        removeNetWatcherListener();
        if (mDataRoamingSettingObserver != null) {
            AppContext.getInstance().getContentProviderProxy().getGlobalSettings()
                    .unregisterContentObserver(mDataRoamingSettingObserver);
            mDataRoamingSettingObserver = null;
        }
    }

    private CapabilityPairs createSmsCapabilityPairs() {
        if (getBooleanCarrierConfig(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL)) {
            int[] supportedRats = getSmsSupportedRats();

            if (supportedRats.length > 0) {
                CapabilityPairs capabilityPairs = new CapabilityPairs();

                for (int i = 0; i < supportedRats.length; i++) {
                    if (supportedRats[i] == AccessNetworkType.EUTRAN) {
                        capabilityPairs.addCapability(NetworkType.LTE, Capability.SMS);
                    } else if (supportedRats[i] == AccessNetworkType.UTRAN) {
                        capabilityPairs.addCapability(NetworkType.UTRAN, Capability.SMS);
                    } else if (supportedRats[i] == AccessNetworkType.NGRAN) {
                        capabilityPairs.addCapability(NetworkType.NR, Capability.SMS);
                    } else if (supportedRats[i] == AccessNetworkType.IWLAN) {
                        if (!isRoaming() || getBooleanCarrierConfig(CarrierConfig
                                .ImsSms.KEY_SUPPORT_SMS_CAPABILITY_IN_WIFI_ROAMING_BOOL)) {
                            capabilityPairs.addCapability(NetworkType.IWLAN, Capability.SMS);
                        }
                    }
                }
                logi("createSmsCapabilityPairs" + capabilityPairs);
                return capabilityPairs;
            }
        }
        return null;
    }

    private boolean getBooleanCarrierConfig(String key) {
        ConfigInterface config = getConfigInterface(mContext.getSlotId());
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return cc != null && cc.getBoolean(key);
    }

    private boolean isVtEnabled() {
        if (SystemProperties.getInt("persist.dbg.vt_avail_ovr"
                + Integer.toString(mContext.getPhoneId()), -1) == 1
                || SystemProperties.getInt("persist.dbg.vt_avail_ovr", -1) == 1) {
            return true;
        }

        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);
        if (ccmp == null) {
            return false;
        }

        PersistableBundle b = ccmp.getConfigForSubId(mContext.getSubId(),
                CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);

        return AppContext.getInstance().getResources().getBoolean(
                com.android.internal.R.bool.config_device_vt_available)
                && (b != null) && b.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
    }

    private boolean isRttSupported() {
        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
        if (isr != null) {
            ImsConfigImpl configImpl = isr.getConfig();
            if ((configImpl != null)
                    && (configImpl.getConfigInt(ProvisioningManager.KEY_RTT_ENABLED)
                            == ProvisioningManager.PROVISIONING_VALUE_ENABLED)) {
                if (isRoaming()) {
                    return getBooleanCarrierConfig(
                            CarrierConfigManager.KEY_RTT_SUPPORTED_WHILE_ROAMING_BOOL);
                }
                return true;
            }
        }
        return false;
    }

    private boolean isCellularPreferredMode() {
        ImsServiceRecord isr = ImsServiceManager.getServiceRecord(mContext.getPhoneId());
        if (isr != null) {
            ImsConfigImpl configImpl = isr.getConfig();
            if ((configImpl != null) && ((configImpl.getConfigInt(
                    ProvisioningManager.KEY_VOICE_OVER_WIFI_ROAMING_ENABLED_OVERRIDE)
                    == ProvisioningManager.PROVISIONING_VALUE_DISABLED)
                    || (configImpl.getConfigInt(
                            ProvisioningManager.KEY_VOICE_OVER_WIFI_MODE_OVERRIDE)
                    == ImsMmTelManager.WIFI_MODE_CELLULAR_PREFERRED))) {
                return true;
            }
        }
        return false;
    }

    private boolean isMobileDataEnabled() {
        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mContext.getSubId());
        if (isRoaming()) {
            return tmp.isDataRoamingEnabled() && tmp.isDataEnabled();
        }
        return tmp.isDataEnabled();
    }

    private boolean isVoiceRoaming() {
        IDcNetWatcher dcnw = getDcNetWatcher(mContext.getSlotId());
        return (dcnw != null) ? dcnw.isVoiceRoaming() : false;
    }

    private boolean isRoaming() {
        IDcNetWatcher dcnw = getDcNetWatcher(mContext.getSlotId());
        return (dcnw != null) ? dcnw.isRoaming() : false;
    }

    private int[] getSmsSupportedRats() {
        ConfigInterface config = getConfigInterface(mContext.getSlotId());
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        int[] supportedRats = null;

        if (cc != null) {
            supportedRats = cc.getIntArray(
                    CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);
        }

        return supportedRats != null ? supportedRats : new int[0];
    }

    private boolean isVoWifiCapabilitySupportedWhenWifiOnlyOrPreferredInRoaming() {
        ConfigInterface config = getConfigInterface(mContext.getSlotId());
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        return cc != null && cc.getBoolean(
                CarrierConfig.ImsWfc
                .KEY_SUPPORT_VOWIFI_CAPABILITY_WHEN_WIFI_ONLY_OR_PREFERRED_IN_ROAMING_BOOL);
    }

    private void registerDataRoamingSettingObserver() {
        if (mDataRoamingSettingObserver != null) {
            return;
        }

        mDataRoamingSettingObserver = new ContentObserver(mContext.getDefaultHandler()) {
            @Override
            public void onChange(boolean bChange) {
                mLocalLog.log("DataRoamingSetting changed=" + bChange);

                CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                if (capabilityPairs != null) {
                    mRegTracker.changeCapabilities(capabilityPairs);
                }
            }
        };

        AppContext.getInstance().getContentProviderProxy().getGlobalSettings()
                .registerContentObserver(DATA_ROAMING, mDataRoamingSettingObserver);
    }
    /**
     * Call aos to send deregistration
     *
     */
    public void onDeregistrationTriggered(int reason) {
        logi("DeregistrationTriggered :: request from framework" + reason);
        mLocalLog.log("DeregistrationTriggered :: request from framework="
                + IAosRegistration.Cause.of(reason));

        mRegTracker.controlRegistration(reason);
    }

    private NetworkType convertToAosNetworkType(int radioTech) {
        switch (radioTech) {
            case ImsRegistrationImpl.REGISTRATION_TECH_LTE:
                return NetworkType.LTE;
            case ImsRegistrationImpl.REGISTRATION_TECH_IWLAN:
                return NetworkType.IWLAN;
            case ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM:
                return NetworkType.CROSS_SIM;
            case ImsRegistrationImpl.REGISTRATION_TECH_NR:
                return NetworkType.NR;
            case ImsRegistrationImpl.REGISTRATION_TECH_NONE: // FALL-THROUGH
            default:
                return NetworkType.NONE;
        }
    }

    private int convertToAosCapability(int capability) {
        switch (capability) {
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                return Capability.VOICE;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                return Capability.VIDEO;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT:
                return Capability.UT;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS:
                return Capability.SMS;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER:
                return Capability.CALL_COMPOSER;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER_BUSINESS_ONLY:
                return Capability.CALL_COMPOSER_BUSINESS_ONLY;
            case MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE: // FALL-THROUGH
            default:
                return Capability.NONE;
        }
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private void addNetWatcherListener() {
        if (mNetWatcherListener == null) {
            IDcNetWatcher dnw = getDcNetWatcher(mContext.getSlotId());
            if (dnw != null) {
                mNetWatcherListener = new NetWatcherListener();
                dnw.addListener(mNetWatcherListener);
            }
        }
    }

    private void removeNetWatcherListener() {
        if (mNetWatcherListener != null) {
            IDcNetWatcher dnw = getDcNetWatcher(mContext.getSlotId());
            if (dnw != null) {
                dnw.removeListener(mNetWatcherListener);
            }
            mNetWatcherListener = null;
        }
    }

    private class NetWatcherListener implements IDcNetWatcher.Listener {
        @Override
        public void onRoamingStateChanged(boolean roaming) {
            mContext.getDefaultHandler().post(() -> {
                mLocalLog.log("RoamingStateChanged: roaming=" + roaming);

                CapabilityPairs capabilityPairs = createCapabilityPairsFromCapabilities();
                if (capabilityPairs != null) {
                    mRegTracker.changeCapabilities(capabilityPairs);
                }
            });
        }
    }

    private class RegTracker implements IAosRegistrationListener,
            ImsStackRegistry.ImsServiceListener {
        private SparseArray<String> mFeatureTags;
        private NetworkType mNetworkType;
        private CapabilityUpdateListener mListener = null;
        private IAosRegistration mAosReg = null;

        public RegTracker() {
            init();
        }

        public void clear() {
            mNetworkType = NetworkType.LTE;

            if (mAosReg != null) {
                mAosReg.removeListener(this);
            }
            ImsStackRegistry.removeImsServiceListener(this);
        }

        public void init() {
            mNetworkType = NetworkType.LTE;
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

        public NetworkType getNetworkType() {
            return mNetworkType;
        }

        public boolean updateNetworkType(NetworkType networkType) {
            if (mNetworkType != networkType) {
                logi("RegTracker :: networkType - " + mNetworkType.toString()
                        + " >> " + networkType.toString() + "; phoneId=" + mContext.getPhoneId());

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
                    IAosRegistration.Pcscf.CURRENT, IAosRegistration.Cause.of(reason));
        }

        @Override
        public void notifyRegistered(@RegistrationType.RegistrationTypeDef int regType,
                NetworkType networkType, @FeatureTagMask.FeatureTagMaskDef int featureTagBits,
                Set<String> featureTags) {
            logi("ImsRegistrationTracker notifyRegistered");

            int radioTech = convertToTelephonyNetworkType(networkType);

            if (featureTags.isEmpty()) {
                mRegImpl.notifyRegistered(regType, radioTech, makeFeatureTags(featureTagBits));
            } else {
                mRegImpl.notifyRegistered(regType, radioTech, featureTags);
            }

            if (regType == RegistrationType.NORMAL) {
                boolean networkTypeChanged = updateNetworkType(networkType);
                boolean featureChanged = updateFeatures(featureTagBits);

                if (networkTypeChanged || featureChanged) {
                    updateFeatureCapabilities();
                }
            }
        }

        @Override
        public void notifyRegistering(@RegistrationType.RegistrationTypeDef int regType,
                NetworkType networkType, @FeatureTagMask.FeatureTagMaskDef int featureTagBits,
                Set<String> featureTags) {
            logi("ImsRegistrationTracker notifyRegistering");

            int radioTech = convertToTelephonyNetworkType(networkType);

            if (featureTags.isEmpty()) {
                mRegImpl.notifyRegistering(regType, radioTech, makeFeatureTags(featureTagBits));
            } else {
                mRegImpl.notifyRegistering(regType, radioTech, featureTags);
            }
        }

        @Override
        public void notifyDeregistered(
                @RegistrationType.RegistrationTypeDef int regType, NetworkType networkType,
                ReasonCode reason, String message, int dataFailCause) {
            logi("ImsRegistrationTracker notifyDeregistered - type=" + regType + ", network="
                    + networkType.toString() + ", reason =" + reason.toString()
                    + ", message =" + message + ", dataFailCause =" + dataFailCause);
            int radioTech = convertToTelephonyNetworkType(networkType);
            mRegImpl.notifyDeregistered(regType, radioTech, reason, message, dataFailCause);

            if (regType == RegistrationType.NORMAL) {
                boolean networkTypeChanged = updateNetworkType(NetworkType.NONE);
                boolean featureChanged = updateFeatures(FeatureTagMask.NONE);

                if (networkTypeChanged || featureChanged) {
                    updateFeatureCapabilities();
                }
            }
        }

        @Override
        public void notifyDeregistering(@RegistrationType.RegistrationTypeDef int regType) {
            if (regType != RegistrationType.NORMAL) {
                logi("notifyDeregistering is ignored");
                return;
            }

            if (updateFeatures(FeatureTagMask.NONE)) {
                updateFeatureCapabilities();
            }
        }

        @Override
        public void notifyTechnologyChangeFailed(
                @RegistrationType.RegistrationTypeDef int regType, NetworkType networkType,
                ReasonCode reason, String message) {
            int radioTech = convertToTelephonyNetworkType(networkType);

            mRegImpl.notifyTechnologyChangeFailed(regType, radioTech, reason, message);
        }

        @Override
        public void notifyAssociatedUriChanged(Uri[] uris) {
            mRegImpl.notifyAssociatedUriChanged(uris);
        }

        @Override
        public void notifyCapabilitiesUpdateFailed(@Capability.CapabilityMask int capabilities,
                NetworkType networkType, @CapabilityReason.CapabilityReasonDef int reason) {
            if (mListener != null) {
                int radioTech = convertToTelephonyNetworkType(networkType);
                int capability = convertToTelephonyCapability(capabilities);

                mListener.onCapabilitiesUpdateFailed(capability, radioTech,
                        ImsFeature.CAPABILITY_ERROR_GENERIC);
            }
        }

        @Override
        public void notifyImsFeatureChanged(
                @RegistrationType.RegistrationTypeDef int regType, NetworkType networkType,
                @FeatureTagMask.FeatureTagMaskDef int featureTagBits) {
            logi("notifyImsFeatureChanged: type=" + regType
                    + ", network=" + networkType.toString() + ", features=" + featureTagBits);

            if (regType != RegistrationType.NORMAL || mFeatures == featureTagBits
                    || !Objects.equals(networkType, mNetworkType)) {
                return;
            }
            updateAvailableCapabilities(featureTagBits);
        }

        @Override
        public void notifyCapabilitiesUpdated(IAosRegistration.CapabilityPairs pairs) {
            // Do nothing.
        }

        @Override
        public void notifyRegEventStateChanged(int statusCode, @NonNull Set<Uri> impus) {
            logi("notifyRegEventStateChanged: statusCode=" + statusCode
                    + ", size of impus=" + impus.size());
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class,
                    mContext.getSlotId());
            UsatInterface usat = (sim != null) ? sim.getUsatInterface() : null;
            if (usat != null && isUsatRegEventDownloadRequired(sim, usat)) {
                UsatBasedRegEvent event = new UsatBasedRegEvent();
                event.mRegEventDownloadCmd = usat.createRegEventDownloadCommand(
                        statusCode, impus, event);
                usat.sendCommand(event.mRegEventDownloadCmd);
            }
        }

        @Override
        public void onImsServiceStarted(int slotId) {
            logi("onImsServiceStarted: slotId=" + slotId + ", mySlotId=" + mContext.getSlotId());

            if (slotId != mContext.getSlotId()) {
                return;
            }

            initialize();

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

            if (isRegistered()) {
                notifyDeregistered();
            }

            if (getIAosRegistration(mContext.getSlotId()) != null) {
                mAosReg.removeListener(this);
            }
            mAosReg = null;

            cleanup();
        }

        private void notifyDeregistered() {
            logi("notifyDeregistered: slotId=" + mContext.getSlotId());
            mRegImpl.notifyDeregistered(RegistrationType.NORMAL,
                    convertToTelephonyNetworkType(getNetworkType()),
                            ReasonCode.UNSPECIFIED, null, 0);
            updateNetworkType(NetworkType.NONE);
            updateFeatures(FeatureTagMask.NONE);
        }

        private boolean isUsatRegEventDownloadRequired(
                @NonNull SimInterface sim, @NonNull UsatInterface usat) {
            ConfigInterface config = getConfigInterface(mContext.getSlotId());
            CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
            if (cc == null) {
                return false;
            }

            return switch (cc.getInt(CarrierConfig.Ims.KEY_USAT_REG_EVENT_DOWNLOAD_POLICY_INT)) {
                case CarrierConfig.Ims.USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD -> true;
                case CarrierConfig.Ims.USAT_REG_EVENT_NOT_DOWNLOAD -> false;
                case CarrierConfig.Ims.USAT_REG_EVENT_CONDITIONAL_DOWNLOAD ->
                        (usat.isInSetupEventList(Usat.SETUP_EVENT_IMS_REGISTRATION)
                                && !sim.getUiccIari().isEmpty());
                default -> false;
            };
        }

        private int convertToTelephonyCapability(int capability) {
            switch (capability) {
                case Capability.VOICE:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE;
                case Capability.VIDEO:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO;
                case Capability.UT:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT;
                case Capability.SMS:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS;
                case Capability.CALL_COMPOSER:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_CALL_COMPOSER;
                case Capability.CALL_COMPOSER_BUSINESS_ONLY:
                    return MmTelFeature.MmTelCapabilities
                            .CAPABILITY_TYPE_CALL_COMPOSER_BUSINESS_ONLY;
                case Capability.NONE: // FALL-THROUGH
                default:
                    return MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_NONE;
            }
        }

        private int convertToTelephonyNetworkType(NetworkType networkType) {
            switch (networkType) {
                case LTE:
                    return ImsRegistrationImpl.REGISTRATION_TECH_LTE;
                case IWLAN:
                    return ImsRegistrationImpl.REGISTRATION_TECH_IWLAN;
                case CROSS_SIM:
                    return ImsRegistrationImpl.REGISTRATION_TECH_CROSS_SIM;
                case NR:
                    return ImsRegistrationImpl.REGISTRATION_TECH_NR;
                case UTRAN: // FALL-THROUGH
                case NONE: // FALL-THROUGH
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

        private class UsatBasedRegEvent implements Usat.Listener {
            public Usat.RegEventDownloadCommand mRegEventDownloadCmd = null;

            UsatBasedRegEvent() {}

            @Override
            public void onCommandResponse(Usat.CommandResponse response) {
                // Do nothing.
            }
        }
    }
}
