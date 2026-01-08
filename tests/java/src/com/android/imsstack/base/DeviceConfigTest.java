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
package com.android.imsstack.base;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.when;

import androidx.test.filters.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class DeviceConfigTest {
    private TestAppContext mTestAppContext;

    @Before
    public void setUp() throws Exception {
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();
    }

    @After
    public void tearDown() throws Exception {
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testInit() {
        DeviceConfig.init(AppContext.getInstance());

        assertEquals(1, DeviceConfig.getActiveSimCount());
        assertEquals(1, DeviceConfig.getSupportedSimCount());
        assertFalse(DeviceConfig.isMultiSimEnabled());

        TelephonyManagerProxy tmp =
                mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(tmp.getActiveModemCount()).thenReturn(2);
        when(tmp.getSupportedModemCount()).thenReturn(2);
        DeviceConfig.init(AppContext.getInstance());

        assertEquals(2, DeviceConfig.getActiveSimCount());
        assertEquals(2, DeviceConfig.getSupportedSimCount());
        assertTrue(DeviceConfig.isMultiSimEnabled());
    }
}
