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
#ifndef IMS_AOS_REASON_H_
#define IMS_AOS_REASON_H_

/**
 * @brief This class provides a certain reason for enabler.
 *
 * The ImsAosReason is to indicate the reason that results in disconnected, disconnecting or
 * suspending the dedicated service.
 *
 * @see IImsAosListener
 */

class ImsAosReason
{
public:
    /// Indicate the reason that contains as argument in the APIs of the IImsAosListener
    enum
    {
        /// Indicate that there is no reason.
        NONE = 0,

        /// Indicate that service is out-of-service.
        OUT_OF_SERVICE = 10000,  // SRV_OUT
                                 /// Indicate that power is down.
        POWER_OFF,
        /// Indicate that the current RAT is not supported in IMS services. e.g.) not LTE/NR/WLAN
        NO_RAT_COVERAGE,  // NO_LTE_COVERAGE
                          /// Indicate that IMS services are not available based on network feature
                          /// or something
        SERVICE_POLICY,
        /// Indicate that service is blocked temporarily. After IMS call is terminated,
        /// the initial registration is tried.
        SERVICE_BLOCKED,
        /// Indicate that data connection is disconnected.
        DATA_DISCONNECTED,
        /// Indicate that registration is terminated with the specific causes.
        REG_TERMINATED,  // REG_REFRESH_FORBIDDEN, REG_TERMINATED_EXPIRE
                         /// Indicate that the initial registration is tried soon.
        REG_NEW_REQUIRED,  // INITIAL_REG_REQUESTED
                           /// Indicate that the reason is not specified.
        NOT_SPECIFIED
    };

    /// Indicate the reason that contains as argument in the IImsAosListener#ImsAos_Suspended()
    enum
    {
        /// Indicate that there is no suspended reason.
        SUSPEND_NONE = 0,

        /// Indicate that service is out-of-service.
        SUSPEND_OUT_OF_SERVICE = 0x0001,
        /// Indicate that the current RAT is not supported in IMS services. e.g.) not LTE/NR/WLAN
        SUSPEND_NO_RAT_COVERAGE = 0x0002  // SUSPEND_NO_LTE_COVERAGE
    };
};
#endif  // IMS_AOS_REASON_H_
