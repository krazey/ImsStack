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
package com.android.imsstack.core.service;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.CapabilityConfigs;
import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.IBatteryState;
import com.android.imsstack.core.agents.ICellInfo;
import com.android.imsstack.core.agents.ILocationAgent;
import com.android.imsstack.core.agents.ILocationAgentManager;
import com.android.imsstack.core.agents.ISharedState;
import com.android.imsstack.core.agents.ISubscription;
import com.android.imsstack.core.agents.LocationPolicy;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.SubscriptionListener;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;

import java.lang.ref.WeakReference;
import java.util.concurrent.ConcurrentHashMap;

public class VoLteService implements IVoLteService {
    protected static final int EVENT_IMS_BOOT_COMPLETE = 101;
    protected static final int EVENT_AOS_START = 102;

    protected Context mContext;
    protected int mSlotID = -1;
    protected IMSBootupHandler mBootHandler = new IMSBootupHandler(this);
    private ConcurrentHashMap<Integer, IService> mServics =
            new ConcurrentHashMap<Integer, IService>();
    private SubscriptionListenerProxy mSubscriptionListener = null;

    /**
     * Temporary information
     */
    private String mOperator = "";
    private String mCountry = "";

    public VoLteService(Context context) {
        ImsLog.i("");

        mContext = context;
    }

    /* ---------------------------------------------------------------------------------------------
        implements IVoLteService
    --------------------------------------------------------------------------------------------- */
    @Override
    public void start(int slotID) {
        ImsLog.i("[" + slotID + "]");
        mSlotID = slotID;

        mOperator = ImsGlobal.getOperator(mSlotID);
        mCountry = ImsGlobal.getCountry(mSlotID);

        startServices();

        // 8. local application load (Auto Configuration...)
        initOperatorSpecificApp();

        // 10. Load Additional Operator Specific logic
        setOperatorSpecificLogic();


        // 11. if native already started, start IMSBootComplete
        // Else, add listener to JNIUpCallEvtHandler to get native boot complete event
        ISharedState ss = (ISharedState)AgentFactory.getAgent(
            AgentFactory.SHARED_STATE, mSlotID);
        if (ss != null) {
            if (ss.isNativeBootCompleted()) {
                mBootHandler.sendEmptyMessage(EVENT_IMS_BOOT_COMPLETE);
            } else {
                ss.registerForNativeBootComplete(
                        mBootHandler, EVENT_IMS_BOOT_COMPLETE, null);
            }
        }

        // set SubscriptionListener for IMS Media camera controller
        mSubscriptionListener = new SubscriptionListenerProxy();
        ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
        if (subs != null) {
            subs.addListener(mSubscriptionListener);
        }
    }

    /* ---------------------------------------------------------------------------------------------
        implements IService
    --------------------------------------------------------------------------------------------- */
    @Override
    public boolean start(IVoLteService voLteService) {
        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i("");

        ISharedState ss = (ISharedState)AgentFactory.getAgent(AgentFactory.SHARED_STATE, mSlotID);
        if (ss != null) {
            ss.unregisterForNativeBootComplete(mBootHandler);
        }

        clearOperatorSpecificApp();
        cleanServices();

        if (mSubscriptionListener != null) {
            ISubscription subs = (ISubscription)AgentFactory.getAgent(AgentFactory.SUBSCRIPTION);
            if (subs != null) {
                subs.removeListener(mSubscriptionListener);
            }
            mSubscriptionListener = null;
        }

    }

    @Override
    public void update(Context context) {
        mContext = context;
        updateServices(context);
    }

    /* ---------------------------------------------------------------------------------------------
        implements IVoLteService
    --------------------------------------------------------------------------------------------- */
    @Override
    public Context getContext() {
        return mContext;
    }

    @Override
    public int getSlotID() {
        return mSlotID;
    }

    @Override
    public IService getService(int type) {
        if(!mServics.containsKey(type)) {
            return null;
        }

        return mServics.get(type);
    }

    /* ---------------------------------------------------------------------------------------------
        protected methods
    --------------------------------------------------------------------------------------------- */
    protected void startServices() {
        ImsLog.i("");

        for (int type = TYPE_DEFAULT; type < TYPE_MAX; type++) {
            IService service = mServics.get(type);
            if (service == null) {
                continue;
            }
            if (!service.start(this)) {
                service.cleanup(mContext);
                service = null;
                mServics.remove(type);
            }
        }
    }

