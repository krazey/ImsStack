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
import android.os.BatteryManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;

import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPIBattery;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

/**
 * This implements an interface to check and control the battery status of the device.
 */
public class BatteryStateAgent implements IBatteryState, ISystemAPIBattery {
    // Refer to res (/values/config.xml - config_lowBatteryWarningLevel) in framework
    private static final int LOW_BATTERY_THRESHOLD = 15;

    // FIXME
    // private static final int LOW_BATTERY_LEVEL = 5;
    // UX scenario changed for VT call (low battery: 5% to 2%)
    private static final int LOW_BATTERY_LEVEL_FOR_CALL = 2;
    private static final int HANDLER_POST_DELAY = 100;
    // Timer id for polling timer to track the battery state
    private static final int TID_POLLING_TIMER = 100;

    private static final int EVENT_BATTERY_CHANGED = 101;
    private static final int EVENT_NOTIFY_LOW_BATTERY_STATE = 102;

    private static final String ACTION_BATTERY_POLLING_TIMER =
            "com.android.imsstack.action.BATTERY_POLLING_TIMER";

    private static BatteryStateAgent sBatteryStateAgent = null;
    private Context mContext;
    private BatteryChangedReceiver mBatteryChangedReceiver = null;
    private BatteryStateHandler mBatteryStateHandler = null;
    private BatteryStateReceiver mBatteryStateReceiver = null;
    private boolean mLowBatteryNotified = false;
    private boolean mPollingTimerStarted = false;
    private boolean mBatteryChangedReceiverInstalled = false;
    private int mBatteryLowThreshold = LOW_BATTERY_LEVEL_FOR_CALL;

    public BatteryStateAgent() {
    }

    public static IBatteryState getInstance() {
        if (sBatteryStateAgent == null) {
            sBatteryStateAgent = new BatteryStateAgent();
        }

        return sBatteryStateAgent;
    }

    @Override
    public void init(Context context) {
        if (context == null) {
            return;
        }

        mContext = context;
        SystemInterface.getInstance().setISystemAPIBattery(this);

        mBatteryStateHandler = new BatteryStateHandler();
        mBatteryChangedReceiver = new BatteryChangedReceiver();
        mBatteryStateReceiver = new BatteryStateReceiver();

        mContext.registerReceiver(mBatteryStateReceiver,
                mBatteryStateReceiver.getFilter(), Context.RECEIVER_EXPORTED);

        if (isPowerPlugged(true)) {
            int interval = getPollingInterval(getBatteryLevel());

            if (interval == 0) {
                installBatteryChangedReceiver();
            }
            else {
                startPollingTimer(interval);
            }
        } else if (getBatteryLevel() <= (LOW_BATTERY_THRESHOLD + 1)) {
            installBatteryChangedReceiver();
        }
    }

    @Override
    public void cleanup() {
        ImsLog.d("");

        if (mBatteryStateHandler != null) {
            mBatteryStateHandler.removeCallbacksAndMessages(null);
            mBatteryStateHandler = null;
        }

        if (mBatteryChangedReceiverInstalled) {
            if (mContext != null && mBatteryChangedReceiver != null) {
                mContext.unregisterReceiver(mBatteryChangedReceiver);
                mBatteryChangedReceiver = null;
            }
            mBatteryChangedReceiverInstalled = false;
        }

        if (mContext != null && mBatteryStateReceiver != null) {
            mContext.unregisterReceiver(mBatteryStateReceiver);
            mBatteryStateReceiver = null;
        }

        stopPollingTimer();

        SystemInterface.getInstance().setISystemAPIBattery(null);
    }

    @Override
    public int getBatteryLevel() {
        if (mContext == null) {
            return (-1);
        }

        Intent stickyIntent = mContext.registerReceiver(null,
                new IntentFilter(Intent.ACTION_BATTERY_CHANGED), Context.RECEIVER_EXPORTED);

        if (stickyIntent == null) {
            return (-1);
        }

        int level = stickyIntent.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);

        ImsLog.i("Battery level=" + level);

