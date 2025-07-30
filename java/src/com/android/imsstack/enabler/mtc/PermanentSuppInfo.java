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
    public static final int SUPP_TYPE_TB_CB_INCOMING_ALL_VOICE = 6; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ALL_VIDEO = 7; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE = 8; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ROAMING_VIDEO = 9; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VOICE = 10; // boolean
    public static final int SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VIDEO = 11; // boolean

    @IntDef(prefix = {"SUPP_TYPE_TB_"}, value = {
        SUPP_TYPE_TB_CW,
        SUPP_TYPE_TB_TIR,
        SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE,
        SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO,
        SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE,
        SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO,
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

    public PermanentSuppInfo() {
    }

    public PermanentSuppInfo(PermanentSuppInfo suppInfo) {
        SuppServiceUtils.addServices(suppInfo.getServices(), mSuppServices);
    }

    public PermanentSuppInfo(Parcel source) {
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

    public void addServiceStr(@PermanentSuppType int type, String value) {
        SuppServiceUtils.addServiceStr(mSuppServices, type, value);
    }

    public void addServiceInt(@PermanentSuppType int type, int value) {
        SuppServiceUtils.addServiceInt(mSuppServices, type, value);
    }

    public void addServiceBool(@PermanentSuppType int type, boolean value) {
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
    public void addService(@PermanentSuppType int type,
            boolean boolValue, int intValue, String strValue) {
        SuppServiceUtils.addService(mSuppServices, type, boolValue, intValue, strValue);
    }

    public List<SuppServiceUtils.SuppService> getServices() {
        return mSuppServices;
    }

    public int getServicesSize() {
        return mSuppServices.size();
    }

    public boolean isService(@PermanentSuppType int type) {
        return SuppServiceUtils.isService(mSuppServices, type);
    }

    public SuppServiceUtils.SuppService getService(@PermanentSuppType int type) {
        return SuppServiceUtils.getService(mSuppServices, type);
    }

    public void updateService(@PermanentSuppType int type,
            SuppServiceUtils.SuppService sourceService) {
        SuppServiceUtils.updateService(mSuppServices, sourceService);
    }

    public void updateServices(PermanentSuppInfo suppInfo) {
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
