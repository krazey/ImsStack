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

import android.os.Looper;

/**
 * This provides an interface to monitor the phone states.
 */
public interface IPhoneState extends IAgent {
    /**
     * Creates the phone state notifier without Handler.
     * Application SHOULD handle the event after posting the event on callback.
     */
    IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener);

    /**
     * Creates the phone state notifier with Handler of the specified Looper.
     * Application can handle the events directly (on callback flow)
     * because event callback is invoked by its Handler.
     */
    IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener, Looper looper);

    /**
     * Adds the notifier to monitor the phone state (call state, service state, ...).
     */
    void addNotifier(IPhoneStateNotifier notifier);

    /**
     * Removes the notifier to monitor the phone state (call state, service state, ...).
     */
    void removeNotifier(IPhoneStateNotifier notifier);

    /**
     * Gets the data RAT for cellular (TelephonyManager.NETWORK_TYPE_XXX).
     */
    int getCellularDataRAT();
}
