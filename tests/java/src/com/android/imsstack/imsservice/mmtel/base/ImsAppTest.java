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

package com.android.imsstack.imsservice.mmtel.base;

import static org.junit.Assert.assertEquals;

import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsAppTest {
    private ImsApp mImsApp;

    private int mPhoneId = 0;

    @Before
    public void setUp() throws Exception {
        //Preparation of the mock to use the real code when a method is invoked
        mImsApp = Mockito.mock(ImsApp.class, Mockito.CALLS_REAL_METHODS);
    }

    @Test
    public void test_getPhoneId() {
        assertEquals(mPhoneId, mImsApp.getPhoneId());
    }

    @Test
    public void test_isConnected() {
        assertEquals(false, mImsApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
            ImsCallProfile.CALL_TYPE_VOICE));
    }

    @After
    public void tearDown() throws Exception {
        mImsApp = null;
    }
}
