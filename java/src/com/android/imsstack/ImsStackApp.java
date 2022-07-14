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
package com.android.imsstack;

import android.app.Application;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.util.SparseArray;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.VoLteFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.imsservice.ImsServiceController;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;

/**
 * This is a main entry point to run ImsStack process.
 */
public class ImsStackApp extends Application {
    private static final String TAG = ImsStackApp.class.getSimpleName();

    private IntentReceiver mIntentReceiver;
    private SparseArray<SlotState> mSlotState = new SparseArray<>(2);

    @Override
    public void onCreate() {
        super.onCreate();

        Log.i(TAG, "onCreate");
        init();
    }

    @Override
    public void onTerminate() {
        super.onTerminate();

        Log.i(TAG, "onTerminate");
        deinit();
    }

    @Override
    public void onTrimMemory(int level) {
        super.onTrimMemory(level);

        Log.i(TAG, "onTrimMemory :: level=" + level);

        Runtime.getRuntime().gc();
    }

    private void init() {
        // AppContext will be used globally,
        // so it should be initialized first at the creation time of ImsStack application.
        AppContext.init(this);

        // Initialize any static data
        ImsUtils.init();

        int simCount = MSimUtils.getMaxSimSlot();

        // Clear Ephemeral properties for IMS internal usage
        for (int i = 0; i < simCount; ++i) {
            ImsPrivateProperties.Ephemeral.removeAll(i);
        }

        mIntentReceiver = new IntentReceiver();
        mIntentReceiver.startListening();

        for (int i = 0; i < simCount; ++i) {
            mSlotState.put(i, new SlotState(i));
        }

        AppContext.runTask(() -> {
            onStart();
        }, 0);
    }

    private void deinit() {
        if (mIntentReceiver != null) {
            mIntentReceiver.stopListening();
        }
    }

    private SlotState getSlotState(int slotId) {
        return mSlotState.get(slotId);
    }

    private void onStart() {
        Log.setDebuggable();

        Log.i(TAG, "onStart");

        int count = mSlotState.size();

        for (int i = 0; i < count; ++i) {
            int subId = MSimUtils.getSubId(i);
            int simState = getSimState(i, subId);

            simState = refineSimState(simState);

            if (simState == Sim.STATE_LOADED
                    || simState == Sim.STATE_LOCKED
                    || simState == Sim.STATE_ABSENT) {
                Log.d(TAG, "SimState: onStart");
                processSimState(i, subId, simState);
            }
        }
    }

    private void handleSimStateChanged(Intent intent) {
        boolean userUnlocked = intent.getBooleanExtra(Intent.EXTRA_REBROADCAST_ON_UNLOCK, false);
        int defaultSlotId = MSimUtils.isMultiSimEnabled()
                ? MSimUtils.INVALID_SLOT_ID
                : MSimUtils.DEFAULT_SLOT_ID;
        int slotId = Sim.getExtraSlotIndex(intent, defaultSlotId);
        int subId = Sim.getExtraSubscriptionIndex(intent);
        int simState = Sim.getSimStateFromTelephonySimState(Sim.getExtraSimState(intent));

        Log.w(TAG, "handleSimStateChanged: state=" + Sim.stateToString(simState)
                + ", slotId=" + slotId + ", subId=" + subId + ", userUnlocked=" + userUnlocked);

        if (slotId == MSimUtils.INVALID_SLOT_ID) {
            Log.e(TAG, "Invalid slotId!!!");
            return;
        }

        // Refine the SIM State using SIM card state
        simState = refineSimState(simState);

        switch (simState) {
            case Sim.STATE_LOADED: // FALL-THROUGH
            case Sim.STATE_LOCKED: // FALL-THROUGH
            case Sim.STATE_ABSENT:
                processSimState(slotId, subId, simState);
                processSimStateForOtherSlots(slotId);
                break;
            default:
                break;
        }
    }

    private void processSimStateForOtherSlots(int slotId) {
        int simCount = MSimUtils.getMaxSimSlot();

        if (simCount > 1) {
            for (int i = 0; i < simCount; ++i) {
                if (i != slotId) {
                    int subId = MSimUtils.getSubId(i);
                    processSimState(i, subId, getSimState(slotId, subId));
                }
            }
        }
    }

