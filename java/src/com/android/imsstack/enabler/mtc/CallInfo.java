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

import android.os.Parcel;
import android.os.Parcelable;

import com.android.imsstack.util.ImsLog;

public class CallInfo implements Parcelable
{
    public int            serviceType;
    public int            callType;

    public boolean        emergency;
    public boolean        offline;
    public boolean        ussi;
    public boolean        isConf;
    public boolean        enabledConf;
    public boolean        confSub;
    public boolean        rttCapable;
    public boolean        videoCapable;

    //------------------------------------------------------------------------------------------//

    public CallInfo() {
        serviceType = IUMtcCall.SERVICETYPE_NORMAL;
        callType = IUMtcCall.CALLTYPE_VOIP;
        emergency = false;
        offline = false;
        ussi = false;
        isConf = false;
        enabledConf = false;
        confSub = false;
        rttCapable = false;
        videoCapable = false;

        logIn("init");
    }

    public CallInfo(CallInfo callInfo) {
        serviceType = callInfo.serviceType;
        callType = callInfo.callType;
        emergency = callInfo.emergency;
        offline = callInfo.offline;
        ussi = callInfo.ussi;
        isConf = callInfo.isConf;
        enabledConf = callInfo.enabledConf;
        confSub = callInfo.confSub;
        rttCapable = callInfo.rttCapable;
        videoCapable = callInfo.videoCapable;

        logIn("init");
    }

    public CallInfo(Parcel source) {
        readFromParcel(source);
    }

    public CallInfo(int _serviceType, int _callType) {
        serviceType = _serviceType;
        callType = _callType;
        emergency = false;
        offline = false;
        ussi = false;
        isConf = false;
        enabledConf = false;
        confSub = false;
        rttCapable = false;
        videoCapable = false;

        logIn("init");
    }

    public CallInfo(int _serviceType, int _callType, boolean _isConf) {
        serviceType = _serviceType;
        callType = _callType;
        emergency = false;
        offline = false;
        ussi = false;
        isConf = _isConf;
        enabledConf = false;
        confSub = false;
        rttCapable = false;
        videoCapable = false;

        logIn("init");
    }

    public void update(CallInfo callInfo) {
        serviceType = callInfo.serviceType;
        callType = callInfo.callType;
        emergency = callInfo.emergency;
        offline = callInfo.offline;
        ussi = callInfo.ussi;
        isConf = callInfo.isConf;
        enabledConf = callInfo.enabledConf;
        confSub = callInfo.confSub;
        rttCapable = callInfo.rttCapable;
        videoCapable = callInfo.videoCapable;

        logIn("update");
    }

    public void logIn(String tag) {
        ImsLog.i(tag + " - serviceType : " + serviceType
                + " callType : " + callType
                + " isConf : " + isConf
                + " enabledConf : " + enabledConf
                + " confSub : " + confSub
                + " rttCapable : " + rttCapable
                + " emergency : " + emergency
                + " offline : " + offline
                + " ussi : " + ussi
                + " videoCapable : " + videoCapable);
    }

    public void readFromParcel(Parcel source) {
        serviceType = source.readInt();
        callType = source.readInt();

        emergency = (source.readInt() == 1) ? true : false;
        offline = (source.readInt() == 1) ? true : false;
        ussi = (source.readInt() == 1) ? true : false;
        isConf = (source.readInt() == 1) ? true : false;
        enabledConf = (source.readInt() == 1) ? true : false;
        confSub = (source.readInt() == 1) ? true : false;
        rttCapable = (source.readInt() == 1) ? true : false;
        videoCapable = (source.readInt() == 1) ? true : false;

        logIn("read");
    }

    public void writeToParcel(Parcel dest, int flags) {
        ImsLog.i("");

        dest.writeInt(serviceType);
        dest.writeInt(callType);

        dest.writeInt(emergency ? 1 : 0);
        dest.writeInt(offline ? 1 : 0);
        dest.writeInt(ussi ? 1 : 0);
        dest.writeInt(isConf ? 1 : 0);
        dest.writeInt(enabledConf ? 1 : 0);
        dest.writeInt(confSub ? 1 : 0);
        dest.writeInt(rttCapable ? 1 : 0);
        dest.writeInt(videoCapable ? 1 : 0);
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<CallInfo> CREATOR
            = new Parcelable.Creator<CallInfo>() {
        public CallInfo createFromParcel(Parcel source) {
            try {
                return new CallInfo(source);
            } catch (Exception e) {
                ImsLog.e("Exception occurred when creating CallInfo from parcel", e);
            }
            return null;
        }

        public CallInfo[] newArray(int size) {
            return new CallInfo[size];
        }
    };

};
