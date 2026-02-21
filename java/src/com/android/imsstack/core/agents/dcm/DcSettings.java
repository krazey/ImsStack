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

package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.telephony.CarrierConfigManager;

import androidx.annotation.NonNull;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcSettings;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Collections;
import java.util.List;
import java.util.stream.IntStream;

/**
 * This class provide interface to get carrier configurations
 */
public class DcSettings implements IDcSettings {
    private final int mSlotId;

    public DcSettings(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    @Override
    public boolean isRoamingAllowed() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        if (config == null) {
            return true;
        }

        return config.getBoolean(
                CarrierConfigManager.ImsVoice.KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL, true);
    }

    @Override
    public boolean isVopsIgnored() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        return config == null || config.getBoolean(
                CarrierConfig.ImsVoice.KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL, true);
    }

    @Override
    public boolean isImsPdnRequestWithoutMmtelRequired() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        return config != null && config.getBoolean(
                CarrierConfig.Ims.KEY_REQUEST_IMS_PDN_WITHOUT_MMTEL_BOOL, false);
    }

    @Override
    public @NonNull List<Integer> getImsSupportedAccessNetworks() {
        CarrierConfig config = getCarrierConfig(mSlotId);
        if (config == null) {
            ImsLog.w(mSlotId, "config is null");
            return Collections.emptyList();
        }

        int[] supportedRats = config.getIntArray(
                CarrierConfigManager.Ims.KEY_SUPPORTED_RATS_INT_ARRAY);

        if (supportedRats == null) {
            ImsLog.w(mSlotId, "supportedRats is null");
            return Collections.emptyList();
        }

        return IntStream.of(supportedRats).boxed().toList();
    }

    @Override
    public boolean isCrossSimEnabledByPlatform() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        if (config != null) {
            return config.getBoolean(CarrierConfigManager.KEY_CARRIER_CROSS_SIM_IMS_AVAILABLE_BOOL,
                    false);
        }
        return false;
    }

    @Override
    public boolean isEmergencyCallbackModeSupported() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        if (config != null) {
            return config.getBoolean(
                    CarrierConfigManager.ImsEmergency.KEY_EMERGENCY_CALLBACK_MODE_SUPPORTED_BOOL,
                    false);
        }
        return false;
    }

    @Override
    public int getPreferredIpVersion() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        if (config != null) {
            return config.getInt(CarrierConfig.Ims.KEY_IMS_PREFERRED_IPTYPE_INT,
                    CarrierConfig.Ims.IPV6_PREFERRED);
        }

        return CarrierConfig.Ims.IPV6_PREFERRED;
    }

    @Override
    public int getEmergencyPreferredIpVersion() {
        CarrierConfig config = getCarrierConfig(mSlotId);

        if (config != null) {
            return config.getInt(CarrierConfig.ImsEmergency.KEY_EPDN_PREFERRED_IPTYPE_INT,
                    CarrierConfig.Ims.IPV6_PREFERRED);
        }
        return CarrierConfig.Ims.IPV6_PREFERRED;
    }

    @Override
    public boolean isPermanentFailure(EApnType apnType, int causeCode) {
        CarrierConfig config = getCarrierConfig(mSlotId);

        if (config != null) {
            int[] permanentFailure = null;

            if (apnType == EApnType.IMS) {
                permanentFailure = config.getIntArray(
                        CarrierConfig.Ims.KEY_PERMANENT_PDN_FAILURE_INT_ARRAY);
            } else {
                return false;
            }

            if (permanentFailure != null) {
                for (int i = 0; i < permanentFailure.length; i++) {
                    if (permanentFailure[i] == causeCode) {
                        ImsLog.w(mSlotId, "permanent failure cause " + causeCode);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    @Override
    public boolean isCrossStackRedialCause(EApnType apnType, int causeCode) {
        if (apnType != EApnType.EMERGENCY) {
            return false;
        }

        CarrierConfig config = getCarrierConfig(mSlotId);
        if (config == null) {
            return false;
        }

        int[] crossStackRedialCauses = config.getIntArray(
                CarrierConfig.ImsEmergency.KEY_EPDN_REJECT_CAUSES_FOR_CROSS_STACK_REDIAL_INT_ARRAY);
        if (crossStackRedialCauses == null) {
            return false;
        }

        for (int i = 0; i < crossStackRedialCauses.length; i++) {
            if (crossStackRedialCauses[i] == causeCode) {
                ImsLog.w(mSlotId, "crossStackRedialCause " + causeCode);
                return true;
            }
        }

        return false;
    }

    @Override
    public boolean disableN1ModeOnImsPduEstablishFailure() {
        CarrierConfig config = getCarrierConfig(mSlotId);
        if (config == null) {
            return false;
        }

        return config.getBoolean(
                CarrierConfig.Ims.KEY_DISABLE_N1_MODE_ON_IMS_PDU_ESTABLISH_FAILURE_BOOL, false);
    }

    @VisibleForTesting
    protected IDcNetWatcher getDcNetWatcher(int slotId) {
        return DcFactory.getDcAgent(IDcNetWatcher.class, slotId);
    }

    /**
     * Gets the CarrierConfig for the specified slot.
     *
     * @param slotId The slot-id to be retrieved.
     * @return The CarrierConfig instance.
     */
    @VisibleForTesting
    protected CarrierConfig getCarrierConfig(int slotId) {
        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, slotId);
        return (config != null) ? config.getCarrierConfig() : null;
    }
}
