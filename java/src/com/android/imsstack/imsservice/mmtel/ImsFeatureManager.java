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

import android.os.Handler;
import android.os.Message;
import android.telephony.ims.feature.MmTelFeature;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.IWifiState;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;
import com.android.imsstack.util.ImsLog;

public class ImsFeatureManager {
    private final Object mLock = new Object();
    private final IBaseContext mContext;
    private final IMmTelFeatureCapabilityListener mFeatureCapabilityListener;
    private IUtInterface mUt;
    private NetworkTracker mNetworkTracker;
    private ImsRegistrationTracker mRegTracker;
    private MmTelFeature.MmTelCapabilities mMmTelCapabilities
            = new MmTelFeature.MmTelCapabilities(0);

    private IUtServiceStateListener mUtServiceStateListener = new IUtServiceStateListener() {
        @Override
        public void onServiceStateChanged() {
            logi("Ut service state changed");
            updateAndNotifyFeatureCapabilitiesIfChanged();
        }
    };

    private IRegistrationFeatureListener mRegFeatureListener = new IRegistrationFeatureListener() {
        @Override
        public void onRegistrationFeatureChanged() {
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

    /**
     * IUtInterface is set for Ut operation.
     */
    public void setUtInterface(IUtInterface ut) {
        if (mUt != null) {
            mUt.setServiceStateListener(null);
        }

        mUt = ut;

        if (mUt != null) {
            mUt.setServiceStateListener(mUtServiceStateListener);
        }
    }

    /**
     * This is invoked to set ImsRegistrationTracker object.
     * And registers IRegistrationFeatureListener via the same object.
    */
    public void setRegistrationTracker(ImsRegistrationTracker regTracker) {
        if (mRegTracker != null) {
            mRegTracker.setRegistrationFeatureListener(null);
        }

        mRegTracker = regTracker;

        if (mRegTracker != null) {
            mRegTracker.setRegistrationFeatureListener(mRegFeatureListener);
        }
    }

    private void updateAndNotifyFeatureCapabilitiesIfChanged() {
        if (updateFeatures()) {
            logi("MmTel feature capabilities changed");
            mFeatureCapabilityListener.onFeatureCapabilityChanged(mMmTelCapabilities);
        }
    }

    private boolean updateFeatures() {
        // Update feature capabilities from the current state
        boolean voiceRegistered = false;
        boolean videoRegistered = false;
        boolean smsRegistered = false;

        if (mRegTracker != null) {
            voiceRegistered = mRegTracker.isCallVoiceRegistered();
            videoRegistered = mRegTracker.isCallVideoRegistered();
            smsRegistered = mRegTracker.isSmsRegistered();
        }

        updateFeatureCapabilityForVoice(voiceRegistered);
        updateFeatureCapabilityForVideo(videoRegistered);
        updateFeatureCapabilityForSms(smsRegistered);
        updateFeatureCapabilityForUt();

        log("updateMmTelCapabilities: " + mMmTelCapabilities);

        return true;
    }

    public void updateFeaturesOnServiceUpDown(boolean serviceConnected) {
        if (!serviceConnected) {
            disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
            disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
            disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT);
            disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS);

            log("updateMmTelCapabilities: " + mMmTelCapabilities);

            mFeatureCapabilityListener.onFeatureCapabilityChanged(mMmTelCapabilities);
            return;
        }

        updateAndNotifyFeatureCapabilitiesIfChanged();
    }

    private void disableFeature(int feature) {
        synchronized (mLock) {
            mMmTelCapabilities.removeCapabilities(feature);
        }
    }

    private void enableFeature(int feature) {
        synchronized (mLock) {
            mMmTelCapabilities.addCapabilities(feature);
        }
    }

    private void updateFeatureCapabilityForVideo(boolean registered) {
        disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO);

        if (registered) {
            enableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VIDEO);
        }
    }

    private void updateFeatureCapabilityForSms(boolean registered) {
        disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS);

        if (registered) {
            enableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_SMS);
        }
    }

    private void updateFeatureCapabilityForVoice(boolean registered) {
        disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);

        if (registered) {
            enableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_VOICE);
        }
    }

    private void updateFeatureCapabilityForUt() {
        disableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT);

        if (isAllSimAbsent()) {
            log("Ut :: No SIM inserted");
            return;
        }

        if (mUt == null) {
            log("Ut :: mUt is null");
            return;
        }

        if (mUt.isUtAvailable()) {
            enableFeature(MmTelFeature.MmTelCapabilities.CAPABILITY_TYPE_UT);
        }
    }

    private static IWifiState getWifiState() {
        return (IWifiState)AgentFactory.getAgent(AgentFactory.WIFI_STATE);
    }

    private static boolean isAllSimAbsent() {
        ISubscription isub = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        return (isub != null) ? isub.isAllSimAbsent() : true;
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
            IDcNetWatcher idnw = mContext.getDcNetWatcher();

            if (idnw != null) {
                idnw.unregisterForRatChanged(this);
            }

            IWifiState iws = getWifiState();

            if (iws != null) {
                iws.unregisterForWifiStateChanged(this);
            }
        }

        public void init() {
            IDcNetWatcher idnw = mContext.getDcNetWatcher();

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
