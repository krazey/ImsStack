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

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * This class manages the timer for config retry. When provisioning request fails, this class set
 * timer. If the request failure is repeated the timer value increased exponentially.
 */
public class ReconfigManager {
    public static final String INTENT_ACTION = "com.android.imsstack.enabler.acs.ALARM";

    @VisibleForTesting
    public static final String ALARM_ID = "com.android.imsstack.enabler.acs.ALARM";
    @VisibleForTesting
    public static final String SUB_ID = "com.android.imsstack.enabler.acs.SUBID";
    @VisibleForTesting
    public static final int TIMER_ID_RETRY = 10246;
    @VisibleForTesting
    public static final int TIMER_ID_VALIDITY = 91306;


    private final EventReceiver.EventReceiverCallback mEventReceiverCallback =
            new EventReceiver.EventReceiverCallback() {
                @Override
                public void onReceivedIntent(Intent intent) {
                    if (intent == null) {
                        ImsLog.i(mSlotId, "parameter is not valid");
                        return;
                    }

                    int subId = intent.getIntExtra(SUB_ID, 0);
                    if (subId != mSubId) {
                        ImsLog.d(mSlotId, "it is not for me");
                        return;
                    }

                    int timerId = intent.getIntExtra(ALARM_ID, 0);
                    switch (timerId) {
                        case TIMER_ID_RETRY:
                            sendEmptyMessage(mRetryExpiredMessage);
                            break;
                        case TIMER_ID_VALIDITY:
                            sendEmptyMessage(mValidityExpiredMessage);
                            break;
                        default:
                            ImsLog.i(mSlotId, "unknown timer id " + timerId);
                            break;
                    }
                }

                @Override
                public void onSubscriptionChanged(Intent intent) {
                    // ignore
                }
            };
    private final Context mContext;
    private final EventReceiver mEventReceiver;
    private final AlarmManager mAlarmManager;
    private final int mSlotId;
    private final int mSubId;

    // default retry time value and count
    private long[] mRetryTimes = {60 * 20L, 60 * 60L, 60 * 60 * 2L, 60 * 60 * 4L, 60 * 60 * 8L};
    private int mMaxRetry = 5;
    private int mRetryCount = 0;

    // Provisioning validity timer
    private int mValidityTimer = 0;

    private Handler mHandler;
    private int mRetryExpiredMessage;
    private int mValidityExpiredMessage;

    @VisibleForTesting
    public ReconfigManager(Context context, int slotId, int subId, Handler handler,
            EventReceiver eventReceiver,
            AlarmManager alarmManager) {
        mContext = context;
        mSlotId = slotId;
        mSubId = subId;
        mHandler = handler;
        mAlarmManager = alarmManager;
        mEventReceiver = eventReceiver;
        mEventReceiver.registerCallback(mEventReceiverCallback);
    }

    /**
     * create instance
     * @param context Context
     * @param slotId SIM slot ID or Phone ID
     * @param handler Handler object
     */
    public ReconfigManager(Context context, int slotId, int subId, Handler handler) {
        this(context, slotId, subId, handler,
                EventReceiver.getInstance(context),
                (AlarmManager) context.getSystemService(Context.ALARM_SERVICE));
    }

    /**
     * release all resource, stop all timer and timer expired event not transfer.
     */
    public void release() {
        mEventReceiver.unregisterCallback(mEventReceiverCallback);

        // TODO : need to synchronized
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
    public boolean setRetryTimer(long[] times, int maxRetry) {
        mRetryTimes = times.clone();
        mMaxRetry = maxRetry;
        mRetryCount = 0;

        return true;
    }

    /**
     * start retry timer. If the times value is not set @link setRetryTimer(), the default value
     * will be used.
     * @param message message value want to receive for retry timer
     * @return true will be return if the operation is success, but reach the max retry count
     * false will be return
     */
    public boolean startRetryTimer(int message) {
        mRetryExpiredMessage = message;

        long duration = mRetryTimes[mRetryTimes.length - 1];
        long time = getCurrentTimeMillis();

        // TODO : get time value associated retry count
        if (mRetryCount < mMaxRetry) {
            if (mRetryCount < mRetryTimes.length) {
                duration = mRetryTimes[mRetryCount];
            }
        } else {
            return false;
        }

        ImsLog.i("retcount " + mRetryCount + " current " + time + " duration " + duration);
        setTimer(INTENT_ACTION, TIMER_ID_RETRY, time + duration);

        mRetryCount++;

        return true;
    }

    /**
     * stop retry timer.
     */
    public void stopRetryTimer() {
        clearTimer(INTENT_ACTION, TIMER_ID_RETRY);
        mRetryCount = 0;
    }

    /**
     * start validity timer.
     * @param time timer should be ended
     * @param message message value want to receive for validity timer
     */
    public void startValidityTimer(long time, int message) {
        mValidityExpiredMessage = message;

        // TODO : cal time - current time
        setTimer(INTENT_ACTION, TIMER_ID_VALIDITY, getCurrentTimeMillis() + time);
    }

    /**
     * stop validity timer.
     */
    public void stopValidityTimer() {
        clearTimer(INTENT_ACTION, TIMER_ID_VALIDITY);
    }

    private void setTimer(String intentName, int timerId, long duration) {
        // if timer exist, clear timer first and set with new duration
        clearTimer(intentName, timerId);

        Intent startIntent = new Intent(intentName);
        startIntent.putExtra(ALARM_ID, timerId);
        startIntent.putExtra(SUB_ID, mSubId);
        startIntent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);

        PendingIntent startPendingIntent =
                getPendingIntent(mContext, timerId, startIntent, 0);
        mAlarmManager.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP, duration, startPendingIntent);
    }

    private void clearTimer(String intentName, int timerId) {
        if (mAlarmManager == null) {
            ImsLog.i(mSlotId, "can not access AlarmManager");
            return;
        }

        Intent stopIntent = new Intent(intentName);
        stopIntent.putExtra(ALARM_ID, timerId);
        stopIntent.putExtra(SUB_ID, mSubId);
        PendingIntent stopPendingIntent =
                getPendingIntent(mContext, timerId, stopIntent, 0);
        mAlarmManager.cancel(stopPendingIntent);
        ImsLog.i("deleted " + timerId);
    }

    private void sendEmptyMessage(int message) {
        // TODO : need to synchronized
        if (mHandler != null) {
            mHandler.sendEmptyMessage(message);
        }
    }
    protected long getCurrentTimeMillis() {
        return System.currentTimeMillis();
    }

    protected PendingIntent getPendingIntent(Context context, int requestCode, Intent intent,
            int flags) {
        return PendingIntent.getBroadcast(context, requestCode, intent, flags);
    }
}
