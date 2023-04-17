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

import android.content.Context;
import android.text.TextUtils;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.SubscriptionListener;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.core.config.FeatureTable;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.MessageExecutor;
import com.android.internal.annotations.VisibleForTesting;

import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

public class ImsServiceManager {
    private static ImsServiceManager sServiceManager;
    private final Context mContext;
    private final MessageExecutor mExecutor;
    protected final SubscriptionListenerProxy mSubscriptionListener;
    protected final CommonPackageListener mCommonPackageListener;
    // PhoneId -> ImsServiceRecord
    private ConcurrentHashMap<Integer, ImsServiceRecord> mServiceRecords
            = new ConcurrentHashMap<Integer, ImsServiceRecord>(4, 0.9f, 1);
    // ServiceId -> ImsCallApp
    private ConcurrentHashMap<Integer, ImsCallApp> mCallApps
            = new ConcurrentHashMap<Integer, ImsCallApp>(4, 0.9f, 1);
    private int mDefaultPhoneId = MSimUtils.DEFAULT_PHONE_ID;
    // Operator changed by hotswap
    private final String[] mOperator;
    private final String[] mCountry;
    protected final int[] mServiceFeatures;
    private final int[] mVoLteServiceFeatures;

    public ImsServiceManager(Context context, MessageExecutor executor) {
        mContext = context;
        mExecutor = executor;
        mDefaultPhoneId = getActivePhoneId();

        int supportedSimCount = MSimUtils.getSupportedSimCount();

        mOperator = new String[supportedSimCount];
        mCountry = new String[supportedSimCount];
        mServiceFeatures = new int[supportedSimCount];
        mVoLteServiceFeatures = new int[supportedSimCount];

        for (int i = 0; i < supportedSimCount; i++) {
            mOperator[i] = null;
            mCountry[i] = null;
            mServiceFeatures[i] = 0;
            mVoLteServiceFeatures[i] = 0;
        }

        createServiceRecords();

        mSubscriptionListener = new SubscriptionListenerProxy();
        ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);

        if (subs != null) {
            subs.addListener(mSubscriptionListener);
        }

