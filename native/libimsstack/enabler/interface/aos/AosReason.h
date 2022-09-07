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
#ifndef AOS_REASON_H_
#define AOS_REASON_H_

class AosReason
{
public:
    enum
    {
        NONE = 0,

        SRV_OUT = 1,
        POWER_OFF,
        BAD_BATTERY,
        AIRPLANE_MODE,
        NO_LTE_COVERAGE,
        SERVICE_POLICY,
        SERVICE_BLOCKED,
        IMS_DISABLED,
        TTYMODEON,
        NOT_SPECIFIED,

        IP_CHANGED = 20,
        DATA_DISCONNECTED,
        DATA_CONNECTION_MAINTAIN,
        DATA_PERMANENTLY_FAILED,

        REG_FAILURE = 30,
        REG_REFRESH_FORBIDDEN,
        REG_TERMINATED,
        REG_TERMINATED_EXPIRE,
        INITIAL_REG_REQUESTED,
        PCSCF_DISCOVERY_FAILED,

        UNKNOWN,
    };

    // Flags for suspend reason
    enum
    {
        SUSPEND_NONE = 0,

        SUSPEND_NO_SERVICE = 0x0001,
        SUSPEND_CS_CALL = 0x0002,
        SUSPEND_LOW_BATTERY = 0x0004,
        SUSPEND_NO_LTE_COVERAGE = 0x0008,
        SUSPEND_INSTANTANEOUS_OFFLINE = 0x0010
    };

    //
    enum
    {
        // UC to Aos - Registration request reason
        REQUEST_NONE = 0,
        REQUEST_OFFLINE
    };
};
#endif  // AOS_REASON_H_
