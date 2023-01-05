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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AppContextTest {
    private static final int SUB_ID = 1;
    private static final int RUN_TASK_DELAY = 500; // milli-seconds
    private static final int RUN_TASK_WAIT_TIMEOUT = 30; // milli-seconds

    private static ContextFixture sContext;
    private TelephonyManager mTelephonyManager;

    @Mock Runnable mRunnable;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTelephonyManager = sContext.getTestDouble().getSystemService(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);

        AppContext.init(sContext.getTestDouble());
    }

    @After
    public void tearDown() throws Exception {
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    @SmallTest
    public void init() throws IllegalStateException {
        AppContext appContext = AppContext.getInstance();

        assertNotNull(appContext);
    }

    @Test
    @SmallTest
    public void deinit() {
        AppContext.deinit();

        assertThrows(IllegalStateException.class, () -> {
            AppContext.getInstance();
        });
    }

    @Test
    @SmallTest
    public void getTelephonyManager() {
        TelephonyManager tm = AppContext.getTelephonyManager();
        assertEquals(tm, mTelephonyManager);
    }

    @Test
    @SmallTest
    public void getTelephonyManager_subscription() {
        TelephonyManager tm = AppContext.getTelephonyManager(SUB_ID);
        assertEquals(tm, mTelephonyManager);
    }

    @Test
    @SmallTest
    public void runTask() {
        AppContext.runTask(mRunnable, 0);
        verify(mRunnable, timeout(RUN_TASK_WAIT_TIMEOUT)).run();
    }

    @Test
    public void runTask_delay() {
        AppContext.runTask(mRunnable, RUN_TASK_DELAY);
        verify(mRunnable, timeout(RUN_TASK_DELAY + RUN_TASK_WAIT_TIMEOUT)).run();
    }

    @Test
    @SmallTest
    public void getMainExecutor() throws IllegalStateException {
        assertNotNull(AppContext.getInstance().getMainExecutor());
    }

    @Test
    @SmallTest
    public void getMainHandler() throws IllegalStateException {
        assertNotNull(AppContext.getInstance().getMainHandler());
    }

    @Test
    @SmallTest
    public void getMainLooper() throws Exception {
        assertNotNull(AppContext.getInstance().getMainLooper());
    }
}
