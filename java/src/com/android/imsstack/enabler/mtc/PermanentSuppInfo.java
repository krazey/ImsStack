/*
 * Copyright (C) 2025 The Android Open Source Project
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

public final class PermanentSuppInfo implements Parcelable {
    // PermanentSuppInfo Type
    public static final int SUPP_TYPE_TB_CW = 0; // boolean
    public static final int SUPP_TYPE_TB_TIR = 1; // boolean
    public static final int SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE = 2; // boolean
    public static final int SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO = 3; // boolean
    public static final int SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE = 4; // boolean
    public static final int SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO = 5; // boolean
    public static final int SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_EXCEPT_HOME_VOICE = 6; // boolean
    public static final int SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_EXCEPT_HOME_VIDEO = 7; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ALL_VOICE = 8; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ALL_VIDEO = 9; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE = 10; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ROAMING_VIDEO = 11; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VOICE = 12; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VIDEO = 13; // boolean

    @IntDef(prefix = {"SUPP_TYPE_TB_"}, value = {
        SUPP_TYPE_TB_CW,
        SUPP_TYPE_TB_TIR,
        SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE,
        SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO,
        SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE,
        SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO,
        SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_EXCEPT_HOME_VOICE,
        SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_EXCEPT_HOME_VIDEO,
        SUPP_TYPE_TB_CB_INCOMING_ALL_VOICE,
        SUPP_TYPE_TB_CB_INCOMING_ALL_VIDEO,
        SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE,
        SUPP_TYPE_TB_CB_INCOMING_ROAMING_VIDEO,
        SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VOICE,
        SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VIDEO
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface PermanentSuppType {}

    private List<SuppServiceUtils.SuppService> mSuppServices =
            new ArrayList<SuppServiceUtils.SuppService>();

    /**
     * Default constructor for creating an empty PermanentSuppInfo object.
     */
    public PermanentSuppInfo() {
    }

    /**
     * Copy constructor for creating a PermanentSuppInfo object from an existing one.
     *
     * @param suppInfo The source PermanentSuppInfo object to copy from.
     */
    public PermanentSuppInfo(PermanentSuppInfo suppInfo) {
        SuppServiceUtils.addServices(suppInfo.getServices(), mSuppServices);
    }

    /**
     * Constructor for creating a PermanentSuppInfo object from a Parcel.
     *
     * @param source The Parcel to read the data from.
     */
    public PermanentSuppInfo(Parcel source) {
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
    public void addServiceStr(@PermanentSuppType int type, String value) {
        SuppServiceUtils.addServiceStr(mSuppServices, type, value);
    }

    /**
     * Adds a supplementary service with an integer value to the list.
     *
     * @param type The type of the service.
     * @param value The integer value of the service.
     */
    public void addServiceInt(@PermanentSuppType int type, int value) {
        SuppServiceUtils.addServiceInt(mSuppServices, type, value);
    }

    /**
     * Adds a supplementary service with a boolean value to the list.
     *
     * @param type The type of the service.
     * @param value The boolean value of the service.
     */
    public void addServiceBool(@PermanentSuppType int type, boolean value) {
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
    public void addService(@PermanentSuppType int type,
            boolean boolValue, int intValue, String strValue) {
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
    public boolean isService(@PermanentSuppType int type) {
        return SuppServiceUtils.isService(mSuppServices, type);
    }

    /**
     * Retrieves a specific supplementary service from the list.
     *
     * @param type The type of the service to retrieve.
     * @return The {@link SuppServiceUtils.SuppService} object, or {@code null} if not found.
     */
    public SuppServiceUtils.SuppService getService(@PermanentSuppType int type) {
        return SuppServiceUtils.getService(mSuppServices, type);
    }

    /**
     * Updates an existing service in the list with the values from a source service.
     *
     * @param type The type of the service to update.
     * @param sourceService The source service containing the new values.
     */
    public void updateService(@PermanentSuppType int type,
            SuppServiceUtils.SuppService sourceService) {
        SuppServiceUtils.updateService(mSuppServices, sourceService);
    }

    /**
     * Updates multiple services in the list from another PermanentSuppInfo object.
     *
     * @param suppInfo The source PermanentSuppInfo object containing the services to update with.
     */
    public void updateServices(PermanentSuppInfo suppInfo) {
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

    public static final Parcelable.Creator<PermanentSuppInfo> CREATOR =
            new Parcelable.Creator<PermanentSuppInfo>() {
                public PermanentSuppInfo createFromParcel(Parcel source) {
                    return new PermanentSuppInfo(source);
                }

                public PermanentSuppInfo[] newArray(int size) {
                    return new PermanentSuppInfo[size];
                }
            };
}
