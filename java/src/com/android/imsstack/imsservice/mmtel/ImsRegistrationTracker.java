package com.android.imsstack.imsservice.mmtel;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.Uri;
import android.telephony.ims.ProvisioningManager;
import android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;
import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener.FeatureTagMask;
import com.android.imsstack.imsservice.mmtel.config.base.ConfigurationListener;
import com.android.imsstack.util.ImsLog;

import java.util.List;
import java.util.Set;


public final class ImsRegistrationTracker {
    public static interface CapabilityUpdateListener {
        public void onCapabilitiesUpdateFailed(int capabilities, int networkType, int reason);
    };

    private final IContext mContext;
    private final ImsRegistrationImpl mRegImpl;
    private ImsFeatureManager mFeatureManager;
    private RegTracker mRegTracker;
    private MmTelCapabilityTracker mMmTelCapabilityTracker;
    private int mFeatures = FeatureTagMask.NONE;

    public ImsRegistrationTracker(IContext context, ImsRegistrationImpl regImpl,
            ImsConfigImpl config) {
        mContext = context;
        mRegImpl = regImpl;
        mFeatureManager = null;
        mFeatures = FeatureTagMask.NONE;
        mMmTelCapabilityTracker = new MmTelCapabilityTracker(config);
        mRegTracker = new RegTracker();

        mRegImpl.setRegistrationTracker(this);
    }

    public void dispose() {
        // FIXME: use proper reason
        mRegImpl.notifyDeregistered(0);
        mRegImpl.setRegistrationTracker(null);
        mRegTracker.clear();
        mMmTelCapabilityTracker.clear();
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
        return mMmTelCapabilityTracker.isCallRegistered();
    }

    public boolean isCallVideoRegistered() {
        logi("ImsRegistrationTracker isCallVideoRegistered");
        return mMmTelCapabilityTracker.isVideoRegistered();
    }

    public boolean isCallVoiceRegistered() {
        logi("ImsRegistrationTracker isCallVoiceRegistered");
        return mMmTelCapabilityTracker.isVoiceRegistered();
    }

    public boolean isCallVoiceAndVideoRegistered() {
        return mMmTelCapabilityTracker.isVoiceAndVideoRegistered();
    }

    public boolean isCallVideoSupported() {
        return mMmTelCapabilityTracker.isVideoSupported();
    }

    public boolean isCallVoiceSupported() {
        return mMmTelCapabilityTracker.isVoiceSupported();
    }

    public boolean isWifiCallSupported() {
        return mMmTelCapabilityTracker.isWfcEnabled();
    }

    public void refreshCallRegistrationState() {
        mMmTelCapabilityTracker.clear();
        mMmTelCapabilityTracker.init();
    }

    public void setFeatureManager(ImsFeatureManager featureManager) {
        synchronized (this) {
            mFeatureManager = featureManager;
            if (mFeatureManager != null) {
                mFeatureManager.setRegistrationTracker(this);
            }
        }
    }

