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
#include "IMtsJni.h"
#include "IMtsService.h"
#include "IuMtsApp.h"
#include "OsMutex.h"
#include "IJniEnablerThread.h"
#include "JniEnablerConnector.h"
#include "JniMtsApp.h"
#include "JniMtsAppThread.h"
#include "EnablerUtils.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTS");

JniMtsApp::JniMtsApp(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId) :
        BaseService(nSlotId),
        m_pJniMtsAppThread(IMS_NULL)
{
    IMS_TRACE_D("+JniMtsApp SlotId[%d]", nSlotId, 0, 0);

    Initialize(pfnSendDataToJava);
}

JniMtsApp::~JniMtsApp()
{
    IMS_TRACE_D("~JniMtsApp SlotId[%d]", GetSlotId(), 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::MTS, IMS_NULL);

    if (m_pJniMtsAppThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_pJniMtsAppThread->GetName());
        m_pJniMtsAppThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL int JniMtsApp::SendData(const Parcel& objParcel)
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

PUBLIC VIRTUAL IJniEnablerThread* JniMtsApp::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pJniMtsAppThread);
}

PROTECTED VIRTUAL void JniMtsApp::HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IuMtsApp::NOTI_MTSENABLER_SEND_MO_SMS:
            TriggerSendMoSms(objParcel);
            break;

        case IuMtsApp::NOTI_MTSENABLER_MO_SMS_TIMED_OUT:
            TriggerNotifyMoSmsTimedOut();
            break;

        default:
            break;
    }
}

PRIVATE
void JniMtsApp::Attach()
{
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::MTS, this);
}

PRIVATE
IMtsJni* JniMtsApp::GetNativeApp()
{
    return DYNAMIC_CAST(IMtsJni*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(GetSlotId(), EnablerType::MTS));
}

PRIVATE
void JniMtsApp::Initialize(IN Jni_SendDataToJava pfnSendDataToJava)
{
    if (pfnSendDataToJava == IMS_NULL)
    {
        return;
    }

    AString strThreadName;
    strThreadName.Sprintf("JniMtsAppThread_%d", GetSlotId());

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniMtsAppThread();
    };

    ImsProcess::GetInstance()->LoadThread(strThreadName, fnEntry, GetSlotId());
    m_pJniMtsAppThread =
            reinterpret_cast<JniMtsAppThread*>(ImsProcess::GetInstance()->GetThread(strThreadName));

    if (m_pJniMtsAppThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtsApp : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniMtsAppThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnSendDataToJava);
    Attach();
}

PRIVATE
void JniMtsApp::TriggerSendMoSms(IN const Parcel& objParcel)
{
    IMS_UINT32 nSmsFormat_ = objParcel.readInt32();
    android::String8 strEncodedPdu(objParcel.readString16());
    android::String8 strAddress_(objParcel.readString16());
    IMS_SINT32 nSeqId = objParcel.readInt32();
    IMS_BOOL bEmergencyNumber = objParcel.readBool();
    IMS_UINT32 nRetryCount = objParcel.readInt32();
    AString strContent = AString::FromBase64(strEncodedPdu.c_str());
    // This object will be deleted by MtsMessageController after being used.
    ByteArray* pContent = new ByteArray(reinterpret_cast<const IMS_BYTE*>(strContent.GetStr()),
            static_cast<IMS_SINT32>(strContent.GetLength()));
    AString strAddress = strAddress_.c_str();

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

    IMtsJni* piMtsJni = GetNativeApp();
    if (piMtsJni == IMS_NULL)
    {
        IMS_TRACE_D("MtsEnabler is not bound.", 0, 0, 0);
        m_pJniMtsAppThread->ReportMoStatus(MO_ERROR_RETRY, eSmsFormat, nSeqId, GetSlotId());
        return;
    }

    piMtsJni->SendMoSmsByServiceType(
            eSmsFormat, pContent, strAddress, nSeqId, bEmergencyNumber, nRetryCount);
}

PRIVATE
void JniMtsApp::TriggerNotifyMoSmsTimedOut()
{
    IMtsJni* piMtsJni = GetNativeApp();
    if (piMtsJni == IMS_NULL)
    {
        IMS_TRACE_D("MtsEnabler is not bound.", 0, 0, 0);
        return;
    }

    piMtsJni->NotifyMoSmsTimedOut();
}
