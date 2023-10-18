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
 * An interface to track and manage the most recent cell information.
 */
public interface CellInfoInterface extends IAgent {
    /**
     * Returns the most recent cell information with the network type and timestamp (UTC format).
     *  [0] : network type
     *  [1] : timestamp (UTC format)
     *  [2...6] : access network information based on network type
     */
    String[] getAccessNetworkInfo();

    /**
     * Returns the most recent cell information with the network type and timestamp (UTC format).
     *
     * @param networkType The network type to get the access network information.
     * @return An array of the most recent access network information.
     */
    String[] getAccessNetworkInfo(int networkType);

    /**
     * Starts tracking the most recent cell information.
     */
    void startTrackingCellInfo();

    /**
     * Stops tracking the most recent cell information.
     */
    void stopTrackingCellInfo();
}
