/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "SipControllerManager.h"

#include "Engine.h"
#include "IConfiguration.h"
#include "ImsServiceConfig.h"
#include "JniEnablerConnector.h"
#include "RcsMessageService.h"
#include "RcsRegistrationService.h"
#include "RcsConfigurationService.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PUBLIC SipControllerManager::SipControllerManager(
        IN const IMS_SINT32 nSlotId, IN const AString& strAppName) :
        ImsApp(strAppName),
        m_nSlotId(nSlotId),
        m_pRcsMsgService(IMS_NULL),
        m_pRcsRegService(IMS_NULL),
        m_pRcsConfService(IMS_NULL)
{
    m_strAppName = GetName();
    IMS_TRACE_D("SipController_M : SipControllerManager = %" PFLS_u, sizeof(SipControllerManager),
            0, 0);
    IMS_TRACE_I("SipControllerManager - strName (%s)", strAppName.GetStr(), 0, 0);
    m_strAppID = ImsServiceConfig::GetAppName(ImsAppId::SIP_DELEGATE);
    m_strServiceID = ImsServiceConfig::GetServiceName(ImsServiceId::SIP_DELEGATE);

    Engine::GetConfiguration()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::SIP_DELEGATE), m_nSlotId);
    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::SIP_DELEGATE, this);

    // RcsRegistrationService
    CreateRegService();
    // RcsConfigurationService
    CreateConfService();
    // RcsMessageService
    CreateMsgService();
}

PUBLIC VIRTUAL SipControllerManager::~SipControllerManager()
{
    IMS_TRACE_D("SipController_F : SipControllerManager = %" PFLS_u, sizeof(SipControllerManager),
            0, 0);
    IMS_TRACE_I("~SipControllerManager", 0, 0, 0);

    DisableService();
    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::SIP_DELEGATE, IMS_NULL);
}

PRIVATE void SipControllerManager::CreateRegService()
{
    IMS_TRACE_I("CreateRegService [%d]", m_nSlotId, 0, 0);
    m_pRcsRegService = new RcsRegistrationService(m_strAppName, m_nSlotId);
}

PRIVATE void SipControllerManager::CreateConfService()
{
    IMS_TRACE_I("CreateConfService [%d]", m_nSlotId, 0, 0);
    m_pRcsConfService = new RcsConfigurationService(m_strAppName, m_nSlotId);
}

PRIVATE void SipControllerManager::CreateMsgService()
{
    IMS_TRACE_I("CreateMsgService [%d]", m_nSlotId, 0, 0);
    m_pRcsMsgService = new RcsMessageService(m_strAppName, m_nSlotId);
}

PRIVATE void SipControllerManager::DisableService()
{
    if (m_pRcsMsgService != IMS_NULL)
    {
        delete m_pRcsMsgService;
    }
    if (m_pRcsRegService != IMS_NULL)
    {
        delete m_pRcsRegService;
    }
    if (m_pRcsConfService != IMS_NULL)
    {
        delete m_pRcsConfService;
    }
}

PROTECTED VIRTUAL void SipControllerManager::UpdateDelegateRegistration(IN IMS_UINTP nParam)
{
    IMS_TRACE_D("HandleUpdateDelegateRegistration()", 0, 0, 0);
    if (m_pRcsRegService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pRcsRegService is null", 0, 0, 0);
        return;
    }
    m_pRcsRegService->UpdateDelegateRegistration(nParam);
}

PROTECTED VIRTUAL void SipControllerManager::TriggerDelegateDeregistration()
{
    IMS_TRACE_D("HandleTriggerDelegateDeregistration", 0, 0, 0);
    if (m_pRcsRegService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pRcsRegService is null", 0, 0, 0);
        return;
    }
    m_pRcsRegService->TriggerDelegateDeregistration();
}

PROTECTED VIRTUAL void SipControllerManager::OpenMessageTracker(IN const AString& /*strThreadName*/)
{
    IMS_TRACE_D("HandleOpenMessage", 0, 0, 0);
    if (m_pRcsMsgService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pRcsMsgService is null", 0, 0, 0);
        return;
    }
    // TODO
    // RcsMessageService call
}

PROTECTED VIRTUAL void SipControllerManager::SendMessage(IN IMS_UINTP /*nParam*/)
{
    IMS_TRACE_D("HandleSendMessage", 0, 0, 0);
    if (m_pRcsMsgService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pRcsMsgService is null", 0, 0, 0);
        return;
    }
    // TODO
    // RcsMessageService call
}

PROTECTED VIRTUAL void SipControllerManager::NotifyMessageReceiveError(IN IMS_UINTP /*nParam*/)
{
    IMS_TRACE_D("HandleNotifyMessageReceiveError", 0, 0, 0);
    if (m_pRcsMsgService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pRcsMsgService is null", 0, 0, 0);
        return;
    }
    // TODO
    // RcsMessageService call
}

PROTECTED VIRTUAL void SipControllerManager::CloseSession(IN const AString& /*strCallId*/)
{
    IMS_TRACE_D("HandleCloseSession", 0, 0, 0);
    if (m_pRcsMsgService == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pRcsMsgService is null", 0, 0, 0);
        return;
    }
    // TODO
    // RcsMessageService call
}
