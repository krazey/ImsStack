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
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServicePhoneInfo.h"
#include "SystemConfig.h"

#include "ImsServiceConfig.h"
#include "IPageMessage.h"
#include "IuMts.h"

#include "utility/MtsStrName.h"
#include "utility/MtsDynamicLoader.h"

#include "MtsApp.h"
#include "MtsCallTracker.h"
#include "MtsService.h"
#include "message/MtsMessageController.h"

__IMS_TRACE_TAG_COM_SMS__;

LOCAL const IMS_CHAR MTS_APP_NAME[] = "MtsApp";

PUBLIC
MtsApp::MtsApp(IN IMS_SINT32 nSlotId) :
        ImsApp(MTS_APP_NAME),
        m_nSlotId(nSlotId),
        m_pMtsService(IMS_NULL),
        m_pMtsMessageController(IMS_NULL),
        m_pMtsDynamicLoader(IMS_NULL),
        m_pMtsServiceState(IMS_NULL),
        m_pCallTracker(IMS_NULL)
{
    IMS_TRACE_I("+MtsApp [slot_%d]", nSlotId, 0, 0);

    Configuration::GetInstance()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTS), nSlotId);
}

PUBLIC MtsApp::~MtsApp()
{
    IMS_TRACE_I("~MtsApp", 0, 0, 0);

    // Remove MtsUtils
    DestroyMtsUtils();

    // Remove Mts Service
    RemoveMtsServices();

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

PUBLIC void MtsApp::AddService(IN MtsService* pService)
{
    if (pService == IMS_NULL)
    {
        return;
    }

    m_lstMtsServices.Append(pService);
    AttachService(pService);

    IMS_TRACE_I("AddService : ID[%s] Size[%d]", pService->GetId().GetStr(),
            m_lstMtsServices.GetSize(), 0);
}

PUBLIC void MtsApp::RemoveMtsServices()
{
    IMS_UINT32 nSize = m_lstMtsServices.GetSize();

    for (IMS_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        MtsService* pService = m_lstMtsServices.GetAt(nIndex);

        if (pService != IMS_NULL)
        {
            DetachService(pService);
            delete pService;
        }
    }
    m_lstMtsServices.RemoveElementsAt(0, nSize);
}

PUBLIC VIRTUAL void MtsApp::Start()
{
    IMS_TRACE_I("SMS Start : m_nSlotId : [%d]", m_nSlotId, 0, 0);

    // MtsUtils
    /*===================*/
    CreateMtsUtils(m_nSlotId);

    // MtsService
    /*===================*/
    CreateMtsService(m_nSlotId);

    // MtsMessageController
    /*===================*/
    CreateMtsMessageController(m_nSlotId, m_pMtsDynamicLoader);

    // Update IP Config & Make MtsServiceState
    GetSmOverIpConfigInfo(m_nSlotId);

    m_pCallTracker = new MtsCallTracker(m_nSlotId);
    m_pCallTracker->AddListener(this);
}

PUBLIC VIRTUAL void MtsApp::Stop()
{
    IMS_TRACE_I("SMS Stop : m_nSlotId : [%d]", m_nSlotId, 0, 0);

    if (m_pMtsServiceState != IMS_NULL)
    {
        IMS_TRACE_I("SetIMSRegState(IMS_FALSE)", 0, 0, 0);
        m_pMtsServiceState->SetImsRegConnected(IMS_FALSE);
    }

    if (m_pCallTracker != IMS_NULL)
    {
        m_pCallTracker->RemoveListener(this);
    }
}

PUBLIC VIRTUAL void MtsApp::MtsMessageController_NoTransaction()
{
    IMS_TRACE_I("MtsApp::MtsMessageController_NoTransaction()", 0, 0, 0);

    if (m_pMtsServiceState == IMS_NULL)
    {
        IMS_TRACE_E(0, "Fail to get MtsServiceState instance", 0, 0, 0);
        return;
    }

    /*
     * TODO: check why SmsOverIp update is needed here
     * If it does not need, then remove
     */
    IMS_BOOL bSmsOverIpNetwork = IMS_TRUE;

    if (bSmsOverIpNetwork)
    {
        m_pMtsServiceState->SetSmsOverIpState(IMS_TRUE);
    }
    else
    {
        m_pMtsServiceState->SetSmsOverIpState(IMS_FALSE);
    }

    if (m_pMtsMessageController != IMS_NULL)
    {
        m_pMtsMessageController->DeregisterNoTransactionListener(this);
    }
}

PUBLIC VIRTUAL void MtsApp::CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("MtsApp::CallTracker_StateChanged, nType = [%d], nState = [%d]", nType, nState, 0);
}

PRIVATE void MtsApp::CreateMtsService(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("CreateMtsService nSlotId : [%d]", nSlotId, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "can't make CreateMtsService", 0, 0, 0);
    }

    m_pMtsService = new MtsService(m_pMtsDynamicLoader->GetMtsStrName()->GetMtsAppId(),
            m_pMtsDynamicLoader->GetMtsStrName()->GetMtsServiceId(), m_nSlotId,
            m_pMtsDynamicLoader);

    if (m_pMtsService != IMS_NULL)
    {
        AddService(m_pMtsService);
    }
    else
    {
        IMS_TRACE_E(0, "m_pMtsService is NULL", 0, 0, 0);
    }
}

PRIVATE void MtsApp::CreateMtsMessageController(
        IN IMS_SINT32 nSlotId, IN MtsDynamicLoader* pMtsDynamicLoader)
{
    m_pMtsMessageController = new MtsMessageController(nSlotId, m_pMtsService, pMtsDynamicLoader);
}

PRIVATE void MtsApp::CreateMtsUtils(IN IMS_SINT32 nSlotId)
{
    m_pMtsDynamicLoader = new MtsDynamicLoader(nSlotId);

    if (m_pMtsDynamicLoader != IMS_NULL)
    {
        m_pMtsDynamicLoader->Initialize(nSlotId);
        MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();
        m_pMtsServiceState = pMtsServiceState;
    }

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is NULL", 0, 0, 0);
    }
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

PRIVATE void MtsApp::GetSmOverIpConfigInfo(IN IMS_SINT32 nSlotId)
{
    /*
     * TODO: check carrier configuration
     * KEY_SMS_OVER_IMS_SUPPORTED_BOOL && KEY_SMS_OVER_IMS_SUPPORTED_RATS_INT_ARRAY
     */
    (void)nSlotId;
    IMS_BOOL bSmsOverIpNetwork = IMS_TRUE;

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState == IMS_NULL)
    {
        IMS_TRACE_E(0, "Fail to get MtsServiceState instance", 0, 0, 0);
        return;
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
