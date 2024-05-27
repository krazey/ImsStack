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

#include "Engine.h"
#include "IConfiguration.h"
#include "ImsServiceConfig.h"
#include "MtsApp.h"
#include "MtsService.h"
#include "IMtsServiceState.h"
#include "ServiceTrace.h"
#include "message/MtsMessageController.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_MTS__;

LOCAL const IMS_CHAR MTS_APP_NAME[] = "MtsApp";

PUBLIC
MtsApp::MtsApp(IN IMS_SINT32 nSlotId) :
        ImsApp(MTS_APP_NAME),
        m_nSlotId(nSlotId),
        m_piMtsService(IMS_NULL),
        m_pMtsDynamicLoader(IMS_NULL),
        m_pMtsMessageController(IMS_NULL)
{
    IMS_TRACE_I("+MtsApp [slot_%d]", m_nSlotId, 0, 0);

    Engine::GetConfiguration()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTS), m_nSlotId);
}

PUBLIC MtsApp::~MtsApp()
{
    IMS_TRACE_I("~MtsApp [slot_%d]", m_nSlotId, 0, 0);

    if (m_pMtsDynamicLoader != IMS_NULL)
    {
        delete m_pMtsDynamicLoader;
        m_pMtsDynamicLoader = IMS_NULL;
    }

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
}

PUBLIC VIRTUAL void MtsApp::Stop()
{
    IMS_TRACE_I("SMS Stop [slot_%d]", m_nSlotId, 0, 0);

    if (m_piMtsService != IMS_NULL)
    {
        m_piMtsService->GetIMtsServiceState()->SetImsRegConnected(IMS_FALSE);
    }
}

PRIVATE void MtsApp::CreateMtsService()
{
    IMS_TRACE_I("CreateMtsService [slot_%d]", m_nSlotId, 0, 0);

    m_piMtsService = new MtsService(m_nSlotId);
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
}
