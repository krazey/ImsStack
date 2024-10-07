/*
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

package com.android.imsstack.core.agents;

import com.android.imsstack.base.DeviceConfig;

/**
 * A class to provide some utility methods for Agent components regardless of a specific SIM slot.
 */
public final class AgentUtils {
    /**
     * Checks whether all SIM cards are absent or not.
     *
     * @return {@code true} if all SIM cards are absent, {@code false} otherwise.
     */
    public static boolean isAllSimAbsent() {
        boolean allSimAbsent = true;
        int activeSimCount = DeviceConfig.getActiveSimCount();

        for (int i = 0; i < activeSimCount; ++i) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, i);
            int simCardState = (sim != null) ? sim.getSimCardState() : Sim.STATE_ABSENT;
            if (simCardState != Sim.STATE_PRESENT) {
                continue;
            }
            int simState = (sim != null) ? sim.getSimState() : Sim.STATE_ABSENT;
            if (simState != Sim.STATE_ABSENT) {
                allSimAbsent = false;
                break;
            }
        }

        return allSimAbsent;
    }

    /**
     * Checks whether all SIM cards are absent/locked or not.
     *
     * @return {@code true} if all SIM cards are absent or locked, {@code false} otherwise.
     */
    public static boolean isAllSimAbsentOrLocked() {
        boolean allSimAbsentOrLocked = true;
        int activeSimCount = DeviceConfig.getActiveSimCount();

        for (int i = 0; i < activeSimCount; ++i) {
            SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, i);
            int simCardState = (sim != null) ? sim.getSimCardState() : Sim.STATE_ABSENT;
            if (simCardState != Sim.STATE_PRESENT) {
                continue;
            }
            int simState = (sim != null) ? sim.getSimState() : Sim.STATE_ABSENT;
            if (simState != Sim.STATE_ABSENT
                    && simState != Sim.STATE_LOCKED) {
                allSimAbsentOrLocked = false;
                break;
            }
        }

        return allSimAbsentOrLocked;
    }

    private AgentUtils() {}
}
