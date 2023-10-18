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
package com.android.imsstack.core.agents.dcmif;

import android.net.NetworkCapabilities;
import android.telephony.data.ApnSetting;

import com.android.imsstack.util.ImsLog;

public enum EApnType {
    /**
     * WIFI type is only for IMS internal usage. (16/05/09)
     * It's defined to synchronize the constant value in Native Layer.
     */
    IMS(DcConstants.TYPE_IMS,
            "mobile_ims",
            NetworkCapabilities.NET_CAPABILITY_IMS),
    INTERNET(DcConstants.TYPE_INTERNET,
            "mobile_internet",
            NetworkCapabilities.NET_CAPABILITY_INTERNET),
    XCAP(DcConstants.TYPE_XCAP,
            "mobile_xcap",
            NetworkCapabilities.NET_CAPABILITY_XCAP),
    EMERGENCY(DcConstants.TYPE_EMERGENCY,
            "mobile_emergency",
            NetworkCapabilities.NET_CAPABILITY_EIMS),
    WIFI(DcConstants.TYPE_WIFI,
            "wifi",
            NetworkCapabilities.NET_CAPABILITY_INTERNET);

    private final int mType;
    private final String mStrType;
    private final int mNetCapability;

    EApnType(int type, String strType, int netCapability) {
        mType = type;
        mStrType = strType;
        mNetCapability = netCapability;
    }

    /**
     * Returns an APN type from the given APN setting type.
     */
    public static int getTypeFromApnSettingType(String type) {
        if (ApnSetting.TYPE_IMS_STRING.equals(type)) {
            return IMS.getType();
        } else if (ApnSetting.TYPE_DEFAULT_STRING.equals(type)) {
            return INTERNET.getType();
        } else if (ApnSetting.TYPE_EMERGENCY_STRING.equals(type)) {
            return EMERGENCY.getType();
        } else if (ApnSetting.TYPE_XCAP_STRING.equals(type)) {
            return XCAP.getType();
        }

        ImsLog.w("Not-Exist-ApnType=" + type);

        return DcConstants.TYPE_NONE;
    }

    /**
     * Returns an APN setting type from the given APN type.
     */
    public static String getApnSettingTypeFromType(int type) {
        if (IMS.getType() == type) {
            return ApnSetting.TYPE_IMS_STRING;
        } else if (INTERNET.getType() == type) {
            return ApnSetting.TYPE_DEFAULT_STRING;
        } else if (EMERGENCY.getType() == type) {
            return ApnSetting.TYPE_EMERGENCY_STRING;
        } else if (XCAP.getType() == type) {
            return ApnSetting.TYPE_XCAP_STRING;
        }

        ImsLog.w("Not-Exist-Type=" + type);

        return null;
    }

    /**
     * Returns a network capability from the given APN type.
     */
    public static int getNetCapabilityFromType(int type) {
        if (IMS.getType() == type) {
            return EApnType.IMS.getNetCapability();
        } else if (INTERNET.getType() == type) {
            return EApnType.INTERNET.getNetCapability();
        } else if (EMERGENCY.getType() == type) {
            return EApnType.EMERGENCY.getNetCapability();
        } else if (XCAP.getType() == type) {
            return EApnType.XCAP.getNetCapability();
        } else if (WIFI.getType() == type) { // DNS :: SimpleResolver
            return EApnType.WIFI.getNetCapability();
        }

        ImsLog.w("Not-Exist-Type=" + type);

        return DcConstants.CAPABILITY_NONE;
    }

    public int getType() {
        return mType;
    }

    public String getString() {
        return mStrType;
    }

    public int getNetCapability() {
        return mNetCapability;
    }
}
