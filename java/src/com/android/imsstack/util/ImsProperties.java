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
package com.android.imsstack.util;

import android.content.Context;
import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;

/**
 * This class provides the wrapper APIs for system/vendor properties.
 */
public final class ImsProperties {
    /**
     * Constants values.
     */
    public static final String TARGET_OPERATOR = "";
    public static final String TARGET_COUNTRY = "";
    public static final String TARGET_REGION = "";
    public static final String IMS_TARGET_OPERATOR = getImsOperator(TARGET_OPERATOR);

    public static final String MODEL = Build.MODEL;
    public static final String MANUFACTURER = Build.MANUFACTURER;
    public static final String SW_VERSION = Build.ID;
    public static final String SW_VERSION_SHORT = SW_VERSION;
    public static final String SW_VERSION_3CHARS =
            (SW_VERSION.length() > 3) ? SW_VERSION.substring(SW_VERSION.length() - 3) : "000";

    public static final String SOC_MANUFACTURER = Build.SOC_MANUFACTURER;
    public static final String SOC_MODEL = Build.SOC_MODEL;

    public static class System {
        public static String get(String key, String def) {
            return SystemProperties.get(key, def);
        }

        public static boolean getBoolean(String key, boolean def) {
            return SystemProperties.getBoolean(key, def);
        }

        public static int getInt(String key, int def) {
            return SystemProperties.getInt(key, def);
        }
    }

    public static boolean isChipVendorMtk() {
        return SOC_MANUFACTURER.equalsIgnoreCase("MediaTek");
    }

    public static boolean isVoNrEnabled() {
        return false;
    }

    public static String getImsOperator(String operator) {
        switch (operator) {
            case "JCM": // FALL-THROUGH
            case "UQ":
                return "KDDI";
            case "SB":
                return "SBM";
            case "LRA": // FALL-THROUGH
            case "CCT": // FALL-THROUGH
            case "CHT": // FALL-THROUGH
            case "VSB": // FALL-THROUGH
            case "TRF_VZW":
                return "VZW";
            case "CRK": // FALL-THROUGH
            case "TRF_ATT":
                return "ATT";
            case "CCA": // FALL-THROUGH
            case "DISH": // FALL-THROUGH
            case "TRF_TMO": // FALL-THROUGH
            case "TRF_SM": // FALL-THROUGH
            case "TRF_WFM":
                return "TMO";
            case "TRF_CLR":
                return "CLR";
            default:
                return operator;
        }
    }

    public static String getSysSimCountry(int slotId) {
        return getSysSimCountry(AppContext.getInstance(), slotId);
    }

    public static String getSysSimOperator(int slotId) {
        return getSysSimOperator(AppContext.getInstance(), slotId);
    }

    public static String getSysSimOperatorSub(int slotId) {
        return getSysSimOperatorSub(AppContext.getInstance(), slotId);
    }

    public static String getSysSimCountry(Context c, int slotId) {
        try {
            return ImsPrivateProperties.Persistent.get(c,
                    ImsPrivateProperties.Persistent.KEY_SIM_COUNTRY, "", slotId);
        } catch (Throwable t) {
            t.printStackTrace();
            return "";
        }
    }

    public static String getSysSimOperator(Context c, int slotId) {
        try {
            return ImsPrivateProperties.Persistent.get(c,
                    ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR, "", slotId);
        } catch (Throwable t) {
            t.printStackTrace();
            return "";
        }
    }

    public static String getSysSimOperatorSub(Context c, int slotId) {
        try {
            return ImsPrivateProperties.Persistent.get(c,
                    ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR_SUB, "", slotId);
        } catch (Throwable t) {
            return "";
        }
    }

    public static boolean isUnknownOperator(String operator) {
        return TextUtils.isEmpty(operator)
                || "OPEN".equalsIgnoreCase(operator)
                || "UNKNOWN".equalsIgnoreCase(operator);
    }
}
