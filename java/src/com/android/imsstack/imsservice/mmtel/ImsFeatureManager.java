package com.android.imsstack.imsservice.mmtel;

import android.os.Handler;
import android.os.Message;
import android.telephony.ims.feature.MmTelFeature;

import com.android.ims.ImsConfig;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.agentif.IWifiState;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterface;
import com.android.imsstack.util.ImsLog;

import java.util.Arrays;

public class ImsFeatureManager {
    private final Object mLock = new Object();
    private final IBaseContext mContext;
    private final IMmTelFeatureCapabilityListener mFeatureCapabilityListener;
    private UtInterface mUt;
    private NetworkTracker mNetworkTracker;
    private ImsRegistrationTracker mRegTracker;
    private MmTelFeature.MmTelCapabilities mMmTelCapabilities
            = new MmTelFeature.MmTelCapabilities(0);

    private int[] mEnabledFeatures = {
        /** Voice over LTE */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Video over LTE */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Voice over WiFi */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Video over WiFi */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Ut over LTE */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Ut over WiFi */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN
    };

    private int[] mDisabledFeatures = {
        /** Voice over LTE */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Video over LTE */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Voice over WiFi */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Video over WiFi */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Ut over LTE */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN,
        /** Ut over WiFi */
        ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN
    };

    private IUtServiceStateListener mUtServiceStateListener = new IUtServiceStateListener() {
        @Override
        public void onServiceStateChanged() {
            logi("Ut service state changed");
            updateAndNotifyFeatureCapabilitiesIfChanged();
        }
    };

    public ImsFeatureManager(IBaseContext context,
            IMmTelFeatureCapabilityListener featureCapabilityListener) {
        mContext = context;
        mFeatureCapabilityListener = featureCapabilityListener;
        mNetworkTracker = new NetworkTracker();
     }

    public void dispose() {
        if (mUt != null) {
            mUt.setServiceStateListener(null);
            mUt = null;
        }

        if (mNetworkTracker != null) {
            mNetworkTracker.clear();
            mNetworkTracker = null;
        }
    }

    public int[] cloneDisabledFeatures() {
        synchronized (mLock) {
            try {
                int[] features = Arrays.copyOf(mDisabledFeatures, mDisabledFeatures.length);
                return (features != null) ? features : mDisabledFeatures;
            } catch (Throwable t) {
                return mDisabledFeatures;
            }
        }
    }

    public int[] cloneEnabledFeatures() {
        synchronized (mLock) {
            try {
                int[] features = Arrays.copyOf(mEnabledFeatures, mEnabledFeatures.length);
                return (features != null) ? features : mEnabledFeatures;
            } catch (Throwable t) {
                return mEnabledFeatures;
            }
        }
    }

    public int[] getDisabledFeatures() {
        return mDisabledFeatures;
    }

    public int[] getEnabledFeatures() {
        return mEnabledFeatures;
    }

    public void setUtInterface(UtInterface ut) {
        if (mUt != null) {
            mUt.setServiceStateListener(null);
        }

        mUt = ut;

        if (mUt != null) {
            mUt.setServiceStateListener(mUtServiceStateListener);
        }
    }

    public void setRegistrationTracker(ImsRegistrationTracker RegTracker) {
        mRegTracker = RegTracker;
    }

    public void updateAndNotifyFeatureCapabilitiesIfChanged() {
        if (updateFeatures()) {
            logi("MmTel feature capabilities changed");
            mFeatureCapabilityListener.onFeatureCapabilityChanged(mMmTelCapabilities);
        }
    }

    public boolean updateFeatures() {
        // Update feature capabilities from the current state
        boolean voiceRegistered = false;
        boolean videoRegistered = false;
        int networkType = IAosRegistrationListener.NetworkType.NONE;

        if (mRegTracker != null) {
            voiceRegistered = mRegTracker.isCallVoiceRegistered();
            videoRegistered = mRegTracker.isCallVideoRegistered();
            networkType = mRegTracker.getRegisteredNetworkType();
        }

        if (networkType == IAosRegistrationListener.NetworkType.NONE) {
            networkType = IAosRegistrationListener.NetworkType.LTE;
        }

        updateFeaturesForVoice(voiceRegistered, networkType);
        updateFeaturesForVideo(videoRegistered, networkType);
        updateFeaturesForUt();

        updateMmTelCapabilities();

        return true;
    }

