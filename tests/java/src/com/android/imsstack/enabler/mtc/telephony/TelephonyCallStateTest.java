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

package com.android.imsstack.enabler.mtc.telephony;

import static org.junit.Assert.assertEquals;

import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.imsstack.enabler.mtc.telephony.TelephonyCallState;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class TelephonyCallStateTest {
    private int mState = TelephonyManager.CALL_STATE_IDLE;
    private String mIncomingNumber = "";
    private TelephonyCallState mTelephonyCallState;
    public static final String TAG = "TelephonyCallStateTest";

    @Before
    public void setUp() throws Exception {
         Log.d(TAG, " Unit Test for TelephonyCallState");
         mTelephonyCallState = new TelephonyCallState();
    }

    /**
     *  Possible call state values
     *      TelephonyManager#CALL_STATE_IDLE (0)
     *      TelephonyManager#CALL_STATE_RINGING (1)
     *      TelephonyManager#CALL_STATE_OFFHOOK (2)
     */
    @Test
    public void getCallState_test(){
        assertEquals(mState, mTelephonyCallState.getState());
    }

    @Test
    public void getIncomingNumber_test(){
        assertEquals(mIncomingNumber, mTelephonyCallState.getIncomingNumber());
    }

    @After
    public void tearDown() throws Exception {
        mTelephonyCallState = null;
    }
}
