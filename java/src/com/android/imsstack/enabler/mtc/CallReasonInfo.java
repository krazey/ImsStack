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

/**
 * Provides details on why an IMS call failed.
 */
public class CallReasonInfo implements Parcelable {
    public static final int CODE_NONE = 0;
    public static final int CODE_UNSPECIFIED = 1;

    public static final int CODE_LOCAL_SERVICE_UNAVAILABLE = 101;
    public static final int CODE_LOCAL_NOT_REGISTERED = 102;
    public static final int CODE_LOCAL_POWER_OFF = 103;
    public static final int CODE_LOCAL_LOW_BATTERY = 104;
    public static final int CODE_LOW_BATTERY = 105;
    public static final int CODE_LOCAL_CALL_END_UNSPECIFIED = 106;

    public static final int CODE_LOCAL_NETWORK_NO_SERVICE = 201;
    public static final int CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 202;
    public static final int CODE_WIFI_LOST = 203;
    public static final int CODE_LOCAL_NETWORK_ROAMING = 204;
    public static final int CODE_LOCAL_NETWORK_IP_CHANGED = 205;
    public static final int CODE_CALL_BARRED = 206;
    public static final int CODE_ACCESS_CLASS_BLOCKED = 207;

    public static final int CODE_USER_TERMINATED = 301;
    public static final int CODE_USER_TERMINATED_BY_REMOTE = 302;
    public static final int CODE_USER_DECLINE = 303;
    public static final int CODE_USER_NOANSWER = 304;
    public static final int CODE_USER_IGNORE = 305;

    public static final int CODE_LOCAL_CALL_CS_RETRY_REQUIRED = 401;
    public static final int CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED = 402;
    public static final int CODE_NO_CSFB_IN_CS_ROAM = 403;

    public static final int CODE_ANSWERED_ELSEWHERE = 501;
    public static final int CODE_REJECTED_ELSEWHERE = 502;
    public static final int CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED = 503;
    public static final int CODE_CALL_END_CAUSE_CALL_PULL = 504;

    public static final int CODE_MEDIA_INIT_FAILED = 601;
    public static final int CODE_MEDIA_NOT_ACCEPTABLE = 602;
    public static final int CODE_MEDIA_NO_DATA = 603;
    public static final int CODE_MEDIA_UNSPECIFIED = 604;

    public static final int CODE_SESSION_INTERNAL_ERROR = 701;
    public static final int CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE = 702;
    public static final int CODE_TIMEOUT_1XX_WAITING = 703;
    public static final int CODE_TIMEOUT_NO_ANSWER = 704;
    public static final int CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE = 705;
    public static final int CODE_NETWORK_RESP_TIMEOUT = 706;

    public static final int CODE_SIP_SERVER_ERROR = 801;
    public static final int CODE_SIP_FORBIDDEN = 802;
    public static final int CODE_SIP_NOT_FOUND = 803;
    public static final int CODE_SIP_NOT_SUPPORTED = 804;
    public static final int CODE_SIP_TEMPORARILY_UNAVAILABLE = 805;
    public static final int CODE_SIP_BAD_ADDRESS = 806;
    public static final int CODE_SIP_BUSY = 807;
    public static final int CODE_SIP_USER_REJECTED = 808;
    public static final int CODE_SIP_REQUEST_CANCELLED = 809;
    public static final int CODE_SIP_BAD_REQUEST = 810;
    public static final int CODE_SIP_NOT_ACCEPTABLE = 811;
    public static final int CODE_SIP_REQUEST_TIMEOUT = 812;
    public static final int CODE_SIP_NOT_REACHABLE = 813;
    public static final int CODE_SIP_REDIRECTED = 814;
    public static final int CODE_SIP_CLIENT_ERROR = 815;
    public static final int CODE_SIP_SERVICE_UNAVAILABLE = 816;
    public static final int CODE_SIP_REQUEST_PENDING = 817;

