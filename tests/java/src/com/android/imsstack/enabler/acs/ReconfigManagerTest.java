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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.enabler.acs.impl.EventReceiver;
import com.android.imsstack.enabler.acs.impl.ReconfigManager;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.invocation.InvocationOnMock;
import org.mockito.stubbing.Answer;

public class ReconfigManagerTest {

    private static class TestReconfigManager extends ReconfigManager {
        private long mCurrentTimeMillis = 0;

        TestReconfigManager(Context context, int slotId, int subId, Handler handler,
                EventReceiver eventReceiver, AlarmManager alarmManager) {
            super(context, slotId, subId, handler, eventReceiver, alarmManager);
        }

        public void setCurrentTimeMillis(long time) {
            mCurrentTimeMillis = time;
        }

        protected long getCurrentTimeMillis() {
            return mCurrentTimeMillis;
        }

        protected PendingIntent getPendingIntent(Context context, int requestCode,
                Intent intent, int flags) {
            return null;
        }
    }

    private final class TestMessageHandler extends Handler {
        private int mReceivedMessage = 0;
        private int mMessageCount = 0;

        TestMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            mReceivedMessage = msg.what;
            mMessageCount++;
        }

        private int getReceivedMessage() {
            return mReceivedMessage;
        }

        private int getMessageCount() {
            return mMessageCount;
        }