    protected void cleanServices() {
        ImsLog.i("");

        for (int type = TYPE_DEFAULT; type < TYPE_MAX; type++) {
            IService service = mServics.get(type);
            if (service == null) {
                continue;
            }
            service.cleanup(mContext);
            service = null;
        }
        mServics.clear();
    }

    protected void updateServices(Context context) {
        ImsLog.i("");

        for (int type = TYPE_DEFAULT; type < TYPE_MAX; type++) {
            IService service = mServics.get(type);
            if (service == null) {
                continue;
            }
            service.update(context);
        }
    }

    protected void initOperatorSpecificApp() {
    }

    protected void setOperatorSpecificLogic() {
        ImsLog.i("");

        ILocationAgentManager lam = (ILocationAgentManager)VoLteFactory.getInstance().getAgent(
                VoLteFactory.AGENT_LOCATION_AGENT_MANAGER);
        ILocationAgent locAgent = (lam != null) ? lam.getAgent(getSlotID()) : null;

        if (locAgent != null) {
            LocationPolicy lp = null;
            int policy = LocationPolicy.POLICY_ENABLE_CACHED_LOCATION
                    | LocationPolicy.POLICY_USE_CACHED_LOCATION;
            int addressResolutionTimeMillis = -1;
            long validityPeriod = -1L;

            if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "TMO", "US")) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_USE_FLP;
                policy |= LocationPolicy.POLICY_NOTIFY_COUNTRY_CHANGED_EVENT;

                addressResolutionTimeMillis = 1000;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD;

