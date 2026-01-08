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
        SUPP_TYPE_SESSION_ID
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface SuppType {}

    private List<SuppServiceUtils.SuppService> mSuppServices =
            new ArrayList<SuppServiceUtils.SuppService>();

    /**
     * Default constructor for creating an empty SuppInfo object.
     */
    public SuppInfo() {
    }

    /**
     * Copy constructor for creating a SuppInfo object from an existing one.
     *
     * @param suppInfo The source SuppInfo object to copy from.
     */
    public SuppInfo(SuppInfo suppInfo) {
        SuppServiceUtils.addServices(suppInfo.getServices(), mSuppServices);
    }

    /**
     * Constructor for creating a SuppInfo object from a Parcel.
     *
     * @param source The Parcel to read the data from.
     */
    public SuppInfo(Parcel source) {
        readFromParcel(source);
    }

    /**
     * Logs the contents of the supplementary services list.
     *
     * @param tag A string tag to use for logging.
     */
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

    /**
     * Adds a supplementary service with a string value to the list.
     *
     * @param type The type of the service.
     * @param value The string value of the service.
     */
    public void addServiceStr(@SuppType int type, String value) {
        SuppServiceUtils.addServiceStr(mSuppServices, type, value);
    }

    /**
     * Adds a supplementary service with an integer value to the list.
     *
     * @param type The type of the service.
     * @param value The integer value of the service.
     */
    public void addServiceInt(@SuppType int type, int value) {
        SuppServiceUtils.addServiceInt(mSuppServices, type, value);
    }

    /**
     * Adds a supplementary service with a boolean value to the list.
     *
     * @param type The type of the service.
     * @param value The boolean value of the service.
     */
    public void addServiceBool(@SuppType int type, boolean value) {
        SuppServiceUtils.addServiceBool(mSuppServices, type, value);
    }

    /**
     * Adds a supplementary service with a boolean, integer, and string value.
     *
     * @param type The service type.
     * @param boolValue The boolean value of the service.
     * @param intValue The integer value of the service.
     * @param strValue The string value of the service.
     */
    public void addService(@SuppType int type, boolean boolValue, int intValue, String strValue) {
        SuppServiceUtils.addService(mSuppServices, type, boolValue, intValue, strValue);
    }

    /**
     * Returns the list of supplementary services.
     *
     * @return A {@link List} of {@link SuppServiceUtils.SuppService} objects.
     */
    public List<SuppServiceUtils.SuppService> getServices() {
        return mSuppServices;
    }

    /**
     * Returns the number of supplementary services in the list.
     *
     * @return The size of the services list.
     */
    public int getServicesSize() {
        return mSuppServices.size();
    }

    /**
     * Checks if a specific supplementary service exists in the list.
     *
     * @param type The type of the service to check.
     * @return {@code true} if the service exists, {@code false} otherwise.
     */
    public boolean isService(@SuppType int type) {
        return SuppServiceUtils.isService(mSuppServices, type);
    }

    /**
     * Retrieves a specific supplementary service from the list.
     *
     * @param type The type of the service to retrieve.
     * @return The {@link SuppServiceUtils.SuppService} object, or {@code null} if not found.
     */
    public SuppServiceUtils.SuppService getService(@SuppType int type) {
        return SuppServiceUtils.getService(mSuppServices, type);
    }

    /**
     * Updates an existing service in the list with the values from a source service.
     *
     * @param type The type of the service to update.
     * @param sourceService The source service containing the new values.
     */
    public void updateService(@SuppType int type, SuppServiceUtils.SuppService sourceService) {
        SuppServiceUtils.updateService(mSuppServices, sourceService);
    }

    /**
     * Updates multiple services in the list from another SuppInfo object.
     *
     * @param suppInfo The source SuppInfo object containing the services to update with.
     */
    public void updateServices(SuppInfo suppInfo) {
        SuppServiceUtils.updateServices(suppInfo.getServices(), mSuppServices);
    }

    /**
     * Reads the supplementary service data from a Parcel.
     *
     * @param source The Parcel from which to read the data.
     */
    public void readFromParcel(Parcel source) {
        SuppServiceUtils.readSuppFromParcel(mSuppServices, source);

        logLn("read");
    }

    /**
     * Writes the supplementary service data to a Parcel.
     *
     * @param dest The Parcel to which to write the data.
     * @param flags Additional flags for writing.
     */
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        logLn("write");

        SuppServiceUtils.writeSuppToParcel(mSuppServices, dest);
    }

    @Override
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
