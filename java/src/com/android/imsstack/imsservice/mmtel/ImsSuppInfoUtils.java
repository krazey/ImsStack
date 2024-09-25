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

import android.location.Location;
import android.os.Bundle;
import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.SuppInfo.SuppService;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.ImsLog;

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
                .getBoolean(CarrierConfig.ImsVoice.KEY_SUPPINFO_CDIV_CAUSE_REQUIRED_BOOL)) {
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

    /**
     * Gets call composer elements from the SuppInfo and adds to ImsCallProfile if they exist.
     *
     * @param suppInfo May contain the call composer elements.
     * @param outProfile Call composer elements will be added to here.
     */
    public static void addCallExtraForCallComposer(
            final SuppInfo suppInfo, ImsCallProfile outProfile) {
        SuppService ss = suppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_PRIORITY);
        if (ss != null) {
            outProfile.setCallExtraInt(ImsCallProfile.EXTRA_PRIORITY, ss.intValue);
        }

        ss = suppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_SUBJECT);
        if (ss != null) {
            outProfile.setCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT, ss.strValue);
        }

        ss = suppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_PICTURE_URL);
        if (ss != null) {
            outProfile.setCallExtra(ImsCallProfile.EXTRA_PICTURE_URL, ss.strValue);
        }

        ss = suppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_IS_BUSINESS);
        if (ss != null) {
            outProfile.setCallExtraBoolean(ImsCallProfile.EXTRA_IS_BUSINESS_CALL, ss.boolValue);
        }

        SuppService ssLat = suppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT);
        SuppService ssLong = suppInfo.getService(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG);
        if (ssLat != null && ssLong != null) {
            try {
                double latitude = Double.parseDouble(ssLat.strValue);
                double longitude = Double.parseDouble(ssLong.strValue);
                String emptyProvider = "";

                Location location = new Location(emptyProvider);
                location.setLatitude(latitude);
                location.setLongitude(longitude);
                outProfile.setCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION, location);
            } catch (NumberFormatException e) {
                ImsLog.e("Location parsing error: " + ssLat.strValue + ", " + ssLong.strValue);
            }
        }
    }

    /**
     * Gets call composer elements from the ImsCallProfile and adds to SuppInfo if they exist.
     *
     * @param profile May contain the call composer elements.
     * @param outSuppInfo Call composer elements will be added to here.
     */
    public static void addSuppInfoForCallComposer(
            final ImsCallProfile profile, SuppInfo outSuppInfo) {
        int priority = profile.getCallExtraInt(ImsCallProfile.EXTRA_PRIORITY);
        if (priority >= 0) {
            outSuppInfo.addService_int(SuppInfo.TYPE_CALL_COMPOSER_PRIORITY, priority);
        }

        String subject = profile.getCallExtra(ImsCallProfile.EXTRA_CALL_SUBJECT);
        if (subject.length() > 0) {
            outSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_SUBJECT, subject);
        }

        String picture = profile.getCallExtra(ImsCallProfile.EXTRA_PICTURE_URL);
        if (picture.length() > 0) {
            outSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_PICTURE_URL, picture);
        }

        Location location = profile.getCallExtraParcelable(ImsCallProfile.EXTRA_LOCATION);
        if (location != null) {
            outSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LAT,
                    String.valueOf(location.getLatitude()));
            outSuppInfo.addService_str(SuppInfo.TYPE_CALL_COMPOSER_LOCATION_LONG,
                    String.valueOf(location.getLongitude()));
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

    private static boolean getCallExtraBoolean(Bundle callExtras,
            String key, boolean defaultValue) {
        if (callExtras == null) {
            return defaultValue;
        }

        return callExtras.getBoolean(key, defaultValue);
    }

    private static boolean hasCallExtra(Bundle callExtras, String key) {
        if (callExtras == null) {
            return false;
        }

        return callExtras.containsKey(key);
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
