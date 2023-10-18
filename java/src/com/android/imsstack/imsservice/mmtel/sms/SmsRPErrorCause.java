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
package com.android.imsstack.imsservice.mmtel.sms;

import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

/**
 * Handles the SMS Error Cause
 */
public enum  SmsRPErrorCause {

    SMS_NO_ERROR(0, ImsSmsImplBase.SEND_STATUS_OK, SmsManager.RESULT_ERROR_NONE),

    //TODO:b/242793727 -Need to add new error result to SMSManager which match with below RPCause
    SMS_UNALLOCATED_NUMBER(1, ImsSmsImplBase.SEND_STATUS_ERROR,
     SmsManager.RESULT_ERROR_GENERIC_FAILURE),

    SMS_OPERATOR_BARRING(8, ImsSmsImplBase.SEND_STATUS_ERROR,
     SmsManager.RESULT_NETWORK_REJECT),

    SMS_CALL_BARRING(10, ImsSmsImplBase.SEND_STATUS_ERROR, SmsManager.RESULT_NETWORK_REJECT),

    SMS_RESERVED(11, ImsSmsImplBase.SEND_STATUS_ERROR, SmsManager.RESULT_INVALID_ARGUMENTS),

    SMS_TRANSFER_REJECTED(21, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                          SmsManager.RESULT_NETWORK_REJECT),

    SMS_DESTINATION_OUT_OF_ORDER(27, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
    SmsManager.RESULT_ERROR_NO_SERVICE),

    SMS_UNIDENTIFIED_SUBSCRIBER(28, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
    SmsManager.RESULT_ERROR_NO_SERVICE),

    SMS_FACILITY_REJECTED(29, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
    SmsManager.RESULT_NETWORK_REJECT),

    SMS_UNKNOWN_SUBSCRIBER(30, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
    SmsManager.RESULT_ERROR_NO_SERVICE),

    SMS_NW_OUT_OF_ORDER(38, ImsSmsImplBase.SEND_STATUS_ERROR,
    SmsManager.RESULT_NETWORK_ERROR),

    SMS_TEMP_FAILURE(41, ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                     SmsManager.RESULT_ERROR_GENERIC_FAILURE),

    SMS_CONGESTION(42, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK, SmsManager.RESULT_NETWORK_ERROR),

    SMS_RESOURCES_UNAVAILABLE(47, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                              SmsManager.RESULT_NO_RESOURCES),

    SMS_FACILITY_NOT_SUBSCRIBED(50, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                                SmsManager.RESULT_ERROR_NO_SERVICE),

    SMS_FACILITY_NOT_IMPLEMENTED(69, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                                 SmsManager.RESULT_NETWORK_ERROR),


    SMS_INVALID_MESSAGE_REF(81, ImsSmsImplBase.SEND_STATUS_ERROR,
                            SmsManager.RESULT_INVALID_ARGUMENTS),

    SMS_INCORRECT_MESSAGE(95, ImsSmsImplBase.SEND_STATUS_ERROR,
    SmsManager.RESULT_INVALID_ARGUMENTS),

    SMS_INVALID_MANDATORY_INFORMATION(96, ImsSmsImplBase.SEND_STATUS_ERROR,
                                      SmsManager.RESULT_INVALID_ARGUMENTS),

    SMS_MESSAGE_TYPE_NOT_IMPLEMENTED(97, ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                                     SmsManager.RESULT_REQUEST_NOT_SUPPORTED),

    SMS_MESSAGE_INCOMPATIBLE_WITH_STATE(98, ImsSmsImplBase.SEND_STATUS_ERROR,
                                        SmsManager.RESULT_INVALID_STATE),

    SMS_IE_NOT_EXISTENT(99, ImsSmsImplBase.SEND_STATUS_ERROR, SmsManager.RESULT_INVALID_ARGUMENTS),

    SMS_PROTOCOL_ERROR(111, ImsSmsImplBase.SEND_STATUS_ERROR, SmsManager.RESULT_NETWORK_ERROR),

    SMS_INTERWORKING_ERROR(127, ImsSmsImplBase.SEND_STATUS_ERROR, SmsManager.RESULT_NETWORK_ERROR);

    final int mRPCauseCode, mSendStatus, mReason;

    SmsRPErrorCause(int rpCauseCode, int sendStatus, int reason) {
        mRPCauseCode = rpCauseCode;
        mSendStatus = sendStatus;
        mReason = reason;
    }

    /*
     * returns the RP-Cause Value
     * @return the RP-Cause Value
     */
    private int getRPCauseCode() {
        return mRPCauseCode;
    }

    /*
     * returns the SendStatus of ImsSmsImplBase
     * @return the SendSms Status to be sent to framework
     */
    private int getSendStatus() {
        return mSendStatus;
    }

    /**
     * returns the reason of SendSms Failure
     * @return the SendSms Fail reason to be sent to framework
     */
    private int getReason() {
        return mReason;
    }


    /**
     * returns the SendSms Status to be sent to framework
     * @param rpCauseCode the RP_Cause Value
     * @return the SendSms Fail Status for the RP-Cause value passed
     */
    public static int getSendSmsStatusByRPCauseCode(int rpCauseCode) {
        for (SmsRPErrorCause r : values()) {
            if (r.getRPCauseCode() == rpCauseCode) {
                return r.getSendStatus();
            }
        }
        //As per TS 24.011, Table 84, All the other Cause codes shall be treated as below
        return SMS_TEMP_FAILURE.getSendStatus();
    }

    /**
     * returns the SendSms Failure reason to be sent to framework
     * @param rpCauseCode the RP_Cause Value
     * @return the SendSms Fail reason for the RP-Cause value passed
     */
    public static int getSendSmsStatusReasonByRPCauseCode(int rpCauseCode) {
        for (SmsRPErrorCause r : values()) {
            if (r.getRPCauseCode() == rpCauseCode) {
                return r.getReason();
            }
        }
        //TS 24.011, Table 84, All the other Cause codes shall be treated as below
        return SMS_TEMP_FAILURE.getReason();
    }

    /**
     * returns the SendSms Status to be sent to framework
     * @param smsRPErrorCause Enum Value
     * @return the SendSms status for the SmsRPErrorCause value passed
     */
    public static int getSendSmsStatusBySmsErrorCause(SmsRPErrorCause smsRPErrorCause) {
        for (SmsRPErrorCause r : values()) {
            if (r == smsRPErrorCause) {
                return r.getSendStatus();
            }
        }
        return SMS_TEMP_FAILURE.getSendStatus();
    }

    /**
     * returns the SendSms Fail reason to be sent to framework
     * @param smsRPErrorCause Enum Value
     * @return the SendSms Fail reason for the SmsRPErrorCause value passed
     */
    public static int getSendSmsStatusReasonBySendErrorCode(SmsRPErrorCause smsRPErrorCause) {
        for (SmsRPErrorCause r : values()) {
            if (r == smsRPErrorCause) {
                return r.getReason();
            }
        }
        return SMS_TEMP_FAILURE.getReason();
    }

    /**
     * returns the SendSmsErrorCause for RP-Cause Value
     * @param rpCauseCode RP-Cause Value
     * @return the SendSmsErrorCause for the SmsRPErrorCause value passed
     */
    public static SmsRPErrorCause getSmsErrorCauseByRPErrorCause(int rpCauseCode) {
        for (SmsRPErrorCause r : values()) {
            if (r.getRPCauseCode() == rpCauseCode) {
                return r;
            }
        }
        return SMS_TEMP_FAILURE;
    }
}
