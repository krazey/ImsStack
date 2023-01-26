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

#ifndef INTERFACE_UI_SIP_CONTROLLER_SERVICEH_
#define INTERFACE_UI_SIP_CONTROLLER_SERVICEH_

#include "AString.h"
#include "AStringArray.h"
#include "ByteArray.h"
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#define INTERNAL_RCS_IM             (IMS_MSG_USER + 5000)  // 105000

enum IMINTERNALMSG
{
    SERVICE_START = INTERNAL_RCS_IM,
    SERVICE_STOP,
    SERVICE_PENDING,
    SEND_QUEUED_MESSAGE,

    MESSAGE_STARTED,
    MESSAGE_STARTFAILED,
    MESSAGE_RECEIVED,
    MESSAGE_RECEIVE_FAILED,
    MESSAGE_SENDFAILED,
    MESSAGE_SENT,
    MESSAGE_TERMINATED,
    SESSION_CLOSED,
    TIMER_EXPIRED,
};

enum class RcsRegState
{
    STATE_DEREGISTERING = 0,
    STATE_DEREGISTERED,
    STATE_REGISTERING,
    STATE_REGISTERED
};

enum class RcsDeRegReason
{
    REASON_UNKNOWN = 0,
    REASON_NOT_PROVISIONED,
    REASON_NOT_REGISTERED,
    REASON_PDN_CHANGE,
    REASON_PROVISIONING_CHANGE,
    REASON_FEATURE_TAGS_CHANGING,
    REASON_DESTROY_PENDING,
    REASON_LOSING_PDN,
    REASON_UNSPECIFIED
};

class IUSncService
{
public:
    static const IMS_SINT32 OPEN_MESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 1);
    static const IMS_SINT32 CLOSE_MESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 2);

    static const IMS_SINT32 SEND_MESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 11);
    static const IMS_SINT32 CLOSE_SESSION_CMD = (IMS_MSG_SIP_DELEGATE + 12);
    static const IMS_SINT32 NOTIFY_MESSAGE_RECEIVE_ERROR_CMD = (IMS_MSG_SIP_DELEGATE + 13);

    static const IMS_SINT32 MESSAGE_RECEIVED_IND = (IMS_MSG_SIP_DELEGATE + 21);
    static const IMS_SINT32 MESSAGE_SENT_IND = (IMS_MSG_SIP_DELEGATE + 22);
    static const IMS_SINT32 SEND_MESSAGE_FAILURE_IND = (IMS_MSG_SIP_DELEGATE + 23);
};

class IUSncControl
{
public:
    // SEND CONTROL COMMAND
    static const IMS_SINT32 UPDATE_SIPREGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 101);
    static const IMS_SINT32 TRIGGER_SIPDEREGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 102);

    // RECEIVE CONTROL COMMAND
    static const IMS_SINT32 ONREGISTRATION_UPDATED_IND = (IMS_MSG_SIP_DELEGATE + 111);
    static const IMS_SINT32 ONCONFIGURATION_UPDATED_IND = (IMS_MSG_SIP_DELEGATE + 121);
};

class IUSncSessionData
{
public:
    inline IUSncSessionData() { m_nSessionID = 0; }
    inline ~IUSncSessionData(){};

public:
    IMS_UINTP m_nSessionID;
    AString m_strThread;
};

class IUSncSendMessageParam : public IUSncSessionData
{
public:
    inline IUSncSendMessageParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncSendMessageParam = %" PFLS_u,
                sizeof(IUSncSendMessageParam), 0, 0);
        m_nContentLength = 0;
        m_nType = 0;
    }
    inline ~IUSncSendMessageParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncSendMessageParam = %" PFLS_u,
                sizeof(IUSncSendMessageParam), 0, 0);
    }

public:
    AString m_strStartLine;
    AString m_strHeaderSection;
    IMS_SINT32 m_nContentLength;
    AString m_strContent;
    AString m_strMethod;
    AString m_strFromParameter;
    AString m_strToParameter;
    IMS_SINT32 m_nType;
};

class IUSncMessageParam : public IUSncSessionData
{
public:
    inline IUSncMessageParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_M : IUSncMessageParam = %" PFLS_u, sizeof(IUSncMessageParam), 0, 0);
        m_nContentLength = 0;
    }
    inline ~IUSncMessageParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_F : IUSncMessageParam = %" PFLS_u, sizeof(IUSncMessageParam), 0, 0);
    }

public:
    AString m_strStartLine;
    AString m_strHeaderSection;
    IMS_SINT32 m_nContentLength;
    AString m_strContent;
};

