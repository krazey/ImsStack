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
#include "DeviceConfig.h"
#include "ServiceConfig.h"
#include "ServiceEvent.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"

#include "Engine.h"
#include "EngineLoader.h"
#include "IConfiguration.h"

#include "EnablerFactory.h"
#include "EnablerThread.h"
#include "GeolocationHelper.h"

__IMS_TRACE_TAG_BASE__;

PUBLIC
EnablerThread::EnablerThread(IN EnablerFactory* pEnablerFactory, IN IMS_SINT32 nSlotId) :
        ImsAppThread(),
        m_pEnablerFactory(pEnablerFactory),
        m_nSlotId(nSlotId),
        m_nState(STATE_INACTIVE)
{
}

PUBLIC
void EnablerThread::ControlEnablers(IN IMS_SINT32 nCtrlFlags)
{
    IMS_MSG_CreateNPostThreadMessage(GetThread(), TMSG_CONTROL_ENABLERS, nCtrlFlags, 0);
}

PROTECTED VIRTUAL IMS_BOOL EnablerThread::Initialize()
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL EnablerThread::OnStart(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("OnStart :: slotId=%d, %s", GetSlotId(), DeviceConfig::ToString().GetStr(), 0);

    ImsAppThread::OnStart(objMsg);

    ConfigService::GetConfigService()->LoadCarrierConfig(GetSlotId());

    return IMS_TRUE;
}

PROTECTED VIRTUAL IMS_BOOL EnablerThread::OnTerminate(IN ImsMessage& objMsg)
{
    IMS_TRACE_D("OnTerminate :: slotId=%d", GetSlotId(), 0, 0);

    if (GetState() == STATE_ACTIVE)
    {
        StopEnablers();
        SetState(STATE_INACTIVE);
    }

    m_pEnablerFactory->DestroyEnablers(GetSlotId());

    UninitializeGlobals();
    EngineLoader::Uninitialize(GetSlotId());

    return ImsAppThread::OnTerminate(objMsg);
}

PROTECTED VIRTUAL IMS_BOOL EnablerThread::OnMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case TMSG_CONTROL_ENABLERS:
            ControlEnablersInternal(LONG_TO_INT(objMsg.nWparam));
            return IMS_TRUE;
        default:
            return ImsAppThread::OnMessage(objMsg);
    }
}

PROTECTED
void EnablerThread::InitializeGlobals()
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(GetSlotId());
}

PROTECTED
void EnablerThread::UninitializeGlobals()
{
    GeolocationHelper::GetInstance()->DestroyPidfCreator(GetSlotId());
}

PROTECTED
void EnablerThread::ControlEnablersInternal(IN IMS_SINT32 nCtrlFlags)
{
    IMS_TRACE_D("ControlEnablersInternal: ctrlFlags=%08X", nCtrlFlags, 0, 0);

    if (IsControlSet(nCtrlFlags, CONTROL_STOP))
    {
        if (GetState() == STATE_ACTIVE)
        {
            StopEnablers();
            SetState(STATE_INACTIVE);
        }
        else
        {
            IMS_TRACE_D("ControlEnablersInternal: Enablers are already stopped.", 0, 0, 0);
        }
    }

    if (IsControlSet(nCtrlFlags, CONTROL_DESTROY))
    {
        m_pEnablerFactory->DestroyEnablers(GetSlotId());

        UninitializeGlobals();
        EngineLoader::Uninitialize(GetSlotId());
    }

    if (IsControlSet(nCtrlFlags, CONTROL_CREATE) && !m_pEnablerFactory->HasEnablers(GetSlotId()))
    {
        EventService::GetEventService()->SetUnregisteredEvents(GetSlotId());
        ConfigService::GetConfigService()->LoadCarrierConfig(GetSlotId());
        Engine::GetConfiguration()->RefreshConfigs(GetSlotId());
        EngineLoader::Initialize(GetSlotId());
        InitializeGlobals();

        m_pEnablerFactory->CreateEnablers(GetSlotId());
    }

    if (IsControlSet(nCtrlFlags, CONTROL_START))
    {
        if (GetState() == STATE_INACTIVE)
        {
            EventService::GetEventService()->SetUnregisteredEvents(GetSlotId());

            if (StartEnablers())
            {
                SetState(STATE_ACTIVE);
                NotifyEnablerStartCompleted();
            }
        }
        else
        {
            IMS_TRACE_D("ControlEnablersInternal: Enablers are already started.", 0, 0, 0);
        }
    }
}

PROTECTED
void EnablerThread::NotifyEnablerStartCompleted()
{
    IMS_EVENT_SendEventForSlotId(IMS_EVENT_NATIVE_BOOT_COMPLETED, 0, 0, GetSlotId());
}

PROTECTED
void EnablerThread::SetState(IN IMS_SINT32 nState)
{
    if (m_nState != nState)
    {
        IMS_TRACE_I("ET%02d :: %d >> %d", GetSlotId(), m_nState, nState);
        m_nState = nState;
    }
}

PROTECTED
IMS_BOOL EnablerThread::StartEnablers()
{
    const ImsList<IEnabler*>* pEnablers = m_pEnablerFactory->GetEnablers(GetSlotId());

    if (pEnablers == IMS_NULL)
    {
        IMS_TRACE_E(0, "No enablers in slot-%d", GetSlotId(), 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bStarted = IMS_FALSE;

    IMS_TRACE_I("StartEnablers :: size=%d", pEnablers->GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < pEnablers->GetSize(); i++)
    {
        IEnabler* piEnabler = pEnablers->GetAt(i);

        if (piEnabler != IMS_NULL)
        {
            bStarted = IMS_TRUE;
            piEnabler->Start();
        }
    }

    return bStarted;
}

PROTECTED
void EnablerThread::StopEnablers()
{
    const ImsList<IEnabler*>* pEnablers = m_pEnablerFactory->GetEnablers(GetSlotId());

    if (pEnablers == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("StopEnablers :: size=%d", pEnablers->GetSize(), 0, 0);

    if (pEnablers->IsEmpty())
    {
        return;
    }

    IMS_SINT32 i = static_cast<IMS_SINT32>(pEnablers->GetSize() - 1);

    for (; i >= 0; i--)
    {
        IEnabler* piEnabler = pEnablers->GetAt(i);

        if (piEnabler != IMS_NULL)
        {
            piEnabler->Stop();
        }
    }
}
