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
#include "ImsMessageDef.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#define IMS_SOLUTION_MSG_SOURCE_LEN 64

class IUSipControllerService
{
public:
    static const IMS_SINT32 OPENMESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 1);
    static const IMS_SINT32 CLOSEMESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 2);

    static const IMS_SINT32 SENDMESSAGE_CMD = (IMS_MSG_SIP_DELEGATE + 11);
    static const IMS_SINT32 CLOSEONGOINGSESSION_CMD = (IMS_MSG_SIP_DELEGATE + 12);
    static const IMS_SINT32 NOTIFYMESSAGERECEIVEERROR_CMD = (IMS_MSG_SIP_DELEGATE + 13);
    static const IMS_SINT32 UPDATEDELEGATEREGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 14);

    static const IMS_SINT32 MESSAGERECEIVED_IND = (IMS_MSG_SIP_DELEGATE + 21);
    static const IMS_SINT32 MESSAGESENT_IND = (IMS_MSG_SIP_DELEGATE + 22);
    static const IMS_SINT32 SENDMESSAGEFAILURE_IND = (IMS_MSG_SIP_DELEGATE + 23);
};

class IUMessageParam
{
public:
    inline IUMessageParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_M : IUMessageParam = %" PFLS_u, sizeof(IUMessageParam), 0, 0);
        pszStartLine = IMS_NULL;
        pszHeaderSection = IMS_NULL;
        nContentLength = 0;
        pszContent = IMS_NULL;
        pszViaBranchParameter = IMS_NULL;
        pszCallIdParameter = IMS_NULL;
    }
    inline ~IUMessageParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_F : IUMessageParam = %" PFLS_u, sizeof(IUMessageParam), 0, 0);
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
        if (pszViaBranchParameter != IMS_NULL)
        {
            IMS_MEM_Free(pszViaBranchParameter);
        }
        if (pszCallIdParameter != IMS_NULL)
        {
            IMS_MEM_Free(pszCallIdParameter);
        }
    }

public:
    IMS_CHAR* pszStartLine;
    IMS_CHAR* pszHeaderSection;
    IMS_CHAR nContentLength;
    IMS_CHAR* pszContent;
    IMS_CHAR* pszViaBranchParameter;
    IMS_CHAR* pszCallIdParameter;
};

class IUSipControllerOpenCmdParam
{
public:
    inline IUSipControllerOpenCmdParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_M : IUSipControllerOpenCmdParam = %" PFLS_u,
                sizeof(IUSipControllerOpenCmdParam), 0, 0);
    }
    inline ~IUSipControllerOpenCmdParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_F : IUSipControllerOpenCmdParam = %" PFLS_u,
                sizeof(IUSipControllerOpenCmdParam), 0, 0);
    }
};

class IUSipControllerCloseOnGoingSessionCmdParam
{
public:
    inline IUSipControllerCloseOnGoingSessionCmdParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_M : IUSipControllerCloseOnGoingSessionCmdParam = %" PFLS_u,
                sizeof(IUSipControllerCloseOnGoingSessionCmdParam), 0, 0);
        pszCloseOngoingSession = IMS_NULL;
    }
    inline ~IUSipControllerCloseOnGoingSessionCmdParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_F : IUSipControllerCloseOnGoingSessionCmdParam = %" PFLS_u,
                sizeof(IUSipControllerCloseOnGoingSessionCmdParam), 0, 0);

        if (pszCloseOngoingSession != IMS_NULL)
        {
            IMS_MEM_Free(pszCloseOngoingSession);
        }
    }

public:
    IMS_CHAR* pszCloseOngoingSession;
};

class IUSipControllerSentMessageIndParam
{
public:
    inline IUSipControllerSentMessageIndParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_M : IUSipControllerSentMessageIndParam = %" PFLS_u,
                sizeof(IUSipControllerSentMessageIndParam), 0, 0);
        szTId[0] = '\0';
    }
    inline ~IUSipControllerSentMessageIndParam() {}

public:
    IMS_CHAR szTId[IMS_SOLUTION_MSG_SOURCE_LEN + 1];
};

class IUSipControllerSendFailureIndParam
{
public:
    inline IUSipControllerSendFailureIndParam()
    {
        IMS_TRACE_MEM("SIP_MSG", "IM_M : IUSipControllerSendFailureIndParam = %" PFLS_u,
                sizeof(IUSipControllerSendFailureIndParam), 0, 0);
        szTId[0] = '\0';
    }
    inline ~IUSipControllerSendFailureIndParam() {}

public:
    IMS_CHAR szTId[IMS_SOLUTION_MSG_SOURCE_LEN + 1];
};

#endif  //_IUSipControllerService_H_