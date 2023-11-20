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
package com.android.imsstack.base;

import static android.provider.Settings.Global.DEVICE_NAME;

import static com.android.imsstack.base.TestAppContext.SUB_ID_1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.ContentProviderProxy.SettingsProxy;
import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class AppContextTest {
    private static final int RUN_TASK_DELAY = 500; // milli-seconds

    @Mock private Context mContext;
    @Mock private ContentProviderProxy mContentProviderProxy;
    @Mock private SystemServiceProxy mSystemServiceProxy;
    @Mock private TelephonyManagerProxy mTelephonyManagerProxy;
    @Mock private SmsManagerProxy mSmsManagerProxy;
    @Mock private Runnable mRunnable;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mSystemServiceProxy.getSystemService(eq(TelephonyManagerProxy.class)))
                .thenReturn(mTelephonyManagerProxy);
        when(mSystemServiceProxy.getSystemService(eq(SmsManagerProxy.class)))
                .thenReturn(mSmsManagerProxy);
        when(mTelephonyManagerProxy.createForSubscriptionId(eq(SUB_ID_1)))
                .thenReturn(mTelephonyManagerProxy);
        when(mSmsManagerProxy.createForSubscriptionId(eq(SUB_ID_1)))
                .thenReturn(mSmsManagerProxy);
        AppContext.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        mContentProviderProxy = null;
        mTelephonyManagerProxy = null;
        mSmsManagerProxy = null;
        mSystemServiceProxy = null;
        mRunnable = null;
        mContext = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testInit() throws IllegalStateException {
        AppContext appContext = AppContext.getInstance();

        assertNotNull(appContext);
    }

    @Test
    @SmallTest
    public void testDeinit() {
        AppContext.deinit();

        assertThrows(IllegalStateException.class, () -> {
            AppContext.getInstance();
        });
    }

    @Test
    @SmallTest
    public void testGetSystemServiceProxy() {
        AppContext.getInstance().setSystemServiceProxy(mSystemServiceProxy);
        AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
        verify(mSystemServiceProxy).getSystemService(eq(TelephonyManagerProxy.class));
    }

    @Test
    @SmallTest
    public void testGetTelephonyManagerProxy() {
        AppContext.getInstance().setSystemServiceProxy(mSystemServiceProxy);
        TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(SUB_ID_1);
        assertEquals(tmp, mTelephonyManagerProxy);
    }

    @Test
    @SmallTest
    public void testGetSmsManagerProxy() {
        AppContext.getInstance().setSystemServiceProxy(mSystemServiceProxy);
        SmsManagerProxy smp = AppContext.getSmsManagerProxy(SUB_ID_1);
        assertEquals(smp, mSmsManagerProxy);
    }

    @Test
    @SmallTest
    public void testRunTask() throws Exception {
        TestableLooper looper = new TestableLooper(AppContext.getInstance().getMainLooper());
        try {
            AppContext.runTask(mRunnable, 0);
            looper.processAllMessages();
            verify(mRunnable).run();
        } finally {
            looper.destroy();
        }
    }

    @Test
    public void testRunTask_delay() throws Exception {
        TestableLooper looper = new TestableLooper(AppContext.getInstance().getMainLooper());
        try {
            AppContext.runTask(mRunnable, RUN_TASK_DELAY);
            looper.moveTimeForward(RUN_TASK_DELAY + 1);
            looper.processAllMessages();
            verify(mRunnable).run();
        } finally {
            looper.destroy();
        }
    }

    @Test
    @SmallTest
    public void testGetMainExecutor() throws IllegalStateException {
        assertNotNull(AppContext.getInstance().getMainExecutor());
    }

    @Test
    @SmallTest
    public void testGetMainHandler() throws IllegalStateException {
        assertNotNull(AppContext.getInstance().getMainHandler());
    }

    @Test
    @SmallTest
    public void testGetMainLooper() throws Exception {
        assertNotNull(AppContext.getInstance().getMainLooper());
    }

    @Test
    @SmallTest
    public void testGetDeviceName() {
        AppContext.getInstance().setContentProviderProxy(mContentProviderProxy);
        String testDeviceName = "Device-A";
        SettingsProxy settingsProxy = mock(SettingsProxy.class);
        when(mContentProviderProxy.getGlobalSettings()).thenReturn(settingsProxy);
        when(settingsProxy.getString(eq(DEVICE_NAME), anyString())).thenReturn(testDeviceName);

        assertEquals(testDeviceName, AppContext.getDeviceName());
    }

    @Test
    @SmallTest
    public void testGetExternalStoragePath() {
        assertNotNull(AppContext.getExternalStoragePath());
    }
}
