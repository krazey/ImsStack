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
package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import android.content.Context;
import android.provider.Settings;
import android.test.mock.MockContentResolver;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.internal.util.test.FakeSettingsProvider;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class DeviceUtilsTest {
    private ContextFixture mContextFixture;
    private Context mContext;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        AppContext.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        AppContext.deinit();
        mContext = null;
        mContextFixture = null;
    }

    @Test
    @SmallTest
    public void testGetDeviceName() {
        String testDeviceName = "Device-A";
        MockContentResolver contentResolver = (MockContentResolver) mContext.getContentResolver();
        contentResolver.addProvider(Settings.AUTHORITY, new FakeSettingsProvider());
        Settings.Global.putString(contentResolver, Settings.Global.DEVICE_NAME, null);

        assertEquals("", DeviceUtils.getDeviceName());

        Settings.Global.putString(contentResolver, Settings.Global.DEVICE_NAME, testDeviceName);

        assertEquals(testDeviceName, DeviceUtils.getDeviceName());
    }

    @Test
    @SmallTest
    public void testGetExternalStoragePath() {
        assertNotNull(DeviceUtils.getExternalStoragePath());
    }
}
