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

import android.annotation.NonNull;
import android.os.Looper;
import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;

/**
 * An interface for monitoring the phone states such as {@link ServiceState}, call state,
 * barring information, and so on.
 */
public interface PhoneStateInterface extends IAgent {
    /**
     * Creates the phone state notifier without Handler.
     * Application SHOULD handle the event after posting the event on callback.
     *
     * @param listener The listener to be registered.
     * @return A {@link IPhoneStateNotifier} instance.
     */
    IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener);

    /**
     * Creates the phone state notifier with Handler of the specified Looper.
     * Application can handle the events directly (on callback flow)
     * because event callback is invoked by its Handler.
     *
     * @param listener The listener to be registered.
     * @param looper The specific {@link Looper} object.
     * @return A {@link IPhoneStateNotifier} instance.
     */
    IPhoneStateNotifier createNotifier(ImsPhoneStateListener listener, @NonNull Looper looper);

    /**
     * Adds the notifier to monitor the phone state (call state, service state, ...).
     *
     * @param notifier A {@link IPhoneStateNotifier} instance to be added.
     */
    void addNotifier(IPhoneStateNotifier notifier);

    /**
     * Removes the notifier to monitor the phone state (call state, service state, ...).
     *
     * @param notifier A {@link IPhoneStateNotifier} instance to be removed.
     */
    void removeNotifier(IPhoneStateNotifier notifier);

    /**
     * Returns the data network type(TelephonyManager#NETWORK_TYPE_XXX) of the cellular network.
     */
    @NetworkType int getCellularDataNetworkType();

    /**
     * Returns the current CS call state.
     *
     * @return The call state. Possible values are:
     *         TelephonyManager#CALL_STATE_IDLE,
     *         TelephonyManager#CALL_STATE_RINGING,
     *         TelephonyManager#CALL_STATE_OFFHOOK.
     */
    @CallState int getCsCallState();

    /**
     * Returns the current IMS call state.
     *
     * @return The call state. Possible values are:
     *         TelephonyManager#CALL_STATE_IDLE,
     *         TelephonyManager#CALL_STATE_OFFHOOK.
     */
    @CallState int getImsCallState();

    /**
     * Sets the IMS call state.
     *
     * @param state The call state. Possible values are:
     *              TelephonyManager#CALL_STATE_IDLE,
     *              TelephonyManager#CALL_STATE_OFFHOOK.
     */
    void setImsCallState(@CallState int state);
}
