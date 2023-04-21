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
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.IntentFilter;
import android.os.Handler;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class TimerAgentTest {
    private static final int MAX_TIMER_ID = 3;
    private static final long NATIVE_TIMER_ID = 100L;
    private static final long SHORT_DURATION = 1000L; // 1 second
    private static final long LONG_DURATION = 10000L; // 1 second

    @Mock SystemInterface mSystemInterface;
    @Mock IWakeLock mWakeLock;
    @Mock TimerInterface.Listener mTimerListener;

    private AlarmManager mAlarmManager;
    private BroadcastReceiver mTimerBroadcastReceiver;
    private ContextFixture mContextFixture;
    private Context mContext;
    private TestableLooper mMainLooper;
    private TestableLooper mTimerLooper;
    private TimerAgent mTimerAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        mAlarmManager = mContext.getSystemService(AlarmManager.class);
        AppContext.init(mContext);
        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.setDefaultAgent(AgentFactory.WAKE_LOCK, mWakeLock);
        mMainLooper = new TestableLooper(AppContext.getInstance().getMainLooper());

        mTimerAgent = new TimerAgent();
        mTimerAgent.init(mContext);
        mTimerAgent.setMaxTimerId(MAX_TIMER_ID);
        mTimerLooper = new TestableLooper(mTimerAgent.getTimerLooper());

        ArgumentCaptor<BroadcastReceiver> captor = ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mContext).registerReceiver(captor.capture(), any(IntentFilter.class),
                eq(null), any(Handler.class), eq(Context.RECEIVER_EXPORTED));
        mTimerBroadcastReceiver = captor.getValue();
    }

    @After
    public void tearDown() throws Exception {
        if (mTimerAgent != null) {
            mTimerAgent.cleanup();
            mTimerAgent = null;
            verify(mContext).unregisterReceiver(any(BroadcastReceiver.class));
        }

        if (mMainLooper != null) {
            mMainLooper.destroy();
            mMainLooper = null;
        }

        if (mTimerLooper != null) {
            mTimerLooper.destroy();
            mTimerLooper = null;
        }

        AgentFactory.setDefaultAgent(AgentFactory.WAKE_LOCK, null);
        SystemInterface.setSystemInterface(null);
        mAlarmManager = null;
        mWakeLock = null;
        mSystemInterface = null;
        mTimerListener = null;
        mContextFixture = null;
        mContext = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testStartTimer() {
        long tid = mTimerAgent.startTimer(SHORT_DURATION, mTimerListener);
        assertEquals(1L, tid);

        tid = mTimerAgent.startTimer(SHORT_DURATION, mTimerListener);
        assertEquals(2L, tid);

        tid = mTimerAgent.startTimer(SHORT_DURATION, mTimerListener);
        assertEquals(TimerInterface.INVALID_TID, tid);
    }

    @Test
    @SmallTest
    public void testStartAndStopTimerWithShortDuration() {
        long tid = mTimerAgent.startTimer(SHORT_DURATION, mTimerListener);

        assertNotEquals(TimerInterface.INVALID_TID, tid);

        mTimerAgent.stopTimer(tid);

        verify(mAlarmManager, never()).setExactAndAllowWhileIdle(
                eq(AlarmManager.ELAPSED_REALTIME_WAKEUP), anyLong(), any(PendingIntent.class));
        verify(mAlarmManager, never()).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStartAndStopTimerWithLongDuration() {
        long tid = mTimerAgent.startTimer(LONG_DURATION, mTimerListener);

        assertNotEquals(TimerInterface.INVALID_TID, tid);

        mTimerAgent.stopTimer(tid);

        verify(mAlarmManager).setExactAndAllowWhileIdle(
                eq(AlarmManager.ELAPSED_REALTIME_WAKEUP), anyLong(), any(PendingIntent.class));
        verify(mAlarmManager).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStartTimerWhenAlarmManagerNull() {
        mContextFixture.setSystemService(Context.ALARM_SERVICE, null);
        long tid = mTimerAgent.startTimer(LONG_DURATION, mTimerListener);

        assertEquals(TimerInterface.INVALID_TID, tid);
    }

    @Test
    @SmallTest
    public void testStopTimerWithInvalidTid() {
        mTimerAgent.stopTimer(TimerInterface.INVALID_TID);

        verify(mAlarmManager, never()).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStopTimerWhenAlarmManagerNull() {
        long tid = mTimerAgent.startTimer(LONG_DURATION, mTimerListener);

        assertNotEquals(TimerInterface.INVALID_TID, tid);

        mContextFixture.setSystemService(Context.ALARM_SERVICE, null);
        mTimerAgent.stopTimer(tid);

        verify(mAlarmManager, never()).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStartAndStopNativeTimerWithShortDuration() {
        boolean result = mTimerAgent.startNativeTimer(NATIVE_TIMER_ID, SHORT_DURATION);

        assertTrue(result);

        mTimerAgent.stopNativeTimer(NATIVE_TIMER_ID);

        verify(mAlarmManager, never()).setExactAndAllowWhileIdle(
                eq(AlarmManager.ELAPSED_REALTIME_WAKEUP), anyLong(), any(PendingIntent.class));
        verify(mAlarmManager, never()).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStartAndStopNativeTimerWithLongDuration() {
        boolean result = mTimerAgent.startNativeTimer(NATIVE_TIMER_ID, LONG_DURATION);

        assertTrue(result);

        mTimerAgent.stopNativeTimer(NATIVE_TIMER_ID);

        verify(mAlarmManager).setExactAndAllowWhileIdle(
                eq(AlarmManager.ELAPSED_REALTIME_WAKEUP), anyLong(), any(PendingIntent.class));
        verify(mAlarmManager).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStartNativeTimerWithInvalidTid() {
        boolean result = mTimerAgent.startNativeTimer(0, SHORT_DURATION);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void testStartNativeTimerWhenAlarmManagerNull() {
        mContextFixture.setSystemService(Context.ALARM_SERVICE, null);
        boolean result = mTimerAgent.startNativeTimer(NATIVE_TIMER_ID, LONG_DURATION);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void testStopNativeTimerWithInvalidTid() {
        mTimerAgent.stopNativeTimer(TimerInterface.INVALID_TID);

        verify(mAlarmManager, never()).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testStopNativeTimerWhenAlarmManagerNull() {
        boolean result = mTimerAgent.startNativeTimer(NATIVE_TIMER_ID, LONG_DURATION);

        assertTrue(result);

        mContextFixture.setSystemService(Context.ALARM_SERVICE, null);
        mTimerAgent.stopNativeTimer(NATIVE_TIMER_ID);

        verify(mAlarmManager, never()).cancel(any(PendingIntent.class));
    }

    @Test
    @SmallTest
    public void testNotifyTimerExpiredForShortDuration() {
        long tid = mTimerAgent.startTimer(SHORT_DURATION, mTimerListener);
        processAllMessagesForTimerLooper(SHORT_DURATION + 10);
        processAllMessages();

        verify(mWakeLock).acquire(anyInt());
        verify(mTimerListener).onTimerExpired(eq(tid));
    }

    @Test
    @SmallTest
    public void testNotifyTimerExpiredForLongDuration() {
        long tid = mTimerAgent.startTimer(LONG_DURATION, mTimerListener);
        mTimerBroadcastReceiver.onReceive(mContext, TimerAgent.createIntent(tid, false, false));
        processAllMessages();

        verify(mWakeLock).acquire(anyInt());
        verify(mTimerListener).onTimerExpired(eq(tid));
    }

    @Test
    @SmallTest
    public void testNotifyNativeTimerExpiredForShortDuration() {
        boolean result = mTimerAgent.startNativeTimer(NATIVE_TIMER_ID, SHORT_DURATION);
        processAllMessagesForTimerLooper(SHORT_DURATION + 10);

        assertTrue(result);
        verify(mWakeLock).acquire(anyInt());
        verify(mSystemInterface).notifyTimerExpired(eq(NATIVE_TIMER_ID));
    }

    @Test
    @SmallTest
    public void testNotifyNativeTimerExpiredForLongDuration() {
        boolean result = mTimerAgent.startNativeTimer(NATIVE_TIMER_ID, LONG_DURATION);
        mTimerBroadcastReceiver.onReceive(mContext,
                TimerAgent.createIntent(NATIVE_TIMER_ID, true, false));

        assertTrue(result);
        verify(mWakeLock).acquire(anyInt());
        verify(mSystemInterface).notifyTimerExpired(eq(NATIVE_TIMER_ID));
    }

    private void processAllMessages() {
        while (!mMainLooper.getLooper().getQueue().isIdle()) {
            mMainLooper.processAllMessages();
        }
    }

    private void processAllMessagesForTimerLooper(long moveTimeMillis) {
        if (moveTimeMillis > 0) {
            mTimerLooper.moveTimeForward(moveTimeMillis);
        }
        while (!mTimerLooper.getLooper().getQueue().isIdle()) {
            mTimerLooper.processAllMessages();
        }
    }
}
