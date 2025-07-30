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

package com.android.imsstack.enabler.mtc;

import android.annotation.IntDef;
import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.List;

public final class SuppInfo implements Parcelable {
    // SuppInfo Type
    public static final int SUPP_TYPE_CALLERID = 0; // int
    public static final int SUPP_TYPE_CNAP = 1; // String
    public static final int SUPP_TYPE_CDIV_CAUSE = 2; // int
    public static final int SUPP_TYPE_CDIV_HISTORY = 3; // String
    public static final int SUPP_TYPE_CW = 4; // String
    public static final int SUPP_TYPE_ENFORCE_LT = 5; // boolean
    public static final int SUPP_TYPE_TARGET_URI = 6; // String
    public static final int SUPP_TYPE_CALLING_NUM_VERIFICATION = 7; // int
    public static final int SUPP_TYPE_TIP = 8;  // int, String
    public static final int SUPP_TYPE_GEOLOCATION = 9; // boolean
    public static final int SUPP_TYPE_CALL_PULL = 10; // boolean
    public static final int SUPP_TYPE_CALL_COMPOSER_PRIORITY = 11; // int
    public static final int SUPP_TYPE_CALL_COMPOSER_SUBJECT = 12; // String
    public static final int SUPP_TYPE_CALL_COMPOSER_LOCATION_LAT = 13; // String (from double)
    public static final int SUPP_TYPE_CALL_COMPOSER_LOCATION_LONG = 14; // String (from double)
    public static final int SUPP_TYPE_CALL_COMPOSER_PICTURE_URL = 15; // String
    public static final int SUPP_TYPE_CALL_COMPOSER_IS_BUSINESS = 16; // boolean
    public static final int SUPP_TYPE_SESSION_ID = 17; // String

    public static final int SUPP_TYPE_TB_CW = 18;                            // boolean
    public static final int SUPP_TYPE_TB_TIR = 19;                           // boolean

    // CallerID
    public static final int CALLERID_NONE = 0;
    public static final int CALLERID_NETWORK = 1;
    public static final int CALLERID_RESTRICTED = 2;
    public static final int CALLERID_IDENTITY = 3;

    // Calling Number Verification
    public static final int CALLING_NUM_VERSTAT_NONE = 0;
    public static final int CALLING_NUM_VERSTAT_VERIFIED = 1;
    public static final int CALLING_NUM_VERSTAT_NOT_VERIFIED = 2;

    // TIP
    public static final int TIP_NONE = 0;
    public static final int TIP_IDENTITY = 1;
    public static final int TIP_RESTRICTED = 2;

    @IntDef(prefix = {"SUPP_TYPE_"}, value = {
        SUPP_TYPE_CALLERID,
        SUPP_TYPE_CNAP,
        SUPP_TYPE_CDIV_CAUSE,
        SUPP_TYPE_CDIV_HISTORY,
        SUPP_TYPE_CW,
        SUPP_TYPE_ENFORCE_LT,
        SUPP_TYPE_TARGET_URI,
        SUPP_TYPE_CALLING_NUM_VERIFICATION,
        SUPP_TYPE_TIP,
        SUPP_TYPE_GEOLOCATION,
        SUPP_TYPE_CALL_PULL,
        SUPP_TYPE_CALL_COMPOSER_PRIORITY,
        SUPP_TYPE_CALL_COMPOSER_SUBJECT,
        SUPP_TYPE_CALL_COMPOSER_LOCATION_LAT,
        SUPP_TYPE_CALL_COMPOSER_LOCATION_LONG,
        SUPP_TYPE_CALL_COMPOSER_PICTURE_URL,
        SUPP_TYPE_CALL_COMPOSER_IS_BUSINESS,
        SUPP_TYPE_SESSION_ID,
        SUPP_TYPE_TB_CW,
        SUPP_TYPE_TB_TIR
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface SuppType {}

    private List<SuppServiceUtils.SuppService> mSuppServices =
            new ArrayList<SuppServiceUtils.SuppService>();

    public SuppInfo() {
    }

    public SuppInfo(SuppInfo suppInfo) {
        SuppServiceUtils.addServices(suppInfo.getServices(), mSuppServices);
    }

    public SuppInfo(Parcel source) {
        readFromParcel(source);
    }

    public void logLn(String tag) {
        ImsLog.i(tag + " - size[" + mSuppServices.size() + "]");

        for (int index = 0; index < mSuppServices.size(); index++) {
            SuppServiceUtils.SuppService service = mSuppServices.get(index);

            ImsLog.i("[" + index + "]"
                    + " type : " + service.type
                    + " strValue : " + ImsLog.hiddenString(service.strValue)
                    + " intValue : " + service.intValue
                    + " boolValue : " + service.boolValue);
        }
    }

    public void addServiceStr(@SuppType int type, String value) {
        SuppServiceUtils.addServiceStr(mSuppServices, type, value);
    }

    public void addServiceInt(@SuppType int type, int value) {
        SuppServiceUtils.addServiceInt(mSuppServices, type, value);
    }

    public void addServiceBool(@SuppType int type, boolean value) {
        SuppServiceUtils.addServiceBool(mSuppServices, type, value);
    }

    /**
     * This method is to set and service values
     *
     * @param type service type
     * @param boolValue service bool value
     * @param intValue service int value
     * @param strValue service string value
     */
    public void addService(@SuppType int type, boolean boolValue, int intValue, String strValue) {
        SuppServiceUtils.addService(mSuppServices, type, boolValue, intValue, strValue);
    }

    public List<SuppServiceUtils.SuppService> getServices() {
        return mSuppServices;
    }

    public int getServicesSize() {
        return mSuppServices.size();
    }

    public boolean isService(@SuppType int type) {
        return SuppServiceUtils.isService(mSuppServices, type);
    }

    public SuppServiceUtils.SuppService getService(@SuppType int type) {
        return SuppServiceUtils.getService(mSuppServices, type);
    }

    public void updateService(@SuppType int type, SuppServiceUtils.SuppService sourceService) {
        SuppServiceUtils.updateService(mSuppServices, sourceService);
    }

    public void updateServices(SuppInfo suppInfo) {
        SuppServiceUtils.updateServices(suppInfo.getServices(), mSuppServices);
    }

    public void readFromParcel(Parcel source) {
        SuppServiceUtils.readSuppFromParcel(mSuppServices, source);

        logLn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        logLn("write");

        SuppServiceUtils.writeSuppToParcel(mSuppServices, dest);
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<SuppInfo> CREATOR =
            new Parcelable.Creator<SuppInfo>() {
                public SuppInfo createFromParcel(Parcel source) {
                    return new SuppInfo(source);
                }

                public SuppInfo[] newArray(int size) {
                    return new SuppInfo[size];
                }
            };
}
