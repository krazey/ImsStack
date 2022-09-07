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

#include "Configuration.h"
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"
#include "SystemConfig.h"

#include "ImsServiceConfig.h"
#include "IPageMessage.h"
#include "IuMts.h"

#include "utility/MtsDynamicLoader.h"

#include "MtsApp.h"
#include "MtsCallTracker.h"
#include "MtsService.h"
#include "message/MtsMessageController.h"

__IMS_TRACE_TAG_COM_MTS__;

LOCAL const IMS_CHAR MTS_APP_NAME[] = "MtsApp";

PUBLIC
MtsApp::MtsApp(IN IMS_SINT32 nSlotId) :
        ImsApp(MTS_APP_NAME),
        m_nSlotId(nSlotId),
        m_piMtsService(IMS_NULL),
        m_pMtsMessageController(IMS_NULL),
        m_pMtsDynamicLoader(IMS_NULL),
        m_pMtsServiceState(IMS_NULL),
        m_pCallTracker(IMS_NULL)
{
    IMS_TRACE_I("+MtsApp [slot_%d]", m_nSlotId, 0, 0);

    Configuration::GetInstance()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTS), m_nSlotId);
}

PUBLIC MtsApp::~MtsApp()
{
    IMS_TRACE_I("~MtsApp [slot_%d]", m_nSlotId, 0, 0);

    // Remove MtsUtils
    DestroyMtsUtils();

    if (m_piMtsService != IMS_NULL)
    {
        delete m_piMtsService;
        m_piMtsService = IMS_NULL;
    }

    if (m_pMtsMessageController != IMS_NULL)
    {
        delete m_pMtsMessageController;
        m_pMtsMessageController = IMS_NULL;
    }

    if (m_pCallTracker != IMS_NULL)
    {
        delete m_pCallTracker;
        m_pCallTracker = IMS_NULL;
    }
}

PUBLIC VIRTUAL void MtsApp::Start()
{
    IMS_TRACE_I("SMS Start [slot_%d]", m_nSlotId, 0, 0);

    // 1. MtsUtils
    /*===================*/
    CreateMtsUtils();

    // 2. MtsService
    /*===================*/
    CreateMtsService();

    // 3. MtsMessageController
    /*===================*/
    CreateMtsMessageController();

    // 4. Update IP Config & Make MtsServiceState
    GetSmOverIpConfigInfo();

    m_pCallTracker = new MtsCallTracker(m_nSlotId);
    m_pCallTracker->AddListener(this);
}

PUBLIC VIRTUAL void MtsApp::Stop()
{
    IMS_TRACE_I("SMS Stop [slot_%d]", m_nSlotId, 0, 0);

    if (m_pMtsServiceState != IMS_NULL)
    {
        m_pMtsServiceState->SetImsRegConnected(IMS_FALSE);
    }

    if (m_pCallTracker != IMS_NULL)
    {
        m_pCallTracker->RemoveListener(this);
    }
}

PUBLIC VIRTUAL void MtsApp::CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("CallTracker_StateChanged : nType[%d], nState[%d]", nType, nState, 0);
}

PRIVATE void MtsApp::CreateMtsService()
{
    IMS_TRACE_I("CreateMtsService [slot_%d]", m_nSlotId, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "can't make CreateMtsService", 0, 0, 0);
    }

    m_piMtsService = new MtsService(m_nSlotId, m_pMtsDynamicLoader);
}

PRIVATE void MtsApp::CreateMtsMessageController()
{
    IMS_TRACE_I("CreateMtsMessageController [slot_%d]", m_nSlotId, 0, 0);

    m_pMtsMessageController =
            new MtsMessageController(m_nSlotId, m_piMtsService, m_pMtsDynamicLoader);
}

PRIVATE void MtsApp::CreateMtsUtils()
{
    IMS_TRACE_I("CreateMtsUtils [slot_%d]", m_nSlotId, 0, 0);

    m_pMtsDynamicLoader = new MtsDynamicLoader(m_nSlotId);
    m_pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();
}

PRIVATE void MtsApp::DestroyMtsUtils()
{
    IMS_TRACE_I("DestroyMtsUtils", 0, 0, 0);

    if (m_pMtsServiceState != IMS_NULL)
    {
        m_pMtsServiceState = IMS_NULL;
    }

    if (m_pMtsDynamicLoader != IMS_NULL)
    {
        delete m_pMtsDynamicLoader;
        m_pMtsDynamicLoader = IMS_NULL;
    }
}

PRIVATE void MtsApp::GetSmOverIpConfigInfo()
{
    IMS_TRACE_I("GetSmOverIpConfigInfo", 0, 0, 0);

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(m_nSlotId);
    IMS_BOOL bSmsOverIpNetwork =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_BOOL);

    // TODO: need to check whether Mts should consider KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
    IMSVector<IMS_SINT32> objSupportedRats =
            piCc->GetIntArray(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY);

    IMS_TRACE_I("GetSmOverIpConfigInfo : bSmsOverIpNetwork[%d]", bSmsOverIpNetwork, 0, 0);

    for (IMS_UINT32 i = 0; i < objSupportedRats.GetSize(); ++i)
    {
        IMS_SINT32 nValue = objSupportedRats.GetAt(i);
        IMS_TRACE_I("GetSmOverIpConfigInfo : objSupportedRats[%d][%d]", i, nValue, 0);
    }

    pMtsServiceState->SetMtsMessageController(m_pMtsMessageController);

    if (bSmsOverIpNetwork)
    {
        pMtsServiceState->SetSmsOverIpState(IMS_TRUE);
    }
    else
    {
        pMtsServiceState->SetSmsOverIpState(IMS_FALSE);
    }
}
