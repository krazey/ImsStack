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

/**
 * This class provides the notifications for the change of the subscription related states.
 */
public class SubscriptionListener {
    /**
     * Invokes if SIM load is completed.
     */
    public void onSimLoadCompleted(int slotId) {
    }

    /**
     * Invokes when default subscription is changed.
     */
    public void onDefaultSubscriptionChanged(int subId) {
    }

    /**
     * Invokes when default data subscription is changed.
     */
    public void onDefaultDataSubscriptionChanged(int subId) {
    }

    /**
     * Invokes when carrier configuration is changed.
     */
    public void onCarrierConfigChanged(int phoneId, int subId) {
    }
}
