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

        /// Indicate that power is down.
        POWER_OFF = 10000,
        /// Indicate that airplane mode is on.
        AIRPLANE_MODE,
        /// Indicate that wifi is off.
        WIFI_OFF,
        /// Indicate that IMS services are not available based on network feature or something.
        SERVICE_POLICY,
        /// Indicate that data connection is disconnected.
        DATA_DISCONNECTED,
        /// Indicate that the data connection is rejected permanently.
        DATA_PERMANENTLY_FAILED,
        /// Indicate that registration is terminated with the specific causes.
        /// Indicate that the initial registration is tried soon.
        REG_TERMINATED,
        /// Indicate that initial registration was required. the reason is not specified.
        REG_NEW_REQUIRED,
        /// Indicate that registration is terminating.
        /// So services may send a BYE message to terminate the session if there is on a session.
        REG_TERMINATING,
        /// Indicate that IP changed during registration.
        IP_CHANGED,
        /// Indicate that the network attach is rejected because the UICC is invalid.
        NETWORK_ATTACH_REJECTED,
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
