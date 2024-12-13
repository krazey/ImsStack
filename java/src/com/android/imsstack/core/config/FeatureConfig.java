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
package com.android.imsstack.core.config;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.util.ArrayMap;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.CarrierConfigManagerProxy;
import com.android.imsstack.util.ImsLog;

/**
 * This class provides the APIs to getting FEATURE_XXX in gims_feature table
 */
public final class FeatureConfig {
    public static final String VOLTE = "VOLTE";
    public static final String VOWIFI = "VOWIFI";
    public static final String VT = "VT";
    public static final String SMS = "SMS";
    public static final String UCE = "UCE";

    public static final int FEATURE_S_VOLTE = 1 << 0;
    public static final int FEATURE_S_VOWIFI = 1 << 1;
    public static final int FEATURE_S_VT = 1 << 2;
    public static final int FEATURE_S_SMS = 1 << 3;
    public static final int FEATURE_S_VOLTE_EMERGENCY = 1 << 4;
    public static final int FEATURE_S_UCE = 1 << 5;

    private static SparseArray<ArrayMap<String, Integer>> sFeatures = new SparseArray<>();

    public static synchronized boolean isEnabled(int slotId, String feature) {
        ArrayMap<String, Integer> features = sFeatures.get(slotId);

        if (features != null) {
            Integer value = features.get(feature);
            return (value != null) && (value.intValue() == 1);
        }

        return false;
    }

    public static synchronized void init(int slotId) {
        CarrierConfigManagerProxy ccmp =
                AppContext.getInstance().getSystemServiceProxy(CarrierConfigManagerProxy.class);
        // If an invalid subId is used, this bundle will contain default values.
        PersistableBundle config = ccmp.getConfigForSubId(MSimUtils.getSubId(slotId),
                CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL,
                CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL,
                CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL,
                CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL,
                CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL,
                CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL);
        ArrayMap<String, Integer> features = new ArrayMap<>();
        boolean available;
        StringBuilder sb = new StringBuilder();

        sb.append("[ ");

        available = config.getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_AVAILABLE_BOOL);
        features.put(VOLTE, available ? 1 : 0);
        sb.append("volte=").append(available ? 1 : 0);

        available = config.getBoolean(CarrierConfigManager.KEY_CARRIER_VT_AVAILABLE_BOOL);
        features.put(VT, available ? 1 : 0);
        sb.append(", vt=").append(available ? 1 : 0);

        available = config.getBoolean(CarrierConfigManager.KEY_CARRIER_WFC_IMS_AVAILABLE_BOOL);
        features.put(VOWIFI, available ? 1 : 0);
        sb.append(", wfc=").append(available ? 1 : 0);

        available = config.getBoolean(CarrierConfigManager.ImsSms.KEY_SMS_OVER_IMS_SUPPORTED_BOOL);
        features.put(SMS, available ? 1 : 0);
        sb.append(", sms=").append(available ? 1 : 0);

        available = config.getBoolean(CarrierConfigManager.Ims.KEY_ENABLE_PRESENCE_PUBLISH_BOOL);
        available |= config.getBoolean(CarrierConfigManager.KEY_USE_RCS_SIP_OPTIONS_BOOL);
        features.put(UCE, available ? 1 : 0);
        sb.append(", uce=").append(available ? 1 : 0);

        sb.append(" ]");

        sFeatures.put(slotId, features);

        ImsLog.i(slotId, "FeatureConfig=" + sb.toString());
    }

    private FeatureConfig() {}
}
