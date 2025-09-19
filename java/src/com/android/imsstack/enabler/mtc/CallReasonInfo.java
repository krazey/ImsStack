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
    public static final int CODE_NONE = -1;
    public static final int CODE_UNSPECIFIED = 0;
    public static final int CODE_LOCAL_ILLEGAL_ARGUMENT =101;
    public static final int CODE_LOCAL_ILLEGAL_STATE = 102;
    public static final int CODE_LOCAL_INTERNAL_ERROR = 103;
    public static final int CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE = 108;
    public static final int CODE_LOCAL_POWER_OFF = 111;
    public static final int CODE_LOCAL_LOW_BATTERY = 112;
    public static final int CODE_LOCAL_NETWORK_NO_SERVICE = 121;
    public static final int CODE_LOCAL_NETWORK_NO_LTE_COVERAGE = 122;
    public static final int CODE_LOCAL_NETWORK_IP_CHANGED = 124;
    public static final int CODE_LOCAL_SERVICE_UNAVAILABLE = 131;
    public static final int CODE_LOCAL_NOT_REGISTERED = 132;
    public static final int CODE_LOCAL_CALL_EXCEEDED = 141;
    public static final int CODE_LOCAL_CALL_BUSY = 142;
    public static final int CODE_LOCAL_CALL_DECLINE = 143;
    public static final int CODE_LOCAL_CALL_VCC_ON_PROGRESSING = 144;
    public static final int CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED = 145;
    public static final int CODE_LOCAL_CALL_CS_RETRY_REQUIRED = 146;
    public static final int CODE_LOCAL_CALL_VOLTE_RETRY_REQUIRED = 147;
    public static final int CODE_LOCAL_CALL_TERMINATED = 148;
    public static final int CODE_LOCAL_HO_NOT_FEASIBLE = 149;
    public static final int CODE_TIMEOUT_1XX_WAITING = 201;
    public static final int CODE_TIMEOUT_NO_ANSWER = 202;
    public static final int CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE = 203;
    public static final int CODE_CALL_BARRED = 240;
    public static final int CODE_SIP_REDIRECTED = 321;
    public static final int CODE_SIP_BAD_REQUEST = 331;
    public static final int CODE_SIP_FORBIDDEN = 332;
    public static final int CODE_SIP_NOT_FOUND = 333;
    public static final int CODE_SIP_NOT_SUPPORTED = 334;
    public static final int CODE_SIP_REQUEST_TIMEOUT = 335;
    public static final int CODE_SIP_TEMPRARILY_UNAVAILABLE = 336;
    public static final int CODE_SIP_BAD_ADDRESS = 337;
    public static final int CODE_SIP_BUSY = 338;
    public static final int CODE_SIP_REQUEST_CANCELLED = 339;
    public static final int CODE_SIP_NOT_ACCEPTABLE = 340;
    public static final int CODE_SIP_NOT_REACHABLE = 341;
    public static final int CODE_SIP_CLIENT_ERROR = 342;
    public static final int CODE_SIP_TRANSACTION_DOES_NOT_EXIST = 343;
    public static final int CODE_SIP_SERVER_INTERNAL_ERROR = 351;
    public static final int CODE_SIP_SERVICE_UNAVAILABLE = 352;
    public static final int CODE_SIP_SERVER_TIMEOUT = 353;
    public static final int CODE_SIP_SERVER_ERROR = 354;
    public static final int CODE_SIP_USER_REJECTED = 361;
    public static final int CODE_SIP_GLOBAL_ERROR = 362;
    public static final int CODE_EMERGENCY_TEMP_FAILURE = 363;
    public static final int CODE_EMERGENCY_PERM_FAILURE = 364;
    public static final int CODE_SIP_METHOD_NOT_ALLOWED = 366;
    public static final int CODE_SIP_PROXY_AUTHENTICATION_REQUIRED = 367;
    public static final int CODE_SIP_REQUEST_ENTITY_TOO_LARGE = 368;
    public static final int CODE_SIP_REQUEST_URI_TOO_LARGE = 369;
    public static final int CODE_SIP_EXTENSION_REQUIRED = 370;
    public static final int CODE_SIP_INTERVAL_TOO_BRIEF = 371;
    public static final int CODE_SIP_CALL_OR_TRANS_DOES_NOT_EXIST = 372;
    public static final int CODE_SIP_LOOP_DETECTED = 373;
    public static final int CODE_SIP_TOO_MANY_HOPS = 374;
    public static final int CODE_SIP_AMBIGUOUS = 376;
    public static final int CODE_SIP_REQUEST_PENDING = 377;
    public static final int CODE_SIP_UNDECIPHERABLE = 378;
    public static final int CODE_MEDIA_INIT_FAILED = 401;
    public static final int CODE_MEDIA_NO_DATA = 402;
    public static final int CODE_MEDIA_NOT_ACCEPTABLE = 403;
    public static final int CODE_MEDIA_UNSPECIFIED = 404;
    public static final int CODE_USER_TERMINATED = 501;
    public static final int CODE_USER_NOANSWER = 502;
    public static final int CODE_USER_IGNORE = 503;
    public static final int CODE_USER_DECLINE = 504;
    public static final int CODE_LOW_BATTERY = 505;
    public static final int CODE_BLACKLISTED_CALL_ID = 506;
    public static final int CODE_USER_TERMINATED_BY_REMOTE = 510;
    public static final int CODE_USER_REJECTED_SESSION_MODIFICATION = 511;
    public static final int CODE_USER_CANCELLED_SESSION_MODIFICATION = 512;
    public static final int CODE_SESSION_MODIFICATION_FAILED = 1517;
    public static final int CODE_MULTIENDPOINT_NOT_SUPPORTED = 902;
    public static final int CODE_ANSWERED_ELSEWHERE = 1014;
    public static final int CODE_CALL_PULL_OUT_OF_SYNC = 1015;
    public static final int CODE_CALL_END_CAUSE_CALL_PULL = 1016;
    public static final int CODE_REJECTED_ELSEWHERE = 1017;
    public static final int CODE_SUPP_SVC_FAILED = 1201;
    public static final int CODE_SUPP_SVC_CANCELLED = 1202;
    public static final int CODE_SUPP_SVC_REINVITE_COLLISION = 1203;
    public static final int CODE_MAXIMUM_NUMBER_OF_CALLS_REACHED = 1403;
    public static final int CODE_REMOTE_CALL_DECLINE = 1404;
    public static final int CODE_WIFI_LOST = 1407;
    public static final int CODE_RADIO_OFF = 1500;
    public static final int CODE_RADIO_INTERNAL_ERROR = 1502;
    public static final int CODE_NETWORK_RESP_TIMEOUT = 1503;
    public static final int CODE_ACCESS_CLASS_BLOCKED = 1512;
    public static final int CODE_NETWORK_DETACH = 1513;
    public static final int CODE_SIP_ALTERNATE_EMERGENCY_CALL = 1514;
    public static final int CODE_REJECT_UNKNOWN = 1600;
    public static final int CODE_REJECT_ONGOING_CALL_WAITING_DISABLED = 1601;
    public static final int CODE_REJECT_CALL_ON_OTHER_SUB = 1602;
    public static final int CODE_REJECT_SERVICE_NOT_REGISTERED = 1604;
    public static final int CODE_REJECT_CALL_TYPE_NOT_ALLOWED = 1605;
    public static final int CODE_REJECT_ONGOING_E911_CALL = 1606;
    public static final int CODE_REJECT_ONGOING_CALL_SETUP = 1607;
    public static final int CODE_REJECT_MAX_CALL_LIMIT_REACHED = 1608;
    public static final int CODE_REJECT_UNSUPPORTED_SIP_HEADERS = 1609;
    public static final int CODE_REJECT_UNSUPPORTED_SDP_HEADERS = 1610;
    public static final int CODE_REJECT_ONGOING_CALL_TRANSFER = 1611;
    public static final int CODE_REJECT_INTERNAL_ERROR = 1612;
    public static final int CODE_REJECT_QOS_FAILURE = 1613;
    public static final int CODE_REJECT_VT_TTY_NOT_ALLOWED = 1615;
    public static final int CODE_REJECT_ONGOING_CALL_UPGRADE = 1616;
    public static final int CODE_REJECT_ONGOING_CONFERENCE_CALL = 1618;
    public static final int CODE_REJECT_ONGOING_CS_CALL = 1621;
    public static final int CODE_EMERGENCY_CALL_OVER_WFC_NOT_AVAILABLE = 1622;
    public static final int CODE_OEM_CAUSE_3 = 61443;

    // CODE_LOCAL_CALL_CS_RETRY_REQUIRED
    public static final int EXTRA_CODE_CALL_RETRY_NORMAL = 0;
    public static final int EXTRA_CODE_CALL_RETRY_SILENT_REDIAL = 1;
    public static final int EXTRA_CODE_CALL_RETRY_EMERGENCY = 2;

    // CODE_USER_TERMINATED
    public static final int EXTRA_USER_TERMINATED_ECT = 0;

    // CODE_SIP_ALTERNATE_EMERGENCY_CALL
    // android.telephony.emergency.EmergencyNumber.EmergencyServiceCategories
    public static final int EXTRA_CODE_EMERGENCYSERVICE_INVALID = -1;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_GENERIC = 0;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_POLICE = 1;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE = 2;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_FIRE = 3;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_MARINE = 4;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN = 5;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_MIEC = 6;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_AIEC = 7;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC = 8;
    public static final int EXTRA_CODE_EMERGENCYSERVICE_UNSPECIFIED = 9;

    public static final int EXTRA_CODE_NOT_ACCEPTABLE_SIP_406 = 1;
    public static final int EXTRA_CODE_NOT_ACCEPTABLE_SIP_488 = 2;
    public static final int EXTRA_CODE_NOT_ACCEPTABLE_SIP_606 = 3;

    // CODE_LOCAL_INTERNAL_ERROR
    // used internally only between native MTC and java MTC
    public static final int EXTRA_CODE_INTERNAL_ERROR_INVALID_CALL_KEY = 0;

    // used internally only between native MTC and java MTC
    public static final String EXTRA_MESSAGE_AOS_DISCONNECTED = "AOS_DISCONNECTED";

    public int mCode;
    public int mExtraCode;
    public String mExtraMessage;

    //------------------------------------------------------------------------------------------//

    public CallReasonInfo() {
        ImsLog.i("");

        mCode = -1;
        mExtraCode = -1;
        mExtraMessage = "";

        logLn();
    }

    public CallReasonInfo(CallReasonInfo callReasonInfo) {
        ImsLog.i("");

        mCode = callReasonInfo.mCode;
        mExtraCode = callReasonInfo.mExtraCode;
        mExtraMessage = callReasonInfo.mExtraMessage;

        logLn();
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

        logLn();
    }

    /**
     * Helper for leaving the logs.
    */
    public void logLn() {
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

        logLn();
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
            return new CallReasonInfo(source);
        }

        public CallReasonInfo[] newArray(int size) {
            return new CallReasonInfo[size];
        }
    };

}
