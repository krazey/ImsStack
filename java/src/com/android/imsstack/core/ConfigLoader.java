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

package com.android.imsstack.core;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.carrier.CarrierInfo;
import com.android.imsstack.core.carrier.SimCarrierId;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.MSimUtils;

public final class ConfigLoader {

    public static void updateCarrierConfig(int slotId) {
        ConfigAgent ca = (ConfigAgent)AgentFactory.getInstance().getAgent(
                ConfigInterface.class, slotId);

        if (ca != null) {
            int subId = MSimUtils.getSubId(slotId);
            SimCarrierId id = CarrierInfo.getCarrierIdFromSim(slotId);

            ca.updateCarrierConfig(subId, id);

            notifyCarrierConfigChanged(slotId);
        }
    }

    public static void notifyCarrierConfigChanged(int slotId) {
        ISystem system = SystemInterface.getInstance().getSystem(slotId);

        if (system != null) {
            system.notifyConfigurationChanged(0);
        }
    }
}