    private void updateFeatureCapabilities() {
        synchronized (this) {
            if (mFeatureManager != null) {
                mFeatureManager.updateAndNotifyFeatureCapabilitiesIfChanged();
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
        logi("changeCapabilities::enabledCaps.size " + enabledCaps.size() +
                " disabledCaps.size " + disabledCaps.size());
        CapabilityPairs capabilityPairs = new CapabilityPairs();
        for (int i = 0; i < enabledCaps.size(); ++i) {
            CapabilityPair capability = enabledCaps.get(i);
            capabilityPairs.addCapability(capability.getRadioTech(), capability.getCapability());
        }
        mRegTracker.changeCapabilities(capabilityPairs);
    }


    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void loge(String s) {
        ImsLog.e("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class RegTracker implements IAosRegistrationListener, ICommonPackageListener {
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
            CommonStarter.getInstance().removeListener(this);
        }

        public void init() {
            mNetworkType = IAosRegistrationListener.NetworkType.LTE;
            mAosReg = AosFactory.getInstance().getAosRegistration(mContext.getSlotId());
            CommonStarter.getInstance().addListener(this);

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
            mAosReg.updateSipDelegateRegistration();
        }

        public void triggerSipDelegateDeregistration() {
            mAosReg.triggerSipDelegateDeregistration();
        }

        public void triggerFullNetworkRegistration(int sipCode, @Nullable String sipReason) {
            mAosReg.triggerFullNetworkRegistration(sipCode, sipReason);
        }

        public void changeCapabilities(CapabilityPairs capabilities) {
            mAosReg.changeCapabilities(capabilities);
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
        public void notifyDeregistered(int reason) {
            mRegImpl.notifyDeregistered(reason);
        }

        @Override
        public void notifyTechnologyChangeFailed(int networkType, int reason) {
            mRegImpl.notifyTechnologyChangeFailed(networkType, reason);
        }

        @Override
        public void notifyAssociatedUriChanged(Uri[] uris) {
            mRegImpl.notifyAssociatedUriChanged(uris);
        }

        @Override
        public void notifyCapabilitiesUpdateFailed(int capabilities, int networkType, int reason) {
            if (mListener != null) {
                mListener.onCapabilitiesUpdateFailed(capabilities, networkType, reason);
            }
        }

        @Override
        public void onCommonPackageReady(int slotId) {
            logi("onCommonPackageReady :: slotId=" + slotId
                        + ", mySlotId=" + mContext.getSlotId());

            if (slotId != mContext.getSlotId()) {
                return;
            }

            if (mAosReg == null) {
                mAosReg = AosFactory.getInstance().getAosRegistration(mContext.getSlotId());
                if (mAosReg != null) {
                    mAosReg.addListener(this);
                }
            }
        }

        @Override
        public void onCommonPackageStop(int slotId) {
            logi("onCommonPackageStop :: slotId=" + slotId);
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

    private class MmTelCapabilityTracker extends ConfigurationListener {
        private boolean mVoiceEnabled;
        private boolean mVideoEnabled;
        private boolean mWfcEnabled;
        private final ImsConfigImpl mConfig;

        public MmTelCapabilityTracker(ImsConfigImpl config) {
            mConfig = config;
            init();
        }

        public void clear() {
            mVoiceEnabled = false;
            mVideoEnabled = false;
            mWfcEnabled = false;
            mConfig.removeListener(this);
        }

        public void init() {
            mConfig.addListener(this);

            updateProvisioningValues(ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS);
            updateProvisioningValues(ProvisioningManager.KEY_VT_PROVISIONING_STATUS);
            updateProvisioningValues(ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE);

            log("voiceEnabled=" + mVoiceEnabled
                    + ", videoEnabled=" + mVideoEnabled
                    + ", wfcEnabled=" + mWfcEnabled);
        }

        public boolean isVideoRegistered() {
            return ((mFeatures & FeatureTagMask.VIDEO) != 0);
        }

        public boolean isVoiceRegistered() {
            return ((mFeatures & FeatureTagMask.MMTEL) != 0);
        }

        public boolean isVoiceAndVideoRegistered() {
            return (isVoiceRegistered() && isVideoRegistered());
        }

        public boolean isCallRegistered() {
            return (isVoiceRegistered() || isVideoRegistered());
        }

        public boolean isVideoSupported() {
            return mVideoEnabled;
        }

        public boolean isVoiceSupported() {
            return mVoiceEnabled || mWfcEnabled;
        }

        public boolean isWfcEnabled() {
            return mWfcEnabled;
        }

        @Override
        public void onImsConfigurationChanged(int item) {
            updateProvisioningValues(item);
        }

        private void updateProvisioningValues(int item) {
            switch (item) {
                case ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS:
                    if (mConfig.getConfigInt(ProvisioningManager.KEY_VOLTE_PROVISIONING_STATUS) ==
                            ProvisioningManager.PROVISIONING_VALUE_ENABLED) {
                        mVoiceEnabled = true;
                    } else {
                        mVoiceEnabled = false;
                    }
                    break;
                case ProvisioningManager.KEY_VT_PROVISIONING_STATUS:
                    if (mConfig.getConfigInt(ProvisioningManager.KEY_VT_PROVISIONING_STATUS) ==
                            ProvisioningManager.PROVISIONING_VALUE_ENABLED) {
                        mVideoEnabled = true;
                    } else {
                        mVideoEnabled = false;
                    }
                    break;
                case ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE:
                    if (mConfig.getConfigInt(
                            ProvisioningManager.KEY_VOICE_OVER_WIFI_ENABLED_OVERRIDE)
                            == ProvisioningManager.PROVISIONING_VALUE_ENABLED) {
                        mWfcEnabled = true;
                    } else {
                        mWfcEnabled = false;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}
