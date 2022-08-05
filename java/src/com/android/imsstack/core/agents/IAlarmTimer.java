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

import android.os.Handler;

/**
 * This provides an interface to start and stop an alarm timer and monitor its expiration.
 */
public interface IAlarmTimer extends IAgent {
    /**
     * Returns unique timer id which is used for identify alarm timer. (Max value = 0xFFFF)
     */
    int getTimerId();

    /**
     * Registers listener to receive alarm timer event
     */
    void registerForTimerExpired(long tid, Handler h, int what, Object obj);

    /**
     * Unregisters alarm timer listener
     */
    void unregisterForTimerExpired(long tid, Handler h);

    /**
     * Starts alarm timer with duration. This API is used for java layer timer event.
     * Please register listener before start timer.
     */
    boolean startTimer(long tid, long duration);

    /**
     * Stops alarm timer before timer expired. This API is used for java layer timer event.
     * Please de-register listener after stop timer.
     */
    void stopTimer(long tid);
}
