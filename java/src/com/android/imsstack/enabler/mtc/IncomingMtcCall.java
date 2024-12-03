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

public class IncomingMtcCall implements Parcelable {

    // Intent parameter

    public int callKey;

    public CallInfo callInfo;
    public MediaInfo mediaInfo;
    public SuppInfo suppInfo;

    public int OIPType;
    public String calleePartyNum;
    public String callerPartyNum;

    public int rejectedReason;

    /* OIPTYPE_nOIPType :: Incoming OIP Type */
    public static final int OIPTYPE_NONE         = 0;
    public static final int OIPTYPE_IDENTITY     = 1;
    public static final int OIPTYPE_RESTRICTED   = 2;
    public static final int OIPTYPE_UNKNOWN      = 3;
    public static final int OIPTYPE_PAYPHONE     = 4;
    public static final int OIPTYPE_UNAVAILABLE  = 5;

    /* CDIVCAUSE_nCDIVCause :: Incoming CDIV Cause */
    // No CDIV, strListCDIVHistory is empty
    public static final int CDIVCAUSE_NONE              = 0;
    public static final int CDIVCAUSE_UNCONDITION       = 1;
    public static final int CDIVCAUSE_BUSY              = 2;
    public static final int CDIVCAUSE_REJECT            = 3;
    public static final int CDIVCAUSE_NOANSWER          = 4;

    /* REJECTEDREASON :: Auto Rejected Call Reason  */
    public static final int AUTOREJECTREASON_NONE         = 0;        // No Rejected Case.

    public String logTag;

    //------------------------------------------------------------------------------------------//
    public IncomingMtcCall(IncomingMtcCall incomingCall) {

        callKey = incomingCall.callKey;

        OIPType = incomingCall.OIPType;
        calleePartyNum = incomingCall.calleePartyNum;
        callerPartyNum = incomingCall.callerPartyNum;

        rejectedReason = AUTOREJECTREASON_NONE;

        callInfo = new CallInfo(incomingCall.callInfo);
        mediaInfo = new MediaInfo(incomingCall.mediaInfo);
        suppInfo = new SuppInfo(incomingCall.suppInfo);

        logTag = incomingCall.logTag;

        ImsLog.d("callKey : " + callKey
                + " OIPType : " + OIPType
                + " calleePartyNum : " + calleePartyNum
                + " callerPartyNum : " + callerPartyNum
                 );
    }

    public IncomingMtcCall(Parcel source) {
        ImsLog.i("");
        readFromParcel(source);
    }

    public boolean isAutoRejectedCall() {

        return false;
    }

    public void readFromParcel(Parcel source) {

        callKey = source.readInt();

        callInfo = new CallInfo(source);
        mediaInfo = new MediaInfo(source);

        OIPType = source.readInt();
        calleePartyNum = source.readString();
        callerPartyNum = source.readString();

        suppInfo = new SuppInfo(source);

        logTag = source.readString();

        ImsLog.d("[" + logTag + "]callKey : " + callKey
                + " OIPType : " + OIPType
                + " calleePartyNum : " + calleePartyNum
                + " callerPartyNum : " + callerPartyNum
                 );

    }

    public void writeToParcel(Parcel dest, int flags) {
        ImsLog.i("");

        dest.writeInt(callKey);

        callInfo.writeToParcel(dest, 1);
        mediaInfo.writeToParcel(dest, 1);

        dest.writeInt(OIPType);
        dest.writeString(calleePartyNum);
        dest.writeString(callerPartyNum);

        suppInfo.writeToParcel(dest, 1);

    }

    public void setCallKey(int key) {
        callKey = key;
        ImsLog.i("setCallKey : " + callKey);
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<IncomingMtcCall> CREATOR =
            new Parcelable.Creator<IncomingMtcCall>() {
        public IncomingMtcCall createFromParcel(Parcel source) {
            return new IncomingMtcCall(source);
        }

        public IncomingMtcCall[] newArray(int size) {
            return new IncomingMtcCall[size];
        }
    };
}
