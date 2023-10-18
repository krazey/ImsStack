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
#include "ISipControllerService.h"
#include "IURcsMessageService.h"
#include "JniEnablerConnector.h"
#include "JniSipControllerService.h"
#include "JniSipControllerServiceThread.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC JniSipControllerService::JniSipControllerService(
        Jni_SendDataToJava pfnSendDataToJava, IN IMS_UINT32 nSimSlot /*= 0*/) :
        BaseService(nSimSlot),
        m_strThreadName(AString::ConstNull())
{
    IMS_TRACE_MEM("SNC_MSG", "JniSipControllerService = %" PFLS_u "/%" PFLS_x,
            sizeof(JniSipControllerService), this, 0);
    IMS_TRACE_I("[%d] +JniSipControllerService :", GetSlotId(), 0, 0);

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
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::SIP_DELEGATE, this);
    OpenMessageTracker();
}

PUBLIC JniSipControllerService::~JniSipControllerService()
{
    IMS_TRACE_MEM("SNC_MSG", "JniSipControllerService = %" PFLS_u "/%" PFLS_x,
            sizeof(JniSipControllerService), this, 0);
    IMS_TRACE_I("[%d] ~JniSipControllerService :", GetSlotId(), 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::SIP_DELEGATE, IMS_NULL);
    if (m_pJniSipControllerServiceThread != NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pJniSipControllerServiceThread = NULL;
    }
    // need to check if this function should be called ate the end of the class.
    // CloseSession(AString::ConstEmpty());
}

PUBLIC VIRTUAL IJniEnablerThread* JniSipControllerService::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pJniSipControllerServiceThread);
}

PUBLIC int JniSipControllerService::SendData(const android::Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    IMS_TRACE_I("[%d] SendData : msg = %d", GetSlotId(), nMsg, 0);

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(pParcel);
    }
    else
    {
        HandleMessage(nMsg, pParcel);
    }

    return 1;
}

PROTECTED VIRTUAL void JniSipControllerService::HandleMessage(
        int nMsg, const android::Parcel& pParcel)
{
    AString strDest = AString::ConstEmpty();
    IMS_TRACE_I("[%d] msg = %d", GetSlotId(), nMsg, 0);

    ISipControllerService* piSipService = GetNativeService();
    if (piSipService == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleMessage:GetNativeService is null", 0, 0, 0);
        return;
    }

    switch (nMsg)
    {
        case IUSncService::SEND_MESSAGE_CMD:
        {
            IUSncSendMessageParam* pParam = makeSendMessageParamFromParcel(pParcel);
            pParam->m_strThread = m_strThreadName;
            SendMessage(piSipService, pParam);
        }
        break;
        case IUSncService::CLOSE_SESSION_CMD:
        {
            IUSncCloseSessionCmdParam* pParam = new IUSncCloseSessionCmdParam();
            pParam->m_strThread = m_strThreadName;
            ConvertString(pParcel.readString16(), strDest);
            CloseSession(piSipService, strDest);
        }
        break;
        case IUSncService::NOTIFY_MESSAGE_RECEIVE_ERROR_CMD:
        {
            IUSncNotifyErrorCmdParam* pParam = new IUSncNotifyErrorCmdParam();
            pParam->m_strThread = m_strThreadName;
            ConvertString(pParcel.readString16(), strDest);
            pParam->m_strTId = strDest;
            NotifyMessageReceiveError(piSipService, pParam);
        }
        break;
        case IUSncControl::UPDATE_SIPREGISTRATION_CMD:
        {
            IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();
            pParam->m_nFeatureCount = pParcel.readInt32();
            IMS_TRACE_D("UPDATEDELEGATEREGISTRATION_CMD featuretag : %d", pParam->m_nFeatureCount,
                    0, 0);

            AString strFeatureTag = AString::ConstEmpty();
            for (IMS_SINT32 i = 0; i < pParam->m_nFeatureCount; i++)
            {
                ConvertString(pParcel.readString16(), strFeatureTag);
                IMS_TRACE_D("UPDATEDELEGATEREGISTRATION_CMD featuretag : [%s]",
                        strFeatureTag.GetStr(), 0, 0);
                pParam->m_objFeatureTags.AddElement(strFeatureTag);
            }
            UpdateRegistration(piSipService, pParam);
        }
        break;
        case IUSncControl::TRIGGER_SIPDEREGISTRATION_CMD:
        {
            TriggerDelegateDeregistration(piSipService);
        }
        break;
        default:
            IMS_TRACE_E(0, "HandleMessage : Can't analysis message, name %d\n", nMsg, 0, 0);
            break;
    }
}

PRIVATE ISipControllerService* JniSipControllerService::GetNativeService()
{
    return DYNAMIC_CAST(ISipControllerService*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(
                    GetSlotId(), EnablerType::SIP_DELEGATE));
}

PRIVATE void JniSipControllerService::OpenMessageTracker()
{
    IMS_TRACE_I("[%d] OpenMessageTracker()", GetSlotId(), 0, 0);
    ISipControllerService* piSipService = GetNativeService();
    if (piSipService == IMS_NULL)
    {
        IMS_TRACE_E(0, "HandleMessage:GetNativeService is null", 0, 0, 0);
        return;
    }
    piSipService->OpenMessageTracker(m_strThreadName);
}

PRIVATE void JniSipControllerService::SendMessage(
        IN ISipControllerService* piSipService, IN IUSncSendMessageParam* pParam)
{
    IMS_TRACE_I("[%d] SendMessage()", GetSlotId(), 0, 0);
    piSipService->SendMessage(reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE void JniSipControllerService::NotifyMessageReceiveError(
        IN ISipControllerService* piSipService, IN IUSncNotifyErrorCmdParam* pParam)
{
    piSipService->NotifyMessageReceiveError(reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE void JniSipControllerService::TriggerDelegateDeregistration(
        IN ISipControllerService* piSipService)
{
    piSipService->TriggerDelegateDeregistration();
}

PRIVATE void JniSipControllerService::CloseSession(
        IN ISipControllerService* piSipService, IN const AString& strCallId)
{
    IMS_TRACE_I("[%d] CloseSession()", GetSlotId(), 0, 0);
    piSipService->CloseSession(strCallId);
}

PRIVATE void JniSipControllerService::UpdateRegistration(
        IN ISipControllerService* piSipService, IN IUSncFeatureTagsParam* pParam)
{
    IMS_TRACE_I("[%d] UpdateRegistration", GetSlotId(), 0, 0);
    piSipService->UpdateDelegateRegistration(reinterpret_cast<IMS_UINTP>(pParam));
}

PRIVATE IUSncSendMessageParam* JniSipControllerService::makeSendMessageParamFromParcel(
        IN const android::Parcel& objParcel)
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
        IN const android::String16& strSource, OUT AString& strDest)
{
    strDest = String8(strSource).c_str();
}