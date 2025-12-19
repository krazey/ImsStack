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

import static android.telephony.TelephonyManager.NETWORK_TYPE_UNKNOWN;

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

public class CallInfo implements Parcelable
{
    public int serviceType;
    public int callType;
    public int emergencyType;

    public boolean offline;
    public boolean ussi;
    public boolean isConf;
    public boolean enabledConf;
    public boolean confSub;
    public boolean rttCapable;
    public boolean videoCapable;
    public boolean crossSim;
    public int ratType;

    //------------------------------------------------------------------------------------------//

    public CallInfo() {
        serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        callType = IUMtcCall.CALLTYPE_VOIP;
        emergencyType = IUMtcCall.EMERGENCYTYPE_NONE;
        offline = false;
        ussi = false;
        isConf = false;
        enabledConf = false;
        confSub = false;
        rttCapable = false;
        videoCapable = false;
        crossSim = false;
        ratType = NETWORK_TYPE_UNKNOWN;

        logLn("init");
    }

    public CallInfo(CallInfo callInfo) {
        serviceType = callInfo.serviceType;
        callType = callInfo.callType;
        emergencyType = callInfo.emergencyType;
        offline = callInfo.offline;
        ussi = callInfo.ussi;
        isConf = callInfo.isConf;
        enabledConf = callInfo.enabledConf;
        confSub = callInfo.confSub;
        rttCapable = callInfo.rttCapable;
        videoCapable = callInfo.videoCapable;
        crossSim = callInfo.crossSim;
        ratType = callInfo.ratType;

        logLn("init");
    }

    public CallInfo(Parcel source) {
        readFromParcel(source);
    }

    public CallInfo(int _serviceType, int _callType) {
        serviceType = _serviceType;
        callType = _callType;
        emergencyType = IUMtcCall.EMERGENCYTYPE_NONE;
        offline = false;
        ussi = false;
        isConf = false;
        enabledConf = false;
        confSub = false;
        rttCapable = false;
        videoCapable = false;
        crossSim = false;
        ratType = NETWORK_TYPE_UNKNOWN;

        logLn("init");
    }

    public CallInfo(int _serviceType, int _callType, boolean _isConf) {
        serviceType = _serviceType;
        callType = _callType;
        emergencyType = IUMtcCall.EMERGENCYTYPE_NONE;
        offline = false;
        ussi = false;
        isConf = _isConf;
        enabledConf = false;
        confSub = false;
        rttCapable = false;
        videoCapable = false;
        crossSim = false;
        ratType = NETWORK_TYPE_UNKNOWN;

        logLn("init");
    }

    public void update(CallInfo callInfo) {
        serviceType = callInfo.serviceType;
        callType = callInfo.callType;
        emergencyType = callInfo.emergencyType;
        offline = callInfo.offline;
        ussi = callInfo.ussi;
        isConf = callInfo.isConf;
        enabledConf = callInfo.enabledConf;
        confSub = callInfo.confSub;
        rttCapable = callInfo.rttCapable;
        videoCapable = callInfo.videoCapable;
        crossSim = callInfo.crossSim;
        ratType = callInfo.ratType;

        logLn("update");
    }

    public void logLn(String tag) {
        ImsLog.i(tag + " - serviceType : " + serviceType
                + " callType : " + callType
                + " emergencyType : " + emergencyType
                + " isConf : " + isConf
                + " enabledConf : " + enabledConf
                + " confSub : " + confSub
                + " rttCapable : " + rttCapable
                + " offline : " + offline
                + " ussi : " + ussi
                + " videoCapable : " + videoCapable
                + " crossSim : " + crossSim
                + " ratType : " + ratType);
    }

    /**
     * Reads the state of this CallInfo instance from a Parcel.
     * The values are read in the same order they were written in
     * {@link #writeToParcel(Parcel, int)}.
     *
     * @param source The Parcel to read the object's data from.
     */
    public void readFromParcel(Parcel source) {
        serviceType = source.readInt();
        callType = source.readInt();
        emergencyType = source.readInt();

        offline = source.readInt() == 1;
        ussi = source.readInt() == 1;
        isConf = source.readInt() == 1;
        enabledConf = source.readInt() == 1;
        confSub = source.readInt() == 1;
        rttCapable = source.readInt() == 1;
        videoCapable = source.readInt() == 1;
        crossSim = source.readInt() == 1;
        ratType = source.readInt();

        logLn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        // This is not actually used.
        ImsLog.i("");

        dest.writeInt(serviceType);
        dest.writeInt(callType);
        dest.writeInt(emergencyType);

        dest.writeInt(offline ? 1 : 0);
        dest.writeInt(ussi ? 1 : 0);
        dest.writeInt(isConf ? 1 : 0);
        dest.writeInt(enabledConf ? 1 : 0);
        dest.writeInt(confSub ? 1 : 0);
        dest.writeInt(rttCapable ? 1 : 0);
        dest.writeInt(videoCapable ? 1 : 0);
        dest.writeInt(crossSim ? 1 : 0);
        dest.writeInt(ratType);
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<CallInfo> CREATOR =
            new Parcelable.Creator<CallInfo>() {
        public CallInfo createFromParcel(Parcel source) {
            return new CallInfo(source);
        }

        public CallInfo[] newArray(int size) {
            return new CallInfo[size];
        }
    };

}
