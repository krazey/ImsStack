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
import android.os.SystemClock;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IAlarmTimer;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * This class manages the timer for config retry. When provisioning request fails, this class set
 * timer. If the request failure is repeated the timer value increased exponentially.
 */
public class ReconfigManager {
    private final IAlarmTimer mIAlarmTimer;
    private final int mSlotId;

    // default retry time value and count
    private long[] mRetryTimes = {60 * 20L, 60 * 60L, 60 * 60 * 2L, 60 * 60 * 4L, 60 * 60 * 8L};
    private int mMaxRetry = 5;
    private int mRetryCount = 0;
    private int mRetryTimerId = 0;

    // Provisioning validity timer
    private int mValidityTimerId = 0;

    private Handler mHandler;

    @VisibleForTesting
    public ReconfigManager(int slotId, Handler handler,
            IAlarmTimer iAlarmTimer) {
        mSlotId = slotId;
        mHandler = handler;
        mIAlarmTimer = iAlarmTimer;
    }

    /**
     * create instance
     * @param context Context
     * @param slotId SIM slot ID or Phone ID
     * @param handler Handler object
     */
    public ReconfigManager(int slotId, Handler handler) {
        this(slotId, handler,
                (IAlarmTimer) AgentFactory.getAgent(AgentFactory.ALARM_TIMER, slotId));
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
        if (mRetryTimerId != 0) {
            mIAlarmTimer.stopTimer(mRetryTimerId);
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

        mRetryTimerId = mIAlarmTimer.getTimerId();
        mIAlarmTimer.registerForTimerExpired(mRetryTimerId, mHandler, message, obj);
        boolean retValue = mIAlarmTimer.startTimer(mRetryTimerId, duration);

        ImsLog.d(mSlotId, "retry count : " + mRetryCount + " tid : " + mRetryTimerId
                + " duration : " + duration + " return : " + retValue);
        if (!retValue) {
            mIAlarmTimer.unregisterForTimerExpired(mRetryTimerId, mHandler);
            mRetryTimerId = 0;
            mRetryCount = 0;
        }

        return retValue;
    }

    /**
     * stop retry timer.
     */
    public void stopRetryTimer() {
        if (mRetryTimerId == 0) {
            return;
        }

        mIAlarmTimer.stopTimer(mRetryTimerId);
        mRetryTimerId = 0;
        mRetryCount = 0;
    }

    /**
     * notified retry timer expired
     */
    public void expiredRetryTimer() {
        mRetryTimerId = 0;
    }

    /**
     * start validity timer.
     * @param time timer should be ended
     * @param message message value want to receive for validity timer
     * @param obj object want to receive with message
     * @return true will be return if the operation is success, but reach the max retry count
     */
    public boolean startValidityTimer(long time, int message, Object obj) {
        if (mValidityTimerId != 0) {
            mIAlarmTimer.stopTimer(mValidityTimerId);
        }

        long duration = time - getCurrentTimeMillis();

        mValidityTimerId = mIAlarmTimer.getTimerId();
        mIAlarmTimer.registerForTimerExpired(mRetryTimerId, mHandler, message, obj);
        boolean retValue = mIAlarmTimer.startTimer(mValidityTimerId, duration);

        ImsLog.d(mSlotId, " tid : " + mValidityTimerId
                + " duration : " + duration + " return : " + retValue);

        if (!retValue) {
            mIAlarmTimer.unregisterForTimerExpired(mValidityTimerId, mHandler);
            mValidityTimerId = 0;
        }

        return retValue;
    }

    /**
     * stop validity timer.
     */
    public void stopValidityTimer() {
        if (mValidityTimerId == 0) {
            return;
        }

        mIAlarmTimer.stopTimer(mValidityTimerId);
        mValidityTimerId = 0;
    }

    /**
     * notified validity timer expired
     */
    public void expiredValidaityTimer() {
        mValidityTimerId = 0;
    }

    /**
     * get retry timer Id
     */
    @VisibleForTesting
    public long getRetryTimerId() {
        return mRetryTimerId;
    }

    /**
     * get validity timer Id
     */
    @VisibleForTesting
    public long getValidityTimerId() {
        return mValidityTimerId;
    }

    protected long getCurrentTimeMillis() {
        return SystemClock.elapsedRealtime();
    }
}