    private void processSimState(int slotId, int subId, @Sim.State int simState) {
        SlotState slotState = getSlotState(slotId);
        int oldSimState = slotState.getSimState();

        slotState.setSimState(simState);

        if (simState == Sim.STATE_LOADED) {
            if (!isCarrierConfigLoaded(slotId, subId)) {
                Log.d(TAG, "SimState: wait for CarrierConfigChanged");
                return;
            }
        } else if (simState == Sim.STATE_ABSENT) {
            if (oldSimState == Sim.STATE_LOCKED || oldSimState == Sim.STATE_LOADED) {
                Log.d(TAG, "SimState: wait for CarrierConfigChanged");
                return;
            }

            //// Test-Purpose {
            String simOp = ImsPrivateProperties.getSimOperator(slotId);
            String simCo = ImsPrivateProperties.getSimCountry(slotId);

            if (simOp.isEmpty() || simCo.isEmpty()) {
                CarrierInfo.setSimOperatorCountry("OPEN", "", "COM", slotId);
            }
            //// }
        }

        boolean carrierChanged = CarrierInfo.getInstance().updateCarrierId(slotId);

        Log.i(TAG, "SimState: carrierChanged=" + carrierChanged + ", " + slotState.toString());

        if (!carrierChanged) {
            if (slotState.isServiceStarted()) {
                Log.d(TAG, "SimState: carrier is not changed.");
            } else {
                loadConfigAndStartServices(slotId);
                slotState.setServiceStarted(true);
            }
        } else {
            if (slotState.isServiceStarted()) {
                stopServices(slotId);
                loadConfigAndStartServices(slotId);
            } else {
                loadConfigAndStartServices(slotId);
            }

            slotState.setServiceStarted(true);
        }
    }

    private void handleCarrierConfigChanged(Intent intent) {
        int subId = Sim.getExtraSubscriptionIndex(intent);
        int slotId = Sim.getExtraSlotIndex(intent);
        int carrierId = intent.getIntExtra(TelephonyManager.EXTRA_CARRIER_ID,
                TelephonyManager.UNKNOWN_CARRIER_ID);
        int specificCarrierId = intent.getIntExtra(TelephonyManager.EXTRA_SPECIFIC_CARRIER_ID,
                TelephonyManager.UNKNOWN_CARRIER_ID);

        Log.d(TAG, "handleCarrierConfigChanged :: slotId=" + slotId + ", subId=" + subId
                + ", carrierId=" + carrierId + ", specificCarrierId=" + specificCarrierId);

        ImsCarrierResolver.Carrier carrier =
                resolveImsCarrier(slotId, subId, carrierId, specificCarrierId);

        CarrierInfo.setSimOperatorCountry(carrier.getOperator(),
                carrier.getOperatorSub(), carrier.getCountry(), slotId);

        if (slotId == MSimUtils.INVALID_SLOT_ID) {
            // TODO: update carrier-info
            return;
        }

        displayCarrierConfigs(slotId, subId);

        processCarrierConfigState(slotId, subId, carrierId, specificCarrierId);
        processCarrierConfigStateForOtherSlots(slotId);
    }

    private void processCarrierConfigState(
            int slotId, int subId, int carrierId, int specificCarrierId) {
        SlotState slotState = getSlotState(slotId);
        processSimState(slotId, subId, slotState.getSimState());
    }

    private void processCarrierConfigStateForOtherSlots(int slotId) {
        int simCount = MSimUtils.getMaxSimSlot();

        if (simCount > 1) {
            for (int i = 0; i < simCount; ++i) {
                if (i != slotId) {
                    int subId = MSimUtils.getSubId(i);

                    displayCarrierConfigs(i, subId);

                    TelephonyManager tm = AppContext.getTelephonyManager(subId);

                    if (tm != null) {
                        processCarrierConfigState(i, subId,
                                tm.getSimCarrierId(), tm.getSimSpecificCarrierId());
                    }
                }
            }
        }
    }

    private void loadConfigAndStartServices(int slotId) {
        CommonStarter cs = CommonStarter.getInstance();

        if (!cs.isCommonAgentReady()) {
            cs.createAgents();
            cs.startAgents(slotId);
            cs.createJNI();
            cs.setCommonAgentCompleted();
            startVoLteService(slotId);
        } else {
            startServices(slotId);
        }
    }

    private void startServices(int slotId) {
        CommonStarter cs = CommonStarter.getInstance();
        cs.startAgents(slotId);

        updateSystemConfigOnSimLoaded(slotId);
        startVoLteService(slotId);
    }

    private void stopServices(int slotId) {
        stopVoLteService(slotId);

        CommonStarter cs = CommonStarter.getInstance();
        cs.stopAgents(slotId);
    }

    private void startVoLteService(int slotId) {
        Log.i(TAG, "startVoLteService(" + slotId + ")");

        com.android.imsstack.core.ImsGlobal.create(this);

        ImsServiceController.start(this, slotId);

        VoLteFactory.getInstance().startService(this, slotId);

        CommonStarter.getInstance().notifyVoltePackageReady(slotId);
    }

    private void stopVoLteService(int slotId) {
        Log.i(TAG, "stopVoLTEService(" + slotId + ")");
        VoLteFactory.getInstance().stopService(slotId);
    }