                lp.setAddressValidityPeriod(LocationPolicy.LOCATION_VALIDITY_PERIOD);
                lp.setAddressTolerableDistance(150);
                lp.setSearchDurationForGps(20);
                lp.setShape(LocationPolicy.SHAPE_ELLIPSOID);
            } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "ATT", "US")) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_NOTIFY_LOCATION_FIXED_FOR_INSTANT_REQUEST;
                policy |= LocationPolicy.POLICY_NOTIFY_COUNTRY_CHANGED_EVENT;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
                policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;

                SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                        SubsInfoInterface.class, getSlotID());
                if ((subsInfo != null) && subsInfo.isTestModeEnabled()) {
                    // For WFC E911 test which should be tested in Puerto Rico area,
                    // allow mock location
                    policy |= LocationPolicy.POLICY_ALLOW_MOCK_LOCATION_UPDATE;
                }

                addressResolutionTimeMillis = LocationPolicy.ADDRESS_RESOLUTION_RTD_TIME;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD;

                lp.setAddressTolerableDistance(150);
            } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "VZW", "US")
                    && ImsGlobal.isWfcEnabled(mContext, getSlotID())) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING;
                policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;

                addressResolutionTimeMillis = LocationPolicy.ADDRESS_RESOLUTION_MAX_TIME;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD_SHORT;
            } else if (ImsGlobal.equalsCountry(mCountry, "CA")) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;
                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;

                lp.setAddressTolerableDistance(3000);
                lp.setDefaultUpdateInterval(3600);
            } else if (ImsGlobal.isWfcEnabled(mContext, mSlotID)) {
                lp = locAgent.getLocationPolicy();

                policy = LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;

                if (isForcedSwitchOnGpsPolicyRequired()) {
                    policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;
                }

                if (ImsGlobal.isOperatorCountry(mSlotID, "VDA" , "AU")) {
                    policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_FROM_USIM;
                }

                if (isCachedLocationRequired()) {
                    policy |= LocationPolicy.POLICY_ENABLE_CACHED_LOCATION;
                    policy |= LocationPolicy.POLICY_USE_CACHED_LOCATION;
                }

                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_TIME;

                // 200 ms
                lp.setAddressTolerableDistance(200);
                // 10 Min: 10 * 60 * 1000 * 1000000L
                lp.setAddressValidityPeriod(10 * 60 * 1000 * 1000000L);
            }

            if (lp != null) {
                lp.setPolicy(policy);

                if (addressResolutionTimeMillis > 0) {
                    lp.setDefaultAddressResolutionTime(addressResolutionTimeMillis);
                }

                if (validityPeriod > 0) {
                    lp.setValidityPeriod(validityPeriod);
                }

                ImsLog.d(mSlotID, lp.toString());

                locAgent.setLocationPolicy(lp);
            }
        }
    }

    protected void clearOperatorSpecificApp() {
        ImsLog.i("");

        ICellInfo ci = (ICellInfo)AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotID);
        if (ci != null) {
            ci.stopTrackingCellInfo(mContext);
            ci.cleanup();
        }
    }

    protected void operatorSpecificImsBootCompleted() {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotID);

        if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "ATT", "US")) {
            // VoLTE initial start case
            updateServiceProvisioned(getSlotID());
        } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "TMO", "US")) {
            ICellInfo ci = (ICellInfo)AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotID);
            if (ci != null) {
                ci.init(mContext);
                ci.startTrackingCellInfo(mContext);
                ci.setLastCellInfoStorage(true);
            }
        } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "VZW", "US")) {
            // check roaming support or test mode for lab test
            boolean bRoamingEnabled = CapabilityConfigs.isVoLteRoamingEnabled(mSlotID);
            if (bRoamingEnabled ||
                    (ImsTestMode.getInstance().getTestMode(mSlotID).getExtraTestmask() &
                        ImsTestMode.TEST_MASK_ROAMING_CONDITION) > 0) {
                ImsLog.d("IMS roaming is supported or allowed by test mode");
            }
        } else if (ImsGlobal.equalsCountry(mCountry, "KR")) {
            if (ImsGlobal.equalsOperator(mOperator, "LGU")) {
                initServiceProvisioningInfo(IUIMS.M_APP_UC | IUIMS.M_APP_SMS);
            } else {
                // APP_MTS is enable if sms_over_network_id is true from DM configuration
                initServiceProvisioningInfo(IUIMS.M_APP_UC | IUIMS.M_APP_VT);
            }
        } else if (ImsGlobal.isWfcEnabled(mContext, mSlotID)) {
            ICellInfo ci = (ICellInfo)AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotID);
            if (ci != null) {
                ci.init(mContext);
                ci.startTrackingCellInfo(mContext);

                if (isSetLastCellInfoStorageRequired()) {
                    ci.setLastCellInfoStorage(true);
                }
            }
        }

        if (mBootHandler != null) {
            ImsLog.i("BootupGov :: AOS_START");
            mBootHandler.sendEmptyMessageDelayed(EVENT_AOS_START, 100);
        } else {
            ImsLog.e("mBootHandler is null");
        }
    }

    protected void handleCarrierConfigChanged(int phoneId, int subId) {
        // SIM hot swap case
        updateServiceProvisioned(phoneId);
    }

    /* ---------------------------------------------------------------------------------------------
        private methods
    --------------------------------------------------------------------------------------------- */
    private void setImsBootCompleted() {
        ImsLog.i("");
        ISystem system = SystemInterface.getInstance().getSystem(mSlotID);
        if (system == null) {
            return;
        }

        IBatteryState bs = (IBatteryState)AgentFactory.getAgent(
                AgentFactory.BATTERY_STATE, mSlotID);
        if ((bs != null) && bs.isLowBattery()) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY,
                    ImsEventDef.IMS_POWER_LOW_BATTERY, 0);
        }

        IDcNetWatcher dcnw = (IDcNetWatcher) DcFactory.getDc(
                DcFactory.NETWORK_WATCHER, mSlotID);
        if (dcnw != null) {
            notifyVopsState(dcnw.isVops());
        }

        // notify system events for setting
        MmTelFeatureRegistry mtfr =
                ImsServiceRegistry.getInstance(mSlotID).getMmTelFeatureRegistry();

        system.notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                mtfr.isAdvancedCallingSettingEnabled()
                ? ImsEventDef.IMS_VOLTE_SETTING_ON
                : ImsEventDef.IMS_VOLTE_SETTING_OFF, 0);

        system.notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                mtfr.isVoWiFiSettingEnabled()
                ? ImsEventDef.IMS_WFC_ON
                : ImsEventDef.IMS_WFC_OFF, mtfr.getVoWiFiModeSetting());

        system.notifyEvent(ImsEventDef.IMS_EVENT_RTT_SETTING, mtfr.getRttMode(), 0);

        operatorSpecificImsBootCompleted();
    }

    private void notifyVopsState(boolean bIsVops) {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotID);
        if (system == null) {
            return;
        }

        ImsLog.i("VOPSState notification : " + bIsVops);
        system.notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE, bIsVops ?
            ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED : ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED,
            0);
    }

    /* ---------------------------------------------------------------------------------------------
        subclass - IMSBootupHandler
    --------------------------------------------------------------------------------------------- */
    public static class IMSBootupHandler extends Handler {
        private final WeakReference<VoLteService> mService;

        IMSBootupHandler(VoLteService service) {
            super(Looper.myLooper());
            mService = new WeakReference<VoLteService>(service);
        }

        @Override
        public void handleMessage(Message msg) {
            VoLteService service = mService.get();
            if (service != null) {
                service.handleMessage(msg);
            }
        }
    }

    public void handleMessage(Message msg) {
        ImsLog.d("IMSServiceHandler - what=" + msg.what);

        switch (msg.what) {
            case EVENT_IMS_BOOT_COMPLETE:
                setImsBootCompleted();
                break;
            case EVENT_AOS_START:
                IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotID);
                if (aosInfo != null) {
                    aosInfo.notifyAosStart();
                }
                break;
            default:
                break;
        }
    }


    /* ---------------------------------------------------------------------------------------------
        Listener class - SubscriptionListener
    --------------------------------------------------------------------------------------------- */
    private final class SubscriptionListenerProxy extends SubscriptionListener {
        public SubscriptionListenerProxy() {
            ImsLog.d("SubscriptionListenerProxy");
        }

        @Override
        public void onCarrierConfigChanged(int phoneId, int subId) {
            ImsLog.i("onCarrierConfigChanged :: subId=" + subId + ", phoneId=" + phoneId);
            handleCarrierConfigChanged(phoneId, subId);
        }
    }

    private void updateServiceProvisioned(int phoneId) {
        if (phoneId < 0) {
            return;
        }

        int voLte = ImsStateStore.STATE_ACTIVE;
        int vt = ImsStateStore.STATE_ACTIVE;
        int wfc = ImsStateStore.STATE_ACTIVE;

        if (ImsGlobal.isVoLteProvisioningRequired(mContext, phoneId)) {
            // Service availabilities are turned ON.
        } else {
            if (!ImsGlobal.isVoLteEnabled(mContext, phoneId)) {
                voLte = ImsStateStore.STATE_INACTIVE;
            }

            if (!ImsGlobal.isVtEnabled(mContext, phoneId)) {
                vt = ImsStateStore.STATE_INACTIVE;
            }

            if (!ImsGlobal.isWfcEnabled(mContext, phoneId)) {
                wfc = ImsStateStore.STATE_INACTIVE;
            }
        }

        ImsStateStore.getMmTelState(phoneId).setProvisioned(voLte, vt, wfc);
    }

    protected void initServiceProvisioningInfo(int availableServices) {

        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, getSlotID());

        if (subsInfo != null) {
            if (ImsConstants.USE_GOOGLE_NATIVE_APPS || subsInfo.isTestModeEnabled()) {
                ImsStateStore.getMmTelState(getSlotID()).setVoLteProvisioned(
                        ImsStateStore.STATE_ACTIVE);

                ConfigInterface config = AgentFactory.getInstance().getAgent(
                        ConfigInterface.class, getSlotID());
                CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;

                boolean smsOverImsSupported = (cc != null)
                        ? cc.getBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL)
                        : false;

                if (smsOverImsSupported) {
                    ImsLog.i("SMS over IMS supported.");
                    availableServices |= IUIMS.M_APP_SMS;
                }
            }
        }
    }

    /**
     * Add here if it requires to turn on forcibly the location setting
     */
    private boolean isForcedSwitchOnGpsPolicyRequired() {
        if (ImsGlobal.isOperatorCountry(mSlotID, "O2", "GB")
                || ImsGlobal.isOperatorCountry(mSlotID, "SUN", "SW")
                || ImsGlobal.isOperatorCountry(mSlotID, "VDA" , "AU")
                || ImsGlobal.isOperatorCountry(mSlotID, "OPT" , "AU")
                || ImsGlobal.isCountry(mSlotID, "HK")) {
            return true;
        }

        return false;
    }

    /**
     * Add here if it requires to use chached location for location information
     */
    private boolean isCachedLocationRequired() {
        if (ImsGlobal.isOperatorCountry(mSlotID, "ORG", "FR")
                || ImsGlobal.isOperatorCountry(mSlotID, "EEO", "GB")
                || ImsGlobal.isOperatorCountry(mSlotID, "BT", "GB")) {
            return true;
        }

        return false;
    }

    /**
     * Add here if it requires to store last cellular information
     */
    private boolean isSetLastCellInfoStorageRequired() {
        if (ImsGlobal.isOperator(mSlotID, "TEL")
                || ImsGlobal.isOperator(mSlotID, "MTS")
                || ImsGlobal.isOperatorCountry(mSlotID, "VDF", "CH")
                || ImsGlobal.isOperatorCountry(mSlotID, "MOV", "PE")) {
            return true;
        }

        return false;
    }
}
