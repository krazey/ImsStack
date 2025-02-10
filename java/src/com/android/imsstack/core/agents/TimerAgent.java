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

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.SystemClock;
import android.util.ArrayMap;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsWakeLock;
import com.android.imsstack.util.MapIntLong;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * This class implements a {@link TimerInterface} that starts and stops a timer, and monitors
 * its expiration.
 */
public class TimerAgent implements TimerInterface {
    private static final boolean DBG_IMS_ALARM = ImsLog.isDebuggable();
    private static final int MAX_TIMER_ID = 0xFFFF;
    private static final long LONG_TIMER_INTERVAL = 30000L; // 30 seconds
    private static final int WAKE_LOCK_TIMEOUT_FOR_LONG_TIMER = 3000; // 3 seconds
    private static final int WAKE_LOCK_TIMEOUT = 1000; // 1 second
    private static final String ACTION_ALARM_NATIVE_TIMER =
            "com.android.imsstack.action.ALARM_NATIVE_TIMER";
    private static final String ACTION_ALARM_TIMER =
            "com.android.imsstack.action.ALARM_TIMER";
    private static final String EXTRA_PARAM_TID = "tid";
    private static final String IMS_ALARM_HANDLER = "ImsAlarmHandler";

    private int mMaxTimerId = MAX_TIMER_ID;
    private final TimerHandler mTimerHandler;
    private final TimerBroadcastReceiver mBroadcastReceiver;
    private final ArrayMap<Long, Listener> mTimerListeners = new ArrayMap<>();
    // Pool for timers which is greater than 30 seconds; it's only for native timers
    private final ArrayList<Long> mLongTimers = new ArrayList<>();
    // Timer management for cancelling : requestCode of PendingIntent / timer id
    private final MapIntLong mActiveTimers = new MapIntLong();
    // Tracked timer id to generate a unique id
    private final AtomicInteger mTimerIdTracker = new AtomicInteger(1);
    private final ArrayList<ImsAlarm> mImsAlarms = new ArrayList<>();
    private ImsWakeLock mWakeLock;

    /**
     * A default constructor for TimerAgent.
     */
    public TimerAgent() {
        mTimerHandler = new TimerHandler(createLooper());
        mBroadcastReceiver = new TimerBroadcastReceiver();
    }

    @Override
    public void init(Context context) {
        mBroadcastReceiver.register();
        mWakeLock = new ImsWakeLock(
                AppContext.getInstance().getSystemService(PowerManager.class),
                IMS_ALARM_HANDLER);
    }

    @Override
    public void cleanup() {
        if (mWakeLock != null) {
            mWakeLock.clear();
            mWakeLock = null;
        }

        mBroadcastReceiver.unregister();
        mTimerHandler.getLooper().quit();
    }

    @Override
    public long startTimer(long duration, Listener listener) {
        long tid = getTimerId();

        if (tid == INVALID_TID) {
            loge(this, "start - Invalid timer id.");
            return INVALID_TID;
        }

        int requestCode = addActiveTimer(tid);

        logi(this, "start - tid=" + tid + ", duration=" + duration + ", reqCode=" + requestCode);

        if (!setTimer(tid, duration, requestCode, false)) {
            removeActiveTimer(tid);
            return INVALID_TID;
        }

        synchronized (mTimerListeners) {
            mTimerListeners.put(Long.valueOf(tid), listener);
        }

        return tid;
    }

    @Override
    public void stopTimer(long tid) {
        synchronized (mTimerListeners) {
            mTimerListeners.remove(Long.valueOf(tid));
        }

        int requestCode = removeActiveTimer(tid);

        logd(this, "stop - tid=" + tid + ", reqCode=" + requestCode);

        if (requestCode == MapIntLong.INVALID_KEY) {
            logi(this, "stop - already expired; tid=" + tid);
            return;
        }

        cancelTimer(tid, requestCode, false);
    }