        private void resetMessageAndCount() {
            mReceivedMessage = 0;
            mMessageCount = 0;
        }
    }

    @Mock Context mContext;
    @Mock EventReceiver mEventReceiver;
    @Mock AlarmManager mAlarmManager;

    private int mSlotId0 = 0;
    private int mSlotId1 = 0;
    private int mSubId0 = 1234;
    private int mSubId1 = 5678;

    private TestMessageHandler mHandler;
    private HandlerThread mHandlerThread;
    private TestableLooper mLooper;

    private TestReconfigManager mReconfigManager;
    private EventReceiver.EventReceiverCallback mEventReceiverCallback;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doAnswer(new Answer() {
            @Override
            public Object answer(InvocationOnMock invocation) throws Throwable {
                mEventReceiverCallback =
                        (EventReceiver.EventReceiverCallback) invocation.getArguments()[0];
                return null;
            }
        }).when(mEventReceiver).registerCallback(any(EventReceiver.EventReceiverCallback.class));

        mHandlerThread = new HandlerThread(ReconfigManagerTest.class.getSimpleName());
        mHandlerThread.start();
        mHandler = new TestMessageHandler(mHandlerThread.getLooper());
        mLooper = new TestableLooper(mHandler.getLooper());
        mReconfigManager = new TestReconfigManager(mContext, mSlotId0, mSubId0, mHandler,
                mEventReceiver, mAlarmManager);
    }

    @After
    public void tearDown() throws Exception {
        mReconfigManager = null;
        if (mLooper != null) {
            mLooper.destroy();
            mLooper = null;
        }
        mHandlerThread = null;
        mHandler = null;
    }

    @Test
    @SmallTest
    public void startStopRetryTimer_withExpireIntent() throws Exception {
        int message = 123;
        long currentTimeMillis = System.currentTimeMillis();
        mReconfigManager.setCurrentTimeMillis(currentTimeMillis);

        mReconfigManager.startRetryTimer(message);

        // verify to stop exist alarm
        verify(mAlarmManager, times(1)).cancel((PendingIntent) any());
        // verify to start new alarm
        verify(mAlarmManager, times(1)).setExact(anyInt(),
                eq(currentTimeMillis + 60 * 20L), any());
        clearInvocations(mAlarmManager);

        // call the callback
        mEventReceiverCallback.onReceivedIntent(getIntent(ReconfigManager.TIMER_ID_RETRY, mSubId0));
        processAllMessages();

        assertEquals(message, mHandler.getReceivedMessage());
        assertTrue(mHandler.getMessageCount() == 1);
        mHandler.resetMessageAndCount();

        mReconfigManager.stopRetryTimer();

        // verify to stop alarm
        verify(mAlarmManager, times(1)).cancel((PendingIntent) any());

        verifyNoMoreInteractions(mAlarmManager);
    }

    @Test
    @SmallTest
    public void retryTimer_oneTime() throws Exception {
        int maxRetry = 1;
        int message = 1234;
        long[] times = {100L};
        long currentTimeMillis = System.currentTimeMillis();
        mReconfigManager.setCurrentTimeMillis(currentTimeMillis);

        mReconfigManager.setRetryTimer(times, maxRetry);

        mReconfigManager.startRetryTimer(message);

        // verify to stop exist alarm
        verify(mAlarmManager, times(1)).cancel((PendingIntent) any());

        // verify to start new alarm
        verify(mAlarmManager, times(1)).setExact(anyInt(),
                eq(currentTimeMillis + times[0]), any());
        clearInvocations(mAlarmManager);

        // exceed max retry count
        assertFalse(mReconfigManager.startRetryTimer(message));
        verifyNoMoreInteractions(mAlarmManager);
    }

    @Test
    @SmallTest
    public void retryTimer_repeat() throws Exception {
        int maxRetry = 7;
        int message = 1234;
        long[] times = {100L, 200L, 300L, 400L, 500L};
        long currentTimeMillis = System.currentTimeMillis();
        mReconfigManager.setCurrentTimeMillis(currentTimeMillis);

        mReconfigManager.setRetryTimer(times, maxRetry);

        int index = 0;
        for (int i = 0; i < maxRetry; i++) {
            mReconfigManager.startRetryTimer(message);

            // verify to stop exist alarm
            verify(mAlarmManager, times(1)).cancel((PendingIntent) any());
            if (i < times.length) {
                index = i;
            }

            // verify to start new alarm
            verify(mAlarmManager, times(1)).setExact(anyInt(),
                    eq(currentTimeMillis + times[index]), any());
            clearInvocations(mAlarmManager);

            // notify timer expired
        }

        // exceed max retry count
        assertFalse(mReconfigManager.startRetryTimer(message));
        verifyNoMoreInteractions(mAlarmManager);
    }

    @Test
    @SmallTest
    public void startStopValidityTimer_withExpireIntent() throws Exception {
        long currentTimeMillis = System.currentTimeMillis();
        long validityTime = 1234567L;
        int message = 456;
        mReconfigManager.setCurrentTimeMillis(currentTimeMillis);

        mReconfigManager.startValidityTimer(validityTime, message);

        // verify to stop exist alarm
        verify(mAlarmManager, times(1)).cancel((PendingIntent) any());
        // verify to start new alarm
        verify(mAlarmManager, times(1)).setExact(anyInt(),
                eq(currentTimeMillis + validityTime), any());
        clearInvocations(mAlarmManager);

        // call the callback
        mEventReceiverCallback.onReceivedIntent(getIntent(
                ReconfigManager.TIMER_ID_VALIDITY, mSubId0));
        processAllMessages();

        assertEquals(message, mHandler.getReceivedMessage());
        assertTrue(mHandler.getMessageCount() == 1);
        mHandler.resetMessageAndCount();

        mReconfigManager.stopValidityTimer();

        // verify to stop alarm
        verify(mAlarmManager, times(1)).cancel((PendingIntent) any());

        verifyNoMoreInteractions(mAlarmManager);
    }

    private Intent getIntent(int timerId, int subId) {
        Intent intent = new Intent(ReconfigManager.INTENT_ACTION);
        intent.putExtra(ReconfigManager.ALARM_ID, timerId);
        intent.putExtra(ReconfigManager.SUB_ID, subId);
        return intent;
    }

    private void processAllMessages() {
        while (!mLooper.getLooper().getQueue().isIdle()) {
            mLooper.processAllMessages();
        }
    }
}
