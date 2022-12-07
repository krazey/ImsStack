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
import android.os.Registrant;
import android.os.SystemClock;

import com.android.imsstack.system.ISystemAPIAlarm;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MapIntLong;

import java.util.ArrayList;
import java.util.Hashtable;

public class AlarmTimerAgent implements IAlarmTimer, ISystemAPIAlarm {

    // Constants--------------------------------------------------
    private static final boolean DBG_IMS_ALARM = ImsLog.isDebuggable();
    //30 sec
    private static final int LONG_TIMER_INTERVAL = 30000;
    private static final int MAX_TIMER_ID = 0xFFFF;

    private static final String EXTRA_PARAM_TID = "tid";

    private static final String ACTION_ALARM_NATIVE_TIMER =
        "com.android.imsstack.action.ALARM_NATIVE_TIMER";
    private static final String ACTION_ALARM_TIMER =
        "com.android.imsstack.action.ALARM_TIMER";

    // Variables--------------------------------------------------
    private static IAlarmTimer sAlarmTimerAgent;
    private Context mContext = null;

    private AlarmTimerReceiver mAlarmTimerReceiver;
    private Handler mAlarmTimerHandler;

    private final Hashtable<Long, Registrant> mTimerExpiredListeners =
            new Hashtable<Long, Registrant>();

    // Pool for timers which is greater than 30 seconds; it's only for native timers
    private final ArrayList<Long> mLongTimers = new ArrayList<Long>();
    // Timer management for cancelling : requestCode of PendingIntent / timer id
    private final MapIntLong mActiveTimers = new MapIntLong();
    // Tracked timer id to generate a unique id
    private int mTimerIdForUniqueness = 1;
    // IMS_ALARM_TIMER {
    private final ArrayList<ImsAlarm> mImsAlarms = new ArrayList<ImsAlarm>();
    private ImsWakeLock mWakeLock;
    // IMS_ALARM_TIMER }

    // Public methods --------------------------------------------
    public static IAlarmTimer getInstance() {
        if (sAlarmTimerAgent == null) {
            sAlarmTimerAgent = new AlarmTimerAgent();
        }

        return sAlarmTimerAgent;
    }

    public AlarmTimerAgent() {
    }

    // Interface implementation methods --------------------------
    @Override
    public void init(Context context) {
        ImsLog.d("");

        mContext = context;

        if (mContext == null) {
            ImsLog.e("mContext is null");
            return;
        }

        SystemInterface.getInstance().setISystemAPIAlarm(this);

        mAlarmTimerHandler = new AlarmTimerHandler(AlarmTimerAgent.class.getSimpleName());

        mAlarmTimerReceiver = new AlarmTimerReceiver();
        mContext.registerReceiver(mAlarmTimerReceiver,
                mAlarmTimerReceiver.getFilter(), Context.RECEIVER_EXPORTED);

        // IMS_ALARM_TIMER {
        mWakeLock = new ImsWakeLock(mContext.getSystemService(PowerManager.class));
        // IMS_ALARM_TIMER }
    }

    @Override
    public void cleanup() {
        ImsLog.d("");

        // IMS_ALARM_TIMER {
        if (mWakeLock != null) {
            mWakeLock.clear();
            mWakeLock = null;
        }
        // IMS_ALARM_TIMER }

        // Remove BR Listener
        if (mAlarmTimerReceiver != null && mContext != null) {
            mContext.unregisterReceiver(mAlarmTimerReceiver);
            mAlarmTimerReceiver = null;
        }

        if (mAlarmTimerHandler != null) {
            mAlarmTimerHandler.removeCallbacksAndMessages(null);
            mAlarmTimerHandler = null;
        }

        SystemInterface.getInstance().setISystemAPIAlarm(null);
    }

