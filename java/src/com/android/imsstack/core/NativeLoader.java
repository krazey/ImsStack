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

import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.core.config.FeatureTable;
import com.android.imsstack.system.SystemConfig;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.MSimUtils;

import java.util.List;

/**
 * This class provides an interface to load the native modules and update the configuration.
 */
public final class NativeLoader {
    /** Sends the system configuration to native on boot-up time. */
    public static void setSystemConfigForBootup() {
        setSystemConfig(-1, SystemConfig.EVENT_ON_BOOT, false);
    }

    /** Sends the system configuration to native when the configuration is changed. */
    public static void setSystemConfigForAllConfigurationChanged(
            int slotId, boolean simRemoved) {
        setSystemConfig(slotId, SystemConfig.EVENT_ALL_CONFIGURATION_CHANGED, simRemoved);
    }

    private static void setSystemConfig(int slotId, int event, boolean simRemoved) {
        int simCount = (slotId < 0) ? MSimUtils.getMaxSimSlot() : 1;
        SystemConfig[] sc = new SystemConfig[simCount];

        for (int i = 0; i < simCount; ++i) {
            int currentSlotId = (slotId < 0) ? i : slotId;

            String operator = ImsProperties.getSysSimOperator(currentSlotId);
            String country = ImsProperties.getSysSimCountry(currentSlotId);

            int extraInfo = 0;

            extraInfo |= (simRemoved) ? SystemConfig.EXTRA_INFO_NO_UICC :
                    (!MSimUtils.hasIccCard(currentSlotId) ? SystemConfig.EXTRA_INFO_NO_UICC : 0);

            int serviceFeatures = 0;
            List<FeatureTable.Feature> serviceFeatureList = FeatureTable.getServiceFeatures();

            for (FeatureTable.Feature feature : serviceFeatureList) {
                serviceFeatures = setOrClearFeature(currentSlotId,
                        feature.getFeature(), feature.getFeatureMask(), serviceFeatures);
            }

            sc[i] = new SystemConfig(currentSlotId, operator, country,
                    "open", extraInfo, 0, serviceFeatures);
        }

        SystemConfig.setConfiguration(event, sc);
    }

    private static int setOrClearFeature(int slotId,
            String feature, int featureMask, int features) {
        if (FeatureConfig.isEnabled(slotId, feature)) {
            features |= featureMask;
        } else {
            features &= (~featureMask);
        }

        return features;
    }
}
