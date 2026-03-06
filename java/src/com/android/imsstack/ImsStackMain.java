/*
 * Copyright (C) 2023 The Android Open Source Project
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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.ImsCarrierResolver;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.imsservice.ImsServiceController;
import com.android.imsstack.test.ImsTestHelper;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;

/**
 * A main entry point of starting and stopping the ImsStack core component.
 */
public class ImsStackMain {
    private static final int DEFAULT_SIM_COUNT = 2;

    private final ServiceLoader mServiceLoader = new ServiceLoader();
    private final SparseArray<SlotState> mSlotState = new SparseArray<>(DEFAULT_SIM_COUNT);
    private int mActiveSimCount = DEFAULT_SIM_COUNT;
    private IntentReceiver mIntentReceiver;

    public ImsStackMain() {}

    /**
     * Starts the ImsStack.
     *
     * @param applicationContext The application context.
     */
    public void start(Context applicationContext) {
        // AppContext will be used globally,
        // so it should be initialized first when ImsStack process starts.
        AppContext.init(applicationContext);
        // Sets the log options once when ImsStack is created.
        String logOptions = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_TEST_LOG_OPTIONS,
                ImsUtils.IS_USER ? Log.RELEASE_LOG_OPTIONS : Log.DEFAULT_LOG_OPTIONS,
                0);
        boolean debugEnabled = ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED, 0);
        Log.init(ImsUtils.hexStringToInt(logOptions), debugEnabled);
        Log.i(this, "start");

        DeviceConfig.init(AppContext.getInstance());
        mActiveSimCount = DeviceConfig.getActiveSimCount();
        int supportedSimCount = DeviceConfig.getSupportedSimCount();

        // Clear Ephemeral properties for IMS internal usage
        for (int i = 0; i < supportedSimCount; ++i) {
            ImsPrivateProperties.Ephemeral.removeAll(i);
        }

        mIntentReceiver = new IntentReceiver();
        mIntentReceiver.startListening();

        for (int i = 0; i < supportedSimCount; ++i) {
            mSlotState.put(i, new SlotState(i));
        }

        AppContext.runTask(() -> {
            onStart();
        }, 0);
    }

    /**
     * Stops the ImsStack.
     */
    public void stop() {
        if (mIntentReceiver != null) {
            mIntentReceiver.stopListening();
        }
    }

    private SlotState getSlotState(int slotId) {
        return mSlotState.get(slotId);
    }

    private void onStart() {
        Log.setDebuggable();

        Log.i(this, "onStart");

        for (int i = 0; i < mActiveSimCount; ++i) {
            int subId = MSimUtils.getSubId(i);
            int simState = getSimState(i, subId);

            simState = refineSimState(simState);

            if (simState == Sim.STATE_LOADED
                    || simState == Sim.STATE_LOCKED
                    || simState == Sim.STATE_ABSENT) {
                Log.d(this, "SimState: onStart");
                processSimState(i, subId, simState);
            }
        }
    }

    private void handleSimStateChanged(Intent intent) {
        boolean userUnlocked = intent.getBooleanExtra(Intent.EXTRA_REBROADCAST_ON_UNLOCK, false);
        int defaultSlotId = DeviceConfig.isMultiSimEnabled()
                ? MSimUtils.INVALID_SLOT_ID
                : MSimUtils.DEFAULT_SLOT_ID;
        int slotId = Sim.getExtraSlotIndex(intent, defaultSlotId);
        int subId = Sim.getExtraSubscriptionIndex(intent);
        int simState = Sim.getSimStateFromTelephonySimState(Sim.getExtraSimState(intent));

        Log.w(this, "handleSimStateChanged: state=" + Sim.stateToString(simState)
                + ", slotId=" + slotId + ", subId=" + subId + ", userUnlocked=" + userUnlocked);

        if (slotId == MSimUtils.INVALID_SLOT_ID) {
            Log.e(this, "Invalid slotId!!!");
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
        if (mActiveSimCount > 1) {
            for (int i = 0; i < mActiveSimCount; ++i) {
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
            if (!isCarrierConfigLoaded(subId)) {
                Log.d(this, "SimState: wait for CarrierConfigChanged on LOADED in slot" + slotId);
                return;
            }
        } else if (simState == Sim.STATE_ABSENT) {
            if (oldSimState == Sim.STATE_LOCKED || oldSimState == Sim.STATE_LOADED) {
                Log.d(this, "SimState: wait for CarrierConfigChanged on ABSENT in slot" + slotId);
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

        Log.i(this, "SimState: carrierChanged=" + carrierChanged + ", " + slotState.toString());

        Context context = AppContext.getInstance();

        if (!carrierChanged) {
            if (slotState.isServiceStarted()) {
                if (slotState.isCarrierConfigChanged()) {
                    Log.d(this, "SimState: carrier-config is changed while running.");
                    ServiceCaps.updateServiceCapabilities(context, slotId, subId);
                    ServiceLoader.updateCarrierConfig(slotId);
                    ServiceLoader.initUserSettings(slotId);
                }
            } else {
                ServiceCaps.updateServiceCapabilities(context, slotId, subId);
                loadConfigAndStartServices(slotId);
                slotState.setServiceStarted(true);
            }
        } else {
            if (slotState.isServiceStarted()) {
                SimCarrierId carrierId = CarrierInfo.getInstance().getCarrierId(slotId);

                if (simState == Sim.STATE_ABSENT && carrierId.isSimAbsent()) {
                    Log.i(this, "SimState: Postpone service restart until SIM is reinserted.");
                    return;
                }

                stopServices(slotId);
            }

            ServiceCaps.updateServiceCapabilities(context, slotId, subId);
            loadConfigAndStartServices(slotId);
            slotState.setServiceStarted(true);
        }
    }

    private void handleCarrierConfigChanged(int slotId,
            int subId, int carrierId, int specificCarrierId) {
        Log.d(this, "handleCarrierConfigChanged :: slotId=" + slotId + ", subId=" + subId
                + ", carrierId=" + carrierId + ", specificCarrierId=" + specificCarrierId);

        ImsCarrierResolver.Carrier carrier =
                resolveImsCarrier(slotId, carrierId, specificCarrierId);

        CarrierInfo.setSimOperatorCountry(carrier.getOperator(),
                carrier.getOperatorSub(), carrier.getCountry(), slotId);

        if (slotId == MSimUtils.INVALID_SLOT_ID) {
            // TODO: update carrier-info
            return;
        }

        displayCarrierConfigs(slotId, subId);
        SlotState slotState = getSlotState(slotId);
        slotState.setCarrierConfigChanged(true);
        processCarrierConfigState(slotId, subId);
        slotState.setCarrierConfigChanged(false);
        processCarrierConfigStateForOtherSlots(slotId);
    }

    private void processCarrierConfigState(int slotId, int subId) {
        SlotState slotState = getSlotState(slotId);
        processSimState(slotId, subId, slotState.getSimState());
    }

    private void processCarrierConfigStateForOtherSlots(int slotId) {
        if (mActiveSimCount > 1) {
            for (int i = 0; i < mActiveSimCount; ++i) {
                if (i != slotId) {
                    int subId = MSimUtils.getSubId(i);

                    displayCarrierConfigs(i, subId);
                    processCarrierConfigState(i, subId);
                }
            }
        }
    }

    private void handleMultiSimConfigChanged(Intent intent) {
        int activeSimCount = intent.getIntExtra(
                TelephonyManager.EXTRA_ACTIVE_SIM_SUPPORTED_COUNT, 0);

        Log.d(this, "handleMultiSimConfigChanged :: " + mActiveSimCount + " >> " + activeSimCount);

        if (activeSimCount > 0) {
            mActiveSimCount = activeSimCount;
        }
    }

    private void loadConfigAndStartServices(int slotId) {
        if (!mServiceLoader.isInitialized()) {
            mServiceLoader.initJni();
            mServiceLoader.init();
            mServiceLoader.start(slotId);
            startVoLteService(slotId);
            mServiceLoader.startNativeEnabler(slotId);
        } else {
            startServices(slotId);
        }
    }

    private void startServices(int slotId) {
        mServiceLoader.start(slotId);
        startVoLteService(slotId);
        mServiceLoader.startNativeEnabler(slotId);
    }

    private void stopServices(int slotId) {
        stopVoLteService(slotId);
        mServiceLoader.stop(slotId);
        mServiceLoader.stopNativeEnabler(slotId);
    }

    private void startVoLteService(int slotId) {
        Log.i(this, "startVoLteService(" + slotId + ")");

        ImsServiceController.start(AppContext.getInstance(), slotId);

        if (ImsTestMode.getInstance().getTestMode(slotId).isGenericTestMode()) {
            ImsTestHelper.getInstance().init();
        }
    }

    private void stopVoLteService(int slotId) {
        Log.i(this, "stopVoLteService(" + slotId + ")");

        if (ImsTestMode.getInstance().getTestMode(slotId).isGenericTestMode()) {
            ImsTestHelper.getInstance().cleanup();
        }
    }

    private void displayCarrierConfigs(int phoneId, int subId) {
        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);
        PersistableBundle b = ccmp.getConfigForSubId(subId,
                CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL,
                CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL,
                CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);

        boolean voLteEnabled = b.getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        boolean vtEnabled = b.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
        boolean wfcEnabled = b.getBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);

        Resources resources = AppContext.getInstance().getResources();
        boolean deviceVoLteEnabled = resources.getBoolean(
                com.android.internal.R.bool.config_device_volte_available);
        boolean deviceVtEnabled = resources.getBoolean(
                com.android.internal.R.bool.config_device_vt_available);
        boolean deviceWfcEnabled = resources.getBoolean(
                com.android.internal.R.bool.config_device_wfc_ims_available);

        Log.i(this, "CarrierConfig(" + phoneId + ") - voLteEnabled=" + voLteEnabled
                + ", vtEnabled=" + vtEnabled + ", wfcEnabled=" + wfcEnabled
                + ", identifiedCarrier=" + ccmp.isConfigForIdentifiedCarrier(b));
        Log.i(this, "Device(" + phoneId + ") - voLteEnabled=" + deviceVoLteEnabled
                + ", vtEnabled=" + deviceVtEnabled + ", wfcEnabled=" + deviceWfcEnabled);
    }

    private boolean isCarrierConfigLoaded(int subId) {
        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);
        return ccmp.isConfigForIdentifiedCarrier(ccmp.getConfigForSubId(
                subId, CarrierConfigManager.KEY_CARRIER_CONFIG_APPLIED_BOOL));
    }

    private ImsCarrierResolver.Carrier resolveImsCarrier(int slotId,
            int carrierId, int specificCarrierId) {
        int testCarrierId = ImsPrivateProperties.Persistent.getInt(
                ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, slotId);
        int testSpecificCarrierId = ImsPrivateProperties.Persistent.getInt(
                ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID, slotId);

        if (testCarrierId > 0) {
            Log.d(this, "resolveImsCarrier: testCarrierId=" + testCarrierId
                    + ", carrierId=" + carrierId);
            carrierId = testCarrierId;
        }

        if (testSpecificCarrierId > 0) {
            Log.d(this, "resolveImsCarrier: testSpecificCarrierId=" + testSpecificCarrierId
                    + ", specificCarrierId=" + specificCarrierId);
            specificCarrierId = testSpecificCarrierId;
        }

        SimCarrierId scid = new SimCarrierId.Builder()
                .setCarrierId(carrierId)
                .setSpecificCarrierId(specificCarrierId)
                .build();
        ImsCarrierResolver.Carrier carrier = ImsCarrierResolver.getCarrierFromCarrierId(scid);

        Log.d(this, "resolveImsCarrier: " + carrier.toString());

        return carrier;
    }

    private static int getSimState(int slotId, int subId) {
        if (!MSimUtils.isUsableSubId(subId)) {
            subId = MSimUtils.getSubId(slotId);
        }

        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(subId);
        return Sim.getSimStateFromTelephonySimState(tmp.getSimApplicationState());
    }

    private static int refineSimState(int simState) {
        int simCardState = Sim.getSimCardStateFromTelephonySimState(simState);

        if (simCardState == Sim.STATE_ABSENT) {
            return Sim.STATE_ABSENT;
        }

        return simState;
    }

    private static final class SlotState {
        private final int mSlotId;
        private boolean mServiceStarted;
        // This holds a flag briefly while processing the carrier configuration changed event.
        private boolean mCarrierConfigChanged;
        private @Sim.State int mSimState = Sim.STATE_INVALID;
        private @Sim.State int mOldSimState = Sim.STATE_INVALID;

        SlotState(int slotId) {
            mSlotId = slotId;
        }

        public boolean isServiceStarted() {
            return mServiceStarted;
        }

        public void setServiceStarted(boolean started) {
            mServiceStarted = started;
        }

        public boolean isCarrierConfigChanged() {
            return mCarrierConfigChanged;
        }

        public void setCarrierConfigChanged(boolean changed) {
            mCarrierConfigChanged = changed;
        }

        public @Sim.State int getSimState() {
            return mSimState;
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
            sb.append(", carrierConfigChanged=");
            sb.append(mCarrierConfigChanged);
            sb.append(", simState=");
            sb.append(Sim.stateToString(mSimState));
            sb.append(", oldSimState=");
            sb.append(Sim.stateToString(mOldSimState));
            sb.append(" ]");

            return sb.toString();
        }
    }

    private final class IntentReceiver extends BroadcastReceiver
            implements CarrierConfigManager.CarrierConfigChangeListener {

        public void startListening() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED);
            filter.addAction(TelephonyManager.ACTION_MULTI_SIM_CONFIG_CHANGED);

            AppContext.getInstance().getBroadcastReceiverProxy().registerReceiver(this, filter);
            CarrierConfigManagerProxy ccmp = AppContext.getInstance()
                    .getSystemServiceProxy(CarrierConfigManagerProxy.class);
            ccmp.registerCarrierConfigChangeListener(
                    AppContext.getInstance().getMainExecutor(), this);
        }

        public void stopListening() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
            CarrierConfigManagerProxy ccmp = AppContext.getInstance()
                    .getSystemServiceProxy(CarrierConfigManagerProxy.class);
            ccmp.unregisterCarrierConfigChangeListener(this);
        }

        @Override
        public void onCarrierConfigChanged(int logicalSlotIndex, int subscriptionId, int carrierId,
                int specificCarrierId) {
            handleCarrierConfigChanged(logicalSlotIndex,
                    subscriptionId, carrierId, specificCarrierId);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            Log.d(this, "ImsStackApp: " + Log.lastSubString(action, "."));

            if (TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED.equals(action)) {
                handleSimStateChanged(intent);
            } else if (TelephonyManager.ACTION_MULTI_SIM_CONFIG_CHANGED.equals(action)) {
                handleMultiSimConfigChanged(intent);
            }
        }
    }
}
