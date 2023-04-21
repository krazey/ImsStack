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

package com.android.imsstack.enabler.acs.impl;

import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.TimerInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * This class manages the timer for config retry. When provisioning request fails, this class set
 * timer. If the request failure is repeated the timer value increased exponentially.
 */
public class RetryManager {
    private static final class TimerObject {
        public long timerId = TimerInterface.INVALID_TID;
        public int message = 0;
        public Object userObj = null;

        TimerObject() {
        }

        void clear() {
            timerId = TimerInterface.INVALID_TID;
            message = 0;
            userObj = null;
        }
    }

    private final TimerInterface mTimer;
    private final int mSlotId;

    // default retry time value and count
    private long[] mRetryTimes = {60 * 20L, 60 * 60L, 60 * 60 * 2L, 60 * 60 * 4L, 60 * 60 * 8L};
    private int mMaxRetry = 5;
    private int mRetryCount = 0;
    private final TimerObject mRetryTimer = new TimerObject();
    // Provisioning validity timer
    private final TimerObject mValidityTimer = new TimerObject();
    private Handler mHandler;

    private final TimerInterface.Listener mTimerListener = new TimerInterface.Listener() {
        @Override
        public void onTimerExpired(long tid) {
            if (mHandler != null) {
                mHandler.post(() -> {
                    if (tid == mRetryTimer.timerId) {
                        Message.obtain(mHandler,
                                mRetryTimer.message, mRetryTimer.userObj).sendToTarget();
                    } else if (tid == mValidityTimer.timerId) {
                        Message.obtain(mHandler,
                                mValidityTimer.message, mValidityTimer.userObj).sendToTarget();
                    }
                });
            }
        }
    };

    @VisibleForTesting
    public RetryManager(int slotId, Handler handler, TimerInterface timer) {
        mSlotId = slotId;
        mHandler = handler;
        mTimer = timer;
    }

    /**
     * create instance
     * @param slotId SIM slot ID or Phone ID
     * @param handler Handler object
     */
    public RetryManager(int slotId, Handler handler) {
        this(slotId, handler, AgentFactory.getInstance().getAgent(TimerInterface.class));
    }

    /**
     * release all resource, stop all timer and timer expired event not transfer.
     */
    public void release() {
        mHandler = null;

        stopValidityTimer();
        stopRetryTimer();
    }

    /**
     * set retry timer values and retry count. If the value are
     *   times = {60, 60 * 10, 60 * 10 *6}
     *   retryCount = 5
     * the timer will be set 60, 60*10, 60*10*6, 60*10*6, 60*10*6.
     * @param times array include timer values
     * @param maxRetry number of retries iterations
     */
    public void setRetryTimer(long[] times, int maxRetry) {
        mRetryTimes = times.clone();
        mMaxRetry = maxRetry;
        mRetryCount = 0;
    }

    /**
     * start retry timer. If the times value is not set @link setRetryTimer(), the default value
     * will be used.
     * @param message message value want to receive for retry timer
     * @param obj object want to receive with message
     * @return true will be return if the operation is success, but reach the max retry count
     * false will be return
     */
    public boolean startRetryTimer(int message, Object obj) {
        if (mRetryTimer.timerId != TimerInterface.INVALID_TID) {
            mTimer.stopTimer(mRetryTimer.timerId);
        }

        long duration = mRetryTimes[mRetryTimes.length - 1];

        // get time value associated retry count
        if (mRetryCount < mMaxRetry) {
            if (mRetryCount < mRetryTimes.length) {
                duration = mRetryTimes[mRetryCount];
            }
        } else {
            return false;
        }

        mRetryCount++;
        mRetryTimer.timerId = mTimer.startTimer(duration, mTimerListener);

        ImsLog.d(mSlotId, "retry count : " + mRetryCount + " tid : " + mRetryTimer.timerId
                + " duration : " + duration);
        if (mRetryTimer.timerId == TimerInterface.INVALID_TID) {
            mRetryTimer.clear();
            mRetryCount = 0;
            return false;
        }

        mRetryTimer.message = message;
        mRetryTimer.userObj = obj;
        return true;
    }

    /**
     * stop retry timer.
     */
    public void stopRetryTimer() {
        if (mRetryTimer.timerId == TimerInterface.INVALID_TID) {
            return;
        }

        mTimer.stopTimer(mRetryTimer.timerId);
        mRetryTimer.clear();
        mRetryCount = 0;
    }

    /**
     * notified retry timer expired
     */
    public void expiredRetryTimer() {
        mRetryTimer.clear();
    }

    /**
     * start validity timer.
     * @param time timer should be ended
     * @param message message value want to receive for validity timer
     * @param obj object want to receive with message
     * @return true will be return if the operation is success, but reach the max retry count
     */
    public boolean startValidityTimer(long time, int message, Object obj) {
        if (mValidityTimer.timerId != TimerInterface.INVALID_TID) {
            mTimer.stopTimer(mValidityTimer.timerId);
        }

        long duration = time - getCurrentTimeMillis();
        mValidityTimer.timerId = mTimer.startTimer(duration, mTimerListener);

        ImsLog.d(mSlotId, " tid : " + mValidityTimer.timerId + " duration : " + duration);

        if (mValidityTimer.timerId == TimerInterface.INVALID_TID) {
            mValidityTimer.clear();
            return false;
        }

        mValidityTimer.message = message;
        mValidityTimer.userObj = obj;
        return true;
    }

    /**
     * stop validity timer.
     */
    public void stopValidityTimer() {
        if (mValidityTimer.timerId == TimerInterface.INVALID_TID) {
            return;
        }

        mTimer.stopTimer(mValidityTimer.timerId);
        mValidityTimer.clear();
    }

    /**
     * notified validity timer expired
     */
    public void expiredValidaityTimer() {
        mValidityTimer.clear();
    }

    /**
     * get retry timer Id
     */
    @VisibleForTesting
    public long getRetryTimerId() {
        return mRetryTimer.timerId;
    }

    /**
     * get validity timer Id
     */
    @VisibleForTesting
    public long getValidityTimerId() {
        return mValidityTimer.timerId;
    }

    protected long getCurrentTimeMillis() {
        return SystemClock.elapsedRealtime();
    }
}