    // The generated id can be used for startTimer / stopTimer
    @Override
    public int getTimerId() {
        synchronized (this) {
            int allocatedTid = (-1);
            int tid = mTimerIdForUniqueness;
            Registrant r = null;

            while (true) {
                r = mTimerExpiredListeners.get(Long.valueOf((long)tid));

                if (r == null) {
                    allocatedTid = tid;
                    break;
                }

                tid++;

                if (tid == MAX_TIMER_ID) {
                    tid = 1;
                }
                else if (tid == mTimerIdForUniqueness) {
                    ImsLog.e("There is no available timer id");
                    break;
                }
            }

            if (allocatedTid > 0) {
                mTimerIdForUniqueness = allocatedTid + 1;

                if (mTimerIdForUniqueness == MAX_TIMER_ID) {
                    mTimerIdForUniqueness = 1;
                }
            }

            return allocatedTid;
        }
    }

    @Override
    public void registerForTimerExpired(long tid, Handler h, int what, Object obj) {
        synchronized (mTimerExpiredListeners) {
            mTimerExpiredListeners.put(Long.valueOf(tid), new Registrant(h, what, obj));
        }
    }

    @Override
    public void unregisterForTimerExpired(long tid, Handler h) {
        synchronized (mTimerExpiredListeners) {
            mTimerExpiredListeners.remove(Long.valueOf(tid));
        }
    }

