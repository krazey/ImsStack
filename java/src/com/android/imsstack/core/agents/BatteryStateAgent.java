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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.BatteryManager;
import android.os.Handler;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.LinkedHashMap;
import java.util.Map;

/**
 * This implements an interface to check and control the battery status of the device.
 */
public class BatteryStateAgent implements BatteryStateInterface {
    @VisibleForTesting
    protected static final int DELAY_INSTALL_BATTERY_CHANGED_RECEIVER = 100;
    // For VT call (low battery: 2%)
    @VisibleForTesting
    protected static final int LOW_BATTERY_WARNING_LEVEL_FOR_CALL = 2;

    private final Handler mHandler;
    private final BatteryChangedReceiver mBatteryChangedReceiver;
    private final BatteryStateReceiver mBatteryStateReceiver;
    private final TimerListener mTimerListener;
    // Map for <level(criteria), time interval(milliseconds)>
    private final Map<Integer, Long> mPollingIntervals = new LinkedHashMap<>();
    private final int mLowBatteryWarningLevel;
    private int mStatus = BatteryManager.BATTERY_STATUS_UNKNOWN;
    private int mPlugged = 0;
    private int mLevel = INVALID_BATTERY_LEVEL;
    private long mPollingTimerId = TimerInterface.INVALID_TID;
    private boolean mLowBatteryNotified;
    private Runnable mBatteryChangedReceiverInstaller;

    public BatteryStateAgent() {
        mHandler = new Handler(AppContext.getInstance().getMainLooper());
        mBatteryChangedReceiver = new BatteryChangedReceiver();
        mBatteryStateReceiver = new BatteryStateReceiver();
        mTimerListener = new TimerListener();

        mLowBatteryWarningLevel = AppContext.getInstance().getResources().getInteger(
                com.android.internal.R.integer.config_lowBatteryWarningLevel);

        // no polling timer; install battery changed receiver
        mPollingIntervals.put(mLowBatteryWarningLevel, 0L);
        mPollingIntervals.put(mLowBatteryWarningLevel + 10, 10 * 60 * 1000L); // 10 minutes
        mPollingIntervals.put(mLowBatteryWarningLevel + 20, 20 * 60 * 1000L); // 20 minutes
        mPollingIntervals.put(mLowBatteryWarningLevel + 30, 40 * 60 * 1000L); // 40 minutes
        mPollingIntervals.put(Integer.MAX_VALUE, 60 * 60 * 1000L); // 1 hour
    }

    @Override
    public void init(Context context) {
        mBatteryStateReceiver.register();
        updateCurrentBatteryStates();

        if (mPlugged != 0 || mStatus == BatteryManager.BATTERY_STATUS_CHARGING) {
            startListeningForBatteryState(mLevel);
        } else if (mLevel <= mLowBatteryWarningLevel) {
            installBatteryChangedReceiver();
        }
    }

    @Override
    public void cleanup() {
        uninstallBatteryChangedReceiver();
        mHandler.removeCallbacksAndMessages(null);
        mBatteryStateReceiver.unregister();
        stopPollingTimer();
    }

    @Override
    public int getBatteryLevel() {
        updateCurrentBatteryStates();
        return mLevel;
    }

    @Override
    public boolean isLowBattery() {
        return getBatteryLevel() <= LOW_BATTERY_WARNING_LEVEL_FOR_CALL;
    }

    @Override
    public boolean isPowerPlugged() {
        updateCurrentBatteryStates();
        return mPlugged != 0 || mStatus == BatteryManager.BATTERY_STATUS_CHARGING;
    }

    @Override
    public void notifyLowBatteryState(int slotId) {
        if (isLowBattery()) {
            SystemInterface.getInstance().notifyLowBatteryState(slotId);
        }
    }

    private void updateCurrentBatteryStates() {
        Intent stickyIntent = AppContext.getInstance().getBroadcastReceiverProxy()
                .registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        updateBatteryStates(stickyIntent);
    }

