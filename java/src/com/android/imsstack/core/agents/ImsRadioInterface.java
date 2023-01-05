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

import android.annotation.IntDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * This provides IMS radio interface for notifying the IMS traffic activities to modem,
 * getting NAS/RRC connection setup result details from modem and triggering EPS fallback
 * to modem.
 */
public interface ImsRadioInterface extends IAgent {
    /**
     * This class provides the notifications for connection setup result.
     */
    public interface ConnectionListener {
        /**
         * Notifies the reason of the connection setup corresponding with the IMS traffic type.
         *
         * @param failureReason The reason of connection failure based on IMS traffic type
         *                      (@see REASON_XXX)
         * @param causeCode Failure cause code from network or modem specific to the failure
         * @param waitTimeMillis Retry wait time provided by network in milliseconds
         */
        void onConnectionFailed(@Reason int failureReason, int causeCode, int waitTimeMillis);

        /**
         * Notifies the preparation of the connection setup corresponding
         * with the IMS traffic type.
         */
        void onConnectionSetupPrepared();
    }

    /**
     * This class provides the notifications for the change of IMS traffic priority.
     */
    public interface TrafficPriorityListener {
        /**
         * Notifies that IMS traffic priority is changed for DSDS. If IMS traffic is pending
         * due to RF_BUSY or IMS traffic priority. It can be checked again
         * using IsImsTrafficPriorityAvailable after this is notified.
         */
        void onTrafficPriorityChanged();
    }

    /**
     * IMS traffic type
     */
    int TRAFFIC_TYPE_EMERGENCY = 1;
    int TRAFFIC_TYPE_EMERGENCY_SMS = 2;
    int TRAFFIC_TYPE_VOICE = 3;
    int TRAFFIC_TYPE_VIDEO = 4;
    int TRAFFIC_TYPE_SMS = 5;
    int TRAFFIC_TYPE_REGISTRATION = 6;
    int TRAFFIC_TYPE_UT_XCAP = 7;

    @IntDef(value = {
        TRAFFIC_TYPE_EMERGENCY,
        TRAFFIC_TYPE_EMERGENCY_SMS,
        TRAFFIC_TYPE_VOICE,
        TRAFFIC_TYPE_VIDEO,
        TRAFFIC_TYPE_SMS,
        TRAFFIC_TYPE_REGISTRATION,
        TRAFFIC_TYPE_UT_XCAP
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface TrafficType {}

    /**
     * Radio access network type
     */
    int ACCESS_NETWORK_TYPE_UNKNOWN = 0;
    int ACCESS_NETWORK_TYPE_UTRAN = 1;
    int ACCESS_NETWORK_TYPE_EUTRAN = 2;
    int ACCESS_NETWORK_TYPE_NGRAN = 3;
    int ACCESS_NETWORK_TYPE_IWLAN = 4;

    @IntDef(value = {
        ACCESS_NETWORK_TYPE_UNKNOWN,
        ACCESS_NETWORK_TYPE_UTRAN,
        ACCESS_NETWORK_TYPE_EUTRAN,
        ACCESS_NETWORK_TYPE_NGRAN,
        ACCESS_NETWORK_TYPE_IWLAN
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AccessNetworkType {}

    /**
     * IMS traffic direction
     */
    int DIRECTION_MO = 0;
    int DIRECTION_MT = 1;

    @IntDef(value = {
        DIRECTION_MO,
        DIRECTION_MT
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface Direction {}

    /**
     * Connection failure reason
     */
    /** Access class check failed */
    int REASON_ACCESS_DENIED = 1;
    /** 3GPP Non-access stratum failure */
    int REASON_NAS_FAILURE = 2;
    /** Random access failure */
    int REASON_RACH_FAILURE = 3;
    /** Radio link failure */
    int REASON_RLC_FAILURE = 4;
    /** Radio connection establishment rejected by network */
    int REASON_RRC_REJECT = 5;
    /** Radio connection establishment timed out */
    int REASON_RRC_TIMEOUT = 6;
    /** Device currently not in service */
    int REASON_NO_SERVICE = 7;
    /** The PDN is no more active */
    int REASON_PDN_NOT_AVAILABLE = 8;
    /** Radio resource is busy with another subscription */
    int REASON_RF_BUSY = 9;
    /** Internal Error */
    int REASON_INTERNAL_ERROR = 10;

    @IntDef(value = {
        REASON_ACCESS_DENIED,
        REASON_NAS_FAILURE,
        REASON_RACH_FAILURE,
        REASON_RLC_FAILURE,
        REASON_RRC_REJECT,
        REASON_RRC_TIMEOUT,
        REASON_NO_SERVICE,
        REASON_PDN_NOT_AVAILABLE,
        REASON_RF_BUSY,
        REASON_INTERNAL_ERROR
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface Reason {}

    /**
     * Indicates whether Ims traffic is available or not after checking the traffic priority
     * within the IMS stack for DSDS in advance. There is no interworking with modem.
     *
     * @param trafficType The type for IMS traffic (@see TRAFFIC_TYPE_XXX)
     *
     * @return Returns the IMS traffic availability
     */
    boolean isImsTrafficAllowed(@TrafficType int trafficType);

    /**
     * Indicates NAS and RRC layers of the modem that the upcoming IMS traffic is
     * for the service mentioned in the TRAFFIC_TYPE_XXX
     *
     * @param trafficType The type for IMS traffic (@see TRAFFIC_TYPE_XXX)
     * @param accessNetworkType The type for radio access network type
     *                          (@see ACCESS_NETWORK_TYPE_XXX)
     * @param direction The direction for IMS traffic (@see DIRECTION_XXX)
     * @param listener The listener to be added. It will be overwritten if multiple times
     *                 are invoked with the same type.
     */
    void startImsTraffic(@TrafficType int trafficType, @AccessNetworkType int accessNetworkType,
            @Direction int direction, ConnectionListener listener);

    /**
     * Indicates IMS traffic has been stopped. For all IMS traffic,
     * notified with startImsTraffic, IMS service shall notify stopImsTraffic
     * when it completes the traffic. The reference listener registered from startImsTraffic()
     * is removed.
     *
     * @param listener The listener to be removed
     */
    void stopImsTraffic(ConnectionListener listener);

    /**
     * Adds the listener to be notified when the traffic priority is changed
     *
     * @param listener The listener to be added
     */
    void addListenerForTrafficPriority(TrafficPriorityListener listener);

    /**
     * Removes the listener to be notified when the traffic priority is changed
     *
     * @param listener The listener to be removed
     */
    void removeListenerForTrafficPriority(TrafficPriorityListener listener);
}
