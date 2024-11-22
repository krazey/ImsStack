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

import static android.provider.Settings.Global.DEVICE_NAME;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.ContentProviderProxy.SettingsProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.system.SystemInterface;

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
    private static final int TEST_PRIORITY_TYPE = 1;

    @Mock private SystemInterface mSystemInterface;
    @Mock private PreferenceInterface mPreferenceInterface;
    @Mock private WakeLockInterface mWakeLockInterface;
    @Mock private WifiInterface mWifiInterface;
    @Mock private BatteryStateInterface mBatteryStateInterface;
    @Mock private TimerAgent mTimerAgent;
    @Mock private ImsTrafficInterface mImsTrafficInterface;

    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private DefaultSystemCallAgent mDefaultSystemCallAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);

        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.getInstance().setAgent(PreferenceInterface.class, mPreferenceInterface);
        AgentFactory.getInstance().setAgent(WakeLockInterface.class, mWakeLockInterface);
        AgentFactory.getInstance().setAgent(WifiInterface.class, mWifiInterface);
        AgentFactory.getInstance().setAgent(TimerInterface.class, mTimerAgent);
        AgentFactory.getInstance().setAgent(BatteryStateInterface.class, mBatteryStateInterface);
        AgentFactory.getInstance().setAgent(ImsTrafficInterface.class, mImsTrafficInterface);

        mDefaultSystemCallAgent = new DefaultSystemCallAgent();
    }

    @After
    public void tearDown() throws Exception {
        if (mDefaultSystemCallAgent != null) {
            mDefaultSystemCallAgent.destroy();
            mDefaultSystemCallAgent = null;
        }

        AgentFactory.getInstance().setAgent(ImsTrafficInterface.class, null);
        AgentFactory.getInstance().setAgent(BatteryStateInterface.class, null);
        AgentFactory.getInstance().setAgent(TimerInterface.class, null);
        AgentFactory.getInstance().setAgent(WifiInterface.class, null);
        AgentFactory.getInstance().setAgent(WakeLockInterface.class, null);
        AgentFactory.getInstance().setAgent(PreferenceInterface.class, null);
        SystemInterface.setSystemInterface(null);

        mTelephonyManagerProxy = null;
        mImsTrafficInterface = null;
        mTimerAgent = null;
        mWifiInterface = null;
        mSystemInterface = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
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
    public void testGetBatteryLevel() {
        mDefaultSystemCallAgent.getBatteryLevel();

        verify(mBatteryStateInterface).getBatteryLevel();

        AgentFactory.getInstance().setAgent(BatteryStateInterface.class, null);

        assertEquals(BatteryStateInterface.INVALID_BATTERY_LEVEL,
                mDefaultSystemCallAgent.getBatteryLevel());
        verifyNoMoreInteractions(mBatteryStateInterface);
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
        SettingsProxy settingsProxy = mock(SettingsProxy.class);
        when(mTestAppContext.getContentProviderProxy().getGlobalSettings())
                .thenReturn(settingsProxy);
        when(settingsProxy.getString(eq(DEVICE_NAME), anyString())).thenReturn(testDeviceName);

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

    @Test
    @SmallTest
    public void testSetTrafficPriority() {
        mDefaultSystemCallAgent.setTrafficPriority(TEST_PRIORITY_TYPE, SLOT0);
        verify(mImsTrafficInterface)
                .setTrafficPriority(eq(TEST_PRIORITY_TYPE), eq(SLOT0));

        AgentFactory.getInstance().setAgent(ImsTrafficInterface.class, null);
        mDefaultSystemCallAgent.setTrafficPriority(TEST_PRIORITY_TYPE, SLOT0);
        verifyNoMoreInteractions(mImsTrafficInterface);
    }

    @Test
    @SmallTest
    public void testIsCrossSimRedialingAvailable() {
        when(mTelephonyManagerProxy.getActiveModemCount()).thenReturn(1);
        assertFalse(mDefaultSystemCallAgent.isCrossSimRedialingAvailable(SLOT0));

        when(mTelephonyManagerProxy.getActiveModemCount()).thenReturn(2);
        when(mTelephonyManagerProxy.getSimState(eq(SLOT1))).thenReturn(
                TelephonyManager.SIM_STATE_NOT_READY);
        assertFalse(mDefaultSystemCallAgent.isCrossSimRedialingAvailable(SLOT0));

        when(mTelephonyManagerProxy.getActiveModemCount()).thenReturn(2);
        when(mTelephonyManagerProxy.getSimState(eq(SLOT1))).thenReturn(
                TelephonyManager.SIM_STATE_READY);
        assertTrue(mDefaultSystemCallAgent.isCrossSimRedialingAvailable(SLOT0));

        when(mTelephonyManagerProxy.getActiveModemCount()).thenReturn(2);
        when(mTelephonyManagerProxy.getSimState(eq(SLOT1))).thenReturn(
                TelephonyManager.SIM_STATE_PRESENT);
        assertTrue(mDefaultSystemCallAgent.isCrossSimRedialingAvailable(SLOT0));
    }
}
