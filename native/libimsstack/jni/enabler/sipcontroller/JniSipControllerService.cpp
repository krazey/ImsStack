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

#define IMS_STL_USE

#include <utils/String8.h>
#include "EnablerUtils.h"
#include "ImsProcess.h"
#include "IURcsMessageService.h"
#include "JniSipControllerService.h"
#include "JniSipControllerServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

JniSipControllerService::JniSipControllerService(
        Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot /*= 0*/) :
        BaseService(nSimSlot),
        m_strTarget(EnablerUtils::GetEnablerThreadName(nSimSlot)),
        m_strThreadName(AString::ConstNull()),
        m_nSessionId(-1)
{
    IMS_TRACE_MEM("SNC_MEM", "JniSipControllerService = %" PFLS_u "/%" PFLS_x,
            sizeof(JniSipControllerService), this, 0);
    m_strTarget.Append(".RcsMessageService");
    IMS_TRACE_D("JniSipControllerService [%s]", m_strTarget.GetStr(), 0, 0);
    m_nSessionId = reinterpret_cast<IMS_SINTP>(this);

    if (pfnSendDataToJava == NULL)
    {
        return;
    }
    m_strThreadName.Sprintf("JniSipControllerServiceThread_%d", GetSlotId());
    auto fnEntry = []() -> BaseThread*
    {
        return new JniSipControllerServiceThread();
    };

    ImsProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry, GetSlotId());
    m_pJniSipControllerServiceThread = reinterpret_cast<JniSipControllerServiceThread*>(
            ImsProcess::GetInstance()->GetThread(m_strThreadName));

    if (m_pJniSipControllerServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniSipControllerService : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniSipControllerServiceThread->SetCallback(
            reinterpret_cast<IMS_UINTP>(this), pfnSendDataToJava);

    IUSncOpenCmdParam* pParam = new IUSncOpenCmdParam();
    pParam->m_nSessionID = m_nSessionId;
    pParam->m_strThread = m_strThreadName;
    IMSMSG objMSG(IUSncService::OPEN_MESSAGE_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
    MessageService::PostMessage(m_strTarget, objMSG);
}

JniSipControllerService::~JniSipControllerService()
{
    IMS_TRACE_I("~JniSipControllerService :", 0, 0, 0);
    IMS_TRACE_MEM("SNC_MEM", "JniSipControllerService = %" PFLS_u "/%" PFLS_x,
            sizeof(JniSipControllerService), this, 0);

    if (m_pJniSipControllerServiceThread != NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pJniSipControllerServiceThread = NULL;
    }
    IUSncCloseSessionCmdParam* pParam = new IUSncCloseSessionCmdParam();
    pParam->m_nSessionID = m_nSessionId;
    IMSMSG objMSG(IUSncService::CLOSE_SESSION_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
    MessageService::PostMessage(m_strTarget, objMSG);
}

int JniSipControllerService::SendData(const Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    IMS_TRACE_I("SendData : msg = %d", nMsg, 0, 0);
    HandleMsg(nMsg, pParcel);
    return 1;
}

PRIVATE
void JniSipControllerService::HandleMsg(int nMsg, const Parcel& pParcel)
{
    AString strDest = AString::ConstEmpty();
    IMS_TRACE_I("msg = %d", nMsg, 0, 0);

    switch (nMsg)
    {
        case IUSncService::SEND_MESSAGE_CMD:
        {
            IUSncSendMessageParam* pParam = makeSendMessageParamFromParcel(pParcel);
            pParam->m_nSessionID = m_nSessionId;
            pParam->m_strThread = m_strThreadName;
            IMSMSG objMSG(IUSncService::SEND_MESSAGE_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUSncService::CLOSE_SESSION_CMD:
        {
            IUSncCloseSessionCmdParam* pParam = new IUSncCloseSessionCmdParam();
            pParam->m_nSessionID = m_nSessionId;
            pParam->m_strThread = m_strThreadName;
            ConvertString(pParcel.readString16(), strDest);
            pParam->m_strCallId = strDest;
            IMSMSG objMSG(IUSncService::CLOSE_SESSION_CMD, 0, reinterpret_cast<IMS_UINTP>(pParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUSncService::NOTIFY_MESSAGE_RECEIVE_ERROR_CMD:
        {
            IUSncNotifyErrorCmdParam* pParam = new IUSncNotifyErrorCmdParam();
            pParam->m_nSessionID = m_nSessionId;
            pParam->m_strThread = m_strThreadName;
            ConvertString(pParcel.readString16(), strDest);
            pParam->m_strTId = strDest;
            IMSMSG objMSG(IUSncService::NOTIFY_MESSAGE_RECEIVE_ERROR_CMD, 0,
                    reinterpret_cast<IMS_UINTP>(pParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUSncControl::UPDATE_SIPREGISTRATION_CMD:
        {
            // TODO : hakjunc
            IMS_TRACE_E(
                    0, "HandleMsg : UPDATESIPREGISTRATION_CMD by hakjunc, name %d\n", nMsg, 0, 0);
        }
        break;
        case IUSncControl::TRIGGER_SIPDEREGISTRATION_CMD:
        {
            // TODO : hakjunc
            IMS_TRACE_E(0, "HandleMsg : TRIGGERSIPDEREGISTRATION_CMD by hakjunc, name %d\n", nMsg,
                    0, 0);
        }
        break;
        default:
            IMS_TRACE_E(0, "HandleMsg : Can't analysis message, name %d\n", nMsg, 0, 0);
            break;
    }
}

PRIVATE
IUSncSendMessageParam* JniSipControllerService::makeSendMessageParamFromParcel(
        const android::Parcel& objParcel)
{
    AString strDest = AString::ConstEmpty();
    IUSncSendMessageParam* pParam = new IUSncSendMessageParam();

    ConvertString(objParcel.readString16(), strDest);
    pParam->m_strStartLine = strDest;

    ConvertString(objParcel.readString16(), strDest);
    pParam->m_strHeaderSection = strDest;

    pParam->m_nContentLength = objParcel.readInt32();

    ConvertString(objParcel.readString16(), strDest);
    pParam->m_strContent = strDest;

    ConvertString(objParcel.readString16(), strDest);
    pParam->m_strMethod = strDest;

    ConvertString(objParcel.readString16(), strDest);
    pParam->m_strFromParameter = strDest;

    ConvertString(objParcel.readString16(), strDest);
    pParam->m_strToParameter = strDest;

    pParam->m_nType = objParcel.readInt32();
    return pParam;
}

PRIVATE GLOBAL void JniSipControllerService::ConvertString(
        IN const String16& strSource, OUT AString& strDest)
{
    String8 str8(strSource);
    strDest = str8.string();
}