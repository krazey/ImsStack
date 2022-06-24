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

/**
 * Utility class for SMS Functionality
 **/
public class SmsUtils {
    public static final int MAX_RPMR_VALUE = 256;
    /* SMS Message Type */
    public static final int TP_MESSAGETYPE = 100;
    public static final int RP_MESSAGETYPE = 200;

    public static final int TP_SMS_SUBMIT = TP_MESSAGETYPE + 1;
    public static final int TP_SMS_SUBMIT_REPORT = TP_MESSAGETYPE + 2;
    public static final int TP_SMS_DELIVER = TP_MESSAGETYPE + 3;
    public static final int TP_SMS_DELIVER_REPORT = TP_MESSAGETYPE + 4;
    public static final int TP_SMS_STATUS_REPORT = TP_MESSAGETYPE + 5;

    public static final int TPDU_MTI_INDEX = 0;
    public static final int TPDU_MR_INDEX = 1;
    public static final int MODIFIED_TPDU_ORIGIN_ADDR_LENGTH_INDEX = 0;

    /* 3GPP TS 23.040 Section 9.2.3.1 */
    public static final int TPDU_MTI_SMS_DELIVER = 0;
    public static final int TPDU_MTI_SMS_DELIVER_REPORT = 0;
    public static final int TPDU_MTI_SMS_STATUS_REPORT = 2;
    public static final int TPDU_MTI_SMS_COMMAND = 2;
    public static final int TPDU_MTI_SMS_SUBMIT = 1;
    public static final int TPDU_MTI_SMS_SUBMIT_REPORT = 1;

    public static final int RP_DATA = RP_MESSAGETYPE + 1;
    public static final int RP_ACK = RP_MESSAGETYPE + 2;
    public static final int RP_ERROR = RP_MESSAGETYPE + 3;
    public static final int RP_SMMA = RP_MESSAGETYPE + 4;

    // SMS-RelayLayer Result
    public static final int RESULT_SUCCESS = 0;
    public static final int RESULT_FAILURE = 1;
    public static final int SMSRL_RESULT_FAILURE = 2;
    public static final int SMSTL_RESULT_FAILURE = 3;
    public static final int SMSRL_RESULT_EXCEPTION = 4;
    public static final int SMSRL_RESULT_MTS_CONTROLLER_FAILED = 4;
    public static final int SMSRL_RESULT_PDU_ENCODING_FAILED = 5;
    public static final int SMSTL_RESULT_EXCEPTION = 6;
    public static final int SMSRL_RESULT_INVALID_RP_MESSAGE_TYPE = 7;
    public static final int SMS_RESULT_INVALID_SMSC_ADDRESS = 8;
    public static final int SMSTL_RESULT_QUEUE_SIZE_EXCEEDED = 9;

    public static final int RPDU_ORIGIN_ADDR_LENGTH_INDEX = 2;
    public static final int RPDU_ORIGIN_ADDR_VALUE_INDEX = 3;

    /*SMS Format*/
    public static final int FORMAT_INT_INVALID = 0;
    public static final int FORMAT_INT_3GPP = 1;
    public static final int FORMAT_INT_3GPP2 = 2;
    public static final String[] FORMAT_STRING = new String[] {"inavlid", "3gpp", "3gpp2"};

    /**
     * Returns SMS Format Value in String
     * @param format indicates the Sms Format in Integer
     *
     * @return SMS Format in String
     */
    public static String getFormatString(int format) {
        if (format == 1) {
            return "3gpp";
        } else if (format == 2) {
            return "3gpp2";
        } else {
            return "invalid";
        }
    }

}
