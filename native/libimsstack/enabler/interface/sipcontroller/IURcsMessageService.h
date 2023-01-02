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
#include "ByteArray.h"
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#define IMS_SOLUTION_MSG_SOURCE_LEN 64
#define IMS_SOLUTION_URI_LEN        128
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

class IUSncService
{
public:
    static const IMS_SINT32 OPENMESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 1);
    static const IMS_SINT32 CLOSEMESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 2);

    static const IMS_SINT32 SENDMESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 11);
    static const IMS_SINT32 CLOSESESSION_CMD = (IMS_MSG_SIP_DELEGATE + 12);
    static const IMS_SINT32 NOTIFYMESSAGERECEIVEERROR_CMD = (IMS_MSG_SIP_DELEGATE + 13);
    static const IMS_SINT32 UPDATEDELEGATEREGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 14);

    static const IMS_SINT32 MESSAGERECEIVED_IND = (IMS_MSG_SIP_DELEGATE + 21);
    static const IMS_SINT32 MESSAGESENT_IND = (IMS_MSG_SIP_DELEGATE + 22);
    static const IMS_SINT32 SENDMESSAGEFAILURE_IND = (IMS_MSG_SIP_DELEGATE + 23);
};

class IUSncControl
{
public:
    static const IMS_SINT32 REGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 100);

    // SEND CONTROL COMMAND
    static const IMS_SINT32 UPDATESIPREGISTRATION_CMD = (REGISTRATION_CMD + 1);
    static const IMS_SINT32 TRIGGERSIPDEREGISTRATION_CMD = (REGISTRATION_CMD + 2);

    // RECEIVE CONTROL COMMAND
    static const IMS_SINT32 ONREGISTRATIONUPDATED_IND = (REGISTRATION_CMD + 11);
    static const IMS_SINT32 ONCONFIGURATIONUPDATED_IND = (REGISTRATION_CMD + 21);
};

class IUSncSessionData
{
public:
    inline IUSncSessionData()
    {
        nSessionID = 0;
        szThread[0] = '\0';
    }
    inline ~IUSncSessionData(){};

public:
    IMS_UINTP nSessionID;
    IMS_CHAR szThread[IMS_SOLUTION_MSG_SOURCE_LEN + 1];
};

class IUSncSendMessageParam : public IUSncSessionData
{
public:
    inline IUSncSendMessageParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncSendMessageParam = %" PFLS_u,
                sizeof(IUSncSendMessageParam), 0, 0);
        pszStartLine = IMS_NULL;
        pszHeaderSection = IMS_NULL;
        nContentLength = 0;
        pszContent = IMS_NULL;
        pszMethod = IMS_NULL;
        pszFromParameter = IMS_NULL;
        pszToParameter = IMS_NULL;
        nType = 0;
    }
    inline ~IUSncSendMessageParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncSendMessageParam = %" PFLS_u,
                sizeof(IUSncSendMessageParam), 0, 0);
        if (pszStartLine != IMS_NULL)
        {
            IMS_MEM_Free(pszStartLine);
        }
        if (pszHeaderSection != IMS_NULL)
        {
            IMS_MEM_Free(pszHeaderSection);
        }
        if (pszContent != IMS_NULL)
        {
            IMS_MEM_Free(pszContent);
        }
        if (pszMethod != IMS_NULL)
        {
            IMS_MEM_Free(pszMethod);
        }
        if (pszFromParameter != IMS_NULL)
        {
            IMS_MEM_Free(pszFromParameter);
        }
        if (pszToParameter != IMS_NULL)
        {
            IMS_MEM_Free(pszToParameter);
        }
    }

public:
    IMS_CHAR* pszStartLine;
    IMS_CHAR* pszHeaderSection;
    IMS_SINT32 nContentLength;
    IMS_CHAR* pszContent;
    IMS_CHAR* pszMethod;
    IMS_CHAR* pszFromParameter;
    IMS_CHAR* pszToParameter;
    IMS_SINT32 nType;
};

class IUSncMessageParam : public IUSncSessionData
{
public:
    inline IUSncMessageParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_M : IUSncMessageParam = %" PFLS_u, sizeof(IUSncMessageParam), 0, 0);
        pszStartLine = IMS_NULL;
        pszHeaderSection = IMS_NULL;
        nContentLength = 0;
        pszContent = IMS_NULL;
    }
    inline ~IUSncMessageParam()
    {
        IMS_TRACE_MEM(
                "SNC_MSG", "IM_F : IUSncMessageParam = %" PFLS_u, sizeof(IUSncMessageParam), 0, 0);
        if (pszStartLine != IMS_NULL)
        {
            IMS_MEM_Free(pszStartLine);
        }
        if (pszHeaderSection != IMS_NULL)
        {
            IMS_MEM_Free(pszHeaderSection);
        }
        if (pszContent != IMS_NULL)
        {
            IMS_MEM_Free(pszContent);
        }
    }

public:
    IMS_CHAR* pszStartLine;
    IMS_CHAR* pszHeaderSection;
    IMS_SINT32 nContentLength;
    IMS_CHAR* pszContent;
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
        szCallId[0] = '\0';
    }
    inline ~IUSncCloseSessionCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncCloseSessionCmdParam = %" PFLS_u,
                sizeof(IUSncCloseSessionCmdParam), 0, 0);
    }

public:
    IMS_CHAR szCallId[IMS_SOLUTION_URI_LEN + 1];
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
        szTId[0] = '\0';
    }
    inline ~IUSncNotifyErrorCmdParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncNotifyErrorCmdParam = %" PFLS_u,
                sizeof(IUSncNotifyErrorCmdParam), 0, 0);
    }

public:
    IMS_CHAR szTId[IMS_SOLUTION_URI_LEN + 1];
};

class IUSncSentMessageIndParam : public IUSncSessionData
{
public:
    inline IUSncSentMessageIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncSentMessageIndParam = %" PFLS_u,
                sizeof(IUSncSentMessageIndParam), 0, 0);
        szTId[0] = '\0';
    }
    inline ~IUSncSentMessageIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncSentMessageIndParam = %" PFLS_u,
                sizeof(IUSncSentMessageIndParam), 0, 0);
    }

public:
    IMS_CHAR szTId[IMS_SOLUTION_URI_LEN + 1];
};

class IUSncSendFailureIndParam : public IUSncSessionData
{
public:
    inline IUSncSendFailureIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_M : IUSncSendFailureIndParam = %" PFLS_u,
                sizeof(IUSncSendFailureIndParam), 0, 0);
        nReason = 0;
        szTId[0] = '\0';
    }
    inline ~IUSncSendFailureIndParam()
    {
        IMS_TRACE_MEM("SNC_MSG", "IM_F : IUSncSendFailureIndParam = %" PFLS_u,
                sizeof(IUSncSendFailureIndParam), 0, 0);
    }

public:
    IMS_SINT32 nReason;
    IMS_CHAR szTId[IMS_SOLUTION_URI_LEN + 1];
};
#endif  //_IUSncService_H_