    public void updateFeaturesOnServiceUpDown(boolean serviceConnected) {
        if (!serviceConnected) {
            disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE);
            disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI);
            disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE);
            disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI);
            disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE);
            disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI);

            updateMmTelCapabilities();

            mFeatureCapabilityListener.onFeatureCapabilityChanged(mMmTelCapabilities);
            return;
        }

        updateAndNotifyFeatureCapabilitiesIfChanged();
    }

    private void disableFeature(int feature) {
        synchronized (mLock) {
            mEnabledFeatures[feature] = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
            mDisabledFeatures[feature] = feature;
        }
    }

    private void enableFeature(int feature) {
        synchronized (mLock) {
            mEnabledFeatures[feature] = feature;
            mDisabledFeatures[feature] = ImsConfig.FeatureConstants.FEATURE_TYPE_UNKNOWN;
        }
    }

    private void updateFeaturesForVideo(boolean registered, int networkType) {
        disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE);
        disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI);

        if (registered) {
            enableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE);
            enableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI);
        }
    }

    private void updateFeaturesForVoice(boolean registered, int networkType) {
        disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE);
        disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI);

        if (registered) {
            enableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE);
            enableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI);
        }
    }

    private void updateFeaturesForUt() {
        disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE);
        disableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI);

        if (isAllSimAbsent()) {
            log("Ut :: No SIM inserted");
            return;
        }

        if (mUt == null) {
            log("Ut :: mUt is null");
            return;
        }

        if (mUt.isUtAvailable()) {
            enableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE);
            enableFeature(ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI);
        }
    }

    private void updateMmTelCapabilities() {
        synchronized (mLock) {
            for (int feature : mEnabledFeatures) {
                switch (feature) {
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE: // FALL-THROUGH
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                    mMmTelCapabilities.addCapabilities(
                            MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE: // FALL-THROUGH
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                    mMmTelCapabilities.addCapabilities(
                            MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE: // FALL-THROUGH
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI:
                    mMmTelCapabilities.addCapabilities(
                            MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT);
                    break;
                default:
                    // no-op
                    break;
                }
            }

            log("1 : " + mMmTelCapabilities);

            for (int feature : mDisabledFeatures) {
                switch (feature) {
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_LTE: // FALL-THROUGH
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VOICE_OVER_WIFI:
                    mMmTelCapabilities.removeCapabilities(
                            MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE: // FALL-THROUGH
                case ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_WIFI:
                    mMmTelCapabilities.removeCapabilities(
                            MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
                    break;
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_LTE: // FALL-THROUGH
                case ImsConfig.FeatureConstants.FEATURE_TYPE_UT_OVER_WIFI:
                    mMmTelCapabilities.removeCapabilities(
                            MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT);
                    break;
                default:
                    // no-op
                    break;
                }
            }

            log("2 : " + mMmTelCapabilities);
        }
    }

    private static IWifiState getWifiState() {
        return (IWifiState)AgentFactory.getAgent(AgentFactory.WIFI_STATE);
    }

    private static boolean isAllSimAbsent() {
        ISubscription isub = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        return (isub != null) ? isub.isAllSimAbsent() : true;
    }

    private static boolean isWifiConnected() {
        IWifiState iws = getWifiState();
        return (iws != null) ? iws.isWifiConnected() : false;
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    private class NetworkTracker extends Handler {
        private static final int EVENT_RAT_CHANGED = 1;
        private static final int EVENT_WIFI_STATE_CHANGED = 2;

        public NetworkTracker() {
            super(mContext.getDefaultLooper());
            init();
        }

        public void clear() {
            IDCNetWatcher idnw = mContext.getDCNetWatcher();

            if (idnw != null) {
                idnw.unregisterForRatChanged(this);
            }

            IWifiState iws = getWifiState();

            if (iws != null) {
                iws.unregisterForWifiStateChanged(this);
            }
        }

        public void init() {
            IDCNetWatcher idnw = mContext.getDCNetWatcher();

            if (idnw != null) {
                idnw.registerForRatChanged(this, EVENT_RAT_CHANGED, null);
            }

            IWifiState iws = getWifiState();

            if (iws != null) {
                iws.registerForWifiStateChanged(this, EVENT_WIFI_STATE_CHANGED, null);
            }
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RAT_CHANGED: {
                    logi("RAT changed");

                    updateAndNotifyFeatureCapabilitiesIfChanged();
                    break;
                }
                case EVENT_WIFI_STATE_CHANGED: {
                    logi("WiFi state changed");

                    updateAndNotifyFeatureCapabilitiesIfChanged();
                    break;
                }
                default:
                    // no-op
                    break;
            }
        }
    }

}
