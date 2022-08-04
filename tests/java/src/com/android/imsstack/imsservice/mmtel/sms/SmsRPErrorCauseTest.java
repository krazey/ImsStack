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
    private int mRpCauseCode;
    private int mSendStatus;
    private int mReason;

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
}
