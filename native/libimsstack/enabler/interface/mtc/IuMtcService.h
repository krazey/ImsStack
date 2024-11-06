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

#ifndef INTERFACE_UI_MTC_SERVICE_H_
#define INTERFACE_UI_MTC_SERVICE_H_

#include "ImsMessageDef.h"
#include "ImsTypeDef.h"

class IuMtcService
{
public:
    static const IMS_SINT32 EVENT_U2I = IMS_MSG_BASE_SERVICE;
    static const IMS_SINT32 EVENT_I2U = IMS_MSG_BASE_SERVICE + 100;
    static const IMS_SINT32 MAXIMUM = (EVENT_I2U + 99);

    // --- Event : UI to IMS ----------------------------------------------------------------------
    static const IMS_SINT32 OPEN_SESSION = (EVENT_U2I + 1);
    static const IMS_SINT32 ATTCH_SESSION = (EVENT_U2I + 2);
    static const IMS_SINT32 CLOSE_SESSION = (EVENT_U2I + 3);
    static const IMS_SINT32 REGISTER_SERVICE = (EVENT_U2I + 4);
    static const IMS_SINT32 SRVCC_STATE_CHANGED = (EVENT_U2I + 5);
    static const IMS_SINT32 SET_TERMINAL_BASED_CALL_WAITING = (EVENT_U2I + 6);
    static const IMS_SINT32 OPEN_EMERGENCY_SERVICE = (EVENT_U2I + 7);
    static const IMS_SINT32 STOP_EMERGENCY_SERVICE = (EVENT_U2I + 8);
    static const IMS_SINT32 TEST_COMMAND = (EVENT_U2I + 9);

    // HO
    static const IMS_SINT32 HO_CONFIRM = (EVENT_U2I + 50 + 1);
    static const IMS_SINT32 HO_HANDOVER = (EVENT_U2I + 50 + 2);

    // --- Event : IMS to UI ----------------------------------------------------------------------
    static const IMS_SINT32 SERVICE_CHANGED = (EVENT_I2U + 1);
    static const IMS_SINT32 E_SERVICE_CHANGED = (EVENT_I2U + 2);
    static const IMS_SINT32 PRE_INCOMING_CALL = (EVENT_I2U + 3);
    static const IMS_SINT32 AUTO_REJECTED_CALL = (EVENT_I2U + 4);
    static const IMS_SINT32 JNI_READY = (EVENT_I2U + 5);
    static const IMS_SINT32 EXTERNAL_CALLS_CHANGED = (EVENT_I2U + 6);

    // HO
    static const IMS_SINT32 HO_CONFIRMED = (EVENT_I2U + 50 + 1);

    enum class ServiceState
    {
        SERVICE_NONE = 0,
        SERVICE_VOIP = 1,
        SERVICE_VT = 2,
        SERVICE_UC = 3,
        SERVICE_EMERGENCY = 4,
        SERVICE_OPENING = 5,
    };

    enum class EmergencyServiceState
    {
        IDLE = 0,
        OPENING = 1,
        OPENED = 2,
        UNAVAILABLE = 3,
    };

    enum class EmergencyCallRoutingPdn
    {
        UNKNOWN,
        EMERGENCY,
        NORMAL
    };

    enum
    {
        SERVICESTATUS_REASON_UNKNOWN = 0,
        SERVICESTATUS_REASON_NETWORKDISABLE = 1,
        SERVICESTATUS_REASON_SIMSINVALID = 2,
        SERVICESTATUS_REASON_BYSERVER = 3,
        SERVICESTATUS_REASON_USERSELECT = 4,
        SERVICESTATUS_REASON_FORBIDDEN = 5,
    };

    enum
    {
        ES_IDLE_REASON_UNKNOWN = -1,
        ES_IDLE_REASON_NONE = 0,
        ES_IDLE_REASON_WITH_ECM = 1,
    };

    enum
    {
        ES_UNAVAILABLE_REASON_UNKNOWN = -1,
        ES_UNAVAILABLE_REASON_NONE = 0,
        ES_UNAVAILABLE_REASON_NO_CSFB = 1,
        ES_UNAVAILABLE_REASON_SSAC = 2
    };
};

#endif
