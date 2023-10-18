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
#ifndef _IURCS_MESSAGE_CONSTANT_H_
#define _IURCS_MESSAGE_CONSTANT_H_

#include "ImsMessageDef.h"

class IURcsMessageTerminateReason
{
public:
    static const IMS_SINT32 USER_ACTION = (0);
    static const IMS_SINT32 SESSION_LIMIT_REACHED = (1);
    static const IMS_SINT32 UNGRACEFUL_TERMINATION = (2);
};

class IURcsMessageFailureReason
{
public:
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_UNKNOWN = (0);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_DELEGATE_DEAD = (1);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_DELEGATE_CLOSED = (2);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_INVALID_START_LINE = (3);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_INVALID_HEADER_FIELDS = (4);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_INVALID_BODY_CONTENT = (5);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_INVALID_FEATURE_TAG = (6);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_TAG_NOT_ENABLED_FOR_DELEGATE = (7);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_NETWORK_NOT_AVAILABLE = (8);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_NOT_REGISTERED = (9);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_STALE_IMS_CONFIGURATION = (10);
    static const IMS_SINT32 MESSAGE_FAILURE_REASON_INTERNAL_DELEGATE_STATE_TRANSITION = (11);

    static const IMS_SINT32 MESSAGE_FAILURE_REASON_INVALID_PARAMETER = (12);
};

#endif  //_IURCS_MESSAGE_CONSTANT_H_
