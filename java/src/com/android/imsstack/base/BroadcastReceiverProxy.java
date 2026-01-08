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
package com.android.imsstack.base;

import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;

import androidx.annotation.NonNull;

/**
 * A proxy interface to monitor the broadcast intent.
 */
public interface BroadcastReceiverProxy {
    /**
     * Register to receive intent broadcasts, to run in the context of main thread that
     * is managed by the {@link AppContext}.
     *
     * @param receiver The BroadcastReceiver to handle the broadcast.
     * @param filter Selects the {@link Intent} broadcasts to be received.
     * @return The first sticky intent found that matches {@code filter}
     *         or null if there are none.
     */
    Intent registerReceiver(BroadcastReceiver receiver, IntentFilter filter);

    /**
     * Register to receive intent broadcasts, to run in the context of {@code scheduler}.
     *
     * @param receiver The BroadcastReceiver to handle the broadcast.
     * @param filter Selects the {@link Intent} broadcasts to be received.
     * @param scheduler Handler identifying the thread that will receive the {@link Intent}.
     * @return The first sticky intent found that matches {@code filter}
     *         or null if there are none.
     */
    Intent registerReceiver(BroadcastReceiver receiver, IntentFilter filter,
            @NonNull Handler scheduler);

    /**
     * Unregister a previously registered {@link BroadcastReceiver}. All filters that has been
     * registered for this BroadcastReceiver will be removed.
     */
    void unregisterReceiver(BroadcastReceiver receiver);
}
