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

public class ApnStateListener {
    /**
     * Notifies the application that IPCAN(IP Connectivity Access Network) category is changed.
     *
     * @param apnType APN type
     *          {@link EApnType#mType}
     * @param ipcanCategory IPCAN category
     *          {@link IApn#IPCAN_CATEGORY_MOBILE}
     *          {@link IApn#IPCAN_CATEGORY_WLAN}
     */
    public void onIpcanCategoryChanged(int apnType, int ipcanCategory) {
        // no-op
    }

    /**
     * Notifies the application that status of handover between WWAN and WLAN is changed.
     *
     * @param handoverState status of handover progress
     *          {@link IApn#HANDOVER_UNKNOWN}
     *          {@link IApn#HANDOVER_STARTED}
     *          {@link IApn#HANDOVER_SUCCESS}
     *          {@link IApn#HANDOVER_FAILURE}
     * @param networkType network type that have connected.
     * In case of handover from LTE to IWLAN, networkType in HANDOVER_STARTED is LTE.
     * If handover fails, networkType in HANDOVER_FAILURE is LTE.
     * If handover is successful, networkType in HANDOVER_SUCCESS is IWLAN.
     *          {@link TelephonyManager#NETWORK_TYPE_LTE}
     *          {@link TelephonyManager#NETWORK_TYPE_IWLAN}
     *          {@link TelephonyManager#NETWORK_TYPE_NR}
     * @param failCause cause code when the handover fails
     *          {@link DataFailCause}
     */
    public void onHandoverInfoChanged(int handoverState, int networkType, int failCause) {
        // no-op
    }
}
