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
package com.android.imsstack.core.agents.agentif;

/**
 * This class provides an interface to access the SIM and subscription related information.
 */
public interface ISubscription extends IAgent {
    /** Adds listener for subscription */
    void addListener(SubscriptionListener listener);
    /** Removes listener for subscription */
    void removeListener(SubscriptionListener listener);

    /**
     * Returns an active subscription id for single SIM
     * or single IMS instance of multiple SIM environment.
     */
    int getActiveSubId();

    /**
     * Returns the default phone id for Ims service.
     */
    int getPhoneId();

    /**
     * Returns a phone id for the specified SIM slot.
     */
    int getPhoneId(int slotId);

    /**
     * Returns the default slot id for Ims service.
     */
    int getSlotId();

    /**
     * Returns the default subscription id for Ims service.
     */
    int getSubId();

    /**
     * Returns a subscription id for the specified SIM slot.
     */
    int getSubId(int slotId);

    /**
     * Checks if all the SIMs are absent or not.
     */
    boolean isAllSimAbsent();

    /**
     * Checks if all the SIM are absent or locked.
     */
    boolean isAllSimAbsentOrLocked();

    /**
     * Checks if SIM is absent or not.
     */
    boolean isSimAbsent(int slotId);

    /**
     * Checks if SIM is loaded or not.
     */
    boolean isSimLoaded(int slotId);

    /**
     * Checks if SIM is locked or not.
     */
    boolean isSimLocked(int slotId);

    /**
     * Checks if SIM load is done or not for specified SIM slot.
     */
    boolean isSimLoadCompleted(int slotId);

    /**
     * Checks if SIM load is done or not.
     */
    boolean isSimLoadCompleted();

    /**
     * Returns the current SIM STATE
     */
    String getSimState();
}
