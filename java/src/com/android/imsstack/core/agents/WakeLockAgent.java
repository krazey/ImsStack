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

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;

import com.android.imsstack.core.agents.agentif.IWakeLock;
import com.android.imsstack.system.ISystemAPIWakeLock;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

public class WakeLockAgent implements IWakeLock {
    private static final int DEFAULT_TIMEOUT = 5000;
    private static final int EVENT_ACQUIRE = 101;

    private static IWakeLock sWakeLockAgent = null;
    private WakeLockHandler mHandler = null;
    private PowerManager.WakeLock mWakeLock = null;

    public WakeLockAgent() {
    }

    public static IWakeLock getInstance() {
        if (sWakeLockAgent == null) {
            sWakeLockAgent = new WakeLockAgent();
        }

        return sWakeLockAgent;
    }

    @Override
    public void init(Context context) {
        mHandler = new WakeLockHandler();
        SystemInterface.getInstance().setWakeLock(mSystemAPI);
    }

    @Override
    public void cleanup() {
        if (mHandler != null) {
            mHandler.removeCallbacksAndMessages(null);
            mHandler = null;
        }

        SystemInterface.getInstance().setWakeLock(null);
    }

    // timeout : milli-seconds
    @Override
    public void acquire(int timeout) {
        if (timeout == 0) {
            // Set a default timeout value
            timeout = DEFAULT_TIMEOUT;
        }

        PowerManager.WakeLock wakeLock = getWakeLock();

        if (wakeLock == null) {
            ImsLog.w("WakeLock is null :: timeout=" + timeout);
            return;
        }

        ImsLog.i("WakeLock :: timeout=" + timeout);

        wakeLock.acquire(timeout);
    }

    // timeout : milli-seconds
    @Override
    public void acquire(int timeout, boolean screenOffOnly) {
        if (timeout == 0) {
            // Set a default timeout value
            timeout = DEFAULT_TIMEOUT;
        }

        PowerManager.WakeLock wakeLock = getWakeLock();

        if (wakeLock == null) {
            ImsLog.w("WakeLock is null :: timeout=" + timeout);
            return;
        }

        // Even though USB or charger is connected, the AP goes to sleep when screen is off
        // So, do not check the charging case.

        //        BatteryStateTracker bst = BatteryStateTracker.getInstance();
        //
        //        if ((bst != null) && bst.isPowerPlugged(false)) {
        //            ImsLog.d("WakeLock :: power plugged; no action; timeout=" + timeout);
        //            return;
        //        }

        if (screenOffOnly) {
            PowerManager pm = getPowerManager();

            if (pm != null) {
                // isScreenOn method was deprecated in API level 20.
                // And this method has been replaced by isInteractive()
                if (pm.isInteractive()) {
                    ImsLog.d("WakeLock :: screen on; no action; timeout=" + timeout);
                    return;
                }
            }

            //4 SCREEN_OFF intent
        }

        ImsLog.i("WakeLock :: timeout=" + timeout);

        wakeLock.acquire(timeout);
    }

    // Private/Protected methods ---------------------------------
    private synchronized PowerManager.WakeLock getWakeLock() {
        if (mWakeLock == null) {
            PowerManager pm = getPowerManager();

            if (pm == null) {
                return null;
            }

            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "ImsStack");
            mWakeLock.setReferenceCounted(true);
        }

        return mWakeLock;
    }

    private PowerManager getPowerManager() {
        return AppContext.getInstance().getSystemService(PowerManager.class);
    }

    private class WakeLockHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_ACQUIRE:
                    acquire(msg.arg1);
                    break;
                default:
                    break;
            }
        }
    }

    private ISystemAPIWakeLock mSystemAPI = new ISystemAPIWakeLock() {
        @Override
        public void acquire(int timeout) {
            if (mHandler == null) {
                WakeLockAgent.this.acquire(timeout);
            } else {
                Message.obtain(mHandler, EVENT_ACQUIRE, timeout, 0).sendToTarget();
            }
        }
    };
}