    private void updateSystemConfigOnSimLoaded(int slotId) {
        CommonStarter cs = CommonStarter.getInstance();

        if (cs.isCommonAgentReady()) {
            cs.updateSystemConfigOnSimLoaded(slotId);
        } else {
            Log.i(TAG, "updateSystemConfigOnSimLoaded(" + slotId + "): ignored...");
        }
    }

    private void displayCarrierConfigs(int phoneId, int subId) {
        CarrierConfigManager ccm = getSystemService(CarrierConfigManager.class);
        PersistableBundle b = (ccm != null)
                ? ccm.getConfigForSubId(subId)
                : CarrierConfigManager.getDefaultConfig();

        boolean voLteEnabled = b.getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        boolean vtEnabled = b.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
        boolean wfcEnabled = b.getBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);

        boolean deviceVoLteEnabled = getResources().getBoolean(
                com.android.internal.R.bool.config_device_volte_available);
        boolean deviceVtEnabled = getResources().getBoolean(
                com.android.internal.R.bool.config_device_vt_available);
        boolean deviceWfcEnabled = getResources().getBoolean(
                com.android.internal.R.bool.config_device_wfc_ims_available);

        Log.i(TAG, "CarrierConfig - voLteEnabled=" + voLteEnabled
                + ", vtEnabled=" + vtEnabled + ", wfcEnabled=" + wfcEnabled);
        Log.i(TAG, "Device - voLteEnabled=" + deviceVoLteEnabled
                + ", vtEnabled=" + deviceVtEnabled + ", wfcEnabled=" + deviceWfcEnabled);
    }

    private boolean isCarrierConfigLoaded(int slotId, int subId) {
        CarrierConfigManager ccm = getSystemService(CarrierConfigManager.class);
        PersistableBundle b = (ccm != null) ? ccm.getConfigForSubId(subId) : null;
        return CarrierConfigManager.isConfigForIdentifiedCarrier(b);
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

    private static int getSimState(int slotId, int subId) {
        if (!MSimUtils.isValidSubId(subId)) {
            subId = MSimUtils.getSubId(slotId);
        }

        TelephonyManager tm = AppContext.getTelephonyManager(subId);

        if (tm != null) {
            return Sim.getSimStateFromTelephonySimState(tm.getSimApplicationState());
        }

        return Sim.STATE_INVALID;
    }

    private static int refineSimState(int simState) {
        int simCardState = Sim.getSimCardStateFromTelephonySimState(simState);

        if (simCardState == Sim.STATE_ABSENT) {
            return Sim.STATE_ABSENT;
        }

        return simState;
    }

    private final class SlotState {
        private final int mSlotId;
        private boolean mServiceStarted;
        private boolean mCarrierChanged;
        private @Sim.State int mSimState = Sim.STATE_INVALID;
        private @Sim.State int mOldSimState = Sim.STATE_INVALID;

        SlotState(int slotId) {
            mSlotId = slotId;
        }

        public int getSlotId() {
            return mSlotId;
        }

        public boolean isServiceStarted() {
            return mServiceStarted;
        }

        public void setServiceStarted(boolean started) {
            mServiceStarted = started;
        }

        public boolean isCarrierChanged() {
            return mCarrierChanged;
        }

        public void setCarrierChanged(boolean changed) {
            mCarrierChanged = changed;
        }

        public @Sim.State int getSimState() {
            return mSimState;
        }

        public @Sim.State int getOldSimState() {
            return mOldSimState;
        }

        public void setSimState(@Sim.State int state) {
            if (mSimState != state) {
                mOldSimState = mSimState;
                mSimState = state;
            }
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ SlotState :: slotId=");
            sb.append(mSlotId);
            sb.append(", serviceStarted=");
            sb.append(mServiceStarted);
            sb.append(", carrierChanged=");
            sb.append(mCarrierChanged);
            sb.append(", simState=");
            sb.append(Sim.stateToString(mSimState));
            sb.append(", oldSimState=");
            sb.append(Sim.stateToString(mOldSimState));
            sb.append(" ]");

            return sb.toString();
        }
    }

    private final class IntentReceiver extends BroadcastReceiver {

        public void startListening() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
            filter.addAction(CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED);

            AppContext.getInstance().registerReceiver(this, filter, null,
                    AppContext.getInstance().getMainHandler(), Context.RECEIVER_EXPORTED);
        }

        public void stopListening() {
            AppContext.getInstance().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            Log.d(Log.TAG, "ImsStackApp: " + Log.lastSubString(action, "."));

            if (TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED.equals(action)) {
                handleSimStateChanged(intent);
            } else if (CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED.equals(action)) {
                handleCarrierConfigChanged(intent);
            }
        }
    }
}
