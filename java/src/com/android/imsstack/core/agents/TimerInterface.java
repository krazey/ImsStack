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

/**
 * An interface to start/stop a timer and monitor its expiration.
 */
public interface TimerInterface extends IAgent {
    /** Invalid timer id value. */
    long INVALID_TID = -1L;

    /**
     * Listener interface to monitor the timer expiration.
     */
    public interface Listener {
        /**
         * Notifies the application that the timer is expired.
         *
         * @param tid The timer id to be expired.
         */
        default void onTimerExpired(long tid) {
        }
    }

    /**
     * Starts a timer with the specified duration and listener.
     *
     * @param duration The timer duration as milli-seconds.
     * @param listener The listener to monitor the timer expiration.
     * @return A timer id if a timer is successfully started, {@link #INVALID_TID} otherwise.
     */
    long startTimer(long duration, Listener listener);

    /**
     * Stops the specified timer.
     * After calling this method, the listener will be also removed.
     *
     * @param tid The timer id to be stopped.
     */
    void stopTimer(long tid);
}
