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
import android.os.PowerManager;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * A class for providing the timed WakeLock while processing SIP signalling.
 */
public class WakeLockAgent implements WakeLockInterface {
    private static final String WAKE_LOCK_TAG = "ImsStack";
    private static final int DEFAULT_TIMEOUT = 5000; // 5 seconds

    private PowerManager.WakeLock mWakeLock;

    /**
     * A default constructor.
     */
    public WakeLockAgent() {
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
    }

    @Override
    public void acquire(int timeoutMillis) {
        acquire(timeoutMillis, "acquire");
    }

    @Override
    public void acquireForNative(int timeoutMillis) {
        AppContext.runTask(() -> {
            acquire(timeoutMillis, "acquireForNative");
        }, 0);
    }

    @VisibleForTesting
    protected synchronized PowerManager.WakeLock getWakeLock() {
        if (mWakeLock == null) {
            PowerManager pm = AppContext.getInstance().getSystemService(PowerManager.class);
            mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, WAKE_LOCK_TAG);
            mWakeLock.setReferenceCounted(true);
        }

        return mWakeLock;
    }

    private void acquire(int timeoutMillis, String tag) {
        if (timeoutMillis == 0) {
            timeoutMillis = DEFAULT_TIMEOUT;
        }

        PowerManager.WakeLock wakeLock = getWakeLock();

        ImsLog.i(this, "WakeLock#" + tag + ": " + timeoutMillis);

        wakeLock.acquire(timeoutMillis);
    }
}
