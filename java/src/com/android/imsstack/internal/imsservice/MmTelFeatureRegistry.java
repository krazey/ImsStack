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

package com.android.imsstack.internal.imsservice;

import android.telecom.TelecomManager;

import com.android.imsstack.util.ImsLog;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A storage for MmTelFeature's states.
 * This manages the MmTelFeature related data such as SRVCC status,
 * and notifies the components who monitor this class that its related data has changed.
 */
public class MmTelFeatureRegistry {
    /** SRVCC state information */
    public static final int SRVCC_STATE_NONE = -1;
    public static final int SRVCC_STATE_STARTED = 0;
    public static final int SRVCC_STATE_COMPLETED = 1;
    public static final int SRVCC_STATE_FAILED = 2;
    public static final int SRVCC_STATE_CANCELED = 3;

    /**
     * Notifies the components who monitor this class that any states have changed.
     */
    public interface Listener {
        /**
         * Notifies the components who monitor the ImsService related data that
         * the terminal-based call waiting status has changed.
         */
        default void onTerminalBasedCallWaitingStatusChanged() {
        }

        /**
         * Notifies the components who monitor the MmTelFeature related data that
         * the SRVCC state has changed.
         *
         * @param srvccState The new SRVCC state.
         */
        default void onSrvccStateChanged(int srvccState) {
        }
    }

    private final Object mLock;
    private final int mSlotId;
    private boolean mTerminalBasedCallWaitingEnabled;
    private int mSrvccState = SRVCC_STATE_NONE;
    private int mTtyMode = TelecomManager.TTY_MODE_OFF;
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    MmTelFeatureRegistry(int slotId, Object lock) {
        mSlotId = slotId;
        mLock = lock;
    }

    /**
     * Checks if the terminal-based call waiting is enabled or not.
     *
     * @return true if the terminal-based call waiting is enabled, false otherwise.
     */
    public boolean isTerminalBasedCallWaitingEnabled() {
        synchronized (mLock) {
            return mTerminalBasedCallWaitingEnabled;
        }
    }

    /**
     * Sets the terminal-based call waiting status.
     *
     * @param enabled The flag specifying that the service is capable.
     */
    public void setTerminalBasedCallWaitingStatus(boolean enabled) {
        boolean notifyChange = false;

        synchronized (mLock) {
            if (mTerminalBasedCallWaitingEnabled != enabled) {
                ImsLog.i(mSlotId, "setTerminalBasedCallWaitingStatus: "
                        + mTerminalBasedCallWaitingEnabled + " >> " + enabled);
                mTerminalBasedCallWaitingEnabled = enabled;
                notifyChange = true;
            }
        }

        if (notifyChange) {
            notifyTerminalBasedCallWaitingStatusChanged();
        }
    }

    /**
     * Returns the current SRVCC state.
     *
     * When the SRVCC state is transited to the state except for {@link #SRVCC_STATE_STARTED},
     * the change notification sends the currently changed state, but the managed SRVCC state
     * is set to {@link #SRVCC_STATE_NONE} because the SRVCC operation is finished.
     *
     * @return The SRVCC state. Valid values are
     *         {@link #SRVCC_STATE_STARTED},
     *         {@link #SRVCC_STATE_NONE}.
     */
    public int getSrvccState() {
        synchronized (mLock) {
            return mSrvccState;
        }
    }

    /**
     * Sets the SRVCC state.
     *
     * @param srvccState The SRVCC state to be set.
     */
    public void setSrvccState(int srvccState) {
        boolean notifyChange = false;

        synchronized (mLock) {
            if (mSrvccState != srvccState) {
                ImsLog.i(mSlotId, "setSrvccState: " + srvccStateToString(mSrvccState)
                        + " >> " + srvccStateToString(srvccState));
                if (srvccState == SRVCC_STATE_STARTED) {
                    mSrvccState = srvccState;
                } else {
                    mSrvccState = SRVCC_STATE_NONE;
                }
                notifyChange = true;
            }
        }

        if (notifyChange) {
            notifySrvccStateChanged(srvccState);
        }
    }

    /**
     * Returns the preferred TTY mode.
     *
     * @return The current TTY mode. Possible values are:
     *         {@link TelecomManager#TTY_MODE_OFF}
     *         {@link TelecomManager#TTY_MODE_FULL}
     *         {@link TelecomManager#TTY_MODE_HCO}
     *         {@link TelecomManager#TTY_MODE_VCO}
     */
    public int getTtyMode() {
        synchronized (mLock) {
            return mTtyMode;
        }
    }

    /**
     * Sets the preferred TTY mode. This is the preferred TTY mode that the user sets in the call
     * settings screen.
     *
     * @param ttyMode The current TTY mode. Possible values are:
     *                {@link TelecomManager#TTY_MODE_OFF}
     *                {@link TelecomManager#TTY_MODE_FULL}
     *                {@link TelecomManager#TTY_MODE_HCO}
     *                {@link TelecomManager#TTY_MODE_VCO}
     */
    public void setTtyMode(int ttyMode) {
        synchronized (mLock) {
            if (mTtyMode != ttyMode) {
                ImsLog.i(mSlotId, "setTtyMode: " + mTtyMode + " >> " + ttyMode);
                mTtyMode = ttyMode;
            }
        }
    }

    /**
     * Adds the listener to monitor the state of this class.
     *
     * @param listener The listener to be aded.
     */
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    private void notifySrvccStateChanged(int srvccState) {
        for (Listener l : mListeners) {
            l.onSrvccStateChanged(srvccState);
        }
    }

    private void notifyTerminalBasedCallWaitingStatusChanged() {
        for (Listener l : mListeners) {
            l.onTerminalBasedCallWaitingStatusChanged();
        }
    }

    private static String srvccStateToString(int srvccState) {
        switch (srvccState) {
            case SRVCC_STATE_STARTED:
                return "STARTED";
            case SRVCC_STATE_COMPLETED:
                return "COMPLETED";
            case SRVCC_STATE_FAILED:
                return "FAILED";
            case SRVCC_STATE_CANCELED:
                return "CANCELED";
            default:
                return "NONE";
        }
    }
}
