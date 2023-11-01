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
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Handler;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.system.SystemInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class BatteryStateAgentTest {
    private static final int SLOT0 = 0;
    private static final int LOW_BATTERY_WARNING_LEVEL = 20;
    private static final int LOW_BATTERY_LEVEL = 10;
    private static final int NORMAL_BATTERY_LEVEL = 70;
    private static final long TIMER_ID = 100L;

    @Mock private SystemInterface mSystemInterface;
    @Mock private TimerInterface mTimerInterface;

    private Context mContext;
    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private BatteryStateAgent mBatteryStateAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        when(mContext.getResources().getInteger(
                eq(com.android.internal.R.integer.config_lowBatteryWarningLevel)))
                .thenReturn(LOW_BATTERY_WARNING_LEVEL);
        AppContext.init(mContext);
        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.getInstance().setAgent(TimerInterface.class, mTimerInterface);
        when(mTimerInterface.startTimer(anyLong(), any(TimerInterface.Listener.class)))
                .thenReturn(TIMER_ID);

        mTestableLooper = new TestableLooper(AppContext.getInstance().getMainLooper());
        mBatteryStateAgent = new BatteryStateAgent();
    }

    @After
    public void tearDown() throws Exception {
        if (mBatteryStateAgent != null) {
            mBatteryStateAgent.cleanup();
            mBatteryStateAgent = null;
        }

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        AgentFactory.getInstance().setAgent(TimerInterface.class, null);
        SystemInterface.setSystemInterface(null);
        mContextFixture = null;
        mContext = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testInitWhenBatteryChargingAndPollingTimerStarts() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_CHARGING, 0, NORMAL_BATTERY_LEVEL);
        mBatteryStateAgent.init(mContext);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryStateReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
        verify(mTimerInterface).startTimer(anyLong(), any(TimerInterface.Listener.class));

        mBatteryStateAgent.cleanup();
        mBatteryStateAgent = null;

        verify(mContext).unregisterReceiver(any(BroadcastReceiver.class));
        verify(mTimerInterface).stopTimer(eq(TIMER_ID));
    }

    @Test
    @SmallTest
    public void testInitWhenBatteryChargingAndBatteryLevelLow() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_CHARGING,
                BatteryManager.BATTERY_PLUGGED_USB, LOW_BATTERY_LEVEL);
        mBatteryStateAgent.init(mContext);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryStateReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryChangedReceiver(filter)),
                eq(null), any(Handler.class), anyInt());

        mBatteryStateAgent.cleanup();
        mBatteryStateAgent = null;

        verify(mContext, times(2)).unregisterReceiver(any(BroadcastReceiver.class));
    }

    @Test
    @SmallTest
    public void testInitWhenBatteryNotChargingAndBatteryLevelLow() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0, LOW_BATTERY_LEVEL);
        mBatteryStateAgent.init(mContext);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryStateReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryChangedReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
    }

    @Test
    @SmallTest
    public void testInitWhenBatteryNotChargingAndBatteryLevelLowAndImmediatelyCleanup() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0, LOW_BATTERY_LEVEL);
        mBatteryStateAgent.init(mContext);
        mBatteryStateAgent.cleanup();
        mBatteryStateAgent = null;
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryStateReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
        verify(mContext, never()).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryChangedReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
    }

    @Test
    @SmallTest
    public void testCheckBatteryState() {
        // When sticky intent is null
        assertFalse(mBatteryStateAgent.isPowerPlugged());
        assertTrue(mBatteryStateAgent.isLowBattery());
        assertEquals(BatteryStateInterface.INVALID_BATTERY_LEVEL,
                mBatteryStateAgent.getBatteryLevel());

        // Not charging & low battery level
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0,
                BatteryStateAgent.LOW_BATTERY_WARNING_LEVEL_FOR_CALL);

        assertFalse(mBatteryStateAgent.isPowerPlugged());
        assertTrue(mBatteryStateAgent.isLowBattery());
        assertEquals(BatteryStateAgent.LOW_BATTERY_WARNING_LEVEL_FOR_CALL,
                mBatteryStateAgent.getBatteryLevel());

        // USB charging & normal battery level
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_CHARGING,
                BatteryManager.BATTERY_PLUGGED_USB, NORMAL_BATTERY_LEVEL);

        assertTrue(mBatteryStateAgent.isPowerPlugged());
        assertFalse(mBatteryStateAgent.isLowBattery());
        assertEquals(NORMAL_BATTERY_LEVEL, mBatteryStateAgent.getBatteryLevel());
    }

    @Test
    @SmallTest
    public void testNotifyLowBatteryState() {
        // Not charging & low battery level
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0,
                BatteryStateAgent.LOW_BATTERY_WARNING_LEVEL_FOR_CALL);

        mBatteryStateAgent.notifyLowBatteryState(SLOT0);

        verify(mSystemInterface).notifyLowBatteryState(eq(SLOT0));

        // Not charging & normal battery level
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0, NORMAL_BATTERY_LEVEL);

        mBatteryStateAgent.notifyLowBatteryState(SLOT0);

        verifyNoMoreInteractions(mSystemInterface);
    }

    @Test
    @SmallTest
    public void testHandleBatteryChanged() {
        Intent intent = setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0,
                BatteryStateAgent.LOW_BATTERY_WARNING_LEVEL_FOR_CALL);
        mBatteryStateAgent.init(mContext);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);
        mContext.sendBroadcast(intent);

        verify(mSystemInterface).notifyBatteryLevelChanged(
                eq(BatteryStateAgent.LOW_BATTERY_WARNING_LEVEL_FOR_CALL));
        verify(mSystemInterface).notifyLowBatteryState();

        mContext.sendBroadcast(createIntentForBatteryChanged(
                BatteryManager.BATTERY_STATUS_CHARGING, 0, NORMAL_BATTERY_LEVEL));

        verify(mSystemInterface).notifyBatteryLevelChanged(eq(NORMAL_BATTERY_LEVEL));
        verify(mSystemInterface).notifyLowBatteryStateChanged();
        verify(mTimerInterface).startTimer(anyLong(), any(TimerInterface.Listener.class));
    }

    @Test
    @SmallTest
    public void testHandleBatteryLow() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0,
                LOW_BATTERY_WARNING_LEVEL + 1);
        mBatteryStateAgent.init(mContext);

        Intent intent = new Intent(Intent.ACTION_BATTERY_LOW);
        // registerReceiver should be called once even if the event happens twice.
        mContext.sendBroadcast(intent);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);
        mContext.sendBroadcast(intent);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryChangedReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
    }

    @Test
    @SmallTest
    public void testHandleBatteryPowerConnected() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0,
                LOW_BATTERY_WARNING_LEVEL + 1);
        mBatteryStateAgent.init(mContext);

        Intent intent = new Intent(Intent.ACTION_POWER_CONNECTED);
        // Timer should be started once even if the event happens twice.
        mContext.sendBroadcast(intent);
        mContext.sendBroadcast(intent);

        verify(mTimerInterface).startTimer(anyLong(), any(TimerInterface.Listener.class));

        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0, LOW_BATTERY_LEVEL);
        mContext.sendBroadcast(intent);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryChangedReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
    }

    @Test
    @SmallTest
    public void testHandleBatteryPowerDisconnected() {
        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0,
                LOW_BATTERY_WARNING_LEVEL + 1);
        mBatteryStateAgent.init(mContext);

        Intent intent = new Intent(Intent.ACTION_POWER_DISCONNECTED);
        // Timer should be started twice if the event happens twice.
        mContext.sendBroadcast(intent);
        mContext.sendBroadcast(intent);

        verify(mTimerInterface, times(2)).startTimer(anyLong(), any(TimerInterface.Listener.class));
        verify(mTimerInterface).stopTimer(eq(TIMER_ID));

        setUpBatteryStates(BatteryManager.BATTERY_STATUS_DISCHARGING, 0, LOW_BATTERY_LEVEL);
        mContext.sendBroadcast(intent);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);

        verify(mContext).registerReceiver(any(BroadcastReceiver.class),
                argThat(filter -> matchBatteryChangedReceiver(filter)),
                eq(null), any(Handler.class), anyInt());
    }

    @Test
    @SmallTest
    public void testPollingTimerExpired() {
        Intent intent = setUpBatteryStates(BatteryManager.BATTERY_STATUS_CHARGING,
                BatteryManager.BATTERY_PLUGGED_USB, NORMAL_BATTERY_LEVEL + 1);
        mBatteryStateAgent.init(mContext);
        processAllMessages(BatteryStateAgent.DELAY_INSTALL_BATTERY_CHANGED_RECEIVER + 1);
        mContext.sendBroadcast(intent);

        ArgumentCaptor<TimerInterface.Listener> captor =
                ArgumentCaptor.forClass(TimerInterface.Listener.class);
        verify(mTimerInterface).startTimer(anyLong(), captor.capture());

        TimerInterface.Listener listener = captor.getValue();
        assertNotNull(listener);

        when(mTimerInterface.startTimer(anyLong(), any(TimerInterface.Listener.class)))
                .thenReturn(TIMER_ID + 1);
        // Same timer id
        listener.onTimerExpired(TIMER_ID);

        verify(mTimerInterface, times(2)).startTimer(anyLong(), any(TimerInterface.Listener.class));

        // Different timer id.
        listener.onTimerExpired(TIMER_ID);

        verifyNoMoreInteractions(mTimerInterface);
    }

    private Intent setUpBatteryStates(int status, int plugged, int level) {
        Intent intent = createIntentForBatteryChanged(status, plugged, level);
        mContext.sendStickyBroadcast(intent);
        return intent;
    }

    private static Intent createIntentForBatteryChanged(int status, int plugged, int level) {
        Intent intent = new Intent(Intent.ACTION_BATTERY_CHANGED);
        intent.putExtra(BatteryManager.EXTRA_STATUS, status);
        intent.putExtra(BatteryManager.EXTRA_PLUGGED, plugged);
        intent.putExtra(BatteryManager.EXTRA_LEVEL, level);
        return intent;
    }

    private void processAllMessages(long moveTimeMillis) {
        if (moveTimeMillis > 0) {
            mTestableLooper.moveTimeForward(moveTimeMillis);
        }
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }

    private static boolean matchBatteryChangedReceiver(IntentFilter filter) {
        if (filter != null) {
            return filter.countActions() == 1
                    && filter.hasAction(Intent.ACTION_BATTERY_CHANGED);
        }
        return false;
    }

    private static boolean matchBatteryStateReceiver(IntentFilter filter) {
        if (filter != null) {
            return filter.countActions() == 3
                    && filter.hasAction(Intent.ACTION_BATTERY_LOW)
                    && filter.hasAction(Intent.ACTION_POWER_CONNECTED)
                    && filter.hasAction(Intent.ACTION_POWER_DISCONNECTED);
        }
        return false;
    }
}
