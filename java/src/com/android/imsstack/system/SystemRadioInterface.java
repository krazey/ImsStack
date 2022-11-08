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

package com.android.imsstack.system;

/**
 * This provides the system radio interface for interworking with IMS radio interface.
 */
public interface SystemRadioInterface {
    /** Result code of execution with no error. */
    int RESULT_OK = 1;
    /** Result code of execution with a specific error. */
    int RESULT_ERROR = -1;

    /**
     * Indicates NAS and RRC layers of the modem that the upcoming IMS traffic is
     * for the service mentioned in the traffic type.
     *
     * @param id The identification for IMS traffic
     * @param trafficType The type for IMS traffic
     * @param accessNetworkType The type for radio access network type
     * @param direction The direction for IMS traffic
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    int startImsTraffic(int id, int trafficType, int accessNetworkType, int direction);

    /**
     * Indicates IMS traffic has been stopped. For all IMS traffic,
     * notified with startImsTraffic, IMS service shall notify stopImsTraffic
     * when it completes the traffic. The reference listener corresponding to id is removed.
     *
     * @param id The identification to be removed
     */
    void stopImsTraffic(int id);

    /**
     * Triggers the EPS fallback procedure by UE for the case where the user is trying to
     * place a voice call in NR network and the voice call is not established
     * within several seconds.
     *
     * @param reason Specifies the reason that causes EPS fallback
     * @return One of {@link #RESULT_ERROR} or {@link #RESULT_OK}.
     */
    int triggerEpsFallback(int reason);
}
