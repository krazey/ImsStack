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

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import java.util.Arrays;

/**
 * The class provides the methods to access the capability related configuration items
 * from the carrier configuration.
 */
public final class CapabilityConfigs {
    /**
     * Checks if the carrier supports VoLTE or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports VoLTE, false otherwise.
     */
    public static boolean isVoLteEnabled(int slotId) {
        CarrierConfig cc = getCarrierConfig(slotId);

        if (cc != null) {
            return cc.getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL, false);
        }

        return false;
    }

    /**
     * Checks if the carrier supports VT or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports VT, false otherwise.
     */
    public static boolean isVtEnabled(int slotId) {
        CarrierConfig cc = getCarrierConfig(slotId);

        if (cc != null) {
            return cc.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL, false);
        }

        return false;
    }

    /**
     * Checks if the carrier supports WFC (Wi-Fi calling) or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports WFC (Wi-Fi calling), false otherwise.
     */
    public static boolean isWfcEnabled(int slotId) {
        CarrierConfig cc = getCarrierConfig(slotId);

        if (cc != null) {
            return cc.getBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL, false);
        }

        return false;
    }

    /**
     * Checks if the carrier supports UCE (User Capability Exchange) or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports UCE (User Capability Exchange), false otherwise.
     */
    public static boolean isUceEnabled(int slotId) {
        CarrierConfig cc = getCarrierConfig(slotId);

        if (cc != null) {
            return cc.getBoolean(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL, false)
                    || cc.getBoolean(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL, false);

        }

        return false;
    }

    /**
     * Checks if the carrier supports RTT feature or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports RTT, false otherwise.
     */
    public static boolean isRttEnabled(int slotId) {
        CarrierConfig cc = getCarrierConfig(slotId);

        if (cc != null) {
            return cc.getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false);
        }

        return false;
    }

    /**
     * Checks if the carrier supports VoNR or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports VoNR, false otherwise.
     */
    public static boolean isVoNrEnabled(int slotId) {
        CarrierConfig cc = getCarrierConfig(slotId);

        if (cc == null) {
            return false;
        }

        int[] nrAvailabilities = cc.getIntArray(
                CarrierConfigManager.KEY_CARRIER_NR_AVAILABILITIES_INT_ARRAY);

        if (nrAvailabilities == null) {
            return false;
        }

        return Arrays.stream(nrAvailabilities)
                .anyMatch(nra -> nra == CarrierConfigManager.CARRIER_NR_AVAILABILITY_SA);
    }

    /**
     * Gets the CarrierConfig for the specified slot.
     *
     * @param slotId The slot-id to be retrieved.
     * @return The CarrierConfig instance.
     */
    private static CarrierConfig getCarrierConfig(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, slotId);
        return (config != null) ? config.getCarrierConfig() : null;
    }
}