   /**
     * Starts a timer with the specified duration for the native service.
     *
     * @param tid The timer id to be started.
     * @param duration The timer duration as milli-seconds.
     * @return {@code true} if a timer is successfully started, {@code false} otherwise.
     */
    public boolean startNativeTimer(long tid, long duration) {
        if (tid == 0) {
            loge(this, "startNative - Invalid timer id.");
            return false;
        }

        addLongTimer(tid, duration);

        int requestCode = addActiveTimer(tid);

        logi(this, "startNative - tid=" + tid
                + ", duration=" + duration + ", reqCode=" + requestCode);

        return setTimer(tid, duration, requestCode, true);
    }

    /**
     * Stops the specified timer for the native service.
     *
     * @param tid The timer id to be stopped.
     */
    public void stopNativeTimer(long tid) {
        int requestCode = removeActiveTimer(tid);

        logi(this, "stopNative - tid=" + tid + ", reqCode=" + requestCode);

        // If the timer is present, remove it from the list
        checkAndRemoveLongTimer(tid);

        if (requestCode == MapIntLong.INVALID_KEY) {
            logi(this, "stopNative - already expired; tid=" + tid);
            return;
        }

        cancelTimer(tid, requestCode, true);
    }

    /**
     * Returns the timer looper for testing.
     */
    @VisibleForTesting
    protected Looper getTimerLooper() {
        return mTimerHandler.getLooper();
    }

    /**
     * Sets the maximum timer id for testing.
     *
     * @param maxTimerId The maximum timer id.
     */
    @VisibleForTesting
    protected void setMaxTimerId(int maxTimerId) {
        mMaxTimerId = maxTimerId;
    }

    private boolean setTimer(long tid, long duration, int requestCode, boolean isNativeTimer) {
        if (isImsAlarmRequired(duration)) {
            startImsAlarm(tid, requestCode, duration, isNativeTimer);
        } else {
            AlarmManager am = AppContext.getInstance().getSystemService(AlarmManager.class);

            if (am == null) {
                loge(this, "AlarmManager is null");
                return false;
            }

            Intent intent = createIntent(tid, isNativeTimer, false);
            PendingIntent sender = PendingIntent.getBroadcast(
                    AppContext.getInstance(), requestCode, intent,
                    PendingIntent.FLAG_CANCEL_CURRENT | PendingIntent.FLAG_IMMUTABLE);

            am.setExactAndAllowWhileIdle(AlarmManager.ELAPSED_REALTIME_WAKEUP,
                    SystemClock.elapsedRealtime() + duration, sender);
        }

        return true;
    }

    private void cancelTimer(long tid, int requestCode, boolean isNativeTimer) {
        if (stopImsAlarm(tid, requestCode, isNativeTimer)) {
            return;
        }

        AlarmManager am = AppContext.getInstance().getSystemService(AlarmManager.class);

        if (am == null) {
            loge(this, "AlarmManager is null");
            return;
        }

        Intent intent = createIntent(tid, isNativeTimer, true);
        PendingIntent sender = PendingIntent.getBroadcast(
                AppContext.getInstance(), requestCode, intent, PendingIntent.FLAG_IMMUTABLE);

        am.cancel(sender);
    }

    // The generated id can be used for {@link #startTimer} and {@link #stopTimer}.
    private long getTimerId() {
        for (int i = 1; i < mMaxTimerId; ++i) {
            int newTid = getNewTimerId();
            if (!isActiveTimer(Long.valueOf(newTid))) {
                return newTid;
            }
        }

        loge(this, "getTimerId - no available timer id.");
        return INVALID_TID;
    }

    private int getNewTimerId() {
        for (;;) {
            final int newTid = mTimerIdTracker.get();
            int nextTid = newTid + 1;
            if (nextTid == mMaxTimerId) {
                nextTid = 1;
            }
            if (mTimerIdTracker.compareAndSet(newTid, nextTid)) {
                return newTid;
            }
        }
    }

    private boolean isActiveTimer(Long tid) {
        synchronized (mTimerListeners) {
            return mTimerListeners.containsKey(tid);
        }
    }

