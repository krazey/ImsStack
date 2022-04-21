package com.android.imsstack.core.service;

import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.text.TextUtils;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IIMSPhoneGov;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.IBatteryState;
import com.android.imsstack.core.agents.agentif.ICallSetting;
import com.android.imsstack.core.agents.agentif.ICellInfo;
import com.android.imsstack.core.agents.agentif.IGBA;
import com.android.imsstack.core.agents.agentif.IIMSPhoneAgent;
import com.android.imsstack.core.agents.agentif.ILocationAgent;
import com.android.imsstack.core.agents.agentif.ILocationAgentManager;
import com.android.imsstack.core.agents.agentif.IRegiProcess;
import com.android.imsstack.core.agents.agentif.ISharedState;
import com.android.imsstack.core.agents.agentif.ISIMState;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.core.agents.agentif.LocationPolicy;
import com.android.imsstack.core.agents.agentif.SubscriptionListener;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcm.DCGov;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCSettings;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.core.config.ImsDbController;
import com.android.imsstack.core.config.ProviderInterface;
import com.android.imsstack.core.service.CallInfoService;
import com.android.imsstack.core.service.CallSettingService;
import com.android.imsstack.core.service.CallStateNotificationService;
import com.android.imsstack.core.service.ECallStateService;
import com.android.imsstack.core.service.EPDGCallService;
import com.android.imsstack.core.service.SCMService;
import com.android.imsstack.core.service.SrvccStateService;
import com.android.imsstack.core.service.TtyService;
import com.android.imsstack.core.service.USATService;
import com.android.imsstack.core.service.serviceif.ICallSettingService;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.enabler.aos.IAosInfo.PhoneNumberState;
import com.android.imsstack.provider.ImsStateController;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.test.ImsTestMask;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.CarrierConfigUtils;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.FeatureUtils;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.SettingsUtils;

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

    private boolean mVOPSState = false;
    private int mCount = 0;

    private SubscriptionListenerProxy mSubscriptionListener = null;
    private TtyService mTtyService = null;

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

        dcGov_SetApnEnable(true);

        // 8. local application load (Auto Configuration, GBA...)
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
        dcGov_SetApnEnable(false);
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

        mServics.put(TYPE_CALLSETTING, new CallSettingService());
        mServics.put(TYPE_SRVCCSTATE, new SrvccStateService());
        mServics.put(TYPE_ACBSKIP, new SCMService());
        mServics.put(TYPE_CALLINFO, new CallInfoService());
        mServics.put(TYPE_EDPGCALL, new EPDGCallService());
        mServics.put(TYPE_ECALLSTATE, new ECallStateService());
        mServics.put(TYPE_USAT, new USATService());
        mServics.put(TYPE_CALLSTATENOTIFICATION, new CallStateNotificationService());

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

    protected void dcGov_SetApnEnable(boolean enable) {
        IDCApn dcapn = (IDCApn)DCFactory.getDC(DCFactory.APN, mSlotID);
        if (dcapn != null) {
            dcapn.changeApnEmployState(EApnType.IMS, enable);
            dcapn.changeApnEmployState(EApnType.EMERGENCY, enable);
            dcapn.changeApnEmployState(EApnType.XCAP, enable);
        }
    }

    protected void initOperatorSpecificApp() {
        ImsLog.i("");

        IGBA gba = (IGBA)AgentFactory.getAgent(AgentFactory.GBA, mSlotID);
        if (gba != null) {
            gba.init(mContext);
        }

        updateRoamingCapability();
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
                    && FeatureConfig.isEnabled(getSlotID(), FeatureConfig.VOWIFI)) {
                lp = locAgent.getLocationPolicy();

                policy |= LocationPolicy.POLICY_LOCATION_NOT_ALLOWED_PERIODIC_POLLING;
                policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;

                addressResolutionTimeMillis = LocationPolicy.ADDRESS_RESOLUTION_MAX_TIME;
                validityPeriod = LocationPolicy.LOCATION_VALIDITY_PERIOD_SHORT;
            } else if (ImsGlobal.equalsCountry(mCountry, "CA")) {
                policy |= LocationPolicy.POLICY_INIT_REQUIRED_ON_GETTING_LAST_LOCATION;
                policy |= LocationPolicy.POLICY_LOCATION_UPDATE_USING_SMD;
                policy |= LocationPolicy.POLICY_UPDATE_COUNTRY_VIA_OTHER_SCHEME;
                policy |= LocationPolicy.POLICY_USE_CACHED_ADDRESS;
                policy |= LocationPolicy.POLICY_CACHED_ADDRESS_VALIDITY_DISTANCE;

                lp.setAddressTolerableDistance(3000);
                lp.setDefaultUpdateInterval(3600);
            } else if (OperatorInfo.isEnablerTypeForNonOperator(mSlotID)
                    && FeatureConfig.isEnabled(mSlotID, FeatureConfig.VOWIFI)) {
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

        IGBA gba = (IGBA)AgentFactory.getAgent(AgentFactory.GBA, mSlotID);
        if (gba != null) {
            gba.cleanup();
        }

        if (mTtyService != null) {
            mTtyService.cleanup(mContext);
            mTtyService = null;
        }

        ICallSettingService callSettingService =
                (ICallSettingService)getService(IVoLteService.TYPE_CALLSETTING);
        if (callSettingService != null) {
            callSettingService.unregisterForVoWIFISetChanged(null);
        }

        ICellInfo ci = (ICellInfo)AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotID);
        if (ci != null) {
            ci.stopTrackingCellInfo(mContext);
            ci.cleanup();
        }

        if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "TMO", "US")) {
            // IMS_RTT {
            if (FeatureUtils.isRttSupported(mContext)) {
                if (callSettingService != null) {
                    callSettingService.unregisterForRttModeSettingChanged(null);
                }
            }
            // IMS_RTT }
        }
    }

    protected void operatorSpecificImsBootCompleted() {
        mTtyService = new TtyService();
        mTtyService.start((IVoLteService)this);

        ISystem system = SystemInterface.getInstance().getSystem(mSlotID);
        ISIMState stg = (ISIMState)AgentFactory.getAgent(AgentFactory.SIM_STATE, mSlotID);

        if (stg != null) {
            stg.requestPSIInfo();

            if (stg.isIccLoaded()) {
                IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotID);
                if (aosInfo != null) {
                    aosInfo.notifyPhoneNumberState(false, PhoneNumberState.SIM_LOADED);
                }
            }
        }

        if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "ATT", "US")) {
            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_MOBILE_DATA_SETTING,
                        SettingsUtils.isMobileDataEnabled(mContext.getContentResolver()) ?
                        ImsEventDef.IMS_MOBILE_DATA_SETTING_ON :
                        ImsEventDef.IMS_MOBILE_DATA_SETTING_OFF, 0);
            }

            // VoLTE initial start case
            updateServiceProvisioned(getSlotID());

            //enable video call with DISS
            SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                    SubsInfoInterface.class, getSlotID());
            if ((subsInfo != null) && subsInfo.isTestModeEnabled()) {
                ICallSetting cso = (ICallSetting)AgentFactory.getAgent(
                        AgentFactory.CALL_SETTING, getSlotID());
                if (cso != null) {
                    cso.registerForVideoCallSetChanged(null, -1, null);
                    cso.registerForMobileDataSettingChanged(null, -1, null);
                }
            }
        } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "TMO", "US")) {
            if (OperatorInfo.getSysSimOperator(mSlotID).equals("CCA")) {
                if (SettingsUtils.isDataNetworkEnhanced4GLteMode(mContext, getSlotID()) == true) {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                            ImsEventDef.IMS_VOLTE_SETTING_ON, 0);
                } else {
                    system.notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                            ImsEventDef.IMS_VOLTE_SETTING_OFF, 0);
                }
            }
            system.notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                SettingsUtils.isWFCImsEnabled(mContext, getSlotID()) ? 1 : 0,
                SettingsUtils.getWFCImsMode(mContext, getSlotID()));

            system.notifyEvent(ImsEventDef.IMS_EVENT_MOBILE_DATA_SETTING,
                    SettingsUtils.isMobileDataEnabled(mContext.getContentResolver()) ?
                    ImsEventDef.IMS_MOBILE_DATA_SETTING_ON :
                    ImsEventDef.IMS_MOBILE_DATA_SETTING_OFF, 0);

            // IMS_RTT {
            if (FeatureUtils.isRttSupported(mContext)) {
                ICallSettingService callSettingService
                        = (ICallSettingService)getService(IVoLteService.TYPE_CALLSETTING);
                if (callSettingService != null) {
                    callSettingService.registerForRttModeSettingChanged(null, -1, null);
                }
            }
            // IMS_RTT }

            ICellInfo ci = (ICellInfo)AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotID);
            if (ci != null) {
                ci.init(mContext);
                ci.startTrackingCellInfo(mContext);
                ci.setLastCellInfoStorage(true);
            }
        } else if (ImsGlobal.equalsOperatorCountry(mOperator, mCountry, "VZW", "US")) {
            // check roaming support or test mode for lab test
            boolean bRoamingEnabled = ImsStateController.RoamingState.getVoLteRoamingForPhoneId(
                    mContext.getContentResolver(), mSlotID) == 1 ? true : false;
            if (bRoamingEnabled ||
                    (ImsTestMode.getInstance().getTestMode(mSlotID).getExtraTestmask() &
                        ImsTestMask.TEST_MASK_ROAMING_CONDITION) > 0) {
                ImsLog.d("IMS roaming is supported or allowed by test mode");
            }

            // Request to connect IMS PDN
            DCGov dcgov = (DCGov)DCFactory.getDC(DCFactory.GOVERNOR, mSlotID);
            if (dcgov != null) {
                dcgov.activateDataConnection4Sys(EApnType.IMS.getType(),
                        IApn.IPCAN_CATEGORY_MOBILE);
            }
        } else if (ImsGlobal.equalsCountry(mCountry, "KR")) {
            if (ImsGlobal.equalsOperator(mOperator, "LGU")) {
                initServiceProvisioningInfo(IUIMS.M_APP_UC | IUIMS.M_APP_SMS);
            } else {
                // APP_MTS is enable if sms_over_network_id is true from DM configuration
                initServiceProvisioningInfo(IUIMS.M_APP_UC | IUIMS.M_APP_VT);
            }

            if (isRoamingSupported()) {
                DCGov dcGov = (DCGov)DCFactory.getDC(DCFactory.GOVERNOR, mSlotID);
                if (dcGov != null) {
                    dcGov.activateDataConnection4Sys(EApnType.IMS.getType(),
                            IApn.IPCAN_CATEGORY_MOBILE);
                }
            }
        } else if (OperatorInfo.isEnablerTypeForNonOperator(mSlotID)
                && FeatureConfig.isEnabled(mSlotID, FeatureConfig.VOWIFI)) {
            ICellInfo ci = (ICellInfo)AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotID);
            if (ci != null) {
                ci.init(mContext);
                ci.startTrackingCellInfo(mContext);

                if (isSetLastCellInfoStorageRequired()) {
                    ci.setLastCellInfoStorage(true);
                }
            }

            ICallSettingService callSettingService
                    = (ICallSettingService)getService(IVoLteService.TYPE_CALLSETTING);
            if (callSettingService != null) {
                callSettingService.registerForVoWIFISetChanged(null, -1, null);
            }

            IRegiProcess rp = (IRegiProcess)getService(IVoLteService.TYPE_REGIPROCESS);
            if (rp != null) {
                rp.setHandleNetworkModeRequired(isHandlingNetworkModeRequired());
            }
        }

        if (mBootHandler != null) {
            ImsLog.i("BootupGov :: AOS_START");
            mBootHandler.sendEmptyMessageDelayed(EVENT_AOS_START, 100);
        } else {
            ImsLog.e("mBootHandler is null");
        }
    }

    // Note : API to block support_roaming for a specific operator
    protected boolean isRoamingSupported() {
        return true;
    }

    protected void updateRoamingCapability() {
        // TODO: VT Roaming
        boolean carrierVoLteRoaming = CarrierConfigUtils.getBooleanForSlot(
                CarrierConfigManager.ImsVoice.KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL, mSlotID);
        boolean carrierVtRoaming = false;
        boolean supportRoaming = false;

        int volteInRoaming = 0;
        int vtInRoaming = 0;

        if (carrierVoLteRoaming) {
            supportRoaming = isRoamingSupported();
            if (supportRoaming) {
                volteInRoaming
                    = FeatureConfig.isEnabled(mSlotID, FeatureConfig.VOLTE_IN_ROAMING) ? 1 : 0;
                vtInRoaming
                    = FeatureConfig.isEnabled(mSlotID, FeatureConfig.VT_IN_ROAMING) ? 1 : 0;
            }
        }

        int curVolteInRoaming = ImsStateController.RoamingState.getVoLteRoamingForPhoneId(
                mContext.getContentResolver(), mSlotID);
        int curVtInRoaming = ImsStateController.RoamingState.getVtRoamingForPhoneId(
                mContext.getContentResolver(), mSlotID);
        if (volteInRoaming != curVolteInRoaming) {
            ImsStateController.RoamingState.putVoLteRoamingForPhoneId(
                    mContext.getContentResolver(), volteInRoaming, mSlotID);
            ImsLog.d("Updated  VOLTE_IN_ROAMING[ " + mSlotID + "/"
                    + carrierVoLteRoaming + "/" + supportRoaming + "/" + volteInRoaming + "]");
        }
        if (vtInRoaming != curVtInRoaming) {
            ImsStateController.RoamingState.putVtRoamingForPhoneId(
                    mContext.getContentResolver(), vtInRoaming, mSlotID);
            ImsLog.d("Updated  VT_IN_ROAMING[ " + mSlotID + "/"
                    + carrierVtRoaming + "/" + supportRoaming + "/" + vtInRoaming + "]");
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

        SystemInterface si = SystemInterface.getInstance();
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

        IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(
                DCFactory.NETWORK_WATCHER, mSlotID);
        if (dcnw != null) {
            notifyVopsState(dcnw.isVops());
        }

        // sync up ssac information
        IIMSPhoneAgent ipa = IIMSPhoneGov.getInstance(mSlotID);
        if (ipa != null) {
            ipa.requestNetworkInfo();
        }

        // notify system events for setting
        ICallSettingService callSettingService = (ICallSettingService)getService(
                IVoLteService.TYPE_CALLSETTING);
        if (callSettingService != null) {
            callSettingService.notifySystemEvents();
        }

        operatorSpecificImsBootCompleted();
    }

    private void notifyVopsState(boolean bIsVops) {
        SystemInterface si = SystemInterface.getInstance();
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

        int volte = ImsStateController.STATE_ACTIVE;
        int vt = ImsStateController.STATE_ACTIVE;
        int wfc = ImsStateController.STATE_ACTIVE;

        if (ImsGlobal.isVoLteProvisioningRequired(mContext, phoneId)) {
            // Service availabilities are turned ON.
        } else {
            if (!ImsGlobal.isVoLteEnabled(mContext, phoneId)) {
                volte = ImsStateController.STATE_INACTIVE;
            }

            if (!ImsGlobal.isVtEnabled(mContext, phoneId)) {
                vt = ImsStateController.STATE_INACTIVE;
            }

            if (!ImsGlobal.isWfcEnabled(mContext, phoneId)) {
                wfc = ImsStateController.STATE_INACTIVE;
            }
        }

        ImsStateController.VoLteState.putVoLteProvisionedForPhoneId(
                mContext.getContentResolver(), volte, phoneId);
        ImsStateController.VoLteState.putVtProvisionedForPhoneId(
                mContext.getContentResolver(), vt, phoneId);
        ImsStateController.VoLteState.putWfcProvisionedForPhoneId(
                mContext.getContentResolver(), wfc, phoneId);
    }

    protected void initServiceProvisioningInfo(int availableServices) {

        SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                SubsInfoInterface.class, getSlotID());

        if (subsInfo != null) {
            if (ImsConstants.USE_GOOGLE_NATIVE_APPS || subsInfo.isTestModeEnabled()) {
                ContentResolver cr = mContext.getContentResolver();

                ImsStateController.VoLteState.putVoLteProvisionedForPhoneId(
                    cr, ImsStateController.STATE_ACTIVE, mSlotID);

                String soipEnableFromDB = DBUtils.CP.getString(cr
                                            , ProviderInterface.SMS.CONTENT_URI
                                            , ImsDbController.selectForSlot(mSlotID)
                                            , ProviderInterface.SMS.SMS_OVER_IP_NETWORK
                                            , null);

                if (TextUtils.isEmpty(soipEnableFromDB)) {
                    soipEnableFromDB = "false";
                }

                if ("true".equalsIgnoreCase(soipEnableFromDB)) {
                    ImsLog.i("add the SMS Service according to DB");
                    availableServices |= IUIMS.M_APP_SMS;
                }
            }

            ISystem system = SystemInterface.getInstance().getSystem(mSlotID);
            if (system == null) {
                return;
            }

            system.notifyEvent(ImsEventDef.IMS_EVENT_SERVICE_SETTING,
                ImsEventDef.IMS_SERVICE_PRESENTITY, availableServices);
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
     * Add here if it requires to handle network mode for IMS de-registration
     */
    private boolean isHandlingNetworkModeRequired() {
        if (ImsGlobal.isOperator(mSlotID, "VDF")
                || ImsGlobal.isOperator(mSlotID, "DTAG")
                || ImsGlobal.isOperatorCountry(mSlotID, "ORG", "FR")
                || ImsGlobal.isOperatorCountry(mSlotID, "O2", "DE")) {
            return false;
        }

        return true;
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
