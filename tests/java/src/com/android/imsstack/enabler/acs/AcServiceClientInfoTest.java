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

package com.android.imsstack.enabler.acs;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.util.Log;

import androidx.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;

public class AcServiceClientInfoTest {
    private static final String TAG = AcServiceClientInfoTest.class.getSimpleName();
    private static final String VERSION = "6.0";
    private static final String PROFILE = "UP_1.0";
    private static final String CLIENT_VENDOR = "Google";
    private static final String CLIENT_VERSION = "1.0";
    private static final boolean ENABLED_BY_USER = true;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {

    }

    @Test
    @SmallTest
    public void creatorTest() throws Exception {
        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);

        assertTrue(acServiceClientInfo.isValid());
        assertEquals(VERSION, acServiceClientInfo.getRcsVersion());
        assertEquals(PROFILE, acServiceClientInfo.getRcsProfile());
        assertEquals(CLIENT_VENDOR, acServiceClientInfo.getClientVendor());
        assertEquals(CLIENT_VERSION, acServiceClientInfo.getClientVersion());
        assertEquals(ENABLED_BY_USER, acServiceClientInfo.isRcsEnabledByUser());

        AcServiceClientInfo acServiceClientInfo1 = new AcServiceClientInfo(acServiceClientInfo);
        assertTrue(acServiceClientInfo1.isValid());
        assertEquals(VERSION, acServiceClientInfo1.getRcsVersion());
        assertEquals(PROFILE, acServiceClientInfo1.getRcsProfile());
        assertEquals(CLIENT_VENDOR, acServiceClientInfo1.getClientVendor());
        assertEquals(CLIENT_VERSION, acServiceClientInfo1.getClientVersion());
        assertEquals(ENABLED_BY_USER, acServiceClientInfo1.isRcsEnabledByUser());

        AcServiceClientInfo acServiceClientInfo2 = new AcServiceClientInfo(
                "", PROFILE, CLIENT_VENDOR, "", ENABLED_BY_USER);
        assertFalse(acServiceClientInfo2.isValid());

        Log.i(TAG, acServiceClientInfo.toString());
        Log.i(TAG, acServiceClientInfo1.toString());
        Log.i(TAG, acServiceClientInfo2.toString());
    }
}