    private void addLongTimer(long tid, long duration) {
        // We define the long timer if the duration is greater than 30 seconds.
        // It will be a criteria to acquire a wakelock when the timer is expired.
        if (duration >= LONG_TIMER_INTERVAL) {
            synchronized (mLongTimers) {
                mLongTimers.add(Long.valueOf(tid));
            }
        }
    }

    private boolean checkAndRemoveLongTimer(long tid) {
        boolean found = false;

        synchronized (mLongTimers) {
            found = mLongTimers.remove(Long.valueOf(tid));
        }

        return found;
    }

    private int addActiveTimer(long tid) {
        int key = mActiveTimers.getNewKey();
        mActiveTimers.add(key, tid);
        return key;
    }

    private int removeActiveTimer(long tid) {
        int key = mActiveTimers.getKey(tid);
        mActiveTimers.remove(key);
        return key;
    }

    private void addImsAlarm(ImsAlarm alarm) {
        synchronized (mImsAlarms) {
            mImsAlarms.add(alarm);
        }

        if (DBG_IMS_ALARM) {
            logd(this, "add=" + alarm + ", count=" + mImsAlarms.size());
        }
    }

    private ImsAlarm getImsAlarm(long tid, int requestCode, boolean isNativeTimer) {
        synchronized (mImsAlarms) {
            for (int i = 0; i < mImsAlarms.size(); ++i) {
                ImsAlarm alarm = mImsAlarms.get(i);

                if ((alarm != null) && alarm.isSameAlarm(tid, requestCode, isNativeTimer)) {
                    return alarm;
                }
            }
        }

        return null;
    }

    private void removeImsAlarm(ImsAlarm alarm) {
        synchronized (mImsAlarms) {
            mImsAlarms.remove(alarm);
        }

        if (DBG_IMS_ALARM) {
            logd(this, "remove=" + alarm + ", count=" + mImsAlarms.size());
        }
    }

    private void startImsAlarm(long tid, int requestCode, long duration, boolean isNativeTimer) {
        ImsAlarm alarm = new ImsAlarm(tid, requestCode, isNativeTimer);
        addImsAlarm(alarm);
        mWakeLock.acquire(alarm);
        mTimerHandler.postDelayed(alarm, duration);
    }

    private boolean stopImsAlarm(long tid, int requestCode, boolean isNativeTimer) {
        ImsAlarm alarm = getImsAlarm(tid, requestCode, isNativeTimer);

        if (alarm == null) {
            return false;
        }

        mTimerHandler.removeCallbacks(alarm);
        mWakeLock.release(alarm);
        removeImsAlarm(alarm);
        return true;
    }

    private void processNativeTimerExpired(final long tid) {
        // Remove the timer from the id management pool
        int requestCode = removeActiveTimer(tid);

        stopImsAlarm(tid, requestCode, true);

        boolean isLongTimer = checkAndRemoveLongTimer(tid);
        WakeLockInterface wakeLock = AgentFactory.getInstance().getAgent(WakeLockInterface.class);

        if (wakeLock != null) {
            wakeLock.acquire(isLongTimer ? WAKE_LOCK_TIMEOUT_FOR_LONG_TIMER : WAKE_LOCK_TIMEOUT);
        }

        SystemInterface.getInstance().notifyTimerExpired(tid);
    }

    private void processTimerExpired(final long tid) {
        // Remove the timer from the id management pool
        int requestCode = removeActiveTimer(tid);

        stopImsAlarm(tid, requestCode, false);

        if (isActiveTimer(tid)) {
            WakeLockInterface wakeLock =
                    AgentFactory.getInstance().getAgent(WakeLockInterface.class);
            if (wakeLock != null) {
                wakeLock.acquire(WAKE_LOCK_TIMEOUT);
            }

            AppContext.runTask(() -> {
                notifyTimerExpired(tid);
            }, 0L);
        } else {
            logd(this, "Not active timer: tid=" + tid);
        }
    }

