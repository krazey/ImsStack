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

import static org.junit.Assert.assertEquals;

import android.telephony.SmsManager;
import android.telephony.ims.stub.ImsSmsImplBase;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SmsRPErrorCauseTest {

    @Test
    public void test_getSendSmsStatusByRPCauseCode() {
        assertEquals(ImsSmsImplBase.SEND_STATUS_OK,
                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(0));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(41));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR,
                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(11));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK,
                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(69));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsRPErrorCause.getSendSmsStatusByRPCauseCode(43));
    }

    @Test
    public void test_getSendSmsStatusReasonByRPCauseCode() {
        assertEquals(SmsManager.RESULT_ERROR_NONE,
                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(0));
        assertEquals(SmsManager.RESULT_ERROR_GENERIC_FAILURE,
                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(41));
        assertEquals(SmsManager.RESULT_INVALID_ARGUMENTS,
                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(11));
        assertEquals(SmsManager.RESULT_NETWORK_ERROR,
                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(69));
        assertEquals(SmsManager.RESULT_ERROR_GENERIC_FAILURE,
                SmsRPErrorCause.getSendSmsStatusReasonByRPCauseCode(43));
    }

    @Test
    public void test_getSendSmsStatusBySmsErrorCause() {
        assertEquals(ImsSmsImplBase.SEND_STATUS_OK,
                SmsRPErrorCause.getSendSmsStatusBySmsErrorCause(SmsRPErrorCause.SMS_NO_ERROR));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsRPErrorCause.getSendSmsStatusBySmsErrorCause(SmsRPErrorCause.SMS_TEMP_FAILURE));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR,
                SmsRPErrorCause.getSendSmsStatusBySmsErrorCause(SmsRPErrorCause.SMS_RESERVED));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR,
                SmsRPErrorCause.getSendSmsStatusBySmsErrorCause(
                SmsRPErrorCause.SMS_NW_OUT_OF_ORDER));
        assertEquals(ImsSmsImplBase.SEND_STATUS_ERROR_RETRY,
                SmsRPErrorCause.getSendSmsStatusBySmsErrorCause(null));
    }

    @Test
    public void test_getSendSmsStatusReasonBySendErrorCode() {
        assertEquals(SmsManager.RESULT_ERROR_NO_SERVICE,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_UNKNOWN_SUBSCRIBER));
        assertEquals(SmsManager.RESULT_ERROR_GENERIC_FAILURE,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_UNALLOCATED_NUMBER));
        assertEquals(SmsManager.RESULT_INVALID_ARGUMENTS,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_INCORRECT_MESSAGE));
        assertEquals(SmsManager.RESULT_NETWORK_ERROR,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_NW_OUT_OF_ORDER));
        assertEquals(SmsManager.RESULT_NETWORK_REJECT,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_TRANSFER_REJECTED));
        assertEquals(SmsManager.RESULT_NO_RESOURCES,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_RESOURCES_UNAVAILABLE));
        assertEquals(SmsManager.RESULT_REQUEST_NOT_SUPPORTED,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_MESSAGE_TYPE_NOT_IMPLEMENTED));
        assertEquals(SmsManager.RESULT_ERROR_NONE,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_NO_ERROR));
        assertEquals(SmsManager.RESULT_ERROR_GENERIC_FAILURE,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(
                SmsRPErrorCause.SMS_TEMP_FAILURE));
        assertEquals(SmsManager.RESULT_ERROR_GENERIC_FAILURE,
                SmsRPErrorCause.getSendSmsStatusReasonBySendErrorCode(null));
    }

    @Test
    public void test_getSmsErrorCauseByRPErrorCause() {
        assertEquals(SmsRPErrorCause.SMS_OPERATOR_BARRING,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(8));
        assertEquals(SmsRPErrorCause.SMS_TEMP_FAILURE,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(41));
        assertEquals(SmsRPErrorCause.SMS_MESSAGE_INCOMPATIBLE_WITH_STATE,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(98));
        assertEquals(SmsRPErrorCause.SMS_IE_NOT_EXISTENT,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(99));
        assertEquals(SmsRPErrorCause.SMS_PROTOCOL_ERROR,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(111));
        assertEquals(SmsRPErrorCause.SMS_INTERWORKING_ERROR,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(127));
        assertEquals(SmsRPErrorCause.SMS_TEMP_FAILURE,
                SmsRPErrorCause.getSmsErrorCauseByRPErrorCause(-1));
    }
}

