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

import android.content.Context;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.media.VideoConfigSpropGenerator;
import com.android.imsstack.internal.ImsStackRegistry;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.jni.JniImsProxy;
import com.android.imsstack.jni.NativeCommands;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.test.ImsTestMode;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.Log;

/**
 * A main entry for initializing, starting, and stopping the internal modules of ImsStack.
 */
public class ServiceLoader {
    private boolean mJniReady;
    private boolean mInitialized;

    ServiceLoader() {}

    /**
     * Checks whether the default agents are initialized and ready to use.
     *
     * @return {@code true} if the default agents are initialized, {@code false} otherwise.
     */
    public boolean isInitialized() {
        return mInitialized;
    }

    /**
     * Sets the device configuration and initializes JNI.
     */
    public void initJni() {
        if (mJniReady) {
            return;
        }
        Log.i(this, "initJni");
        NativeCommands.setDeviceConfig(AppContext.getInstance());
        JniImsProxy.init();
        mJniReady = true;
    }

    /**
     * Creates the default agents and initializes static components.
     */
    public void init() {
        if (isInitialized()) {
            return;
        }

        Log.i(this, "init");

        Context context = AppContext.getInstance();
        ImsLog.init();
        NativeCommands.setDeviceConfig(context);
        SystemInterface.getInstance().init();
        AgentFactory agentFactory = AgentFactory.getInstance();
        agentFactory.createAgents();
        agentFactory.initAgents(context);
        mInitialized = true;
    }

    /**
     * Starts the IMS service on the specified slot.
     * This starts the agent components and notifies the interested components
     * that the service is started.
     *
     * @param slotId The slot-id to be started.
     */
    public void start(int slotId) {
        Log.i(this, "Started on slot" + slotId);

        Context context = AppContext.getInstance();
        ImsServiceRegistry.getInstance(slotId).getMmTelFeatureRegistry().initUserSettings();
        ImsTestMode.getInstance().init(slotId);
        FeatureConfig.init(slotId);
        SystemInterface.getInstance().start(slotId);
        AgentFactory agentFactory = AgentFactory.getInstance();
        agentFactory.createAgentsForSlot(slotId);
        agentFactory.initAgentsForSlot(context, slotId);
        AosFactory.getInstance().init(slotId);
        updateCarrierConfig(slotId);
        DcFactory.createDcAgents(slotId);
        DcFactory.initDcAgents(context, slotId);

        ImsStackRegistry.setImsServiceState(slotId, true);

        AosFactory.getInstance().start(slotId);
        VideoConfigSpropGenerator.init(slotId);
    }

    /**
     * Stops the IMS service on the specified slot.
     *
     * @param slotId The slot-id to be stopped.
     */
    public void stop(int slotId) {
        Log.i(this, "Stopped on slot" + slotId);

        ImsStackRegistry.setImsServiceState(slotId, false);

        AosFactory.getInstance().stop(slotId);
        DcFactory.cleanUpDcAgents(slotId);
        AosFactory.getInstance().cleanup(slotId);
        AgentFactory.getInstance().destroyAgentsForSlot(slotId);
        SystemInterface.getInstance().stop(slotId);
        ImsTestMode.getInstance().cleanUp(slotId);
        VideoConfigSpropGenerator.cleanup(slotId);
    }

    /**
     * Starts the native enablers on the specified slot.
     *
     * @param slotId The slot-id.
     */
    public void startNativeEnabler(int slotId) {
        NativeCommands.startEnabler(slotId);
    }

    /**
     * Stops the native enablers on the specified slot.
     *
     * @param slotId The slot-id.
     */
    public void stopNativeEnabler(int slotId) {
        NativeCommands.stopEnabler(slotId);
    }

    /**
     * Updates the carrier configuration for the specified slot.
     *
     * @param slotId The slot-id to be updated.
     */
    public static void updateCarrierConfig(int slotId) {
        ConfigAgent ca = (ConfigAgent) AgentFactory.getInstance().getAgent(
                ConfigInterface.class, slotId);

        if (ca != null) {
            int subId = MSimUtils.getSubId(slotId);
            SimCarrierId id = CarrierInfo.getCarrierIdFromSim(slotId);

            ca.updateCarrierConfig(subId, id);

            notifyCarrierConfigChanged(slotId);
        }
    }

    /**
     * Notifies the carrier configuration change to the native layer.
     *
     * @param slotId The slot-id to be notified.
     */
    public static void notifyCarrierConfigChanged(int slotId) {
        ISystem system = SystemInterface.getInstance().getSystem(slotId);

        if (system != null) {
            system.notifyConfigurationChanged(0);
        }
    }
}