    private void notifyTimerExpired(final long tid) {
        Listener listener;

        synchronized (mTimerListeners) {
            listener = mTimerListeners.remove(Long.valueOf(tid));
        }

        if (listener != null) {
            listener.onTimerExpired(tid);
        } else {
            logd(this, "No listener for tid=" + tid);
        }
    }

    @VisibleForTesting
    protected static Intent createIntent(long tid, boolean isNativeTimer, boolean isCancel) {
        Intent intent = new Intent(isNativeTimer
                ? ACTION_ALARM_NATIVE_TIMER : ACTION_ALARM_TIMER);
        intent.putExtra(EXTRA_PARAM_TID, tid);
        if (!isCancel) {
            intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        }
        return intent;
    }

    private static Looper createLooper() {
        HandlerThread thread = new HandlerThread(IMS_ALARM_HANDLER);
        thread.start();
        return thread.getLooper();
    }

    private static boolean isImsAlarmRequired(long duration) {
        return (duration < ImsAlarm.MAX_INTERVAL);
    }

    private static void logd(Object o, String s) {
        ImsLog.d(o, "Timer: " + s);
    }

    private static void loge(Object o, String s) {
        ImsLog.e(o, "Timer: " + s);
    }

    private static void logi(Object o, String s) {
        ImsLog.i(o, "Timer: " + s);
    }

    private final class TimerBroadcastReceiver extends BroadcastReceiver {
        public void register() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(ACTION_ALARM_NATIVE_TIMER);
            filter.addAction(ACTION_ALARM_TIMER);

            AppContext.getInstance().getBroadcastReceiverProxy()
                    .registerReceiver(this, filter, mTimerHandler);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            final long tid = intent.getLongExtra(EXTRA_PARAM_TID, INVALID_TID);
            logi(this, ImsLog.lastSubString(action, ".") + ": tid = " + tid);

            if (ACTION_ALARM_NATIVE_TIMER.equals(action)) {
                processNativeTimerExpired(tid);
            } else if (ACTION_ALARM_TIMER.equals(action)) {
                processTimerExpired(tid);
            }
        }
    }

    private static final class TimerHandler extends Handler {
        TimerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.obj instanceof Runnable) {
                notifyAlarm((Runnable) msg.obj);
            } else {
                logd(this, "Unknown message - " + msg.what);
            }
        }

        private void notifyAlarm(Runnable r) {
            try {
                r.run();
            } catch (Throwable t) {
                loge(this, "notifyAlarm - task=" + r);
                t.printStackTrace();
            }
        }
    }

    private final class ImsAlarm implements Runnable {
        public static final long MAX_INTERVAL = 5000L;

        // To guard the process restart
        private long mTid;
        private int mRequestCode;
        private boolean mNativeTimer;

        ImsAlarm(long tid, int requestCode, boolean nativeTimer) {
            mTid = tid;
            mRequestCode = requestCode;
            mNativeTimer = nativeTimer;
        }

        @Override
        public void run() {
            logd(this, "expired=" + this);

            if (mNativeTimer) {
                processNativeTimerExpired(mTid);
            } else {
                processTimerExpired(mTid);
            }
        }

        @Override
        public boolean equals(Object o) {
            if (o instanceof ImsAlarm) {
                ImsAlarm alarm = (ImsAlarm) o;
                return isSameAlarm(alarm.mTid, alarm.mRequestCode, alarm.mNativeTimer);
            }

            return false;
        }

        @Override
        public int hashCode() {
            int primeNumber = 31;

            return (Long.valueOf(mTid).hashCode()
                    + (mRequestCode * primeNumber)
                    + (mNativeTimer ? 1 : 0));
        }

        @Override
        public String toString() {
            return "{ ImsAlarm: tid=" + mTid
                    + ", reqCode=" + mRequestCode
                    + ", native=" + mNativeTimer + " }";
        }

        public boolean isSameAlarm(long tid, int requestCode, boolean nativeTimer) {
            return ((mTid == tid)
                    && (mRequestCode == requestCode)
                    && (mNativeTimer == nativeTimer));
        }
    }
}