    public static final int CODE_REJECT_ONGOING_CALL_WAITING_DISABLED = 901;
    public static final int CODE_REJECT_VT_TTY_NOT_ALLOWED = 902;
    public static final int CODE_REJECT_ONGOING_E911_CALL = 903;
    public static final int CODE_REJECT_MAX_CALL_LIMIT_REACHED = 904;
    public static final int CODE_LOCAL_CALL_BUSY = 905;
    public static final int CODE_REJECT_UNSUPPORTED_SIP_HEADERS = 906;
    public static final int CODE_REJECT_ONGOING_CALL_SETUP = 907;
    public static final int CODE_REJECT_ONGOING_CS_CALL = 908;
    public static final int CODE_REJECT_CALL_ON_OTHER_SUB = 909;
    public static final int CODE_LOCAL_CALL_EXCEEDED = 910;
    public static final int CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED = 911;
    public static final int CODE_REJECT_QOS_FAILURE = 912;

    public static final int CODE_EARLYDIALOG_FORKED_TERMINATED_INTERNALONLY = 1001;
    public static final int CODE_RETRY_AFTER_INTERNALONLY = 1002;
    public static final int CODE_REJECT_ONGOING_CALL_UPDATE = 1003;

    // CODE_LOCAL_CALL_CS_RETRY_REQUIRED
    public static final int EXTRA_CODE_CALL_RETRY_NORMAL = 0;
    public static final int EXTRA_CODE_CALL_RETRY_SILENT_REDIAL = 1;
    public static final int EXTRA_CODE_CALL_RETRY_EMERGENCY = 2;

    // CODE_USER_TERMINATED
    public static final int EXTRA_USER_TERMINATED_ECT = 0;

    // FAIL_REASON_SESSION_RETRY1X_E_1X|VOLTE :: CODE
    public static final int EXTRA_CODE_EMERGENCYSERVICE_INVALID = -1;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_GENERIC = 0;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE = 1;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_ANIMAL_CONTROL = 2;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_FIRE = 3;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_GAS = 4;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_MARINE = 5;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN = 6;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_PHYSICIAN = 7;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_POISON = 8;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_POLICE = 9;

    public int mCode;
    public int mExtraCode;
    public String mExtraMessage;

    //------------------------------------------------------------------------------------------//

    public CallReasonInfo() {
        ImsLog.i("");

        mCode = -1;
        mExtraCode = -1;
        mExtraMessage = "";

        logIn();
    }

    public CallReasonInfo(CallReasonInfo callReasonInfo) {
        ImsLog.i("");

        mCode = callReasonInfo.mCode;
        mExtraCode = callReasonInfo.mExtraCode;
        mExtraMessage = callReasonInfo.mExtraMessage;

        logIn();
    }

    public CallReasonInfo(Parcel source) {
        ImsLog.i("");
        readFromParcel(source);
    }

    public CallReasonInfo(int code, int extraCode, String extraMessage) {
        ImsLog.i("");

        mCode = code;
        mExtraCode = extraCode;
        mExtraMessage = extraMessage;

        logIn();
    }

    /**
     * Helper for leaving the logs.
    */
    public void logIn() {
        ImsLog.i("mCode : " + mCode
                + " mExtraCode : " + mExtraCode
                + " mExtraMessage : " + mExtraMessage
                 );
    }

    /**
     * Helper for unparcelling this object.
    */
    public void readFromParcel(Parcel source) {
        ImsLog.i("");

        mCode = source.readInt();
        mExtraCode = source.readInt();
        mExtraMessage = source.readString();

        logIn();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        ImsLog.i("");

        dest.writeInt(mCode);
        dest.writeInt(mExtraCode);
        dest.writeString(mExtraMessage);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<CallReasonInfo> CREATOR =
            new Parcelable.Creator<CallReasonInfo>() {
        public CallReasonInfo createFromParcel(Parcel source) {
            try {
                return new CallReasonInfo(source);
            } catch (Exception e) {
                ImsLog.e("Exception occurred when creating CallReasonInfo from parcel", e);
            }
            return null;
        }

        public CallReasonInfo[] newArray(int size) {
            return new CallReasonInfo[size];
        }
    };

};
