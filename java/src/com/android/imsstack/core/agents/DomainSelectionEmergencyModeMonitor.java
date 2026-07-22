/*
 * Copyright (C) 2026 The Android Open Source Project
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

import android.telephony.TelephonyCallback;
import android.telephony.TelephonyManager;

import com.android.imsstack.base.TelephonyManagerProxy;

import java.util.concurrent.Executor;

/** Monitors Android 17 domain-selection emergency-mode callbacks. */
final class DomainSelectionEmergencyModeMonitor extends TelephonyCallback implements
        TelephonyCallback.DomainSelectionEmergencyModeListener {
    interface Listener {
        void onEmergencyModeEntered(int type, int slotIndex, int subscriptionId);

        void onEmergencyModeExited(int type, int slotIndex, int subscriptionId);
    }

    private final Listener mListener;

    DomainSelectionEmergencyModeMonitor(Listener listener) {
        mListener = listener;
    }

    void register(TelephonyManagerProxy telephonyManager, Executor executor) {
        telephonyManager.registerTelephonyCallback(executor, this);
    }

    void unregister(TelephonyManagerProxy telephonyManager) {
        telephonyManager.unregisterTelephonyCallback(this);
    }

    @Override
    public void onDomainSelectionEmergencyModeEntered(
            @TelephonyManager.DomainSelectionEmergencyType int type,
            int slotIndex, int subscriptionId) {
        mListener.onEmergencyModeEntered(type, slotIndex, subscriptionId);
    }

    @Override
    public void onDomainSelectionEmergencyModeExited(
            @TelephonyManager.DomainSelectionEmergencyType int type,
            int slotIndex, int subscriptionId) {
        mListener.onEmergencyModeExited(type, slotIndex, subscriptionId);
    }
}
