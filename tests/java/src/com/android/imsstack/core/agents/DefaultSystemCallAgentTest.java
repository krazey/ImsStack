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
package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.provider.Settings;
import android.test.mock.MockContentResolver;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.internal.util.test.FakeSettingsProvider;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class DefaultSystemCallAgentTest {
    private static final String TEST_FILE = "test_prefs";
    private static final String TEST_KEY = "test-key";
    private static final String TEST_VALUE = "test-value";
    private static final int SLOT0 = 0;

    @Mock private Context mContext;
    @Mock private SystemInterface mSystemInterface;
    @Mock private PreferenceInterface mPreferenceInterface;
    @Mock private WakeLockInterface mWakeLockInterface;
    @Mock private WifiInterface mWifiInterface;
    @Mock private TimerAgent mTimerAgent;

    private DefaultSystemCallAgent mDefaultSystemCallAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        AppContext.init(mContext);
        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.getInstance().setAgent(PreferenceInterface.class, mPreferenceInterface);
        AgentFactory.getInstance().setAgent(WakeLockInterface.class, mWakeLockInterface);
        AgentFactory.getInstance().setAgent(WifiInterface.class, mWifiInterface);
        AgentFactory.getInstance().setAgent(TimerInterface.class, mTimerAgent);

        mDefaultSystemCallAgent = new DefaultSystemCallAgent();
    }

    @After
    public void tearDown() throws Exception {
        if (mDefaultSystemCallAgent != null) {
            mDefaultSystemCallAgent.destroy();
            mDefaultSystemCallAgent = null;
        }

        AgentFactory.getInstance().setAgent(TimerInterface.class, null);
        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
        AgentFactory.getInstance().setAgent(WakeLockInterface.class, null);
        AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);
        SystemInterface.setSystemInterface(null);

        mTimerAgent = null;
        mWifiInterface = null;
        mSystemInterface = null;
        mContext = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testCreateAndDestroy() {
        verify(mSystemInterface).setSystemCallInterface(eq(mDefaultSystemCallAgent));

        mDefaultSystemCallAgent.destroy();
        mDefaultSystemCallAgent = null;

        verify(mSystemInterface).setSystemCallInterface(eq(null));
    }

    @Test
    @SmallTest
    public void testAcquireWakeLock() {
        int timeout = 1000; // 1 second
        mDefaultSystemCallAgent.acquireWakeLock(timeout);

        verify(mWakeLockInterface).acquireForNative(eq(timeout));
    }

    @Test
    @SmallTest
    public void testStartTimer() {
        long tid = 1L;
        long duration = 1000L;
        when(mTimerAgent.startNativeTimer(eq(tid), eq(duration))).thenReturn(true);
        boolean result = mDefaultSystemCallAgent.startTimer(tid, duration);

        assertTrue(result);
        verify(mTimerAgent).startNativeTimer(eq(tid), eq(duration));

        AgentFactory.getInstance().setAgent(TimerInterface.class, null);
        result = mDefaultSystemCallAgent.startTimer(tid, duration);

        assertFalse(result);
        verifyNoMoreInteractions(mTimerAgent);
    }

    @Test
    @SmallTest
    public void testStopTimer() {
        long tid = 1L;
        mDefaultSystemCallAgent.stopTimer(tid);

        verify(mTimerAgent).stopNativeTimer(eq(tid));

        AgentFactory.getInstance().setAgent(TimerInterface.class, null);
        mDefaultSystemCallAgent.stopTimer(tid);

        verifyNoMoreInteractions(mTimerAgent);
    }

    @Test
    @SmallTest
    public void testGetWifiInterface() {
        WifiInterface wifi = mDefaultSystemCallAgent.getWifiInterface();

        assertEquals(mWifiInterface, wifi);

        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
        wifi = mDefaultSystemCallAgent.getWifiInterface();

        assertNull(wifi);
    }

    @Test
    @SmallTest
    public void testGetDeviceName() {
        String testDeviceName = "Device-A";
        MockContentResolver contentResolver = new MockContentResolver();
        FakeSettingsProvider settingsProvider = new FakeSettingsProvider();
        contentResolver.addProvider(Settings.AUTHORITY, settingsProvider);
        Settings.Global.putString(contentResolver, Settings.Global.DEVICE_NAME, testDeviceName);
        when(mContext.getContentResolver()).thenReturn(contentResolver);

        assertEquals(testDeviceName, mDefaultSystemCallAgent.getDeviceName());
    }

    @Test
    @SmallTest
    public void testGetExternalStoragePath() {
        assertNotNull(mDefaultSystemCallAgent.getExternalStoragePath());
    }

    @Test
    @SmallTest
    public void testGetPreference() {
        mDefaultSystemCallAgent.getPreference(TEST_FILE, TEST_KEY, SLOT0);

        verify(mPreferenceInterface).getString(eq(TEST_FILE), eq(TEST_KEY), eq(SLOT0));

        AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);
        String value = mDefaultSystemCallAgent.getPreference(TEST_FILE, TEST_KEY, SLOT0);

        assertNull(value);
        verifyNoMoreInteractions(mPreferenceInterface);
    }

    @Test
    @SmallTest
    public void testSetPreference() {
        mDefaultSystemCallAgent.setPreference(TEST_FILE, TEST_KEY, TEST_VALUE, SLOT0);

        verify(mPreferenceInterface)
                .putString(eq(TEST_FILE), eq(TEST_KEY), eq(TEST_VALUE), eq(SLOT0));

        AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);
        boolean result = mDefaultSystemCallAgent
                .setPreference(TEST_FILE, TEST_KEY, TEST_VALUE, SLOT0);

        assertFalse(result);
        verifyNoMoreInteractions(mPreferenceInterface);
    }
}
