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

package com.android.imsstack.test;

import static com.android.imsstack.util.ImsPrivateProperties.Persistent.KEY_TEST_DEBUG_ENABLED;
import static com.android.imsstack.util.ImsPrivateProperties.Persistent.KEY_TEST_IMS_DISABLED;
import static com.android.imsstack.util.ImsPrivateProperties.Persistent.KEY_TEST_TESTMODE_ENABLED;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

public class ImsTestModeTest {
    private static final int SLOT0 = 0;

    @Mock private Context mContext;
    @Mock private SharedPreferences mSp;

    private ImsTestMode mImsTestModeTest;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(mSp).when(mContext).getSharedPreferences(anyString(), anyInt());
        when(mSp.getString(eq(KEY_TEST_TESTMODE_ENABLED), anyString()))
                .thenReturn(String.valueOf(true));
        when(mSp.getString(eq(KEY_TEST_IMS_DISABLED), anyString()))
                .thenReturn(String.valueOf(true));
        when(mSp.getString(eq(KEY_TEST_DEBUG_ENABLED), anyString()))
                .thenReturn(String.valueOf(true));
        AppContext.init(mContext);

        mImsTestModeTest = ImsTestMode.getInstance();
        // This is a singleton object and it needs to be cleaned up before starting the test.
        mImsTestModeTest.cleanUp(SLOT0);
    }

    @After
    public void tearDown() throws Exception {
        if (mImsTestModeTest != null) {
            mImsTestModeTest.cleanUp(SLOT0);
            mImsTestModeTest = null;
        }
        AppContext.deinit();
        mSp = null;
        mContext = null;
    }

    @Test
    @SmallTest
    public void testInit() {
        mImsTestModeTest.init(SLOT0);
        IImsTestMode testMode = mImsTestModeTest.getTestMode(SLOT0);

        assertNotNull(testMode);
        assertEquals(0, testMode.getExtraTestmask());
        assertTrue(testMode.isGenericTestMode());
        assertTrue(testMode.isImsOff());
        assertTrue(testMode.isDebugEnabled());
        assertTrue(testMode.isDebuggable());
        assertTrue(testMode.isLocalHoldToneEnabled());
    }

    @Test
    @SmallTest
    public void testGetTestMode() {
        IImsTestMode testMode = mImsTestModeTest.getTestMode(SLOT0);

        assertNotNull(testMode);
        assertEquals(0, testMode.getExtraTestmask());
        assertTrue(testMode.isGenericTestMode());
        assertTrue(testMode.isImsOff());
        assertTrue(testMode.isDebugEnabled());
        assertTrue(testMode.isDebuggable());
        assertTrue(testMode.isLocalHoldToneEnabled());
    }
}
