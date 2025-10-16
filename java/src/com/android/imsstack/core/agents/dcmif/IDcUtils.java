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
package com.android.imsstack.core.agents.dcmif;

import androidx.annotation.NonNull;

/**
 * This provides the utility interfaces that are related to the data network.
 */
public interface IDcUtils extends IDc {
    /**
     * The data structure that contains the network type and its access network information.
     */
    class AccessNetworkInfo {
        public int mNetworkType;
        public String[] mAni;

        public AccessNetworkInfo(int networkType, String[] ani) {
            mNetworkType = networkType;
            mAni = ani;
        }
    }

    /**
     * Returns the access network information of the network that the IMS is registering
     * or was registered.
     *
     * @param defaultNetworkType The default network type when the network is unknown.
     * @return The access network information.
     */
    AccessNetworkInfo getAccessNetworkInfo(int defaultNetworkType);

    /**
     * Returns PLMN consists of MCC and MNC from network UE attaches to. The PLMN is getting from
     * the {@code CellIdentity} of the {@code NetworkRegistrationInfo} of the {@code ServiceState}.
     *
     * @return The PLMN consists of MCC and MNC from network UE camps in.
     */
    @NonNull
    String getAccessNetworkPlmn();

    /**
     * Return mobile data is enabled or not in setting menu.
     */
    boolean isMobileDataEnabled();

    /**
     * Update all cell-info forcingly if the device doesn't have SIM card
     * or is in limited service state (emergency only).
     */
    void updateAllCellInfoForcinglyOnLimitedServiceState();
}
