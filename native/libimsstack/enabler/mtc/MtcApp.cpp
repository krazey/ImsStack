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

#include "AString.h"
#include "Configuration.h"
#include "IMtcService.h"
#include "ImsServiceConfig.h"
#include "JniEnablerConnector.h"
#include "JniMtcCall.h"
#include "MtcApp.h"
#include "MtcContextRepository.h"
#include "MtcImsEventReceiver.h"
#include "MtcService.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "call/MtcCallController.h"
#include "call/MtcCallManager.h"
#include "conferencecall/ConferenceManager.h"
#include "configuration/MtcConfigurationManager.h"
#include "dialingplan/MtcDialingPlan.h"
#include "dialogevent/MultiEndpointFactory.h"
#include "dialogevent/MultiEndpointManager.h"
#include "ect/EctManager.h"
#include "emergency/MtcEmergencyServiceManager.h"
#include "helper/CallStateProxy.h"
#include "helper/LastComeFirstServedHelper.h"
#include "helper/OperationAsyncRunner.h"
#include "helper/PassiveTimerHolder.h"
#include "utility/MessageUtils.h"
#include <functional>

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR MTC_APP_NAME[] = "MtcApp";

PUBLIC
MtcApp::MtcApp(IN IMS_SINT32 nSlotId) :
        ImsApp(MTC_APP_NAME),
        m_nSlotId(nSlotId),
        m_objConfigurationProxy(MtcConfigurationProxy(new MtcConfigurationManager())),
        m_lstServices(ImsList<IMtcService*>()),
        m_objDialingPlan(MtcDialingPlan(
                *this, *PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId))),
        m_objCallManager(MtcCallManager(*this)),
        m_objCallController(MtcCallController(*this)),
        m_objCallStateProxy(CallStateProxy(m_objCallManager)),
        m_objImsEventReceiver(MtcImsEventReceiver(nSlotId)),
        m_objSipInterfaceFactory(MtcSipInterfaceFactory()),
        m_objConferenceManager(ConferenceManager(*this)),
        m_pEctManager(nullptr),
        m_pEmergencyServiceManager(nullptr),
        m_objMessageUtils(MessageUtils()),
        m_objPassiveTimerHolder(PassiveTimerHolder()),
        m_pMultiEndpointManager(nullptr),
        m_objMtcRadioChecker(*this, m_objCallController),
        m_pLastComeFirstServedHelper(nullptr),
        m_bWifiTestMode(IMS_FALSE)
{
    IMS_TRACE_I("+MtcApp [slot_%d]", nSlotId, 0, 0);
    m_bWifiTestMode = (UtilService::GetUtilService()->GetPrivateProperty()->GetPersistentInt(
                               ImsPrivateProperties::Persistent::KEY_WIFI_TEST, 0) == 1);
    MtcContextRepository::GetInstance()->AddContext(nSlotId, this);
    Configuration::GetInstance()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTC), nSlotId);
}

PUBLIC VIRTUAL MtcApp::~MtcApp()
{
    IMS_TRACE_I("~MtcApp [slot_%d]", m_nSlotId, 0, 0);
    MtcContextRepository::GetInstance()->RemoveContext(m_nSlotId);
    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::MTC_CALL, IMS_NULL);
}

PUBLIC VIRTUAL void MtcApp::Start()
{
    IMS_TRACE_I("Start", 0, 0, 0);

    InitConfiguration();
    CreateServices();
    InitCallManager();
    m_objMtcRadioChecker.Init();
    m_objPassiveTimerHolder.SetNormalService(GetServiceByType(ServiceType::NORMAL));

    if (MultiEndpointManager::IsRequired(GetConfigurationProxy()))
    {
        // TODO: depends on which configuration to be checked, MultiEndpointManager can be created
        // regardless of configuration value.
        m_pMultiEndpointManager = std::make_unique<MultiEndpointManager>(
                *this, std::make_unique<MultiEndpointFactory>());
    }
}

PUBLIC VIRTUAL void MtcApp::Stop()
{
    IMS_TRACE_I("Stop", 0, 0, 0);
    DestroyServices();
    m_objCallManager.DeInit();
    m_objPassiveTimerHolder.SetNormalService(IMS_NULL);
}

PUBLIC VIRTUAL IMtcService* MtcApp::GetServiceByType(IN ServiceType eServiceType)
{
    for (IMS_UINT32 i = 0; i < m_lstServices.GetSize(); i++)
    {
        IMtcService* piService = m_lstServices.GetAt(i);

        if (eServiceType == piService->GetServiceType())
        {
            IMS_TRACE_I("GetServiceByType : Type[%d]", eServiceType, 0, 0);
            return piService;
        }
    }

    IMS_TRACE_D("GetServiceByType : Service Type[%d] Not Found", eServiceType, 0, 0);
    return IMS_NULL;
}

PUBLIC VIRTUAL IMtcAosConnector* MtcApp::GetAosConnector(IN ServiceType eServiceType)
{
    for (IMS_UINT32 i = 0; i < m_lstServices.GetSize(); i++)
    {
        IMtcService* piService = m_lstServices.GetAt(i);

        if (eServiceType == piService->GetServiceType())
        {
            return piService->GetAosConnector();
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL IEctManager& MtcApp::GetEctManager()
{
    if (m_pEctManager == nullptr)
    {
        m_pEctManager = std::make_unique<EctManager>(*this);
    }

    return *m_pEctManager.get();
}

PUBLIC VIRTUAL IMtcEmergencyServiceManager& MtcApp::GetEmergencyServiceManager()
{
    if (m_pEmergencyServiceManager == nullptr)
    {
        m_pEmergencyServiceManager = std::make_unique<MtcEmergencyServiceManager>(*this);
    }

    return *m_pEmergencyServiceManager.get();
}

PUBLIC VIRTUAL OperationAsyncRunner* MtcApp::GetAsyncRunner(IN std::function<void()> objOperation)
{
    if (objOperation == nullptr)
    {
        return IMS_NULL;
    }
    // object is deleted by itself
    return new OperationAsyncRunner(m_nSlotId, objOperation);
}

PUBLIC VIRTUAL ILastComeFirstServedHelper& MtcApp::GetLastComeFirstServedHelper()
{
    if (m_pLastComeFirstServedHelper == nullptr)
    {
        m_pLastComeFirstServedHelper = std::make_unique<LastComeFirstServedHelper>(*this);
    }

    return *m_pLastComeFirstServedHelper.get();
}

PROTECTED VIRTUAL void MtcApp::InitConfiguration()
{
    m_objConfigurationProxy.Init();
}

PROTECTED VIRTUAL void MtcApp::CreateServices()
{
    DestroyServices();

    m_lstServices.Append(new MtcService(*this, ServiceType::NORMAL));
    m_lstServices.Append(new MtcService(*this, ServiceType::EMERGENCY));
}

PROTECTED VIRTUAL void MtcApp::InitCallManager()
{
    m_objCallManager.Init();
    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::MTC_CALL, &GetCallController());
}

PROTECTED VIRTUAL void MtcApp::DestroyServices()
{
    for (IMS_UINT32 i = 0; i < m_lstServices.GetSize(); i++)
    {
        delete m_lstServices.GetAt(i);
    }
    m_lstServices.Clear();
}
