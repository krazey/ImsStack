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
package com.android.imsstack.imsservice.mmtel;

import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

/**
 * Extension class for operator specific supplementary information handling.
 */
public final class ImsSuppInfoUtils {
    // Internal usage
    public static final String EXTRA_GEOLOCATION = "geolocation";

    public static void init() {
        /**
         * If there is any information to pass the call framework or call application,
         * it needs to be registered in SuppInfoUtils object.
         */
    }

    public static void addCallExtraForApp(ICallContext context,
            final SuppInfo suppInfo, ImsCallProfile outProfile) {
        if (getConfigInterface(context.getSlotId()).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_SUPPINFO_CDIV_CAUSE_REQUIRED_BOOL)) {
            setCallExtraInt(suppInfo, SuppInfo.TYPE_CDIV_CAUSE,
                    ImsCallUtils.EXTRA_CDIV_CAUSE, outProfile);
        }
    }

    public static void addSuppInfoForIms(ICallContext context,
            final ImsCallProfile profile, SuppInfo outSuppInfo) {
        Bundle callExtras = profile.getCallExtras();
        if (hasCallExtra(callExtras, EXTRA_GEOLOCATION)) {
            outSuppInfo.addService_bool(SuppInfo.TYPE_GEOLOCATION,
                    getCallExtraBoolean(callExtras, EXTRA_GEOLOCATION, true));
        }
    }

    public static String getCallExtraNameForBoolean(ICallContext context, int suppInfo) {
        // no-op
        return null;
    }

    public static String getCallExtraNameForInt(ICallContext context,int suppInfo) {
        // no-op
        return null;
    }

    public static String getCallExtraNameForString(ICallContext context,int suppInfo) {
        return null;
    }

    private static String getCallExtra(Bundle callExtras,
            String key, String defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getString(key, defaultValue);
    }

    private static boolean getCallExtraBoolean(Bundle callExtras,
            String key, boolean defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getBoolean(key, defaultValue);
    }

    private static int getCallExtraInt(Bundle callExtras,
            String key, int defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getInt(key, defaultValue);
    }

    private static boolean hasCallExtra(Bundle callExtras, String key) {
        if (callExtras == null) {
            return false;
        }

        return callExtras.containsKey(key);
    }

    private static void setCallExtra(SuppInfo si, int type,
            String key, ImsCallProfile outProfile) {
        SuppInfo.SuppService ss = si.getService(type);

        if (ss != null) {
            outProfile.setCallExtra(key, ss.strValue);
        } else {
            if (outProfile.getCallExtras() != null) {
                outProfile.getCallExtras().remove(key);
            }
        }
    }

    private static void setCallExtraBoolean(SuppInfo si, int type,
            String key, ImsCallProfile outProfile) {
        SuppInfo.SuppService ss = si.getService(type);

        if (ss != null) {
            outProfile.setCallExtraBoolean(key, ss.boolValue);
        } else {
            if (outProfile.getCallExtras() != null) {
                outProfile.getCallExtras().remove(key);
            }
        }
    }

    private static void setCallExtraInt(SuppInfo si, int type,
            String key, ImsCallProfile outProfile) {
        SuppInfo.SuppService ss = si.getService(type);

        if (ss != null) {
            outProfile.setCallExtraInt(key, ss.intValue);
        } else {
            if (outProfile.getCallExtras() != null) {
                outProfile.getCallExtras().remove(key);
            }
        }
    }

    private static ConfigInterface getConfigInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
    }
}
