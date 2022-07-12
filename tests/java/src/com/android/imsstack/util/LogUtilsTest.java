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
package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.content.SharedPreferences;
import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class LogUtilsTest {
    private static final int SLOT_ID = 0;

    @Mock Context mContext;
    @Mock SharedPreferences mSharedPreferences;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        AppContext.init(mContext);
        when(mContext.getSharedPreferences(anyString(), anyInt())).thenReturn(mSharedPreferences);
    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void getLogOptions() throws Exception {
        // Default value
        when(mSharedPreferences.getString(anyString(), anyString()))
                .thenReturn(LogUtils.DEFAULT_LOG_OPTIONS);
        int logOptions = LogUtils.getLogOptions(SLOT_ID);
        assertEquals(SystemUtils.hexStringToInt(LogUtils.DEFAULT_LOG_OPTIONS), logOptions);

        // Specific value
        String testLogOptions = "0x00010003";
        when(mSharedPreferences.getString(anyString(), anyString())).thenReturn(testLogOptions);

        logOptions = LogUtils.getLogOptions(SLOT_ID);
        assertEquals(SystemUtils.hexStringToInt(testLogOptions), logOptions);
    }

    @Test
    @SmallTest
    public void isDebugOn() throws Exception {
        assertFalse(LogUtils.isDebugOn(SLOT_ID));

        when(mSharedPreferences.getString(anyString(), anyString()))
                .thenReturn(String.valueOf(true));
        assertTrue(LogUtils.isDebugOn(SLOT_ID));

        when(mSharedPreferences.getString(anyString(), anyString()))
                .thenReturn(String.valueOf(false));
        assertFalse(LogUtils.isDebugOn(SLOT_ID));
    }
}
