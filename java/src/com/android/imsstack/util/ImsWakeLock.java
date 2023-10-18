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
package com.android.imsstack.util;

import android.os.PowerManager;
import android.text.TextUtils;
import android.util.ArraySet;

import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;

/**
 * A wrapper class for acquiring or releasing the wake lock.
 */
public class ImsWakeLock {
    private final String mTag;
    private final PowerManager mPowerManager;
    private final Set<Object> mHolders = new ArraySet<>();
    private PowerManager.WakeLock mWakeLock;

    public ImsWakeLock(PowerManager pm, String tag) {
        mPowerManager = pm;
        mTag = TextUtils.isEmpty(tag) ? ImsWakeLock.class.getSimpleName() : tag;
    }

    /**
     * Clears the wake lock and removes all the holder objects.
     */
    public synchronized void clear() {
        ImsLog.i(mTag + " - clear :: refCount=" + mHolders.size());

        mHolders.clear();
        release(null);
    }

    /**
     * Acquires the wake lock.
     *
     * @param holder The holder object that owns this wake lock.
     */
    public synchronized void acquire(Object holder) {
        mHolders.add(holder);

        if (mWakeLock == null) {
            mWakeLock = mPowerManager.newWakeLock(
                    PowerManager.PARTIAL_WAKE_LOCK,
                    mTag);
        }

        if (!isHeld()) {
            mWakeLock.acquire();
        }

        if (ImsLog.isDebuggable()) {
            ImsLog.d(mTag + " - acquire :: refCount=" + mHolders.size());
        }
    }

    /**
     * Releases the wake lock.
     *
     * @param holder The holder object that owns this wake lock.
     */
    public synchronized void release(Object holder) {
        mHolders.remove(holder);

        if (mHolders.isEmpty() && isHeld()) {
            mWakeLock.release();
        }

        if (ImsLog.isDebuggable()) {
            ImsLog.d(mTag + " - release :: refCount=" + mHolders.size());
        }
    }

    @VisibleForTesting
    protected boolean isHeld() {
        return mWakeLock != null && mWakeLock.isHeld();
    }
}