        mCommonPackageListener = new CommonPackageListener();
        CommonStarter.getInstance().addListener(mCommonPackageListener);
    }

    public static ImsServiceManager getDefault() {
        return sServiceManager;
    }

    public static void setDefault(ImsServiceManager sm) {
        sServiceManager = sm;
    }

    public static ImsServiceRecord getServiceRecord(int phoneId) {
        ImsServiceManager sm = getDefault();
        return (sm != null) ? sm.getServiceRecordInternal(phoneId) : null;
    }

    public void dispose() {
        if (mSubscriptionListener != null) {
            ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);

            if (subs != null) {
                subs.removeListener(mSubscriptionListener);
            }
        }

        if (mCommonPackageListener != null) {
            CommonStarter.getInstance().removeListener(mCommonPackageListener);
        }

        destroyAllCallApps();
        destroyAllServiceRecords();
    }

    public ImsCallApp createCallApp(ImsContext imsContext,
            IMmTelFeatureCapabilityListener featureCapabilityListener,
            IMmTelCallListener callListener) {
        int phoneId = imsContext.getPhoneId();
        log("createCallApp :: phoneId=" + phoneId);

        ImsServiceRecord isr = getServiceRecordInternal(phoneId);

        if (isr == null) {
            logi("Fatal :: ServiceRecord is not present; phoneId=" + phoneId);
            return null;
        }

        boolean callAppCreated = false;
        ImsCallApp callApp = getCallApp(phoneId);

        if (callApp == null) {
            if (mOperator[phoneId] == null) {
                mOperator[phoneId] = new String(ImsGlobal.getOperator(phoneId));
                mCountry[phoneId] = new String(ImsGlobal.getCountry(phoneId));
                mServiceFeatures[phoneId] = getServiceFeatures(phoneId);
                mVoLteServiceFeatures[phoneId] = getVoLteServiceFeatures(phoneId);
            }

            callApp = createCallAppInternal(imsContext, isr, featureCapabilityListener,
                    callListener);
            addCallApp(phoneId, callApp);
            setCallAppForServiceRecord(callApp.getPhoneId(), callApp);

            logi("createCallApp :: appCount=" + getCallAppCount());

            callAppCreated = true;
        } else {
            logi("createCallApp :: app is already opened");

            // FIXME: P-GII
            //callApp.getCallManager().setPendingIntentForIncomingRequest(incomingCallIntent);
            //callApp.setRegistrationListener(listener);
        }

        // Bind ImsCallApp and other required modules
        bindActiveCallAppOnCreated(callApp, callAppCreated);

        return callApp;
    }

    public void destroyCallApp(int phoneId) {
        ImsCallApp callApp = getCallApp(phoneId);

        if (callApp == null) {
            return;
        }

        setCallAppForServiceRecord(callApp.getPhoneId(), null);

        // FIXME: according to the "flags", do any proper operationis...
        callApp.close();

        removeCallApp(phoneId);
    }

    public ImsCallApp getCallApp(int phoneId) {
        return mCallApps.get(phoneId);
    }

    @VisibleForTesting
    protected ImsCallApp createCallAppInternal(ImsContext imsContext, ImsServiceRecord isr,
            IMmTelFeatureCapabilityListener featureCapabilityListener,
            IMmTelCallListener callListener) {
        return new ImsCallApp(imsContext, isr.getRegistrationTracker(), featureCapabilityListener,
                callListener);
    }

    @VisibleForTesting
    protected  ConcurrentHashMap<Integer, ImsCallApp> getCallAppMap() {
        return mCallApps;
    }

    @VisibleForTesting
    protected ConcurrentHashMap<Integer, ImsServiceRecord> getServiceRecordMap() {
        return mServiceRecords;
    }

    public ImsCallApp getCallAppByPhoneId(int phoneId) {
        if (getDefaultPhoneId() != phoneId) {
            if (MSimUtils.isMultiSimEnabled() && !isMultiImsEnabled()) {
                return null;
            }
        }

        ImsServiceRecord isr = getServiceRecordInternal(phoneId);
        return (isr != null) ? isr.getCallApp() : null;
    }

    public int getCallAppCount() {
        return mCallApps.size();
    }

    public Context getContext() {
        return mContext;
    }

    public int getDefaultPhoneId() {
        // FIXME: we need to consider the case of multi-sim & multi-volte.
        return mDefaultPhoneId;
    }

    public ImsServiceRecord getDefaultServiceRecord() {
        return getServiceRecordInternal(getDefaultPhoneId());
    }

    public boolean isValidPhoneId(int phoneId) {
        if (!isMultiImsEnabled()) {
            return phoneId == getDefaultPhoneId();
        }

        return (phoneId >= MSimUtils.DEFAULT_PHONE_ID)
                && (phoneId < mServiceRecords.size());
    }

    private void addCallApp(int phoneId, ImsCallApp app) {
        mCallApps.put(phoneId, app);
    }

    private void removeCallApp(int phoneId) {
        mCallApps.remove(phoneId);
    }

    @SuppressWarnings("unchecked")
    private void destroyAllCallApps() {
        logi("destroyAllCallApps :: appCount=" + getCallAppCount());

        if (mCallApps.isEmpty()) {
            return;
        }

        for (Map.Entry<Integer, ImsCallApp> entry : mCallApps.entrySet()) {
            ImsCallApp callApp = entry.getValue();

            if (callApp != null) {
                callApp.close();
            }
        }

        mCallApps.clear();

        setCallAppForServiceRecord(MSimUtils.INVALID_PHONE_ID, null);
    }

    private void bindActiveCallAppOnCreated(ImsCallApp callApp, boolean created) {
        if (callApp.getPhoneId() == MSimUtils.INVALID_PHONE_ID) {
            if (!created) {
                callApp.unbindCallApp();
                callApp.bindCallApp();
            }

            return;
        } else if (isMultiImsEnabled()) {
            if (!created) {
                callApp.unbindCallApp();
                callApp.bindCallApp();
            }

            return;
        }

        // Multi-SIM & SIM switching is executed
        // Unbind the previous active ImsCallApp if present.
        for (int i = 0; i < mServiceRecords.size(); ++i) {
            ImsServiceRecord isr = mServiceRecords.get(i);

            if (isr == null) {
                continue;
            }

            if (getDefaultPhoneId() != i) {
                ImsCallApp app = isr.getCallApp();

                if (app != null) {
                    app.unbindCallApp();
                }
            }
        }

        ImsServiceRecord defaultIsr = getDefaultServiceRecord();
        ImsCallApp defaultApp = (defaultIsr != null) ? defaultIsr.getCallApp() : null;

        if ((defaultApp != null) && !created) {
            defaultApp.bindCallApp();
        }
    }

    private void createServiceRecords() {
        int supportedSimCount = MSimUtils.getSupportedSimCount();

        for (int i = 0; i < supportedSimCount; ++i) {
            mServiceRecords.put(i, new ImsServiceRecord(mContext, mExecutor, i));
        }

        // Not reachable: Phone count is zero.
        if (mServiceRecords.isEmpty()) {
            mServiceRecords.put(
                MSimUtils.DEFAULT_PHONE_ID,
                new ImsServiceRecord(mContext, mExecutor, MSimUtils.DEFAULT_PHONE_ID));
        }
    }

    private void destroyAllServiceRecords() {
        for (int i = 0; i < mServiceRecords.size(); ++i) {
            ImsServiceRecord isr = mServiceRecords.get(i);

            if (isr == null) {
                continue;
            }

            if (isr.isServiceUp()) {
                isr.broadcastServiceDown();
            }
        }

        mServiceRecords.clear();
    }

    private ImsServiceRecord getServiceRecordInternal(int phoneId) {
        if ((phoneId >= MSimUtils.DEFAULT_PHONE_ID)
                && (phoneId < mServiceRecords.size())) {
            return mServiceRecords.get(phoneId);
        }

        // As default, first one will be selected
        if (!MSimUtils.isMultiSimEnabled()) {
            return mServiceRecords.get(MSimUtils.DEFAULT_PHONE_ID);
        }

        return null;
    }

    private void setCallAppForServiceRecord(int phoneId, ImsCallApp callApp) {
        if (phoneId == MSimUtils.INVALID_PHONE_ID) {
            for (int i = 0; i < mServiceRecords.size(); ++i) {
                ImsServiceRecord isr = mServiceRecords.get(i);

                if (isr == null) {
                    continue;
                }

                isr.setCallApp(callApp);
            }
            return;
        }

        ImsServiceRecord isr = getServiceRecordInternal(phoneId);

        if (isr != null) {
            isr.setCallApp(callApp);
        }
    }

    private void setDefaultPhoneId(int phoneId) {
        int oldPhoneId = getDefaultPhoneId();

        if (phoneId != oldPhoneId) {
            mDefaultPhoneId = phoneId;

            if (!isMultiImsEnabled()) {
                switchImsService(oldPhoneId, phoneId);
            }
        }
    }

    private void switchImsService(int oldPhoneId, int newPhoneId) {
        logi("switchService :: " + oldPhoneId + " >> " + newPhoneId);

        ImsServiceRecord oldIsr = getServiceRecordInternal(oldPhoneId);
        ImsServiceRecord newIsr = getServiceRecordInternal(newPhoneId);

        ImsCallApp oldApp = (oldIsr != null) ? oldIsr.getCallApp() : null;
        ImsCallApp newApp = (newIsr != null) ? newIsr.getCallApp() : null;

        if ((oldIsr != null) && oldIsr.isServiceUp()) {
            if (oldApp != null) {
                oldApp.unbindCallApp();
            }

            // reset old mOperator value for non DSDV SIM-hotswap case
            if (MSimUtils.isMultiSimEnabled()) {
                mOperator[oldPhoneId] = null;
            }

            oldIsr.broadcastServiceDown();
        }

        // SIM-hotswap
        int serviceFeatures = getVoLteServiceFeatures(newPhoneId);

        if (serviceFeatures > 0) {
            if (newIsr != null) {
                newIsr.broadcastServiceUp();
            }
        } else {
            logi("switchService :: No service available");
        }

        if (newApp != null) {
            newApp.bindCallApp();
        }
    }

    // Check whether ImsService should be down by hotswap
    private void checkImsServiceAvailabilityAndBroadcastServiceUpDown(
            int phoneId) {
        if (phoneId < MSimUtils.DEFAULT_PHONE_ID || phoneId >= MSimUtils.getActiveSimCount()) {
            return;
        }

        if (ImsConstants.USE_CARRIER_CONFIG) {
            return;
        }
    }

    // Operator changed by hotswap
    private void checkOperatorAndRebindCallApp(int phoneId) {
        if (phoneId < MSimUtils.DEFAULT_PHONE_ID || phoneId >= MSimUtils.getActiveSimCount()) {
            return;
        }

        String newOperator = ImsGlobal.getOperator(phoneId);

        if (TextUtils.isEmpty(mOperator[phoneId])) {
            mOperator[phoneId] = new String(newOperator);
            mCountry[phoneId] = new String(ImsGlobal.getCountry(phoneId));
            mServiceFeatures[phoneId] = getServiceFeatures(phoneId);
            mVoLteServiceFeatures[phoneId] = getVoLteServiceFeatures(phoneId);

            checkImsServiceAvailabilityAndBroadcastServiceUpDown(phoneId);

            if (!TextUtils.isEmpty(mOperator[phoneId])
                    && (mServiceFeatures[phoneId] > 0)) {
                refreshServiceRecordAndCallApp(phoneId, true);
            }
            return;
        }

        boolean operatorChanged = false;
        boolean operatorOrServiceFeaturesChanged = false;

        if (!mOperator[phoneId].equals(newOperator)) {
            int serviceFeatures = getServiceFeatures(phoneId);

            logi("operatorChanged :: " + mOperator[phoneId] + " >> " + newOperator
                    + ", serviceFeatures=0x" + Integer.toHexString(serviceFeatures));

            mOperator[phoneId] = null;
            mOperator[phoneId] = new String(newOperator);
            mCountry[phoneId] = new String(ImsGlobal.getCountry(phoneId));
            mServiceFeatures[phoneId] = serviceFeatures;
            mVoLteServiceFeatures[phoneId] = getVoLteServiceFeatures(phoneId);

            operatorChanged = true;
            operatorOrServiceFeaturesChanged = true;
        } else {
            int serviceFeatures = getServiceFeatures(phoneId);

            if (serviceFeatures != mServiceFeatures[phoneId]) {
                logi("serviceFeaturesChanged :: 0x" + Integer.toHexString(mServiceFeatures[phoneId])
                        + " >> 0x" + Integer.toHexString(serviceFeatures));

                mServiceFeatures[phoneId] = serviceFeatures;

                operatorOrServiceFeaturesChanged = true;
            }

            // IMS emergency call when no SIM present
            int voLteServiceFeatures = getVoLteServiceFeatures(phoneId);

            if (voLteServiceFeatures != mVoLteServiceFeatures[phoneId]) {
                logi("voLteServiceFeaturesChanged :: 0x"
                        + Integer.toHexString(mVoLteServiceFeatures[phoneId])
                        + " >> 0x" + Integer.toHexString(voLteServiceFeatures));

                mVoLteServiceFeatures[phoneId] = voLteServiceFeatures;

                if (!"KR".equals(ImsGlobal.getCountry(phoneId))) {
                    operatorOrServiceFeaturesChanged = true;
                }
            }
        }

        checkImsServiceAvailabilityAndBroadcastServiceUpDown(phoneId);

        if (operatorOrServiceFeaturesChanged) {
            refreshServiceRecordAndCallApp(phoneId, operatorChanged);
        }
    }

    protected int getVoLteServiceFeatures(int phoneId) {
        int serviceFeatures = getVoLteServiceFeaturesFromPlatformConfig(phoneId);

        // VOLTE_EMERGENCY_CALLING
        if (serviceFeatures == 0) {
            ISubscription isub = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);

            if ((isub != null) && (isub.isSimAbsent(phoneId) || isub.isSimLocked(phoneId))) {
                logi("SimAbsentOrLocked: VoLTE is enabled for IMS e-call");
                serviceFeatures |= FeatureConfig.FEATURE_S_VOLTE;
                serviceFeatures |= FeatureConfig.FEATURE_S_VOLTE_EMERGENCY;
            }
        }

        // VOLTE_EMERGENCY_CALLING
        // FIXME: Dual VoLTE
        if (serviceFeatures == 0) {
            logi("NonVoLteSim: VoLTE is enabled for IMS e-call");
            serviceFeatures |= FeatureConfig.FEATURE_S_VOLTE;
            serviceFeatures |= FeatureConfig.FEATURE_S_VOLTE_EMERGENCY;
        }

        return serviceFeatures;
    }

    private int getVoLteServiceFeaturesFromPlatformConfig(int phoneId) {
        ServiceCaps serviceCaps = ServiceCaps.getServiceCaps(phoneId);
        int serviceFeatures = 0;

        logi(serviceCaps.toString());

        if (serviceCaps.isVoLteEnabled()) {
            serviceFeatures |= FeatureConfig.FEATURE_S_VOLTE;
        }

        if (serviceCaps.isWfcEnabled()) {
            serviceFeatures |= FeatureConfig.FEATURE_S_VOWIFI;
        }

        if (serviceCaps.isVtEnabled()) {
            serviceFeatures |= FeatureConfig.FEATURE_S_VT;
        }

        return serviceFeatures;
    }

    private void refreshServiceRecordAndCallApp(int phoneId, boolean operatorChanged) {
        logi("refreshServiceRecordAndCallApp :: phoneId=" + phoneId);

        ImsServiceRecord isr = getServiceRecordInternal(phoneId);

        if (isr != null) {
            if (operatorChanged) {
                isr.reconfigure();
            }

            ImsCallApp app = isr.getCallApp();

            if (app != null) {
                app.unbindCallApp();
                app.bindCallApp();
            }
        }
    }

    private static int getActivePhoneId() {
        ISubscription isub = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        return (isub != null) ? isub.getPhoneId() : MSimUtils.DEFAULT_PHONE_ID;
    }

    private static int getServiceFeatures(int phoneId) {
        int serviceFeatures = 0;
        List<FeatureTable.Feature> serviceFeatureList = FeatureTable.getServiceFeatures();

        for (FeatureTable.Feature feature : serviceFeatureList) {
            serviceFeatures = setOrClearFeature(phoneId,
                    feature.getFeature(), feature.getFeatureMask(), serviceFeatures);
        }

        return serviceFeatures;
    }

    private static int setOrClearFeature(int phoneId,
            String feature, int featureMask, int features) {
        if (FeatureConfig.isEnabled(phoneId, feature)) {
            features |= featureMask;
        } else {
            features &= (~featureMask);
        }

        return features;
    }

    @VisibleForTesting
    protected boolean isMultiImsEnabled() {
        return MSimUtils.isMultiImsEnabled();
    }

    @VisibleForTesting
    protected int getPhoneIdFromMSimUtils(int subId) {
        return MSimUtils.getPhoneId(subId);
    }

    private static void log(String s) {
        ImsLog.d("[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }

    protected final class SubscriptionListenerProxy extends SubscriptionListener {
        public SubscriptionListenerProxy() {
            log("SubscriptionListenerProxy :: phoneId=" + getActivePhoneId());
        }

        @Override
        public void onSimLoadCompleted(int slotId) {
            logi("onSimLoadCompleted :: slotId=" + slotId);

            // If DDS slot is not in LOADED state (such as LOCKED),
            // call-frw may not determine the operator which supports VoLTE.
            if (MSimUtils.isMultiSimEnabled() && !isMultiImsEnabled()) {
                ISubscription isub
                        = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);

                if ((isub != null)
                        && (slotId == isub.getSlotId())
                        && isub.isSimLoaded(slotId)) {
                    checkImsServiceAvailabilityAndBroadcastServiceUpDown(slotId);
                }
            }
        }

        @Override
        public void onDefaultSubscriptionChanged(int subId) {
            logi("onDefaultSubscriptionChanged :: subId=" + subId);
            setDefaultPhoneId(getPhoneIdFromMSimUtils(subId));
        }

        @Override
        public void onDefaultDataSubscriptionChanged(int subId) {
            logi("onDefaultDataSubscriptionChanged :: subId=" + subId);
            setDefaultPhoneId(getPhoneIdFromMSimUtils(subId));
        }

        @Override
        public void onCarrierConfigChanged(int phoneId, int subId) {
            logi("onCarrierConfigChanged :: subId=" + subId + ", phoneId=" + phoneId);

            if (ImsConstants.USE_CARRIER_CONFIG) {
                checkImsServiceAvailabilityAndBroadcastServiceUpDown(phoneId);
            }
        }
    }

    protected class CommonPackageListener implements ICommonPackageListener {
        public CommonPackageListener() {
        }

        @Override
        public void onCommonPackageReady(int slotId) {
            logi("onCommonPackageReady :: slotId=" + slotId);

            mExecutor.execute(() -> {
                checkOperatorAndRebindCallApp(slotId);
            });
        }

        @Override
        public void onCommonPackageStop(int slotId) {
            logi("onCommonPackageStop :: slotId=" + slotId);

            if (slotId < MSimUtils.DEFAULT_PHONE_ID || slotId >= MSimUtils.getSupportedSimCount()) {
                return;
            }

            if (ImsConstants.USE_CARRIER_CONFIG) {
                // Non-VoLte SIM
                if (getVoLteServiceFeaturesFromPlatformConfig(slotId) == 0) {
                    logi("No VoLte services");
                    mVoLteServiceFeatures[slotId] = 0;
                }
            }
        }
    }
}
