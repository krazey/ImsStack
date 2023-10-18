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

#ifndef _IUCE_H_
#define _IUCE_H_
#include <stdio.h>
#include <string.h>

#include "AString.h"
#include "ImsList.h"
#include "ImsMessageDef.h"

#define UI2UCEAPP IMS_MSG_BASE_UCE  // 3000
#define UCEAPP2UI (IMS_MSG_BASE_UCE + 50)

class IUUceService
{
public:
    /*
     * Commands
     */
    // Publish Commands
    static const IMS_SINT32 UCE_SEND_PUBLISH_CMD = UI2UCEAPP + 1;
    // Subscribe Commands
    static const IMS_SINT32 UCE_SEND_SINGLE_SUBSCRIBE_CMD = UI2UCEAPP + 2;
    static const IMS_SINT32 UCE_SEND_LIST_SUBSCRIBE_CMD = UI2UCEAPP + 3;
    // Options Commands
    static const IMS_SINT32 UCE_SEND_OPTIONS_CMD = UI2UCEAPP + 4;
    static const IMS_SINT32 UCE_SEND_OPTIONS_RESP_CMD = UI2UCEAPP + 5;
    // Registration Commands
    static const IMS_SINT32 UCE_GET_IMS_REGISTRATION_CMD = UI2UCEAPP + 6;
    /*
     * Indications
     */
    // Publish Indication
    static const IMS_SINT32 UCE_PUBLISH_RESPONSE_IND = UCEAPP2UI + 1;
    static const IMS_SINT32 UCE_PUBLISH_UPDATED_IND = UCEAPP2UI + 2;
    static const IMS_SINT32 UCE_UNPUBLISHED_IND = UCEAPP2UI + 3;
    static const IMS_SINT32 UCE_PUBLISH_CMD_ERROR_IND = UCEAPP2UI + 4;

    // Subscribe Indication
    static const IMS_SINT32 UCE_SUBSCRIBE_RESPONSE_IND = UCEAPP2UI + 10;
    static const IMS_SINT32 UCE_PRESENCE_NOTIFY_IND = UCEAPP2UI + 11;
    static const IMS_SINT32 UCE_SUBSCRIBE_CMD_ERROR_IND = UCEAPP2UI + 12;
    static const IMS_SINT32 UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND = UCEAPP2UI + 13;
    static const IMS_SINT32 UCE_SUBSCRIBE_TERMINATED_IND = UCEAPP2UI + 14;

    // Options Indication
    static const IMS_SINT32 UCE_OPTIONS_RESPONSE_IND = UCEAPP2UI + 20;
    static const IMS_SINT32 UCE_OPTIONS_CMD_ERROR_IND = UCEAPP2UI + 21;
    static const IMS_SINT32 UCE_OPTIONS_RECEIVED_IND = UCEAPP2UI + 22;

    // connection Indication
    static const IMS_SINT32 UCE_IMS_AGENT_CONNECTED_IND = UCEAPP2UI + 30;
    static const IMS_SINT32 UCE_IMS_AGENT_DISCONNECTED_IND = UCEAPP2UI + 31;
    static const IMS_SINT32 UCE_IMS_AGENT_REFRESHED_IND = UCEAPP2UI + 32;
    static const IMS_SINT32 UCE_NETWORK_CHANGED = UCEAPP2UI + 33;

    static const IMS_SINT32 UCE_XML_PARSE_COMPLETED_IND = UCEAPP2UI + 34;
    static const IMS_SINT32 UCE_SUBSCRIBE_DELETED_IND = UCEAPP2UI + 35;
    static const IMS_SINT32 UCE_OPTIONS_DELETED_IND = UCEAPP2UI + 36;
    // command error
    /**
     * Service is unknown.
     */
    static const IMS_UINT32 COMMAND_CODE_SERVICE_UNKNOWN = 0;

    /**
     * The command failed with an unknown error.
     */
    static const IMS_UINT32 COMMAND_CODE_GENERIC_FAILURE = 1;

    /**
     * Invalid parameter(s).
     */
    static const IMS_UINT32 COMMAND_CODE_INVALID_PARAM = 2;

    /**
     * Fetch error.
     */
    static const IMS_UINT32 COMMAND_CODE_FETCH_ERROR = 3;

    /**
     * Request timed out.
     */
    static const IMS_UINT32 COMMAND_CODE_REQUEST_TIMEOUT = 4;

    /**
     * Failure due to insufficient memory available.
     */
    static const IMS_UINT32 COMMAND_CODE_INSUFFICIENT_MEMORY = 5;

    /**
     * Network connection is lost.
     */
    static const IMS_UINT32 COMMAND_CODE_LOST_NETWORK_CONNECTION = 6;

    /**
     * Requested feature/resource is not supported.
     */
    static const IMS_UINT32 COMMAND_CODE_NOT_SUPPORTED = 7;

    /**
     * Contact or resource is not found.
     */
    static const IMS_UINT32 COMMAND_CODE_NOT_FOUND = 8;

    /**
     * Service is not available.
     */
    static const IMS_UINT32 COMMAND_CODE_SERVICE_UNAVAILABLE = 9;

    /**
     * Command resulted in no change in state, ignoring.
     */
    static const IMS_UINT32 COMMAND_CODE_NO_CHANGE = 10;
};

typedef enum
{
    eUCE_RAT_INVALID = -1,
    eUCE_RAT_GERAN = 0,
    eUCE_RAT_HRPD,
    eUCE_RAT_UTRAN,
    eUCE_RAT_EHRPD,
    eUCE_RAT_LTE,
    eUCE_RAT_LTE_NO_VOPS,
    eUCE_RAT_WIFI,
    eUCE_RAT_NR,
    eUCE_RAT_NR_NO_VOPS
} UCE_NETWORK_ENTYPE;

class IUceTerminatedReason
{
public:
    inline IUceTerminatedReason() {}
    inline ~IUceTerminatedReason() {}

public:
    AString m_strContact;
    AString m_strReason;
};
#endif  //_IUCE_H_
