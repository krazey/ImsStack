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

#include <utils/String8.h>

#define IMS_STL_USE

#include "EnablerUtils.h"
#include "ImsProcess.h"
#include "IUUceService.h"
#include "IUce.h"
#include "JniUceService.h"
#include "JniUceServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_UCE");

static const AString STR_UCE_LISTENER_THREAD_NAME("JniUceServiceThread");

JniUceService::JniUceService(IN IMS_UINT32 _nSimSlot /* = 0*/) :
        m_nSimSlot(_nSimSlot)
{
    IMS_TRACE_D("UCE_M : JniUceService = %" PFLS_u, sizeof(JniUceService), 0, 0);
    //---------------------------------------------------------------------------------------------
    // m_strTarget.Sprintf("%s.UceApp%02d",
    // EnablerUtils::GetEnablerThreadName(m_nSimSlot), nSlotId);
    m_strTarget = EnablerUtils::GetEnablerThreadName(m_nSimSlot);
    m_strTarget.Append(".UceApp");

    IMS_TRACE_D("JniUceService [%s]", m_strTarget.GetStr(), 0, 0);
    m_pJniUceServiceThread = NULL;
}

JniUceService::JniUceService(Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot /*= 0*/) :
        m_nSimSlot(nSimSlot)
{
    IMS_TRACE_D("UCE_M : JniUceService = %" PFLS_u, sizeof(JniUceService), 0, 0);
    //---------------------------------------------------------------------------------------------
    // m_strTarget.Sprintf("%s.UceApp%02d",
    // EnablerUtils::GetEnablerThreadName(m_nSimSlot), nSlotId);
    m_strTarget = EnablerUtils::GetEnablerThreadName(m_nSimSlot);
    m_strTarget.Append(".UceApp");
    IMS_TRACE_D("JniUceService [%s]", m_strTarget.GetStr(), 0, 0);

    if (pfnSendDataToJava == NULL)
    {
        IMS_TRACE_E(0, "JniUceService:pfnSendDataToJava is null", 0, 0, 0);
    }
    ImsProcess::GetInstance()->LoadAppThread(
            STR_UCE_LISTENER_THREAD_NAME, JniUceServiceThread::GetInstance, m_nSimSlot);
    m_pJniUceServiceThread = (JniUceServiceThread*)(ImsProcess::GetInstance()->GetApplicationThread(
            STR_UCE_LISTENER_THREAD_NAME));
    if (m_pJniUceServiceThread != NULL)
    {
        m_pJniUceServiceThread->SetCallback(reinterpret_cast<IMS_UINTP>(this), pfnSendDataToJava);
    }
    else
    {
        IMS_TRACE_I("can't create listener thread", 0, 0, 0);
    }
}

JniUceService::~JniUceService()
{
    IMS_TRACE_D("UCE_F : JniUceService = %" PFLS_u, sizeof(JniUceService), 0, 0);
    IMS_TRACE_I("~JniUceService :", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    if (m_pJniUceServiceThread != NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(STR_UCE_LISTENER_THREAD_NAME);
        m_pJniUceServiceThread = NULL;
    }
}

int JniUceService::SendData(const Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    IMS_TRACE_I("SendData : msg = %d", nMsg, 0, 0);
    HandleMessage(nMsg, pParcel);

    return 1;
}

PRIVATE
void JniUceService::HandleMessage(int nMsg, const Parcel& pParcel)
{
    IMS_TRACE_I("msg = %d", nMsg, 0, 0);
    //---------------------------------------------------------------------------------------------
    switch (nMsg)
    {
        case IUUceService::UCE_SEND_PUBLISH_CMD:
        {
            IUcePubCmdPrm* pCmdParam = new IUcePubCmdPrm();

            pCmdParam->m_nKey = pParcel.readInt32();

            String16 str16PidfXml = pParcel.readString16();
            String8 str8PidfXml((const char16_t*)str16PidfXml);
            pCmdParam->m_strPidfXml = str8PidfXml.string();

            pCmdParam->m_nExtended = pParcel.readInt32();
            pCmdParam->m_nCapability = pParcel.readInt32();

            if (pParcel.readInt32() == 1)
            {
                String16 str16ETag = pParcel.readString16();
                String8 str8ETag((const char16_t*)str16ETag);
                pCmdParam->m_strEtag = str8ETag.string();
            }

            IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pCmdParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUUceService::UCE_SEND_SINGLE_SUBSCRIBE_CMD:
        {
            IUceSingleSubCmdPrm* pCmdParam = new IUceSingleSubCmdPrm();

            pCmdParam->m_nKey = pParcel.readInt32();
            pCmdParam->m_nSize = pParcel.readInt32();

            String16 str16User = pParcel.readString16();
            String8 str8User((const char16_t*)str16User);
            pCmdParam->m_strUser = str8User.string();

            IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pCmdParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUUceService::UCE_SEND_LIST_SUBSCRIBE_CMD:
        {
            IUceListSubCmdPrm* pCmdParam = new IUceListSubCmdPrm();

            pCmdParam->m_nKey = pParcel.readInt32();
            pCmdParam->m_nSize = pParcel.readInt32();
            for (IMS_SINT32 i = 0; i < pCmdParam->m_nSize; i++)
            {
                String16 str16User = pParcel.readString16();
                String8 str8User((const char16_t*)str16User);
                AString szUser = str8User.string();
                pCmdParam->userList.Append(szUser);
            }
            IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pCmdParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUUceService::UCE_SEND_OPTIONS_CMD:
        {
            IUceOptionsCmdPrm* pCmdParam = new IUceOptionsCmdPrm();

            pCmdParam->m_nKey = pParcel.readInt32();

            String16 str16RemoteUri = pParcel.readString16();
            String8 str8RemoteUri((const char16_t*)str16RemoteUri);
            pCmdParam->m_strRemoteUri = str8RemoteUri.string();

            pCmdParam->m_nMyCaps = pParcel.readInt32();

            IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pCmdParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUUceService::UCE_SEND_OPTIONS_RESP_CMD:
        {
            IUceOptionsRespCmdPrm* pCmdParam = new IUceOptionsRespCmdPrm();

            pCmdParam->m_nKey = pParcel.readInt32();
            pCmdParam->m_nResponseCode = pParcel.readInt32();

            String16 str16Reason = pParcel.readString16();
            String8 str8Reason((const char16_t*)str16Reason);
            pCmdParam->m_strReason = str8Reason.string();

            pCmdParam->m_nMyCaps = pParcel.readInt32();

            IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pCmdParam));
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        case IUUceService::UCE_GET_IMS_REGISTRATION_CMD:
        {
            IMSMSG objMSG(nMsg, 0, 0);
            MessageService::PostMessage(m_strTarget, objMSG);
        }
        break;
        default:
            IMS_TRACE_E(0, "unknown message : %d", nMsg, 0, 0);
            break;
    }
}
