/**
 * Copyright (C) 2024 The Android Open Source Project
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

package com.android.imsstack.enabler.media;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;

public class VideoConfigSpropGenerator {
    /* Holds SIM slot-id and corresponding CarrierConfigChangeListener */
    private static final HashMap<Integer, CarrierConfigChangeListener> sConfigListeners =
            new HashMap<>();

    /** VideoConfigSpropGenerator is a pure static class. Default constructor is made private to
     * restrict instantiation.
     */
    private VideoConfigSpropGenerator() {
        throw new AssertionError("Cannot instantiate VideoConfigSpropGenerator");
    }

    /**
     * Registers carrier config change listener with the slot specific instance of ConfigInterface.
     *
     * @param slotId SIM slot id for which carrier config change should be monitored.
     */
    public static void init(int slotId) {
        ImsLog.i(slotId, "VideoConfigSpropGenerator: init");
        if (!hasConfigChangeListener(slotId)) {
            ConfigInterface configInterface = AgentFactory.getInstance()
                    .getAgent(ConfigInterface.class, slotId);
            if (configInterface == null) {
                ImsLog.e(slotId, "VideoConfigSpropGenerator: ConfigInterface is null. "
                        + "Listener for CarrierConfig change not added");
                return;
            }

            CarrierConfigChangeListener configListener = new CarrierConfigChangeListener();
            configInterface.addListener(configListener);
            ImsLog.i(slotId, "VideoConfigSpropGenerator: ConfigInterface listener added");

            sConfigListeners.put(slotId, configListener);
        }
    }

    /**
     * Removes the listener previously registered for the give slot id and stops listening to
     * carrier config changes.
     *
     * @param slotId SIM slot for which carrier config change monitoring should be stopped.
     */
    public static void cleanup(int slotId) {
        ImsLog.i(slotId, "VideoConfigSpropGenerator: cleanup");

        if (hasConfigChangeListener(slotId)) {
            ConfigInterface configInterface = AgentFactory.getInstance()
                    .getAgent(ConfigInterface.class, slotId);
            if (configInterface != null) {
                configInterface.removeListener(sConfigListeners.get(slotId));
                ImsLog.i(slotId, "VideoConfigSpropGenerator: ConfigInterface listener removed");
            }
            sConfigListeners.remove(slotId);
        }
    }

    private static class CarrierConfigChangeListener implements ConfigInterface.Listener {

        @Override
        public void onCarrierConfigChanged(int slotId, int subId) {
            ImsLog.d(slotId, "onCarrierConfigChanged subId:" + subId);
            //TODO:
            // 1. Read video params from carrier config
            // 2. Call ImsMedia service to generate SPROP
            // 3. Update carrier config with sprop param
        }
    };

    @VisibleForTesting
    protected static boolean hasConfigChangeListener(int slotId) {
        return sConfigListeners.containsKey(slotId);
    }
}
