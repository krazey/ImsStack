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

#include "JniSipControllerServiceThread.h"

#include "IURcsMessageService.h"
#include "ImsMessageDef.h"
#include "ServiceTrace.h"
#include "ServiceMemory.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PRIVATE
JniSipControllerServiceThread::JniSipControllerServiceThread() :
        m_nNativeObj(0),
        m_pfnSendDataToJava(NULL)
{
    IMS_TRACE_I("+JniSipControllerServiceThread : ", 0, 0, 0);
    IMS_TRACE_MEM("SNC_MSG", "IM_M : JniSipControllerServiceThread = %" PFLS_u,
            sizeof(JniSipControllerServiceThread), 0, 0);
}

PUBLIC GLOBAL ImsAppThread* JniSipControllerServiceThread::GetInstance()
{
    return new JniSipControllerServiceThread();
}

PUBLIC VIRTUAL JniSipControllerServiceThread::~JniSipControllerServiceThread()
{
    IMS_TRACE_I("-JniSipControllerServiceThread : ", 0, 0, 0);
    IMS_TRACE_MEM("SNC_MSG", "IM_F : JniSipControllerServiceThread = %" PFLS_u,
            sizeof(JniSipControllerServiceThread), 0, 0);
}

PUBLIC VIRTUAL int JniSipControllerServiceThread::SetCallback(
        IN IMS_UINTP nNativeObj, IN Jni_SendDataToJava pfnSendDataToJava)
{
    IMS_TRACE_I("SetCallback : ", 0, 0, 0);
    this->m_nNativeObj = nNativeObj;
    this->m_pfnSendDataToJava = pfnSendDataToJava;
    return 1;
}

PRIVATE VIRTUAL IMS_BOOL JniSipControllerServiceThread::Initialize()
{
    IMS_TRACE_I("Initialize : ", 0, 0, 0);
    return IMS_TRUE;
}

PRIVATE VIRTUAL void JniSipControllerServiceThread::Uninitialize()
{
    IMS_TRACE_I("Uninitialize : ", 0, 0, 0);
}

PRIVATE VIRTUAL IMS_BOOL JniSipControllerServiceThread::OnStart(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnStart : %d", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL JniSipControllerServiceThread::OnTerminate(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnTerminate : %d", objMSG.GetName(), 0, 0);
    return IMS_TRUE;
}

PRIVATE VIRTUAL IMS_BOOL JniSipControllerServiceThread::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_D("OnMessage : MSG = %d, wParam = %" PFLS_u ", lParam = %" PFLS_u, objMSG.nMSG,
            objMSG.nWparam, objMSG.nLparam);
    IMS_TRACE_D("OnMessage : target = %s", objMSG.GetTargetName(), 0, 0);

    HandleMsg(objMSG);

    return IMS_TRUE;
}

PRIVATE
void JniSipControllerServiceThread::HandleMsg(IN IMSMSG& objMSG)
{
    android::Parcel parcel;
    parcel.writeInt32(objMSG.nMSG);

    switch (objMSG.nMSG)
    {
        case IUSncService::MESSAGERECEIVED_IND:
        {
            IUSncMessageParam* pParam = reinterpret_cast<IUSncMessageParam*>(objMSG.nLparam);

            WriteStringToParcel(pParam->pszStartLine, parcel);
            WriteStringToParcel(pParam->pszHeaderSection, parcel);
            parcel.writeInt32(pParam->nContentLength);
            WriteStringToParcel(pParam->pszContent, parcel);
            WriteStringToParcel(pParam->pszContent, parcel);
            WriteStringToParcel(pParam->pszViaBranchParameter, parcel);
            WriteStringToParcel(pParam->pszCallIdParameter, parcel);
        }
        break;
        case IUSncService::MESSAGESENT_IND:
        {
            IUSncSentMessageIndParam* pParam =
                    reinterpret_cast<IUSncSentMessageIndParam*>(objMSG.nLparam);
            parcel.writeString16(android::String16(pParam->szTId));
        }
        break;
        case IUSncService::SENDMESSAGEFAILURE_IND:
        {
            IUSncSendFailureIndParam* pParam =
                    reinterpret_cast<IUSncSendFailureIndParam*>(objMSG.nLparam);
            parcel.writeString16(android::String16(pParam->szTId));
        }
        break;
        default:
            IMS_TRACE_E(0, "HandleMsg : Can't analysis message, name %d\n", objMSG.nMSG, 0, 0);
            break;
    }
}

PRIVATE
inline void JniSipControllerServiceThread::WriteStringToParcel(
        IN CONST IMS_CHAR* pszValue, OUT android::Parcel& parcel)
{
    if (pszValue == IMS_NULL)
    {
        parcel.writeString16(android::String16(""));
        return;
    }
    parcel.writeString16(android::String16(pszValue));
}