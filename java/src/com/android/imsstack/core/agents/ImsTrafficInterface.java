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
 * This provides IMS traffic interface to check if IMS traffic can be sent or not for DSDS.
 */
public interface ImsTrafficInterface extends IAgent {
    /**
     * This class provides the notifications for the change of IMS traffic priority.
     */
    public interface PriorityListener {
        /**
         * Notifies the change of IMS traffic priority.
         */
        void onTrafficPriorityChanged();
    }

    /**
     * Indicates whether IMS traffic is available or not after checking the traffic priority
     * within the IMS stack for DSDS in advance.
     *
     * @param trafficType The type for IMS traffic (@see ImsRadioInterface.TRAFFIC_TYPE_XXX)
     * @param slotId The slot-id to be checked
     *
     * @return The IMS traffic availability
     */
    boolean isAllowed(int trafficType, int slotId);

    /**
     * Sets the traffic priority by native side for a specified slot.
     *
     * @param priorityType The priority type of the traffic
     * @param slotId The slot-id to be set
     */
    void setTrafficPriority(int priorityType, int slotId);

    /**
     * Sets modem's simultaneous calling support for a specified slot.
     *
     * @param supported The information whether modem supports simultaneous calling.
     * @param slotId The slot-id to be set.
     */
    void setSimultaneousCallingSupported(boolean supported, int slotId);

    /**
     * Sets WLAN category for IPCAN.
     *
     * @param enabled The information if IPCAN is WLAN or mobile
     * @param slotId The slot-id to be set
     */
    void setWlan(boolean enabled, int slotId);

    /**
     * Adds the listener to be notified when the traffic priority is changed.
     *
     * @param listener The listener to be added
     */
    void addListener(PriorityListener listener);

    /**
     * Removes the listener to be notified when the traffic priority is changed.
     *
     * @param listener The listener to be removed
     */
    void removeListener(PriorityListener listener);
}