    @Override
    public boolean startTimer(long tid, long duration) {
        AlarmManager am = getAlarmMngr();

        if (am == null) {
            ImsLog.e("AlarmManager is null");
            return false;
        }

        int requestCode = addActiveTimer(tid);

        ImsLog.i("tid=" + tid + ", duration=" + duration + ", reqCode=" + requestCode);

        if (isImsAlarmRequired(duration)) {
            // IMS_ALARM_TIMER {
            startImsAlarm(tid, requestCode, duration, false);
            // IMS_ALARM_TIMER }
        } else {
            Intent intent = new Intent(ACTION_ALARM_TIMER);
            intent.putExtra(EXTRA_PARAM_TID, tid);
            intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);

            PendingIntent sender = PendingIntent.getBroadcast(
                    mContext, requestCode, intent,
                    PendingIntent.FLAG_CANCEL_CURRENT | PendingIntent.FLAG_IMMUTABLE);

            am.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP,
                    SystemClock.elapsedRealtime() + duration, sender);
        }
        return true;
    }

    @Override
    public void stopTimer(long tid) {
        AlarmManager am = getAlarmMngr();

        if (am == null) {
            ImsLog.e("AlarmManager is null");
            return;
        }

        int requestCode = removeActiveTimer(tid);

        ImsLog.d("tid=" + tid + ", reqCode=" + requestCode);

        if (requestCode == MapIntLong.INVALID_KEY) {
            ImsLog.i("timer is not set or already expired; tid=" + tid);
            return;
        }

        // IMS_ALARM_TIMER {
        if (stopImsAlarm(tid, requestCode, false)) {
            return;
        }
        // IMS_ALARM_TIMER }

        Intent intent = new Intent(ACTION_ALARM_TIMER);
        intent.putExtra(EXTRA_PARAM_TID, tid);

        PendingIntent sender = PendingIntent.getBroadcast(
                mContext, requestCode, intent, PendingIntent.FLAG_IMMUTABLE);

        am.cancel(sender);
    }

    @Override
    public int startAlarm4Sys(long duration, long tid) { //startNativeTimer
        AlarmManager am = getAlarmMngr();

        if (am == null) {
            ImsLog.e("AlarmManager is null");
            return 0;
        }

        addLongTimer(tid, duration);

        int requestCode = addActiveTimer(tid);

        ImsLog.i("startNativeTimer :: tid=" + tid
                + ", duration=" + duration + ", reqCode=" + requestCode);

        if (isImsAlarmRequired(duration)) {
            // IMS_ALARM_TIMER
            startImsAlarm(tid, requestCode, duration, true);
            // IMS_ALARM_TIMER
        } else {
            Intent intent = new Intent(ACTION_ALARM_NATIVE_TIMER);
            intent.putExtra(EXTRA_PARAM_TID, tid);
            intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);

            PendingIntent sender = PendingIntent.getBroadcast(
                    mContext, requestCode, intent,
                    PendingIntent.FLAG_CANCEL_CURRENT | PendingIntent.FLAG_IMMUTABLE);

            am.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP,
                    SystemClock.elapsedRealtime() + duration, sender);
        }
        return 1;
    }

    @Override
    public int killAlarm4Sys(long tid) { //stopNativeTimer
        AlarmManager am = getAlarmMngr();

        if (am == null) {
            ImsLog.e("AlarmManager is null");
            return 0;
        }

        int requestCode = removeActiveTimer(tid);

        ImsLog.i("stopNativeTimer :: tid=" + tid + ", reqCode=" + requestCode);

        // If the timer is present, remove it from the list
        checkAndRemoveLongTimer(tid);

        if (requestCode == MapIntLong.INVALID_KEY) {
            ImsLog.i("stopNativeTimer :: timer is not set or already expired; tid=" + tid);
            return 1;
        }

        // IMS_ALARM_TIMER {
        if (stopImsAlarm(tid, requestCode, true)) {
            return 1;
        }
        // IMS_ALARM_TIMER }

        Intent intent = new Intent(ACTION_ALARM_NATIVE_TIMER);
        intent.putExtra(EXTRA_PARAM_TID, tid);

        PendingIntent sender = PendingIntent.getBroadcast(
                mContext, requestCode, intent, PendingIntent.FLAG_IMMUTABLE);

        am.cancel(sender);

        return 1;
    }


    // Private/Protected methods ---------------------------------
    private AlarmManager getAlarmMngr() {
        if (mContext == null) {
            return null;
        }

        return mContext.getSystemService(AlarmManager.class);
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

    // IMS_ALARM_TIMER {
    private void addImsAlarm(ImsAlarm alarm) {
        synchronized (mImsAlarms) {
            mImsAlarms.add(alarm);
        }

        if (DBG_IMS_ALARM) {
            ImsLog.d("add :: " + alarm + ", count=" + mImsAlarms.size());
        }
    }

    private ImsAlarm getImsAlarm(long tid, int requestCode, boolean nativeTimer) {
        synchronized (mImsAlarms) {
            for (int i = 0; i < mImsAlarms.size(); ++i) {
                ImsAlarm alarm = mImsAlarms.get(i);

                if ((alarm != null) && alarm.isSameAlarm(tid, requestCode, nativeTimer)) {
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
            ImsLog.d("remove :: " + alarm + ", count=" + mImsAlarms.size());
        }
    }

    private void startImsAlarm(long tid, int requestCode, long duration, boolean nativeTimer) {
        ImsAlarm alarm = new ImsAlarm(tid, requestCode, nativeTimer);

        addImsAlarm(alarm);
        mWakeLock.acquire(alarm);
        mAlarmTimerHandler.postDelayed(alarm, duration);
    }

    private boolean stopImsAlarm(long tid, int requestCode, boolean nativeTimer) {
        ImsAlarm alarm = getImsAlarm(tid, requestCode, nativeTimer);

        if (alarm == null) {
            return false;
        }

        mAlarmTimerHandler.removeCallbacks(alarm);
        mWakeLock.release(alarm);
        removeImsAlarm(alarm);

        return true;
    }
    // IMS_ALARM_TIMER }

    private void notifyNativeTimerExpired(final long tid) {
        // Remove the timer from the id management pool
        int requestCode = removeActiveTimer(tid);

        // IMS_ALARM_TIMER {
        stopImsAlarm(tid, requestCode, true);
        // IMS_ALARM_TIMER }

        IWakeLock wl = (IWakeLock)AgentFactory.getAgent(AgentFactory.WAKE_LOCK);

        if (wl != null) {
            if (checkAndRemoveLongTimer(tid)) {
                // wakelock - 3000ms
                wl.acquire(3000);
            } else {
                // wakelock - 1000ms
                wl.acquire(1000);
            }
        }

        SystemInterface.getInstance().notifyAlarmExpired(tid);
    }

    private void notifyTimerExpired(final long tid) {
        // Remove the timer from the id management pool
        int requestCode = removeActiveTimer(tid);

        // IMS_ALARM_TIMER {
        stopImsAlarm(tid, requestCode, false);
        // IMS_ALARM_TIMER }

        Registrant r = null;

        synchronized (mTimerExpiredListeners) {
            r = mTimerExpiredListeners.get(Long.valueOf(tid));
        }

        if (r != null) {
            // wakelock - 1000ms
            IWakeLock wl = (IWakeLock)AgentFactory.getAgent(AgentFactory.WAKE_LOCK);
            if (wl != null) {
                wl.acquire(1000);
            }
            r.notifyRegistrant();
        } else {
            ImsLog.d("No listener for tid=" + tid);
        }
    }

    // IMS_ALARM_TIMER {
    private static Looper createLooper(String name) {
        HandlerThread thread = new HandlerThread(name);
        thread.start();

        Looper looper = thread.getLooper();

        if (looper == null) {
            return Looper.getMainLooper();
        }

        return looper;
    }

    private static boolean isImsAlarmRequired(long duration) {
        return (duration < ImsAlarm.MAX_INTERVAL);
    }
    // IMS_ALARM_TIMER }

    //------------------------------------------------------------------------------
    private class AlarmTimerReceiver extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public AlarmTimerReceiver() {
            mIntentFilter.addAction(ACTION_ALARM_NATIVE_TIMER);
            mIntentFilter.addAction(ACTION_ALARM_TIMER);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            if (mAlarmTimerHandler == null) {
                return;
            }

            String action = intent.getAction();
            final long tid = intent.getLongExtra(EXTRA_PARAM_TID, 0);
            ImsLog.i(ImsLog.lastSubString(action, ".") + " :: tid = " + tid);

            if (ACTION_ALARM_NATIVE_TIMER.equals(action)) {
                mAlarmTimerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        notifyNativeTimerExpired(tid);
                    }
                });
            } else if (ACTION_ALARM_TIMER.equals(action)) {
                mAlarmTimerHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        notifyTimerExpired(tid);
                    }
                });
            }
        }
    }

    // IMS_ALARM_TIMER {
    private class AlarmTimerHandler extends Handler {

        public AlarmTimerHandler(String name) {
            super(createLooper(name));
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.obj instanceof Runnable) {
                notifyAlarm((Runnable)msg.obj);
            } else {
                ImsLog.d("AlarmTimerHandler :: Unknown message; msg=" + msg.what);
            }
        }

        private void notifyAlarm(Runnable r) {
            try {
                r.run();
            } catch (Throwable t) {
                ImsLog.e("AlarmTimerHandler :: wakeAlarm - run task=" + r);
                t.printStackTrace();
            } finally {
            }
        }
    }

    private class ImsAlarm implements Runnable {
        public static final long MAX_INTERVAL = 5000;

        // To guard the process restart
        private long mTid;
        private int mRequestCode;
        private boolean mNativeTimer;

        public ImsAlarm(long tid, int requestCode, boolean nativeTimer) {
            mTid = tid;
            mRequestCode = requestCode;
            mNativeTimer = nativeTimer;
        }

        @Override
        public void run() {
            ImsLog.d("ImsAlarmTimerExpired :: " + this);

            if (mNativeTimer) {
                notifyNativeTimerExpired(mTid);
            } else {
                notifyTimerExpired(mTid);
            }
        }

        @Override
        public boolean equals(Object o) {
            if (o instanceof ImsAlarm) {
                ImsAlarm alarm = (ImsAlarm)o;

                return (isSameAlarm(alarm.mTid, alarm.mRequestCode, alarm.mNativeTimer));
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
            return "ImsAlarm={ tid=" + mTid +
                    ", reqCode=" + mRequestCode +
                    ", native=" + mNativeTimer + " }";
        }

        public boolean isSameAlarm(long tid, int requestCode, boolean nativeTimer) {
            return ((mTid == tid)
                    && (mRequestCode == requestCode)
                    && (mNativeTimer == nativeTimer));
        }
    }
    // IMS_ALARM_TIMER }
}
