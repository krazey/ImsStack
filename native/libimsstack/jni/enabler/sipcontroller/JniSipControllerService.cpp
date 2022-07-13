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
#include "IUSipControllerService.h"
#include "JniSipControllerService.h"
#include "JniSipControllerServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SIPCONTROLLER");

JniSipControllerService::JniSipControllerService(
        Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot /*= 0*/) :
        m_nSlotId(nSimSlot),
        m_strTarget(AString::ConstNull()),
        m_strThreadName(AString::ConstNull())
{
    IMS_TRACE_D("JniSipControllerService = %" PFLS_u, sizeof(JniSipControllerService), 0, 0);

    m_strTarget = EnablerUtils::GetEnablerThreadName(m_nSlotId);
    m_strTarget.Append(".RCSMessageService");
    IMS_TRACE_D("JniSipControllerService [%s]", m_strTarget.GetStr(), 0, 0);

    if (pfnSendDataToJava != NULL)
    {
        m_strThreadName.Sprintf("JniAosServiceThread_%d", m_nSlotId);
        LoadThread(m_strThreadName);

        if (m_pJniSipControllerServiceThread != NULL)
        {
            m_pJniSipControllerServiceThread->SetCallback(
                    reinterpret_cast<IMS_UINTP>(this), pfnSendDataToJava);

            IUSipControllerOpenCmdParam* pParam = new IUSipControllerOpenCmdParam();
            IMSMSG objMSG(IUSipControllerService::OPENMESSAGE_CMD, 0,
                    reinterpret_cast<IMS_UINTP>(pParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        else
        {
            IMS_TRACE_I("can't create listener thread", 0, 0, 0);
        }
    }
}

JniSipControllerService::~JniSipControllerService()
{
    IMS_TRACE_D("JniSipControllerService = %" PFLS_u, sizeof(JniSipControllerService), 0, 0);
    IMS_TRACE_I("~JniSipControllerService :", 0, 0, 0);

    if (m_pJniSipControllerServiceThread != NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pJniSipControllerServiceThread = NULL;
    }
}

PRIVATE
void JniSipControllerService::LoadThread(IN CONST AString& strThreadName)
{
    if (strThreadName.GetLength() <= 0)
    {
        IMS_TRACE_E(0, "LoadThread() : strThreadName is empty", 0, 0, 0);
        return;
    }

    if (!ImsProcess::GetInstance()->LoadAppThread(
                strThreadName, JniSipControllerServiceThread::GetInstance, m_nSlotId))
    {
        IMS_TRACE_E(0, "LoadThread() : failed to load a thread(%s)", strThreadName.GetStr(), 0, 0);
        return;
    }
    m_pJniSipControllerServiceThread = (JniSipControllerServiceThread*)((
            ImsProcess::GetInstance()->GetApplicationThread(strThreadName)));
}

int JniSipControllerService::SendData(const Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    IMS_TRACE_I("SendData : msg = %d", nMsg, 0, 0);
    HandleMessage(nMsg, pParcel);
    return 1;
}

PRIVATE
void JniSipControllerService::HandleMessage(int nMsg, const Parcel& pParcel)
{
    AString strDest = AString::ConstEmpty();
    IMS_TRACE_I("msg = %d", nMsg, 0, 0);

    switch (nMsg)
    {
        case IUSipControllerService::SENDMESSAGE_CMD:
        {
            IUMessageParam* pParam = makeSendMessageParamFromParcel(pParcel);
            IMSMSG objMSG(IUSipControllerService::SENDMESSAGE_CMD, 0,
                    reinterpret_cast<IMS_UINTP>(pParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUSipControllerService::CLOSEONGOINGSESSION_CMD:
        {
            IUSipControllerCloseOnGoingSessionCmdParam* pParam =
                    new IUSipControllerCloseOnGoingSessionCmdParam();

            ConvertString(pParcel.readString16(), strDest);
            pParam->pszCloseOngoingSession = strDest.GetStr();
            IMSMSG objMSG(IUSipControllerService::CLOSEONGOINGSESSION_CMD, 0,
                    reinterpret_cast<IMS_UINTP>(pParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUSipControllerService::NOTIFYMESSAGERECEIVEERROR_CMD:
        {
            IMSMSG objMSG(IUSipControllerService::NOTIFYMESSAGERECEIVEERROR_CMD, 0, 0);
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;

        default:
            IMS_TRACE_E(0, "HandleMsg : Can't analysis message, name %d\n", nMsg, 0, 0);
            break;
    }
}

PRIVATE
IUMessageParam* JniSipControllerService::makeSendMessageParamFromParcel(
        const android::Parcel& objParcel)
{
    AString strDest = AString::ConstEmpty();
    IUMessageParam* pParam = new IUMessageParam();

    ConvertString(objParcel.readString16(), strDest);

    ConvertString(objParcel.readString16(), strDest);
    pParam->pszStartLine =
            static_cast<IMS_CHAR*>(IMS_MEM_Malloc(sizeof(IMS_CHAR) * (strDest.GetLength() + 1)));
    IMS_StrCpy(pParam->pszStartLine, strDest.GetLength(), strDest.GetStr());

    ConvertString(objParcel.readString16(), strDest);
    pParam->pszHeaderSection =
            static_cast<IMS_CHAR*>(IMS_MEM_Malloc(sizeof(IMS_CHAR) * (strDest.GetLength() + 1)));
    IMS_StrCpy(pParam->pszHeaderSection, strDest.GetLength(), strDest.GetStr());
    pParam->nContentLength = objParcel.readInt32();

    ConvertString(objParcel.readString16(), strDest);
    pParam->pszContent =
            static_cast<IMS_CHAR*>(IMS_MEM_Malloc(sizeof(IMS_CHAR) * (strDest.GetLength() + 1)));
    IMS_StrCpy(pParam->pszContent, strDest.GetLength(), strDest.GetStr());

    ConvertString(objParcel.readString16(), strDest);
    pParam->pszViaBranchParameter =
            static_cast<IMS_CHAR*>(IMS_MEM_Malloc(sizeof(IMS_CHAR) * (strDest.GetLength() + 1)));
    IMS_StrCpy(pParam->pszViaBranchParameter, strDest.GetLength(), strDest.GetStr());

    ConvertString(objParcel.readString16(), strDest);
    pParam->pszCallIdParameter =
            static_cast<IMS_CHAR*>(IMS_MEM_Malloc(sizeof(IMS_CHAR) * (strDest.GetLength() + 1)));
    IMS_StrCpy(pParam->pszCallIdParameter, strDest.GetLength(), strDest.GetStr());
    return pParam;
}

PRIVATE GLOBAL void JniSipControllerService::ConvertString(
        IN const String16& strSource, OUT AString& strDest)
{
    String8 str8(strSource);
    strDest = str8.string();
}