package com.android.imsstack;

import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.carrier.CarrierCodeLoader;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.imsservice.ImsServiceController;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SODConfig;
import com.android.imsstack.util.SettingsUtils;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class ImsStackApp extends Application implements IStateInfoChangedObserver {

    private static final String TAG = ImsStackApp.class.getSimpleName();

    private static Handler sHandler = null;

    private final Map<String, Integer> mMsgMap = new HashMap<String, Integer>();

    private static SIMOperatorDetector sSOD = null;

    private final StateInfoChangedReceiver mStateInfoChangedReceiver
            = new StateInfoChangedReceiver();

    private class InternalMsg {
        public static final int START = 100;
        public static final int SIM_CHANGED_INFO = 101;
        public static final int DDS_CHANGED = 102;
        public static final int DELAYED_DDS_CHANGED = 103;
        public static final int CARRIER_CONFIG_CHANGED = 104;
        public static final int SIM_REMOVAL_COMPLETED = 105;
    }

    private static final int DDS_DELAY_INTERVAL = 1500;
    private static final int INTERVAL_FOR_SIM_REMOVAL_COMPLETED = 800;

    class IMSMngrHandler extends Handler {
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            switch (msg.what) {
                case InternalMsg.START:
                    handleStart(msg);
                    break;
                case InternalMsg.SIM_CHANGED_INFO:
                    handleSIMChangedInfo(msg);
                    break;
                case InternalMsg.DDS_CHANGED:
                    handleDDSChanged(msg);
                    break;
                case InternalMsg.CARRIER_CONFIG_CHANGED:
                    handleCarrierConfigChanged(msg);
                    break;
                case InternalMsg.SIM_REMOVAL_COMPLETED:
                    handleOperatorOrServiceOnSimRemovalCompleted(msg.arg1);
                    break;
                case InternalMsg.DELAYED_DDS_CHANGED:
                    handleDelayedDDSChange(msg);
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        Log.i(TAG, "onCreate - IMS=(always true)");

        // AppContext will be used globally,
        // so it should be initialized first at the creation time of IMS application.
        AppContext.init(this);

        // Initialize any static data
        ImsUtils.init();

        // Do something at once on device's boot-up
        initOnDeviceBootUp();

        // Clear Ephemeral properties for IMS internal usage
        int simCount = MSimUtils.getMaxSimSlot();

        for (int i = 0; i < simCount; i++) {
            ImsPrivateProperties.Ephemeral.removeAll(i);
        }

        sHandler = new IMSMngrHandler();

        initMsgMap();

        sHandler.sendEmptyMessage(InternalMsg.START);

        mStateInfoChangedReceiver.init(this, this);
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);

        Log.i(TAG, "onTrimMemory :: level=" + level);

        Runtime.getRuntime().gc();
    }

    @Override
    public void notifyStateInfoChanged(Intent intent) {
        if (intent == null) {
            return;
        }

        String action = intent.getAction();

        if (mMsgMap.containsKey(action)) {
            Message msg = Message.obtain();

            msg.what = mMsgMap.get(action);
            msg.obj = new Intent(intent);

            sHandler.sendMessage(msg);
        }
    }

    private void initMsgMap() {
        mMsgMap.put(TelephonyIntents.ACTION_SIM_STATE_CHANGED,
                InternalMsg.SIM_CHANGED_INFO);
        mMsgMap.put(TelephonyManager.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED,
                InternalMsg.DDS_CHANGED);
        mMsgMap.put(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED,
                InternalMsg.CARRIER_CONFIG_CHANGED);
    }

    private void handleStart(Message msg) {
        Log.i(TAG, "IMS app :: start");

        sSOD = new SIMOperatorDetector(this);

        // Initialize SIM operator if required
        initSimOperator();
    }

    // Handling of SIM_STATE_CHANGED
    private void handleSIMChangedInfo(Message msg) {
        Log.i(TAG, "IMS app :: sim changed info");

        Intent intent = (Intent)msg.obj;
        if (intent == null) {
            Log.w(TAG, "handleSIMChangedInfo :: Intent is null");
            return;
        }

        boolean userUnlocked = intent.getBooleanExtra(
                Intent.EXTRA_REBROADCAST_ON_UNLOCK, false);
        int defaultSlotId = MSimUtils.isMultiSimEnabled() ?
                MSimUtils.INVALID_SLOT_ID : MSimUtils.DEFAULT_SLOT_ID;
        int slotId = intent.getIntExtra(SubscriptionManager.EXTRA_SLOT_INDEX, defaultSlotId);
        int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                MSimUtils.INVALID_SUB_ID);
        String ss = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);

        Log.w(TAG, "SimState :: state=" + ss + ", slotId=" + slotId
                + ", subId=" + subId + ", userUnlocked=" + userUnlocked);

        if (slotId == MSimUtils.INVALID_PHONE_ID) {
            Log.e(TAG, "Invalid SlotId!!!");
            return;
        }

        if (sSOD.isSIMLoaded(ss) || sSOD.isSIMLocked(ss)) {
            procSimStateChanged(slotId, ss, subId);

            if (sSOD.getMaxSlot() > 1) {
                doSimStateChangedForOtherSlots(slotId);
            }
        } else if (sSOD.isSIMAbsent(ss)) {
            procSimStateChanged(slotId, ss, subId);

            if (!sSOD.isSIMRemoved(slotId) && (sSOD.getMaxSlot() > 1)) {
                doSimStateChangedForOtherSlots(slotId);
            }
        } else if (sSOD.isSIMRemoved(ss)) {
            procSimStateChanged(slotId, ss, subId);
        }
    }

    private synchronized void procSimStateChanged(int slotId, String ss, int subId) {
        Log.i(TAG, "procSimStateChanged(" + slotId + "/" + ss + "/" + subId + ")");

        sSOD.setSimState(slotId, ss);

        Log.d(TAG, "SIM[" + slotId + "] STATE(UNKNOWN/ABSENT/LOADED/LOCKED/REMOVED)="
                + Integer.toHexString(sSOD.getSimState(slotId)));

        // Process of SIM_STATE
        if (sSOD.isSIMLoaded(ss)) {
            Log.setDebuggable();
            decideSimOperator(slotId, ss);

            if (ImsConstants.USE_CARRIER_CONFIG) {
                if (!isCarrierConfigLoaded(slotId, subId)) {
                    Log.d(TAG, "Wait for CarrierConfigChanged");
                    return;
                } else {
                    Log.d(TAG, "CarrierConfig is already loaded.");
                }
            }

            proc_SIM_LOADED(slotId, ss);
        } else if (sSOD.isSIMAbsent(ss)) {
            // If previous SIM state was LOADED or LOCKED, and REMOVED state is not yet received,
            // REMOVED state will be set in here
            // because SIM state is transited again by the abnormal behavior (e.g. modem restart).
            // Normal SIM Removal: LOADED >> REMOVED >> ABSENT
            // Abnormal SIM Removal: LOADED >> (UNKNOWN >>) (CarrierConfigChanged) >> ABSENT
            boolean simLoaded = sSOD.isSIMLoaded(slotId);
            boolean simLocked = sSOD.isSIMLocked(slotId);
            boolean simRemoved = sSOD.isSIMRemoved(slotId);

            if ((simLoaded || simLocked) && !simRemoved) {
                Log.i(TAG, "ImsPolicy :: Enforce SIM REMOVED on SIM ABSENT");
                sSOD.setSimState(slotId, ImsExtApi.Uicc.SIM_REMOVED);
            }

            // Emulate SIM_REMOVED case
            if (ImsConstants.PLATFORM_LAMPLITE) {
                if (simLoaded || simLocked) {
                    proc_SIM_REMOVED(slotId, ImsExtApi.Uicc.SIM_REMOVED);

                    // Keep ABSENT/REMOVED state only
                    sSOD.setSimState(slotId, 0x00);
                    sSOD.setSimState(slotId, ss);
                    sSOD.setSimState(slotId, ImsExtApi.Uicc.SIM_REMOVED);

                    if (simLoaded) {
                        Log.d(TAG, "Wait for CarrierConfigChanged");
                        return;
                    }
                }
            }

            proc_SIM_ABSENT(slotId, ss);
        } else if (sSOD.isSIMLocked(ss)) {
            decideSimOperator(slotId, ss);
            proc_SIM_LOCKED(slotId, ss);
        } else if (sSOD.isSIMRemoved(ss)) {
            proc_SIM_REMOVED(slotId, ss);
        }
    }

    private void proc_SIM_LOADED(int slotId, String ss) {
        Log.i(TAG, "proc_SIM_LOADED(" + slotId + "/" + ss + ")");

        removeInternalMessageIfPresent(InternalMsg.SIM_REMOVAL_COMPLETED);

        sSOD.setEquipInSim(slotId, true);
        if (sSOD.isSIMAbsent(slotId) || sSOD.isSIMLocked(slotId)) {
            sSOD.setSimState(slotId, 0x00);
            sSOD.setSimState(slotId, ss);
        }

        sSOD.decideOperator(slotId);
        sSOD.showAllInfo(slotId);

        boolean initialStart = sSOD.isInitialStart(slotId);
        boolean activeSlot = sSOD.isActive(slotId);
        boolean voLteEnabled = activeSlot && sSOD.isVoLTEAvailable(slotId);

        // Check whether service should be started or not
        if (!initialStart && !sSOD.isChanged(slotId)) {
            boolean serviceChanged = false;

            if (!"KR".equalsIgnoreCase(ImsProperties.TARGET_COUNTRY)) {
                serviceChanged = sSOD.isUpdated(slotId);
            }

            Log.w(TAG, "Operator is not changed; serviceChanged=" + serviceChanged);

            updateImsFeatures(slotId, true);

            // If service feature is changed, Ims stop/start is required
            if (serviceChanged) {
                if (sSOD.isNaOpen() && "ATT".equalsIgnoreCase(sSOD.getOperator(slotId))) {
                    deliverOperatorInfo(slotId);
                }

                stopServices(slotId);

                deliverUpdateServiceInfo(slotId);
                updateIMSDB(slotId);
                updateVoLteConfigurationForOperatorSpecificOnSimLoaded(slotId);

                if (voLteEnabled) {
                    deliverDDSInfo(slotId, true);
                    startAgents(slotId);
                }

                updateSystemConfigOnServiceChanged(slotId);
            } else {
                if (voLteEnabled) {
                    sSOD.deliverSimInfo(slotId);
                }

                if (voLteEnabled) {
                    if (updateVoLteConfigurationForOperatorSpecificOnSimLoaded(slotId)) {
                        // If system-config needs to be updated for synchronization with native,
                        // system-config update will be triggered.
                        updateSystemConfigOnServiceChanged(slotId);
                    }
                }
            }

            if (voLteEnabled && !sSOD.isCommonStart(slotId)) {
                deliverDDSInfo(slotId, true);
                startAgents(slotId);
                updateSystemConfigOnSimLoaded(slotId);
                serviceChanged = true;
            }

            if (voLteEnabled && serviceChanged) {
                startVoLTEService(slotId);
            }

            return;
        }

        Log.i(TAG, "SLOT[" + slotId + "/" + (activeSlot ? "active" : "non-active") + "] "
            + "Initial/VoLTE=" + initialStart + "/" + voLteEnabled);

        if (voLteEnabled != true) {
            Log.w(TAG, "ImsPolicy :: No enabled services");

            if (!initialStart) {
                if (sSOD.isSIMBasedOn()) {
                    if(sSOD.isDelivered(slotId) != true) {
                        deliverOperatorInfo(slotId);
                        updateIMSDB(slotId);
                    }
                    stopServices(slotId);

                    updateSystemConfigOnSimLoaded(slotId);
                }

                if (sSOD.isEquipInSim(slotId) && sSOD.isChanged(slotId)) {
                    sSOD.setChanged(slotId, false);
                }
            } else {
                // ImsDisabled
            }

            // Re-calculate IMS features when non-VoLTE SIM inserted
            updateImsFeatures(slotId, false);

            return;
        } else if (!voLteEnabled && initialStart) {
            // ImsDisabled
        }

        loadConfigAndStartServices(slotId, initialStart, voLteEnabled, true, true);

        sSOD.setChanged(slotId, false);
    }

    private void proc_SIM_ABSENT(int slotId, String ss) {
        Log.i(TAG, "proc_SIM_ABSENT(" + slotId + "/" + ss + ")");

        sSOD.setEquipInSim(slotId, false);

        if (sSOD.isSIMRemoved(slotId)) {
            if (ImsConstants.USE_CARRIER_CONFIG) {
                if (ImsConstants.PLATFORM_LAMPLITE) {
                    handleOperatorOrServiceOnSimRemovalCompleted(slotId);
                } else {
                    // Waits CarrierConfigChanged intent or timer timeout
                    sHandler.sendMessageDelayed(
                            sHandler.obtainMessage(InternalMsg.SIM_REMOVAL_COMPLETED, slotId, 0),
                            INTERVAL_FOR_SIM_REMOVAL_COMPLETED);
                }
            } else {
                handleOperatorOrServiceOnSimRemovalCompleted(slotId);
            }
        } else {
            //// Test-Purpose {
            boolean tempOpCoUsed = false;
            String prefOperator = ImsPrivateProperties.Persistent.get(
                    ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, "", slotId);

            if (prefOperator.isEmpty()) {
                String recentOperator = SODConfig.getValueForCachedOperatorInfo(
                        slotId, SODConfig.KEY_OPERATOR);
                String tempOp = "LGU";
                String tempCo = "KR";

                if (recentOperator != null && !recentOperator.isEmpty()) {
                    tempOp = recentOperator;
                    tempCo = SODConfig.getValueForCachedOperatorInfo(
                            slotId, SODConfig.KEY_COUNTRY);
                }

                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, tempOp, slotId);
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY, tempCo, slotId);

                tempOpCoUsed = true;
            }
            //// }

            sSOD.decideOperator(slotId);
            sSOD.showAllInfo(slotId);

            //// Test-Purpose {
            if (tempOpCoUsed) {
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, "", slotId);
                ImsPrivateProperties.Persistent.set(
                        ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY, "", slotId);
            }
            //// }

            // Check whether service should be started or not
            if (sSOD.isChanged(slotId) != true) {
                Log.w(TAG, "ImsPolicy :: Operator not changed");
                return;
            }

            boolean initialStart = sSOD.isInitialStart(slotId);
            boolean activeSlot = sSOD.isActive(slotId);
            boolean voLteEnabled = activeSlot && sSOD.isVoLTEAvailable(slotId);

            Log.i(TAG, "SLOT[" + slotId + "/" + (activeSlot ? "active" : "non-active") + "] "
                    + "Initial/VoLTE=" + initialStart + "/" + voLteEnabled);

            if (voLteEnabled != true) {
                Log.w(TAG, "ImsPolicy :: VoLTE disabled");

                if (initialStart) {
                    // ImsDisabled
                }
                return;
            }

            loadConfigAndStartServices(slotId, initialStart, voLteEnabled, true, false);

            sSOD.setChanged(slotId, false);
        }
    }

    private void proc_SIM_LOCKED(int slotId, String ss) {
        Log.i(TAG, "proc_SIM_LOCKED(" + slotId + "/" + ss + ")");

        removeInternalMessageIfPresent(InternalMsg.SIM_REMOVAL_COMPLETED);

        sSOD.setEquipInSim(slotId, true);
        if (sSOD.isSIMAbsent(slotId) || sSOD.isSIMLoaded(slotId)) {
            sSOD.setSimState(slotId, 0x00);
            sSOD.setSimState(slotId, ss);
        }

        sSOD.decideOperator(slotId);
        sSOD.showAllInfo(slotId);

        boolean initialStart = sSOD.isInitialStart(slotId);
        boolean activeSlot = sSOD.isActive(slotId);
        boolean voLteEnabled = activeSlot && sSOD.isVoLTEAvailable(slotId);

        Log.i(TAG, "SLOT[" + slotId + "/" + (activeSlot ? "active" : "non-active") + "] "
                + "Initial/VoLTE=" + initialStart + "/" + voLteEnabled);

        // Check whether service should be started or not
        if (!initialStart && !sSOD.isChanged(slotId)) {
            Log.w(TAG, "ImsPolicy :: Operator not changed");

            if (ImsConstants.USE_CARRIER_CONFIG) {
                if (!"KR".equalsIgnoreCase(ImsProperties.TARGET_COUNTRY)) {
                    boolean serviceChanged = sSOD.isUpdated(slotId);

                    Log.w(TAG, "ImsPolicy :: serviceChanged=" + serviceChanged);

                    // If service feature is changed, Ims stop/start is required
                    if (serviceChanged) {
                        stopServices(slotId);

                        deliverUpdateServiceInfo(slotId);

                        if (voLteEnabled) {
                            startAgents(slotId);
                        }

                        updateSystemConfigOnServiceChanged(slotId);
                    }

                    if (voLteEnabled && serviceChanged) {
                        startVoLTEService(slotId);
                    }
                }
            }
            return;
        }

        if (voLteEnabled != true) {
            Log.w(TAG, "ImsPolicy :: No enabled services");

            if (initialStart) {
                // ImsDisabled
            } else if (sSOD.isChanged(slotId)) {
                if (sSOD.isSIMBasedOn()) {
                    if (sSOD.isDelivered(slotId) != true) {
                        deliverOperatorInfo(slotId);
                        updateIMSDB(slotId);
                    }
                    stopServices(slotId);

                    updateSystemConfigOnSimLoaded(slotId);

                    sSOD.setChanged(slotId, false);
                }
            }
            return;
        } else if (!voLteEnabled && initialStart) {
            // ImsDisabled
        }

        loadConfigAndStartServices(slotId, initialStart, voLteEnabled, true, false);

        sSOD.setChanged(slotId, false);
    }

    private void proc_SIM_REMOVED(int slotId, String ss) {
        Log.i(TAG, "proc_SIM_REMOVED(" + slotId + "/" + ss + ")");

        // FIXME: move this logic to a proper location
        CommonStarter cs = CommonStarter.getInstance();

        if (sSOD.isCommonStart(slotId)) {
            cs.updateSystemConfigOnSimRemoved(slotId);
        }
    }
    // End of Handling of SIM_STATE_CHANGED

    // Handling of SIM_STATE_CHANGED
    private void handleDDSChanged(Message msg) {
        Intent intent = (Intent)msg.obj;

        if (intent == null) {
            Log.w(TAG, "DDSChanged :: Intent is null");
            return;
        }

        int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                MSimUtils.INVALID_SUB_ID);
        int slotId = SubscriptionManager.getSlotIndex(subId);

        Log.w(TAG, "DDSChanged - SLOT_ID/SUB_ID/DDS=" + slotId + "/" + subId + "/true");

        if (sSOD.getMaxSlot() == 1) {
            sSOD.setActive(sSOD.getDefaultSlot(), true);
            return;
        }

        if (sSOD.isMultiImsEnabled()) {
            Log.w(TAG, "Multi-IMS enabled");
            return;
        }

        if (slotId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            return;
        }

        Message delayedMsg = Message.obtain();
        delayedMsg.what = InternalMsg.DELAYED_DDS_CHANGED;
        delayedMsg.obj = new Intent(intent);

        sHandler.sendMessageDelayed(delayedMsg, DDS_DELAY_INTERVAL);
    }

    private void handleDelayedDDSChange(Message msg) {
        Intent intent = (Intent)msg.obj;

        if (intent == null) {
            return;
        }

        int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                MSimUtils.INVALID_SUB_ID);
        int slotId = SubscriptionManager.getSlotIndex(subId);
        Log.w(TAG, "DDSChanged(delay) - SLOT_ID/SUB_ID/DDS=" + slotId + "/" + subId + "/true");

        if (!sSOD.isSlotIdValid(slotId)) {
            return;
        }

        List<Integer> nonDDSSlotIdList = sSOD.getOtherSlotIds(slotId);
        if (nonDDSSlotIdList != null) {
            for (int i = 0; i < nonDDSSlotIdList.size(); i++) {
                int nonDDSSlotId = (int)nonDDSSlotIdList.get(i);
                Log.w(TAG, "DDSChanged - SLOT_ID/SUB_ID/DDS="
                        + nonDDSSlotId + "/" + sSOD.getSubId(nonDDSSlotId) + "/false");

                if (sSOD.isAvailable(nonDDSSlotId)) {
                    updateDDSChanged(nonDDSSlotId, false);
                    sSOD.setActive(nonDDSSlotId, false);
                    stopServices(nonDDSSlotId);
                }
            }
        }

        if (!sSOD.isAvailable(slotId)) {
            Log.i(TAG, "DDSChanged - slot not available");
            return;
        }

        if (!sSOD.isFinalSimState(slotId)) {
            Log.w(TAG, "DDSChanged - Not final SIM state: " + sSOD.getSimState(slotId));
            return;
        }

        if (sSOD.isActive(slotId) && (subId == sSOD.getSubId(slotId))) {
            Log.w(TAG, "DDSChanged - duplicated event");
            return;
        }

        // IMS-LAMPLITE
        if (ImsConstants.USE_CARRIER_CONFIG) {
            Log.d(TAG, "DDSChanged - decideOperator");
            sSOD.decideOperator(slotId);
            sSOD.showAllInfo(slotId);
        }

        sSOD.setActive(slotId, true);
        updateDDSChanged(slotId, true);

        boolean initialStart = sSOD.isInitialStart(slotId);
        boolean voLteEnabled = sSOD.isVoLTEAvailable(slotId);

        Log.i(TAG, "DDSChanged - Initial/VoLTE/=" + initialStart + "/" + voLteEnabled);

        if (voLteEnabled != true) {
            Log.w(TAG, "ImsPolicy :: No enabled services");

            if (initialStart) {
                // ImsDisabled
            }
            return;
        } else if (!voLteEnabled && initialStart) {
            // ImsDisabled
        }

        loadConfigAndStartServices(slotId, initialStart, voLteEnabled, false, false);

        // IMS-LAMPLITE
        if (ImsConstants.USE_CARRIER_CONFIG && sSOD.isChanged(slotId)) {
            Log.d(TAG, "DDSChanged - reset CHANGED");
        }

        sSOD.setChanged(slotId, false);
    }

    private void handleCarrierConfigChanged(Message msg) {
        Intent intent = (Intent)msg.obj;

        if (intent == null) {
            // Not reachable
            return;
        }

        int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                MSimUtils.INVALID_SUB_ID);
        int slotId = intent.getIntExtra(CarrierConfigManager.EXTRA_SLOT_INDEX,
                MSimUtils.INVALID_SLOT_ID);
        int carrierId = intent.getIntExtra(TelephonyManager.EXTRA_CARRIER_ID,
                TelephonyManager.UNKNOWN_CARRIER_ID);
        int specificCarrierId = intent.getIntExtra(TelephonyManager.EXTRA_SPECIFIC_CARRIER_ID,
                TelephonyManager.UNKNOWN_CARRIER_ID);

        Log.d(TAG, "CarrierConfigChanged :: slotId=" + slotId +
                ", subId=" + subId + ", carrierId=" + carrierId + ", specificCarrierId=" +
                specificCarrierId);

        ImsCarrierResolver.Carrier carrier =
                resolveImsCarrier(slotId, subId, carrierId, specificCarrierId);

        if (!ImsConstants.USE_CARRIER_CONFIG) {
            return;
        }

        CarrierCodeLoader.setSimOperatorCountry(carrier.getOperator(),
                carrier.getOperatorSub(), carrier.getCountry(), slotId);

        if (slotId == MSimUtils.INVALID_SLOT_ID || subId == MSimUtils.INVALID_SUB_ID) {
            if ((slotId == MSimUtils.INVALID_SLOT_ID) && !MSimUtils.isMultiSimEnabled()) {
                slotId = MSimUtils.DEFAULT_SLOT_ID;
            }

            if (doSimEventOnCarrierConfigChanged(slotId, false)) {
                return;
            }

            if (removeInternalMessageIfPresent(InternalMsg.SIM_REMOVAL_COMPLETED)) {
                handleOperatorOrServiceOnSimRemovalCompleted(slotId);
            }
            return;
        }

        displayCarrierConfigs(slotId, subId);

        doSimEventOnCarrierConfigChanged(slotId, true);

        if (sSOD.getMaxSlot() > 1) {
            doCarrierConfigChangedForOtherSlots(slotId);
        }
    }

    private void handleOperatorOrServiceOnSimRemovalCompleted(int slotId) {
        Log.i(TAG, "OperatorOrServiceOnSimRemovalCompleted :: slotId=" + slotId);

        // For KR OPEN, keep the enablers
        if (!sSOD.isKrOpen()
                && ("KR".equalsIgnoreCase(ImsProperties.TARGET_COUNTRY)
                    || ImsUtils.isEmergencyCallEnabledOnNonVoLteSim())) {
            sSOD.decideOperator(slotId);
            sSOD.showAllInfo(slotId);

            boolean initialStart = sSOD.isInitialStart(slotId);
            boolean activeSlot = sSOD.isActive(slotId);
            boolean voLteEnabled = activeSlot && sSOD.isVoLTEAvailable(slotId);

            Log.i(TAG, "SLOT[" + slotId + "/" + (activeSlot ? "active" : "non-active") + "] "
                    + "VoLTE=" + voLteEnabled);

            // Check whether service should be started or not
            if (!sSOD.isChanged(slotId)) {
                Log.w(TAG, "ImsPolicy :: Operator not changed");

                if (!"KR".equalsIgnoreCase(ImsProperties.TARGET_COUNTRY)) {
                    boolean serviceChanged = sSOD.isUpdated(slotId);

                    Log.w(TAG, "ImsPolicy :: serviceChanged=" + serviceChanged);

                    // If service feature is changed, Ims stop/start is required
                    if (serviceChanged) {
                        stopServices(slotId);

                        deliverUpdateServiceInfo(slotId);

                        if (voLteEnabled) {
                            startAgents(slotId);
                        }

                        updateSystemConfigOnServiceChanged(slotId);
                    }

                    if (voLteEnabled && serviceChanged) {
                        startVoLTEService(slotId);
                    }
                }

                return;
            }

            if (voLteEnabled != true) {
                Log.w(TAG, "ImsPolicy :: VoLTE disabled");
                return;
            }

            loadConfigAndStartServices(slotId, initialStart, voLteEnabled, true, false);

            sSOD.setChanged(slotId, false);
        }
    }

    private void doSimStateChangedForOtherSlots(int slotId) {
        List<Integer> otherSlotIds = sSOD.getOtherSlotIds(slotId);

        if (otherSlotIds == null) {
            return;
        }

        for (int i = 0; i < otherSlotIds.size(); i++) {
            int otherSlotId = (int)otherSlotIds.get(i);
            procSimStateChanged(otherSlotId, getSimState(otherSlotId),
                    MSimUtils.getSubId(otherSlotId));
        }
    }

    private void doCarrierConfigChangedForOtherSlots(int slotId) {
        List<Integer> otherSlotIds = sSOD.getOtherSlotIds(slotId);

        if (otherSlotIds == null) {
            return;
        }

        for (int i = 0; i < otherSlotIds.size(); i++) {
            int otherSlotId = (int)otherSlotIds.get(i);
            int subId = MSimUtils.getSubId(otherSlotId);

            if (!SubscriptionManager.isUsableSubscriptionId(subId)) {
                continue;
            }

            displayCarrierConfigs(otherSlotId, subId);

            doSimEventOnCarrierConfigChanged(otherSlotId, true);
        }
    }

    private boolean doSimEventOnCarrierConfigChanged(int slotId, boolean doSimLoaded) {
        if (doSimLoaded && sSOD.isSIMLoaded(slotId)) {
            Log.i(TAG, "CarrierConfigChanged :: SimLoaded");
            proc_SIM_LOADED(slotId, IccCardConstants.INTENT_VALUE_ICC_LOADED);
            return true;
        } else if (sSOD.isSIMAbsent(slotId)) {
            if (ImsConstants.PLATFORM_LAMPLITE) {
                Log.i(TAG, "CarrierConfigChanged :: SimAbsent");
                proc_SIM_ABSENT(slotId, IccCardConstants.INTENT_VALUE_ICC_ABSENT);
                return true;
            }
        }

        return false;
    }

    private boolean removeInternalMessageIfPresent(int msg) {
        if (sHandler.hasMessages(msg)) {
            sHandler.removeMessages(msg);
            return true;
        }

        return false;
    }

    private void loadConfigAndStartServices(int slotId, boolean initialStart,
            boolean voLteEnabled, boolean stopServicesIfRunning, boolean updateFeatures) {
        if (updateFeatures) {
            updateImsFeatures(slotId, false);
        }

        // Deliver operator information if it's not delivered
        if (!sSOD.isDelivered(slotId)) {
            deliverOperatorInfo(slotId);
            updateIMSDB(slotId);
        }

        if (voLteEnabled) {
            updateVoLteConfigurationForOperatorSpecific(slotId);
        }

        if (!isCommonAgentReady()) {
            startServicesWithDefaultAgents(slotId, voLteEnabled);
        } else {
            if (stopServicesIfRunning) {
                stopServices(slotId);
            }

            startServices(slotId, voLteEnabled);
        }

        if (initialStart) {
            sSOD.setInitialStart(slotId, false);
        }
    }

    private void startServices(int slotId, boolean voLteEnabled) {
        startAgents(slotId);
        updateSystemConfigOnSimLoaded(slotId);

        if (voLteEnabled) {
            startVoLTEService(slotId);
        }
    }

    private void startServicesWithDefaultAgents(int slotId, boolean voLteEnabled) {
        initCommonAgent();
        startAgents(slotId);
        completeCommonAgent();

        if (voLteEnabled) {
            startVoLTEService(slotId);
        }
    }

    private void stopServices(int slotId) {
        if (sSOD.isVoLTEStart(slotId)) {
            stopVoLTEService(slotId);
        }

        if (sSOD.isCommonStart(slotId)) {
            stopAgents(slotId);
        }
    }

    private void startAgents(int slotId) {
        if (sSOD.isCommonStart(slotId)) {
            Log.i(TAG, "startAgents(" + slotId + ") is already done!!!");
            return;
        }

        CommonStarter cs = CommonStarter.getInstance();
        cs.startAgents(slotId);

        sSOD.setCommonStart(slotId, true);
    }

    private void stopAgents(int slotId) {
        if (sSOD.isCommonStart(slotId) != true) {
            Log.i(TAG, "stopAgents(" + slotId + ") is already done!!!");
            return;
        }

        CommonStarter cs = CommonStarter.getInstance();
        cs.stopAgents(slotId);

        sSOD.setCommonStart(slotId, false);
    }

    private void startVoLTEService(int slotId) {
        if (sSOD.isVoLTEStart(slotId)) {
            Log.i(TAG, "startVoLTEService(" + slotId + ") is already done!!!");
            return;
        }

        Log.i(TAG, "startVoLTEService(" + slotId + ")");

        com.android.imsstack.core.ImsGlobal.create(this);

        ImsServiceController.start(this, slotId);

        // TODO
        // Set Extra with slotId in Intent

        VoLteFactory.getInstance().startService(this, slotId);

        sSOD.setVoLTEStart(slotId, true);

        CommonStarter.getInstance().notifyVoltePackageReady(slotId);
    }

    private void stopVoLTEService(int slotId) {
        if (sSOD.isVoLTEStart(slotId) != true) {
            Log.i(TAG, "stopVoLTEService(" + slotId + ") is already done!!!");
            return;
        }

        Log.i(TAG, "stopVoLTEService(" + slotId + ")");

        VoLteFactory.getInstance().stopService(slotId);

        sSOD.setVoLTEStart(slotId, false);
    }

    private synchronized boolean isCommonAgentReady() {
        CommonStarter cs = CommonStarter.getInstance();
        return cs.isCommonAgentReady();
    }

    private synchronized void initCommonAgent() {
        Log.i(TAG, "initCommonAgent");
        CommonStarter cs = CommonStarter.getInstance();
        cs.createAgents();
    }

    private synchronized void completeCommonAgent() {
        Log.i(TAG, "completeCommonAgent");
        CommonStarter cs = CommonStarter.getInstance();
        cs.createJNI();
        cs.setCommonAgentCompleted();
    }

    private synchronized void updateImsFeatures(int slotId, boolean notifyFeatureChanged) {
        CommonStarter cs = CommonStarter.getInstance();
        cs.updateImsFeatures(this, slotId, notifyFeatureChanged);
    }

    private void deliverOperatorInfo(int slotId) {
        sSOD.deliverOperatorInfo(slotId);
        sSOD.setDelivered(slotId, true);
        sSOD.setUpdated(slotId, false);
    }

    private void deliverUpdateServiceInfo(int slotId) {
        sSOD.deliverUpdateServiceInfo(slotId);
        sSOD.setUpdated(slotId, false);
    }

    private void deliverDDSInfo(int slotId, boolean dds) {
        CommonStarter cs = CommonStarter.getInstance();
        cs.deliverDDSInfo(slotId, dds);
    }

    private void updateDDSChanged(int slotId, boolean ddsChanged) {
        if (isCommonAgentReady()) {
            CommonStarter cs = CommonStarter.getInstance();
            cs.updateSystemConfigOnDDSChanged(slotId, ddsChanged);
        } else {
            Log.i(TAG, "updateDDSChanged(" + slotId + "/" + ddsChanged + "): ignored...");
        }
    }

    private void updateIMSDB(int slotId) {
        CommonStarter cs = CommonStarter.getInstance();

        cs.updateIMSDBByConfig(this, slotId);
        updateServiceAvailability(slotId);
    }

    private void updateServiceAvailability(int slotId) {
        com.android.imsstack.core.config.IConfigDBLoader dbLoader
                = CommonStarter.getInstance().getDBLoader(slotId);

        if (dbLoader != null) {
            com.android.imsstack.core.config.ProviderDBUpdateHelper dbHelper
                    = new com.android.imsstack.core.config.ProviderDBUpdateHelper(
                            slotId, dbLoader.getXmlLoader());

            dbHelper.updateServiceAvailability(this);
        }
    }

    private void updateSystemConfigOnServiceChanged(int slotId) {
        CommonStarter cs = CommonStarter.getInstance();
        cs.updateSystemConfigOnServiceChanged(slotId);
    }

    private void updateSystemConfigOnSimLoaded(int slotId) {
        if (isCommonAgentReady()) {
            CommonStarter cs = CommonStarter.getInstance();
            cs.updateSystemConfigOnSimLoaded(slotId);
        } else {
            Log.i(TAG, "updateSystemConfigOnSimLoaded(" + slotId + "): ignored...");
        }
    }

    private void updateVoLteConfigurationForOperatorSpecific(int slotId) {
        Log.i(TAG, "updateVoLteConfigurationForOperatorSpecific(" + slotId + ")");

        com.android.imsstack.core.config.IConfigDBLoader dbLoader
                = CommonStarter.getInstance().getDBLoader(slotId);

        if (dbLoader != null) {
            com.android.imsstack.core.config.ProviderDBUpdateHelper dbHelper
                    = new com.android.imsstack.core.config.ProviderDBUpdateHelper(
                            slotId, dbLoader.getXmlLoader());

            dbHelper.updateDBForOperatorSpecific(this);
        }
    }

    private boolean updateVoLteConfigurationForOperatorSpecificOnSimLoaded(int slotId) {
        if (com.android.imsstack.core.config.ProviderDBUpdateHelper
                .isDBUpdateRequiredForOperatorSpecificOnSimLoaded(slotId)) {
            Log.i(TAG, "updateVoLteConfigurationForOperatorSpecificOnSimLoaded :: slotId=" + slotId);

            com.android.imsstack.core.config.IConfigDBLoader dbLoader
                    = CommonStarter.getInstance().getDBLoader(slotId);

            if (dbLoader != null) {
                com.android.imsstack.core.config.ProviderDBUpdateHelper dbHelper
                        = new com.android.imsstack.core.config.ProviderDBUpdateHelper(
                                slotId, dbLoader.getXmlLoader());

                return dbHelper.updateDBForOperatorSpecificOnSimLoaded(this);
            }
        }

        return false;
    }

    private void initOnDeviceBootUp() {
        boolean deviceBootUp = false;
        int bootCount = SettingsUtils.getBootCount(this.getContentResolver());
        int lastBootCount = ImsPrivateProperties.Persistent.getInt(
                ImsPrivateProperties.Persistent.KEY_LAST_BOOT_COUNT, -1, 0);

        if ((bootCount != -1) && (bootCount != lastBootCount)) {
            ImsPrivateProperties.Persistent.setInt(
                    ImsPrivateProperties.Persistent.KEY_LAST_BOOT_COUNT, bootCount, 0);

            deviceBootUp = true;
        }

        if (deviceBootUp) {
            Log.i(TAG, "Device boot-up");
        }
    }

    // Carrier Configurations -- starts
    private void displayCarrierConfigs(int phoneId, int subId) {
        boolean isVoLteEnabled = getBooleanCarrierConfig(subId,
                CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        boolean isVtEnabled = getBooleanCarrierConfig(subId,
                CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
        boolean isWfcEnabled = getBooleanCarrierConfig(subId,
                CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);

        boolean isDeviceVoLteEnabled = getResources().getBoolean(
                com.android.internal.R.bool.config_device_volte_available);
        boolean isDeviceVtEnabled = getResources().getBoolean(
                com.android.internal.R.bool.config_device_vt_available);
        boolean isDeviceWfcEnabled = getResources().getBoolean(
                com.android.internal.R.bool.config_device_wfc_ims_available);

        boolean isDeviceVoLteEnabledForSub = false;
        boolean isDeviceVtEnabledForSub = false;
        boolean isDeviceWfcEnabledForSub = false;
        boolean logForMultiIms = false;

        if (ImsConstants.PLATFORM_LAMPLITE && ImsConstants.DBG && MSimUtils.isMultiSimEnabled()) {
            logForMultiIms = true;

            int mcc = -1;
            int mnc = -1;
            TelephonyManager tm = AppContext.getTelephonyManager(subId);
            String mccmnc = (tm != null) ? tm.getSimOperator() : "";

            if (!TextUtils.isEmpty(mccmnc)) {
                try {
                    mcc = Integer.parseInt(mccmnc.substring(0, 3));
                    mnc = Integer.parseInt(mccmnc.substring(3));
                } catch (NumberFormatException e) {
                    Log.i(TAG, "Resources :: op=" + mccmnc
                            + "; phoneId=" + phoneId + ", exception=" + e);
                }
            }

            if (mcc != -1 && mnc != -1) {
                Configuration config = new Configuration();

                config.mcc = mcc;
                config.mnc = (mnc == 0 ? Configuration.MNC_ZERO : mnc);

                Context context = createConfigurationContext(config);

                isDeviceVoLteEnabledForSub = context.getResources().getBoolean(
                        com.android.internal.R.bool.config_device_volte_available);

                isDeviceVtEnabledForSub = context.getResources().getBoolean(
                        com.android.internal.R.bool.config_device_vt_available);

                isDeviceWfcEnabledForSub = context.getResources().getBoolean(
                        com.android.internal.R.bool.config_device_wfc_ims_available);

                context = null;
            }
        }

        Log.i(TAG, "CarrierConfig - isVoLteEnabled="
                + isVoLteEnabled + ", isVtEnabled=" + isVtEnabled
                + ", isWfcEnabled=" + isWfcEnabled);
        Log.i(TAG, "Device - isVoLteEnabled="
                + isDeviceVoLteEnabled + ", isVtEnabled=" + isDeviceVtEnabled
                + ", isWfcEnabled=" + isDeviceWfcEnabled);

        if (logForMultiIms) {
            Log.i(TAG, "Device - subId=" + subId + ", isVoLteEnabled="
                    + isDeviceVoLteEnabledForSub + ", isVtEnabled=" + isDeviceVtEnabledForSub
                    + ", isWfcEnabled=" + isDeviceWfcEnabledForSub);
        }
    }

    private boolean getBooleanCarrierConfig(int subId, String key) {
        CarrierConfigManager ccm = getSystemService(CarrierConfigManager.class);
        PersistableBundle b = (ccm != null) ? ccm.getConfigForSubId(subId) : null;

        if (b != null) {
            return b.getBoolean(key);
        } else {
            return CarrierConfigManager.getDefaultConfig().getBoolean(key);
        }
    }

    private int getIntCarrierConfig(int subId, String key) {
        CarrierConfigManager ccm = getSystemService(CarrierConfigManager.class);
        PersistableBundle b = (ccm != null) ? ccm.getConfigForSubId(subId) : null;

        if (b != null) {
            return b.getInt(key);
        } else {
            return CarrierConfigManager.getDefaultConfig().getInt(key);
        }
    }
    // Carrier Configurations -- ends

    private boolean isCarrierConfigLoaded(int slotId, int subId) {
        CarrierConfigManager ccm = getSystemService(CarrierConfigManager.class);
        PersistableBundle b = (ccm != null) ? ccm.getConfigForSubId(subId) : null;
        return CarrierConfigManager.isConfigForIdentifiedCarrier(b);
    }

    private void initSimOperator() {
        /*
        for (int i = 0; i < MSimUtils.getMaxSimSlot(); i++) {
            String simOperator = ImsProperties.getSysSimOperator(this, i);

            if (TextUtils.isEmpty(simOperator)) {
                CarrierCodeLoader.setDefaultSimOperatorCountry(i);
            }
        }
        */
    }

    private void decideSimOperator(int slotId, String iccState) {
        /*
        boolean simLocked = sSOD.isSIMLocked(iccState);
        CarrierCodeLoader ccl = CarrierCodeLoader.getInstance();
        SimCarrierId id = CarrierCodeLoader.getCarrierIdFromSim(slotId, simLocked);
        CarrierCode oldCc = ccl.getCarrierCode(slotId);
        CarrierCode cc = ccl.fetchCarrierCode(id, slotId);

        if (cc == null) {
            // Use the latest SIM operator/country
            Log.d(TAG, "CarrierCode is null");
            return;
        }

        if (!cc.equals(oldCc)) {
            // SimState: locked or locked -> loaded
            if (sSOD.isSIMLocked(slotId)) {
                if (sSOD.isSIMLoaded(slotId)) {
                    CarrierCodeLoader.setSimOperatorCountryOnSimLoaded(cc, slotId);
                } else {
                    CarrierCodeLoader.setSimOperatorCountry(cc, slotId);
                }
            }
            // SimState: loaded
            else if (sSOD.isSIMLoaded(slotId)) {
                CarrierCodeLoader.setSimOperatorCountryOnSimLoaded(cc, slotId);
            }
        } else {
            Log.dc(TAG, "CarrierCode is same");

            // SimState: locked -> loaded
            if (sSOD.isSIMLocked(slotId)) {
                if (sSOD.isSIMLoaded(slotId)) {
                    CarrierCodeLoader.setSimOperatorCountryOnSimLoaded(cc, slotId);
                }
            }
        }
        */
    }

    private static ImsCarrierResolver.Carrier resolveImsCarrier(int slotId, int subId,
            int carrierId, int specificCarrierId) {
        int testCarrierId = ImsPrivateProperties.Persistent.getInt(
                ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, slotId);

        if (testCarrierId > 0) {
            Log.d(TAG, "resolveImsCarrier: testCarrierId=" + testCarrierId +
                    ", carrierId=" + carrierId);
            carrierId = testCarrierId;
        }

        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(
                slotId, subId, carrierId, specificCarrierId);

        Log.d(TAG, "resolveImsCarrier: " + carrier.toString());

        return carrier;
    }

    private static String getSimState(int slotId) {
        return com.android.imsstack.util.ImsExtApi.Uicc.getSimState(slotId);
    }

    private static class SbpInfo {
        static final String KEY_VOLTE = "volte";
        static final String KEY_VILTE = "vilte";
        static final String KEY_VOWIFI = "vowifi";

        static final String KEY_VOLTE2 = "volte2";
        static final String KEY_VILTE2 = "vilte2";
        static final String KEY_VOWIFI2 = "vowifi2";

        static final int NOT_SET = -1;
        static final int SERVICE_ON = 1;

        int mSlotId = 0;
        int mVoLte = NOT_SET;
        int mViLte = NOT_SET;
        int mVoWifi = NOT_SET;

        int getSlotId() {
            return mSlotId;
        }

        int toSODConfig(int activeServices) {
            int sodConfig = activeServices;

            if (mVoLte != NOT_SET) {
                sodConfig = setOrClear(sodConfig,
                        (mVoLte == SERVICE_ON), SODConfig.SUPPORT_VOLTE);
            }

            if (mViLte != NOT_SET) {
                sodConfig = setOrClear(sodConfig,
                        (mViLte == SERVICE_ON), SODConfig.SUPPORT_VT);
            }

            if (mVoWifi != NOT_SET) {
                sodConfig = setOrClear(sodConfig,
                        (mVoWifi == SERVICE_ON), SODConfig.SUPPORT_VOWIFI);
            }

            return sodConfig;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ SbpInfo: slotId=");
            sb.append(mSlotId);
            sb.append(", voLte=");
            sb.append(mVoLte);
            sb.append(", viLte=");
            sb.append(mViLte);
            sb.append(", voWifi=");
            sb.append(mVoWifi);
            sb.append(" ]");

            return sb.toString();
        }

        static SbpInfo fromVoLteInfo(Intent intent) {
            SbpInfo sbpInfo = null;

            String voLte = intent.getStringExtra(KEY_VOLTE);
            String viLte = intent.getStringExtra(KEY_VILTE);
            String voWifi = intent.getStringExtra(KEY_VOWIFI);

            if (!TextUtils.isEmpty(voLte)
                    || !TextUtils.isEmpty(viLte)
                    || !TextUtils.isEmpty(voWifi)) {
                sbpInfo = new SbpInfo();
                sbpInfo.mSlotId = 0;
                sbpInfo.mVoLte = toIntValue(voLte);
                sbpInfo.mViLte = toIntValue(viLte);
                sbpInfo.mVoWifi = toIntValue(voWifi);
            } else {
                voLte = intent.getStringExtra(KEY_VOLTE2);
                viLte = intent.getStringExtra(KEY_VILTE2);
                voWifi = intent.getStringExtra(KEY_VOWIFI2);

                if (!TextUtils.isEmpty(voLte)
                        || !TextUtils.isEmpty(viLte)
                        || !TextUtils.isEmpty(voWifi)) {
                    sbpInfo = new SbpInfo();
                    sbpInfo.mSlotId = 1;
                    sbpInfo.mVoLte = toIntValue(voLte);
                    sbpInfo.mViLte = toIntValue(viLte);
                    sbpInfo.mVoWifi = toIntValue(voWifi);
                }
            }

            if ((sbpInfo == null) && (sSOD.getMaxSlot() == 1)) {
                sbpInfo = new SbpInfo();
            }

            return sbpInfo;
        }

        static int setOrClear(int services, boolean value, int flag) {
            if (value) {
                services |= flag;
            } else {
                services &= (~flag);
            }

            return services;
        }

        static int toIntValue(String value) {
            if (TextUtils.isEmpty(value)) {
                return NOT_SET;
            }

            try {
                return Integer.parseInt(value);
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }

            return NOT_SET;
        }
    }
}