class IUSncOpenCmdParam : public IUSncSessionData
{
public:
    inline IUSncOpenCmdParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_M : IUSncOpenCmdParam = %" PFLS_u, sizeof(IUSncOpenCmdParam), 0, 0);
    }
    inline ~IUSncOpenCmdParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_F : IUSncOpenCmdParam = %" PFLS_u, sizeof(IUSncOpenCmdParam), 0, 0);
    }
};

class IUSncCloseCmdParam : public IUSncSessionData
{
public:
    inline IUSncCloseCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncCloseCmdParam = %" PFLS_u, sizeof(IUSncCloseCmdParam),
                0, 0);
    }
    inline ~IUSncCloseCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncCloseCmdParam = %" PFLS_u, sizeof(IUSncCloseCmdParam),
                0, 0);
    }
};

class IUSncCloseSessionCmdParam : public IUSncSessionData
{
public:
    inline IUSncCloseSessionCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncCloseSessionCmdParam = %" PFLS_u,
                sizeof(IUSncCloseSessionCmdParam), 0, 0);
    }
    inline ~IUSncCloseSessionCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncCloseSessionCmdParam = %" PFLS_u,
                sizeof(IUSncCloseSessionCmdParam), 0, 0);
    }

public:
    AString m_strCallId;
};

class IUSncSendCmdParam : public IUSncSessionData
{
public:
    inline IUSncSendCmdParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_M : IUSncSendCmdParam = %" PFLS_u, sizeof(IUSncSendCmdParam), 0, 0);
    }
    inline ~IUSncSendCmdParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_F : IUSncSendCmdParam = %" PFLS_u, sizeof(IUSncSendCmdParam), 0, 0);
    }
};

class IUSncNotifyErrorCmdParam : public IUSncSessionData
{
public:
    inline IUSncNotifyErrorCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncNotifyErrorCmdParam = %" PFLS_u,
                sizeof(IUSncNotifyErrorCmdParam), 0, 0);
    }
    inline ~IUSncNotifyErrorCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncNotifyErrorCmdParam = %" PFLS_u,
                sizeof(IUSncNotifyErrorCmdParam), 0, 0);
    }

public:
    AString m_strTId;
};

class IUSncFeatureTagsParam : public IUSncSessionData
{
public:
    inline IUSncFeatureTagsParam() :
            m_objFeatureTags()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncFeatureTagsParam = %" PFLS_u,
                sizeof(IUSncFeatureTagsParam), 0, 0);
        m_nFeatureCount = 0;
        m_nRegState = 0;
        m_nReason = 0;
    }
    inline ~IUSncFeatureTagsParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncFeatureTagsParam = %" PFLS_u,
                sizeof(IUSncFeatureTagsParam), 0, 0);
    }

public:
    AStringArray m_objFeatureTags;
    IMS_SINT32 m_nFeatureCount;
    IMS_SINT32 m_nRegState;
    IMS_SINT32 m_nReason;
};

class IUSncTriggerDeregistrationParam : public IUSncSessionData
{
public:
    inline IUSncTriggerDeregistrationParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncTriggerDeregistrationParam = %" PFLS_u,
                sizeof(IUSncTriggerDeregistrationParam), 0, 0);
    }
    inline ~IUSncTriggerDeregistrationParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncTriggerDeregistrationParam = %" PFLS_u,
                sizeof(IUSncTriggerDeregistrationParam), 0, 0);
    }
};

class IUSncSentMessageIndParam : public IUSncSessionData
{
public:
    inline IUSncSentMessageIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncSentMessageIndParam = %" PFLS_u,
                sizeof(IUSncSentMessageIndParam), 0, 0);
    }
    inline ~IUSncSentMessageIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncSentMessageIndParam = %" PFLS_u,
                sizeof(IUSncSentMessageIndParam), 0, 0);
    }

public:
    AString m_strTId;
};

class IUSncSendFailureIndParam : public IUSncSessionData
{
public:
    inline IUSncSendFailureIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncSendFailureIndParam = %" PFLS_u,
                sizeof(IUSncSendFailureIndParam), 0, 0);
        m_nReason = 0;
    }
    inline ~IUSncSendFailureIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncSendFailureIndParam = %" PFLS_u,
                sizeof(IUSncSendFailureIndParam), 0, 0);
    }

public:
    IMS_SINT32 m_nReason;
    AString m_strTId;
};
#endif  //_IUSncService_H_