        return level;
    }

    @Override
    public boolean isLowBattery() {
        return (getBatteryLevel() <= mBatteryLowThreshold);
    }

    @Override
    public boolean isPowerPlugged(boolean log) {
        if (mContext == null) {
            return false;
        }

        Intent stickyIntent = mContext.registerReceiver(null,
                new IntentFilter(Intent.ACTION_BATTERY_CHANGED), Context.RECEIVER_EXPORTED);

        if (stickyIntent == null) {
            return false;
        }

        int status = stickyIntent.getIntExtra(BatteryManager.EXTRA_STATUS, 0);
        int plugged = stickyIntent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);

        if (log) {
            ImsLog.i("Battery status=" + status + ", plugged=" + plugged);
        }

        return ((plugged != 0) || (status == BatteryManager.BATTERY_STATUS_CHARGING));
    }

    @Override
    public void notifyLowBatteryState(int slotId) {
        if (mBatteryStateHandler == null) {
            if (isLowBattery()) {
                notifyBatteryStateForSlot(slotId, ImsEventDef.IMS_POWER_LOW_BATTERY);
            }
        } else {
            Message.obtain(mBatteryStateHandler,
                    EVENT_NOTIFY_LOW_BATTERY_STATE, slotId, 0).sendToTarget();
        }
    }

    @Override
    public int getBatteryLevel4Sys() {
        return getBatteryLevel();
    }

    private AlarmManager getAlarmManager() {
        return (mContext != null) ?
                mContext.getSystemService(AlarmManager.class) : null;
    }

    private void handleBatteryChanged(Intent intent) {
        if (intent == null) {
            return;
        }

        int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, mBatteryLowThreshold);

        ImsLog.i("Battery level=" + level);

        // Manages the battery level in the native logic
        if (level <= (LOW_BATTERY_THRESHOLD + 1)) {
            SystemInterface.getInstance().notifyBatteryLevelChanged(level);
        } else if ((level % 5) == 0) {
            SystemInterface.getInstance().notifyBatteryLevelChanged(level);
        }

        if (level <= mBatteryLowThreshold) {
            if (!mLowBatteryNotified) {
                notifyBatteryState(ImsEventDef.IMS_POWER_LOW_BATTERY);
                mLowBatteryNotified = true;
            }
        } else {
            if (mLowBatteryNotified) {
                notifyBatteryState(ImsEventDef.IMS_POWER_LOW_CHANGED);
                mLowBatteryNotified = false;
            }
        }

        if (level > (LOW_BATTERY_THRESHOLD + 1)) {
            int status = intent.getIntExtra(BatteryManager.EXTRA_STATUS, 0);
            int plugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);

            ImsLog.i("Battery status=" + status + ", plugged=" + plugged);

            if ((plugged != 0) || (status == BatteryManager.BATTERY_STATUS_CHARGING)) {
                int interval = getPollingInterval(level);

                if (interval != 0) {
                    uninstallBatteryChangedReceiver();
                    startPollingTimer(interval);
                }
            } else {
                uninstallBatteryChangedReceiver();
            }
        }
    }

    private void installBatteryChangedReceiver() {
        if (mBatteryStateHandler == null) {
            return;
        }

        if (!mBatteryChangedReceiverInstalled) {
            mBatteryChangedReceiverInstalled = true;
            mBatteryStateHandler.postDelayed(
                    new BatteryChangedReceiverInstaller(), HANDLER_POST_DELAY);

            stopPollingTimer();
        } else {
            ImsLog.d("BatteryChangedReceiver is already installed");
        }
    }

    private void uninstallBatteryChangedReceiver() {
        if (mBatteryStateHandler == null) {
            return;
        }

        if (mBatteryChangedReceiverInstalled) {
            mBatteryChangedReceiverInstalled = false;
            mBatteryStateHandler.postDelayed(
                    new BatteryChangedReceiverUninstaller(), HANDLER_POST_DELAY);
        } else {
            ImsLog.d("BatteryChangedReceiver is already uninstalled");
        }
    }

    private void startPollingTimer(int interval) {
        if (mPollingTimerStarted) {
            ImsLog.i("Polling timer is already started");
            return;
        }

        AlarmManager am = getAlarmManager();
        if (am == null) {
            return;
        }

        Intent intent = new Intent(ACTION_BATTERY_POLLING_TIMER);
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);

        PendingIntent sender = PendingIntent.getBroadcast(mContext,
                TID_POLLING_TIMER, intent,
                PendingIntent.FLAG_CANCEL_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        am.setExact(AlarmManager.ELAPSED_REALTIME_WAKEUP,
                SystemClock.elapsedRealtime() + interval, sender);

        mPollingTimerStarted = true;

        ImsLog.i("Polling timer is started - interval=" + interval);
    }

    private void stopPollingTimer() {
        if (!mPollingTimerStarted) {
            ImsLog.i("Polling timer is already stopped");
            return;
        }

        AlarmManager am = getAlarmManager();
        if (am == null) {
            return;
        }

        Intent intent = new Intent(ACTION_BATTERY_POLLING_TIMER);
        PendingIntent sender = PendingIntent.getBroadcast(mContext,
                TID_POLLING_TIMER, intent, PendingIntent.FLAG_IMMUTABLE);

        am.cancel(sender);

        mPollingTimerStarted = false;

        ImsLog.i("Polling timer is stopped");
    }

    private int getPollingInterval(int level) {
        // Interval (milliseconds) for battery level check
        int multiplicationToMinute = 60 * 1000;
        if (level <= (LOW_BATTERY_THRESHOLD + 1)) {
            // do not start polling timer; install the battery changed receiver
            return 0;
        } else if (level < 20) {
            // 5 minutes
            return 5 * multiplicationToMinute;
        } else if (level < 30) {
            // 10 minutes
            return 10 * multiplicationToMinute;
        } else if (level < 40) {
            // 20 minutes
            return 20 * multiplicationToMinute;
        } else if (level < 50) {
            // 40 minutes
            return 40 * multiplicationToMinute;
        } else {
            // 60 minutes
            return 60 * multiplicationToMinute;
        }
    }

    private void notifyBatteryState(int state) {
        for (int i = 0; i < MSimUtils.getMaxSimSlot(); i++) {
            ISystem system = SystemInterface.getInstance().getSystem(i);
            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY, state, 0);
            }
        }
    }

    private void notifyBatteryStateForSlot(int slotId, int state) {
        if (slotId == MSimUtils.INVALID_PHONE_ID) {
            notifyBatteryState(state);
        } else {
            ISystem system = SystemInterface.getInstance().getSystem(slotId);
            if (system != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_POWER_LOW_BATTERY, state, 0);
            }
        }
    }

    private final class BatteryStateHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.i("BatteryState :: msg=" + msg.what);

            switch (msg.what) {
            case EVENT_BATTERY_CHANGED: {
                handleBatteryChanged((Intent)msg.obj);
                break;
            }
            case EVENT_NOTIFY_LOW_BATTERY_STATE: {
                if (isLowBattery()) {
                    notifyBatteryStateForSlot(msg.arg1, ImsEventDef.IMS_POWER_LOW_BATTERY);
                }
                break;
            }
            default:
                break;
            }
        }
    }

    private final class BatteryChangedReceiver extends BroadcastReceiver {

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(ImsLog.lastSubString(action, "."));

            if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {
                if (mBatteryStateHandler != null) {
                    Message.obtain(mBatteryStateHandler,
                            EVENT_BATTERY_CHANGED, intent).sendToTarget();
                } else {
                    handleBatteryChanged(intent);
                }
            }
        }
    }

    private final class BatteryStateReceiver extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public BatteryStateReceiver() {
            mIntentFilter.addAction(Intent.ACTION_BATTERY_LOW);
            mIntentFilter.addAction(Intent.ACTION_POWER_CONNECTED);
            mIntentFilter.addAction(Intent.ACTION_POWER_DISCONNECTED);
            mIntentFilter.addAction(ACTION_BATTERY_POLLING_TIMER);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(ImsLog.lastSubString(action, "."));

            if (Intent.ACTION_BATTERY_LOW.equals(action)) {
                installBatteryChangedReceiver();
            } else if (Intent.ACTION_POWER_CONNECTED.equals(action)) {
                int interval = getPollingInterval(getBatteryLevel());

                if (interval == 0) {
                    installBatteryChangedReceiver();
                } else {
                    startPollingTimer(interval);
                }
            } else if (Intent.ACTION_POWER_DISCONNECTED.equals(action)) {
                stopPollingTimer();

                if (getBatteryLevel() <= (LOW_BATTERY_THRESHOLD + 1)) {
                    installBatteryChangedReceiver();
                } else {
                    uninstallBatteryChangedReceiver();
                }
            } else if (ACTION_BATTERY_POLLING_TIMER.equals(action)) {
                mPollingTimerStarted = false;

                int interval = getPollingInterval(getBatteryLevel());

                if (interval == 0) {
                    installBatteryChangedReceiver();
                } else {
                    startPollingTimer(interval);
                }
            }
        }
    }

    private final class BatteryChangedReceiverInstaller implements Runnable {

        @Override
        public void run() {
            ImsLog.d("BatteryChangedReceiverInstaller is run...");

            if (mContext == null) {
                return;
            }

            if (mBatteryChangedReceiver != null) {
                mContext.registerReceiver(mBatteryChangedReceiver,
                        new IntentFilter(Intent.ACTION_BATTERY_CHANGED),
                        Context.RECEIVER_EXPORTED);
            }
        }
    }

    private final class BatteryChangedReceiverUninstaller implements Runnable {

        @Override
        public void run() {
            ImsLog.d("BatteryChangedReceiverUninstaller is run...");

            if (mContext == null) {
                return;
            }

            mContext.unregisterReceiver(mBatteryChangedReceiver);
        }
    }
}
