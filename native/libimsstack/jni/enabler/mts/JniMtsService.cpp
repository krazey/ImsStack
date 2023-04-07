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

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "ImsProcess.h"
#include "IMtsService.h"
#include "IuMtsService.h"
#include "OsMutex.h"
#include "IJniEnablerThread.h"
#include "JniEnablerConnector.h"
#include "JniMtsService.h"
#include "JniMtsServiceThread.h"
#include "EnablerUtils.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTS");

JniMtsService::JniMtsService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId) :
        BaseService(nSlotId),
        m_pJniMtsServiceThread(IMS_NULL)
{
    IMS_TRACE_D("+JniMtsService SlotId[%d]", nSlotId, 0, 0);

    Initialize(pfnSendDataToJava);
}

JniMtsService::~JniMtsService()
{
    IMS_TRACE_D("~JniMtsService SlotId[%d]", GetSlotId(), 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::MTS_SERVICE, IMS_NULL);

    if (m_pJniMtsServiceThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_pJniMtsServiceThread->GetName());
        m_pJniMtsServiceThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL
int JniMtsService::SendData(const Parcel& objParcel)
{
    int nMessage = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMessage))
    {
        SendDataUsingEnablerThread(objParcel);
    }
    else
    {
        HandleMessage(nMessage, objParcel);
    }

    return 1;
}

PUBLIC VIRTUAL IJniEnablerThread* JniMtsService::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pJniMtsServiceThread);
}

PROTECTED VIRTUAL
void JniMtsService::HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IuMtsService::NOTI_MTSENABLER_SEND_MO_SMS:
            TriggerSendMoSms(objParcel);
            break;

        case IuMtsService::NOTI_MTSENABLER_SEND_MT_RESULT:
            NotifyMtResult(objParcel);
            break;

        default:
            break;
    }
}

PRIVATE
void JniMtsService::Attach()
{
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::MTS_SERVICE, this);
}

PRIVATE
IMtsService* JniMtsService::GetNativeService()
{
    return DYNAMIC_CAST(IMtsService*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(
                    GetSlotId(), EnablerType::MTS_SERVICE));
}

PRIVATE
void JniMtsService::Initialize(IN Jni_SendDataToJava pfnSendDataToJava)
{
    if (pfnSendDataToJava == IMS_NULL)
    {
        return;
    }

    AString strThreadName;
    strThreadName.Sprintf("JniMtsServiceThread_%d", GetSlotId());

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniMtsServiceThread();
    };

    ImsProcess::GetInstance()->LoadThread(strThreadName, fnEntry, GetSlotId());
    m_pJniMtsServiceThread = reinterpret_cast<JniMtsServiceThread*>(
            ImsProcess::GetInstance()->GetThread(strThreadName));

    if (m_pJniMtsServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtsService : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniMtsServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnSendDataToJava);
    Attach();
}

PRIVATE
void JniMtsService::TriggerSendMoSms(IN const Parcel& objParcel)
{
    IMS_UINT32 nSmsFormat_ = objParcel.readInt32();
    android::String8 strEncodedPdu(objParcel.readString16());
    android::String8 strAddress_(objParcel.readString16());
    IMS_SINT32 nSeqId = objParcel.readInt32();
    IMS_BOOL bEmergency = objParcel.readBool();
    AString strContent = AString::FromBase64(strEncodedPdu.string());
    // This object will be deleted by MtsMessageController after being used.
    ByteArray* pContent = new ByteArray(reinterpret_cast<const IMS_BYTE*>(strContent.GetStr()),
            static_cast<IMS_SINT32>(strContent.GetLength()));
    AString strAddress = strAddress_.string();

    SmsFormatType eSmsFormat;
    if (nSmsFormat_ == (IMS_UINT32)SmsFormatType::SMSFORMAT_3GPP)
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    }
    else if (nSmsFormat_ == (IMS_UINT32)SmsFormatType::SMSFORMAT_3GPP2)
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_3GPP2;
    }
    else
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_INVALID;
    }

    IMtsService* piMtsService = GetNativeService();
    if (piMtsService == IMS_NULL)
    {
        IMS_TRACE_D("MtsEnabler is not bound.", 0, 0, 0);
        m_pJniMtsServiceThread->ReportMoStatus(MO_ERROR_RETRY, eSmsFormat, 0, nSeqId, GetSlotId());
        return;
    }

    piMtsService->SendMoSms(eSmsFormat, pContent, strAddress, nSeqId, bEmergency);
}

PRIVATE
void JniMtsService::NotifyMtResult(IN const Parcel& objParcel)
{
    IMS_SINT32 nMtResult = objParcel.readInt32();
    IMS_TRACE_I("MT result = (%d)", nMtResult, 0, 0);

    IMtsService* piMtsService = GetNativeService();
    if (piMtsService == IMS_NULL)
    {
        // TODO: error handling is needed when call back is added
        IMS_TRACE_D("MtsEnabler is not bound.", 0, 0, 0);
        return;
    }

    piMtsService->SendMtResult(nMtResult);
}