    private void updateBatteryStates(Intent intent) {
        if (intent != null) {
            mStatus = intent.getIntExtra(BatteryManager.EXTRA_STATUS,
                    BatteryManager.BATTERY_STATUS_UNKNOWN);
            mPlugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);
            mLevel = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, INVALID_BATTERY_LEVEL);
        } else {
            mStatus = BatteryManager.BATTERY_STATUS_UNKNOWN;
            mPlugged = 0;
            mLevel = INVALID_BATTERY_LEVEL;
        }

        ImsLog.i(this, "BatteryState: status=" + mStatus
                + ", plugged=" + mPlugged + ", level=" + mLevel);
    }

    private void handleBatteryChanged() {
        int level = (mLevel == INVALID_BATTERY_LEVEL)
                ? LOW_BATTERY_WARNING_LEVEL_FOR_CALL : mLevel;

        // Manages the battery level in the native logic
        if (level <= mLowBatteryWarningLevel) {
            SystemInterface.getInstance().notifyBatteryLevelChanged(level);
        } else if ((level % 5) == 0) {
            SystemInterface.getInstance().notifyBatteryLevelChanged(level);
        }

        if (level <= LOW_BATTERY_WARNING_LEVEL_FOR_CALL) {
            if (!mLowBatteryNotified) {
                SystemInterface.getInstance().notifyLowBatteryState();
                mLowBatteryNotified = true;
            }
        } else {
            if (mLowBatteryNotified) {
                SystemInterface.getInstance().notifyLowBatteryStateChanged();
                mLowBatteryNotified = false;
            }
        }

        if (level > mLowBatteryWarningLevel) {
            uninstallBatteryChangedReceiver();

            if ((mPlugged != 0) || (mStatus == BatteryManager.BATTERY_STATUS_CHARGING)) {
                startListeningForBatteryState(level);
            }
        }
    }

    private void installBatteryChangedReceiver() {
        if (mBatteryChangedReceiverInstaller == null) {
            mBatteryChangedReceiverInstaller = () -> {
                ImsLog.d(this, "BatteryChangedReceiverInstaller is run");
                AppContext.getInstance().getBroadcastReceiverProxy()
                        .registerReceiver(mBatteryChangedReceiver,
                                new IntentFilter(Intent.ACTION_BATTERY_CHANGED), mHandler);
            };
            mHandler.postDelayed(mBatteryChangedReceiverInstaller,
                    DELAY_INSTALL_BATTERY_CHANGED_RECEIVER);
            stopPollingTimer();
        } else {
            ImsLog.d(this, "BatteryChangedReceiver is already installed");
        }
    }

    private void uninstallBatteryChangedReceiver() {
        if (mBatteryChangedReceiverInstaller != null) {
            if (mHandler.hasCallbacks(mBatteryChangedReceiverInstaller)) {
                mHandler.removeCallbacks(mBatteryChangedReceiverInstaller);
            } else {
                AppContext.getInstance().getBroadcastReceiverProxy()
                        .unregisterReceiver(mBatteryChangedReceiver);
            }
            mBatteryChangedReceiverInstaller = null;
        } else {
            ImsLog.d(this, "BatteryChangedReceiver is already uninstalled");
        }
    }

    private void startPollingTimer(long interval) {
        if (mPollingTimerId != TimerInterface.INVALID_TID) {
            ImsLog.i(this, "Polling timer is already started");
            return;
        }

        TimerInterface timer = AgentFactory.getInstance().getAgent(TimerInterface.class);

        if (timer != null) {
            mPollingTimerId = timer.startTimer(interval, mTimerListener);
        }

        ImsLog.i(this, "Polling timer is started - tid="
                + mPollingTimerId + ", interval=" + interval);
    }

    private void stopPollingTimer() {
        if (mPollingTimerId == TimerInterface.INVALID_TID) {
            ImsLog.i(this, "Polling timer is already stopped");
            return;
        }

        TimerInterface timer = AgentFactory.getInstance().getAgent(TimerInterface.class);

        if (timer != null) {
            timer.stopTimer(mPollingTimerId);
        }

        mPollingTimerId = TimerInterface.INVALID_TID;
        ImsLog.i(this, "Polling timer is stopped");
    }

    private long getPollingInterval(int level) {
        return mPollingIntervals.entrySet().stream()
                .filter(e -> level <= e.getKey())
                .map(Map.Entry::getValue)
                .findFirst()
                .orElse(60 * 60 * 1000L);
    }

    private void startListeningForBatteryState(int batteryLevel) {
        long interval = getPollingInterval(batteryLevel);

        if (interval == 0L) {
            installBatteryChangedReceiver();
        } else {
            startPollingTimer(interval);
        }
    }

    private final class TimerListener implements TimerInterface.Listener {
        @Override
        public void onTimerExpired(long tid) {
            if (mPollingTimerId == tid) {
                ImsLog.i(this, "Polling timer is expired.");
                mPollingTimerId = TimerInterface.INVALID_TID;
                startListeningForBatteryState(getBatteryLevel());
            }
        }
    }

    private final class BatteryChangedReceiver extends BroadcastReceiver {
        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(this, ImsLog.lastSubString(action, "."));

            if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {
                updateBatteryStates(intent);
                handleBatteryChanged();
            }
        }
    }

    private final class BatteryStateReceiver extends BroadcastReceiver {
        public void register() {
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_BATTERY_LOW);
            filter.addAction(Intent.ACTION_POWER_CONNECTED);
            filter.addAction(Intent.ACTION_POWER_DISCONNECTED);

            AppContext.getInstance().getBroadcastReceiverProxy()
                    .registerReceiver(this, filter, mHandler);
        }

        public void unregister() {
            AppContext.getInstance().getBroadcastReceiverProxy().unregisterReceiver(this);
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            ImsLog.i(this, ImsLog.lastSubString(action, "."));

            if (Intent.ACTION_BATTERY_LOW.equals(action)) {
                installBatteryChangedReceiver();
            } else if (Intent.ACTION_POWER_CONNECTED.equals(action)) {
                startListeningForBatteryState(getBatteryLevel());
            } else if (Intent.ACTION_POWER_DISCONNECTED.equals(action)) {
                stopPollingTimer();
                startListeningForBatteryState(getBatteryLevel());
            }
        }
    }
}
