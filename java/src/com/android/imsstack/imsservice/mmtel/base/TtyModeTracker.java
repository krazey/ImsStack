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
package com.android.imsstack.imsservice.mmtel.base;

import android.telecom.TelecomManager;

import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public class TtyModeTracker {
    /**
     * Listener to monitor the TTY mode changes
     */
    public static interface Listener {
        public void onTtyModeChanged(int newTtyMode);
    }

    private final Set<Listener> mListeners = new CopyOnWriteArraySet<Listener>();
    private int mTtyMode = TelecomManager.TTY_MODE_OFF;

    public TtyModeTracker() {
    }

    public void addListener(TtyModeTracker.Listener listener) {
        mListeners.add(listener);
    }

    public void removeListener(TtyModeTracker.Listener listener) {
        mListeners.remove(listener);
    }

    public int getTtyMode() {
        return mTtyMode;
    }

    public void setTtyMode(int ttyMode) {
        if (mTtyMode != ttyMode) {
            logi("setTtyMode :: " + mTtyMode + " >> " + ttyMode);
            mTtyMode = ttyMode;

            AppContext.runTask(new Runnable() {
                @Override
                public void run() {
                    onTtyModeChanged();
                }
            }, 0);
        }
    }

    public static int getTtyModeOff() {
        return TelecomManager.TTY_MODE_OFF;
    }

    public static boolean isTtyModeOff(int ttyMode) {
        return (ttyMode == TelecomManager.TTY_MODE_OFF);
    }

    private void onTtyModeChanged() {
        for (Listener l : mListeners) {
            l.onTtyModeChanged(mTtyMode);
        }
    }

    private static void logi(String s) {
        ImsLog.i("[GII-IMPL] " + s);
    }
}
