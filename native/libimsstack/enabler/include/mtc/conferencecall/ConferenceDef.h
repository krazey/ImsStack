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

#ifndef CONFERENCE_DEF_H_
#define CONFERENCE_DEF_H_

#include "MtcDef.h"

//
enum
{
    REFERENCE_TYPE_INVALID = -1,
    REFERENCE_TYPE_INVITE = 0,
    REFERENCE_TYPE_BYE = 1
};

enum
{
    CONTROL_OPERATION_NONE = 0,
    CONTROL_OPERATION_CREATE_CONFERENCE_SESSION = 1,
    CONTROL_OPERATION_SUBSCRIBE = 2,
    CONTROL_OPERATION_UNSUBSCRIBE = 3,
    CONTROL_OPERATION_REFER_INVITE = 4,
    CONTROL_OPERATION_REFER_BYE = 5,
    CONTROL_OPERATION_CHECK_CONNECTED = 6,
    CONTROL_OPERATION_NOTIFY_RESULT_TO_UI = 7,
    CONTROL_OPERATION_TERMINATE_1TO1_SESSION = 8,
    CONTROL_OPERATION_TERMINATE_CONFERENCE = 9,
    CONTROL_OPERATION_DESTROY_CONTROLLER = 10,
    CONTROL_OPERATION_NOTIFY_RESULT_TO_UCSESSION = 11
};

enum
{
    STATUS_IDLE = 0,
    STATUS_PROGRESSING = 1,
    STATUS_CONNECTED = 2,
    STATUS_DISCONNECTED = 3,
    STATUS_ON_HOLD = 4,
    STATUS_MUTED_VIA_FOCUS = 5,
    STATUS_PENDING = 6,
    STATUS_ALERTING = 7,
    STATUS_DIALING_IN = 8,
    STATUS_DIALING_OUT = 9,
    STATUS_DISCONNECTING = 10,

    STATUS_FAIL = 20,
    STATUS_REJECT = 21,
    STATUS_BUSY = 22,
    STATUS_SERVERERROR = 23,
    STATUS_NOTSUPPORTED = 24,
    STATUS_NOTACCEPTABLE = 25,
    STATUS_NOANSWER = 26,
    STATUS_NOTREACHABLE = 27,
    STATUS_LOWBATTERY = 28,
    STATUS_FORBIDDEN = 29,
    STATUS_INTSERVERERROR = 30
};

enum
{
    CONF_MEDIA_TYPE_AUDIO = 0,
    CONF_MEDIA_TYPE_VIDEO = 1,
    CONF_MEDIA_TYPE_TEXT = 2  // No ???
};

enum
{
    CONF_MEDIA_STATUS_SENDRECV = 0,
    CONF_MEDIA_STATUS_SENDONLY = 1,
    CONF_MEDIA_STATUS_RECVONLY = 2,
    CONF_MEDIA_STATUS_INVALID = 3
};

enum
{
    CONF_SUBSCRIPTION_DIALOG_TYPE_IN = 0,
    CONF_SUBSCRIPTION_DIALOG_TYPE_OUT = 1,
    CONF_SUBSCRIPTION_DIALOG_TYPE_FALLBACK = 2  // IN failed, so OUT is being used.
};

struct ConfUser
{
public:
    inline ConfUser() :
            nConnectionId(0),
            strTarget(AString::ConstNull()),
            strUserEntity(AString::ConstNull()),
            strEpEntity(AString::ConstNull()),
            strDisplayName(AString::ConstNull()),
            eStatus(STATUS_IDLE),
            eStatusCode(-1),
            eCcType(COPYCONTROLTYPE_TO),
            bAnonymize(IMS_FALSE)
    {
    }
    inline ConfUser(IN const ConfUser& objRhs) :

            nConnectionId(objRhs.nConnectionId),
            strTarget(objRhs.strTarget),
            strUserEntity(objRhs.strUserEntity),
            strEpEntity(objRhs.strEpEntity),
            strDisplayName(objRhs.strDisplayName),
            eStatus(objRhs.eStatus),
            eStatusCode(objRhs.eStatusCode),
            eCcType(objRhs.eCcType),
            bAnonymize(objRhs.bAnonymize)
    {
    }
    inline ~ConfUser() {}

    ConfUser& operator=(IN const ConfUser&) = delete;

public:
    // connection id for a specific MtcCall
    IMS_UINT32 nConnectionId;
    // Phone Number for User Paricinpant - ex) Join
    AString strTarget;
    // Main Key after subscription for confernece from NOTIFY
    AString strUserEntity;
    // from NOTIFY about Conference Event package
    AString strEpEntity;
    // from NOTIFY about Conference Event package, by converting the operator's requirement
    AString strDisplayName;
    // Main Information from NOTIFY about Conference Event package
    IMS_UINT32 eStatus;
    // the detail code for eStatus
    IMS_SINT32 eStatusCode;
    // from NOTIFY about Conference Event package
    IMS_UINT32 eCcType;
    // from NOTIFY about Conference Event package
    IMS_BOOL bAnonymize;
};

#endif
