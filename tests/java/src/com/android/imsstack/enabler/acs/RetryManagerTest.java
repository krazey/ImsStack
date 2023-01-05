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

package com.android.imsstack.enabler.acs;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.core.agents.IAlarmTimer;
import com.android.imsstack.enabler.acs.impl.RetryManager;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

public class RetryManagerTest {
    private static final int MESSAGE_RETRY = 12345;
    private static final int MESSAGE_VALIDITY = 67890;
    private static final int SLOT_ID = 0;

    private static class TestRetryManager extends RetryManager {
        private long mCurrentTimeMillis = 0;

        TestRetryManager(int slotId, Handler handler, IAlarmTimer iAlarmTimer) {
            super(slotId, handler, iAlarmTimer);
        }

        public void setCurrentTimeMillis(long time) {
            mCurrentTimeMillis = time;
        }

        protected long getCurrentTimeMillis() {
            return mCurrentTimeMillis;
        }
    }

    @Mock IAlarmTimer mIAlarmTimer;
    @Mock Handler mHandler;
    private int mTimerId;

    private TestRetryManager mRetryManager;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        when(mIAlarmTimer.startTimer(anyLong(), anyLong())).thenReturn(true);
        doNothing().when(mIAlarmTimer).stopTimer(anyLong());
        mTimerId = 1;
        when(mIAlarmTimer.getTimerId()).thenReturn(mTimerId++);

        mRetryManager = new TestRetryManager(SLOT_ID, mHandler, mIAlarmTimer);
    }

    @After
    public void tearDown() throws Exception {
        mRetryManager.release();
        mRetryManager = null;
    }

    @Test
    @SmallTest
    public void startStopRetryTimer() throws Exception {
        // using default time value
        boolean retValue = mRetryManager.startRetryTimer(MESSAGE_RETRY, null);
        assertTrue(retValue);

        long retryTimerId = mRetryManager.getRetryTimerId();

        // verify to start new alarm
        verify(mIAlarmTimer, times(1)).startTimer(eq(retryTimerId), eq(60 * 20L));
        // verify to start new alarm
        verify(mIAlarmTimer, never()).stopTimer(eq(retryTimerId));

        clearInvocations(mIAlarmTimer);

        mRetryManager.stopRetryTimer();

        // verify to stop alarm
        verify(mIAlarmTimer, times(1)).stopTimer(eq(retryTimerId));
        clearInvocations(mIAlarmTimer);

        // after timer expired, verify never try to stop alarm
        mRetryManager.stopRetryTimer();
        verify(mIAlarmTimer, never()).stopTimer(anyLong());
    }

    @Test
    @SmallTest
    public void startRetryTimerAgain_withoutCallingExpired() throws Exception {
        assertTrue(mRetryManager.startRetryTimer(MESSAGE_RETRY, null));
        long retryTimerId = mRetryManager.getRetryTimerId();
        verify(mIAlarmTimer, times(1)).startTimer(eq(retryTimerId), eq(60 * 20L));
        clearInvocations(mIAlarmTimer);

        assertTrue(mRetryManager.startRetryTimer(MESSAGE_RETRY, null));
        // verify to stop exist alarm first
        verify(mIAlarmTimer, times(1)).stopTimer(eq(retryTimerId));
        retryTimerId = mRetryManager.getRetryTimerId();
        // verify to start new alarm
        verify(mIAlarmTimer, times(1)).startTimer(eq(retryTimerId), eq(60 * 60L));
    }

    @Test
    @SmallTest
    public void retryTimer_oneTime() throws Exception {
        int maxRetry = 1;
        long[] times = {100L};

        mRetryManager.setRetryTimer(times, maxRetry);

        boolean retValue = mRetryManager.startRetryTimer(MESSAGE_RETRY, null);
        assertTrue(retValue);

        long retryTimerId = mRetryManager.getRetryTimerId();

        // verify to start new alarm
        verify(mIAlarmTimer, times(1)).startTimer(eq(retryTimerId), eq(times[0]));
        // verify to start new alarm
        verify(mIAlarmTimer, never()).stopTimer(eq(retryTimerId));
        clearInvocations(mIAlarmTimer);

        mRetryManager.expiredRetryTimer();

        // exceed max retry count
        assertFalse(mRetryManager.startRetryTimer(MESSAGE_RETRY, null));
    }

    @Test
    @SmallTest
    public void retryTimer_repeat() throws Exception {
        int maxRetry = 7;
        long[] times = {100L, 200L, 300L, 400L, 500L};

        mRetryManager.setRetryTimer(times, maxRetry);

        boolean retValue;
        long retryTimerId;
        int index = 0;
        for (int i = 0; i < maxRetry; i++) {
            retValue = mRetryManager.startRetryTimer(MESSAGE_RETRY, null);
            assertTrue(retValue);

            retryTimerId = mRetryManager.getRetryTimerId();

            if (i < times.length) {
                index = i;
            }

            // verify to start new alarm
            verify(mIAlarmTimer, times(1)).startTimer(eq(retryTimerId), eq(times[index]));
            clearInvocations(mIAlarmTimer);

            mRetryManager.expiredRetryTimer();
        }

        // exceed max retry count
        retValue = mRetryManager.startRetryTimer(MESSAGE_RETRY, null);
        assertFalse(retValue);
    }

    @Test
    @SmallTest
    public void startStopValidityTimer() throws Exception {
        long currentTimeMillis = System.currentTimeMillis();
        long validityTime = 1234567L;
        mRetryManager.setCurrentTimeMillis(currentTimeMillis);

        boolean retValue = mRetryManager.startValidityTimer(validityTime + currentTimeMillis,
                MESSAGE_VALIDITY, null);
        assertTrue(retValue);

        long validityTimerId = mRetryManager.getValidityTimerId();

        // verify to start new alarm
        verify(mIAlarmTimer, times(1)).startTimer(eq(validityTimerId), eq(validityTime));
        verify(mIAlarmTimer, never()).stopTimer(anyLong());
        clearInvocations(mIAlarmTimer);

        mRetryManager.stopValidityTimer();

        // verify to stop alarm
        verify(mIAlarmTimer, times(1)).stopTimer(eq(validityTimerId));
        clearInvocations(mIAlarmTimer);

        // after timer expired, verify never try to stop alarm
        mRetryManager.stopValidityTimer();
        verify(mIAlarmTimer, never()).stopTimer(anyLong());
    }

    @Test
    @SmallTest
    public void startValidityTimer_withoutCaliingExpired() throws Exception {
        long currentTimeMillis = System.currentTimeMillis();
        long validityTime = 1234567L;
        mRetryManager.setCurrentTimeMillis(currentTimeMillis);

        // call startTimer 2 times without calling stopTimer
        assertTrue(mRetryManager.startValidityTimer(validityTime + currentTimeMillis,
                MESSAGE_VALIDITY, null));
        long validityTimerId = mRetryManager.getValidityTimerId();
        verify(mIAlarmTimer, times(1)).startTimer(eq(validityTimerId), eq(validityTime));
        clearInvocations(mIAlarmTimer);

        validityTime = 78912345L;
        assertTrue(mRetryManager.startValidityTimer(validityTime + currentTimeMillis,
                MESSAGE_VALIDITY, null));

        // verify to stop exist alarm first
        verify(mIAlarmTimer, times(1)).stopTimer(eq(validityTimerId));
        validityTimerId = mRetryManager.getValidityTimerId();
        // verify to start new alarm
        verify(mIAlarmTimer, times(1)).startTimer(eq(validityTimerId), eq(validityTime));
    }
}
