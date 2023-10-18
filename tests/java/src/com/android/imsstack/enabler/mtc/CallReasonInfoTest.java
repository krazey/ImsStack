/*
 * Copyright (C) 2023 The Android Open Source Project
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

import static org.junit.Assert.assertEquals;

import android.os.Parcel;
import android.testing.AndroidTestingRunner;

import org.junit.Test;
import org.junit.runner.RunWith;

@RunWith(AndroidTestingRunner.class)
public class CallReasonInfoTest  {
    @Test
    public void testDefaultConstructor() {
        CallReasonInfo callReasonInfo = new CallReasonInfo();

        assertEquals(CallReasonInfo.CODE_NONE, callReasonInfo.mCode);
        assertEquals(CallReasonInfo.CODE_NONE, callReasonInfo.mExtraCode);
        assertEquals("", callReasonInfo.mExtraMessage);
    }

    @Test
    public void testConstructorWithCodeExtraCodeExtraMessage() {
        CallReasonInfo callReasonInfoTerminated = new CallReasonInfo(
                CallReasonInfo.CODE_USER_TERMINATED, CallReasonInfo.EXTRA_USER_TERMINATED_ECT,
                "extra message");

        assertEquals(CallReasonInfo.CODE_USER_TERMINATED, callReasonInfoTerminated.mCode);
        assertEquals(CallReasonInfo.EXTRA_USER_TERMINATED_ECT, callReasonInfoTerminated.mExtraCode);
        assertEquals("extra message", callReasonInfoTerminated.mExtraMessage);
    }

    @Test
    public void testConstructorWithCallReasonInfo() {
        CallReasonInfo callReasonInfo = new CallReasonInfo();
        callReasonInfo.mCode = CallReasonInfo.CODE_SIP_REQUEST_TIMEOUT;
        callReasonInfo.mExtraCode = CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_FIRE;
        callReasonInfo.mExtraMessage = "fire";

        CallReasonInfo callReasonInfoParamCallReasonInfo =
                new CallReasonInfo(callReasonInfo);

        assertEquals(CallReasonInfo.CODE_SIP_REQUEST_TIMEOUT,
                callReasonInfoParamCallReasonInfo.mCode);
        assertEquals(CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_FIRE,
                callReasonInfoParamCallReasonInfo.mExtraCode);
        assertEquals("fire", callReasonInfoParamCallReasonInfo.mExtraMessage);
    }

    @Test
    public void testParcelReadWrite() {
        CallReasonInfo callReasonInfo = new CallReasonInfo();
        callReasonInfo.mCode = CallReasonInfo.CODE_SIP_REQUEST_TIMEOUT;
        callReasonInfo.mExtraCode = CallReasonInfo.EXTRA_CODE_EMERGENCYSERVICE_FIRE;
        callReasonInfo.mExtraMessage = "fire";

        Parcel dest = Parcel.obtain();
        callReasonInfo.writeToParcel(dest, 0);
        dest.setDataPosition(0);

        CallReasonInfo callReasonInfo2 = new CallReasonInfo(dest);

        assertEquals(callReasonInfo.mCode, callReasonInfo2.mCode);
        assertEquals(callReasonInfo.mExtraCode, callReasonInfo2.mExtraCode);
        assertEquals(callReasonInfo.mExtraMessage, callReasonInfo2.mExtraMessage);
    }
}
