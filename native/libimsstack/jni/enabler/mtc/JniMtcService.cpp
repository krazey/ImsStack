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
#include "EnablerUtils.h"
#include "IJniEnablerThread.h"
#include "IMtcService.h"
#include "ImsProcess.h"
#include "IuMtcService.h"
#include "JniEnablerConnector.h"
#include "JniMtcService.h"
#include "JniMtcServiceThread.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

JniMtcService::JniMtcService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId) :
        BaseService(nSlotId),
        m_pThread(IMS_NULL)
{
    IMS_TRACE_D("+JniMtcService SlotId[%d]", nSlotId, 0, 0);

    Initialize(pfnSendDataToJava);
}

JniMtcService::~JniMtcService()
{
    IMS_TRACE_D("~JniMtcService", 0, 0, 0);

    if (m_pThread)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_pThread->GetName());
    }
    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::MTC_SERVICE, IMS_NULL);
}

PUBLIC VIRTUAL int JniMtcService::SendData(IN const android::Parcel& objParcel)
{
    int nMsg = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(objParcel);
    }
    else
    {
        HandleMessage(nMsg, objParcel);
    }

    return 1;
}

PUBLIC
void JniMtcService::Initialize(IN Jni_SendDataToJava pfnSendDataToJava)
{
    if (pfnSendDataToJava == IMS_NULL)
    {
        return;
    }
    AString strThreadName;
    strThreadName.Sprintf("JniMtcServiceThread_%08" PFLS_x, reinterpret_cast<IMS_SINTP>(this));

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniMtcServiceThread();
    };
    ImsProcess::GetInstance()->LoadThread(strThreadName, fnEntry, GetSlotId());
    m_pThread = reinterpret_cast<JniMtcServiceThread*>(
            ImsProcess::GetInstance()->GetThread(strThreadName));

    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcService : can't create listener thread", 0, 0, 0);
        return;
    }
    IMS_TRACE_D("Initialize()", 0, 0, 0);
    m_pThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnSendDataToJava);
    Attach();
}

PUBLIC VIRTUAL void JniMtcService::NotifyNativeEnablerSet()
{
    m_pThread->OnJniReady();
}

PUBLIC
IJniEnablerThread* JniMtcService::GetJniThread() const
{
    IMS_TRACE_D("GetJniThread()", 0, 0, 0);
    return DYNAMIC_CAST(IJniEnablerThread*, m_pThread);
}

PROTECTED VIRTUAL void JniMtcService::HandleMessage(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("HandleServiceMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IuMtcService::SRVCC_STATE_CHANGED:
            NotifySrvccStateChanged(objParcel);
            break;
        case IuMtcService::SET_TERMINAL_BASED_CALL_WAITING:
            SetTerminalBasedCallWaiting(objParcel);
            break;
        case IuMtcService::OPEN_EMERGENCY_SERVICE:
            OpenEmergencyService(objParcel);
            break;
        case IuMtcService::STOP_EMERGENCY_SERVICE:
            StopEmergencyService();
            break;
        case IuMtcService::TEST_COMMAND:
            ProcessTestCommand(objParcel);
            break;
        default:
            break;
    }
}

PRIVATE
void JniMtcService::Attach()
{
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::MTC_SERVICE, this);
}

PRIVATE
IMtcService* JniMtcService::GetNativeService()
{
    return DYNAMIC_CAST(IMtcService*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(
                    GetSlotId(), EnablerType::MTC_SERVICE));
}

PRIVATE
void JniMtcService::NotifySrvccStateChanged(IN const android::Parcel& objParcel)
{
    IMtcService* piNativeService = GetNativeService();
    if (piNativeService == IMS_NULL)
    {
        return;
    }
    piNativeService->UpdateSrvccState(static_cast<SrvccState>(objParcel.readInt32()));
}

PRIVATE
void JniMtcService::SetTerminalBasedCallWaiting(IN const android::Parcel& objParcel)
{
    IMtcService* piNativeService = GetNativeService();
    if (piNativeService == IMS_NULL)
    {
        return;
    }
    IMS_BOOL bEnabled = (objParcel.readInt32() == 1) ? IMS_TRUE : IMS_FALSE;
    piNativeService->SetTerminalBasedCallWaiting(bEnabled);
}

PRIVATE
void JniMtcService::OpenEmergencyService(IN const android::Parcel& objParcel)
{
    IMtcService* piNativeService = GetNativeService();
    if (piNativeService == IMS_NULL)
    {
        return;
    }
    piNativeService->OpenEmergencyService(static_cast<ServiceType>(objParcel.readInt32()));
}

PRIVATE
void JniMtcService::StopEmergencyService()
{
    IMtcService* piNativeService = GetNativeService();
    if (piNativeService == IMS_NULL)
    {
        return;
    }
    piNativeService->StopEmergencyService();
}

PRIVATE
void JniMtcService::ProcessTestCommand(IN const android::Parcel& objParcel)
{
    IMtcService* piNativeService = GetNativeService();
    if (piNativeService == IMS_NULL)
    {
        return;
    }
    piNativeService->ProcessTestCommand(
            objParcel.readInt32(), objParcel.readInt32(), objParcel.readInt32());
}
