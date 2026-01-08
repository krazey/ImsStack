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
#include "ServiceTrace.h"

#include "interface/IAosBuilder.h"
#include "interface/IAosBlock.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriber.h"
#include "interface/IAosPcscf.h"
#include "interface/IAosApplication.h"
#include "interface/IAosHandle.h"

#include "app/AosAppContext.h"
#include "provider/AosCallTracker.h"
#include "connection/AosConnection.h"
#include "provider/AosNConfiguration.h"
#include "network/AosNetTracker.h"
#include "provider/AosProvider.h"
#include "provider/AosRegStateManager.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosSubscriberManager.h"
#include "provider/AosRetryRepository.h"
#include "provider/AosTracer.h"
#include "provider/AosTransaction.h"
#include "external/AosService.h"

#include "manager/AosBuildDirector.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosBuildDirector::AosBuildDirector(IN IAosBuilder* piBuilder, IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_piBuilder(piBuilder),
        m_objConnection(ImsMap<IMS_SINT32, IAosConnection*>()),
        m_objNetTracker(ImsMap<IMS_SINT32, IAosNetTracker*>()),
        m_objAppContext(ImsMap<AString, IAosAppContext*>())
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [SLOT%d] AosBuildDirector = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosBuildDirector), this);
};

PUBLIC VIRTUAL AosBuildDirector::~AosBuildDirector()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [SLOT%d] AosBuildDirector = %" PFLS_u "/%" PFLS_x, m_nSlotId,
            sizeof(AosBuildDirector), this);
};

PUBLIC
IAosAppContext* AosBuildDirector::ConstructAos(IN AosStaticProfile* pProfile)
{
    // MUST maintain the order
    IMS_TRACE_D("ConstructAos :: slot ID(%d), app ID %s", m_nSlotId, pProfile->GetId().GetStr(), 0);

    /// AosAppContext
    IAosAppContext* piAppContext = m_piBuilder->BuildAppContext(pProfile);
    piAppContext->SetSlotId(m_nSlotId);

    /// AosConnection
    IAosConnection* piConnection = IMS_NULL;
    if (m_objConnection.GetIndexOfKey(pProfile->GetConnectionType()) < 0)
    {
        piConnection = m_piBuilder->BuildConnection(piAppContext);
        m_objConnection.Add(pProfile->GetConnectionType(), piConnection);
        piAppContext->SetConnection(piConnection);
    }
    else
    {
        piConnection = m_objConnection.GetValue(pProfile->GetConnectionType());
        piAppContext->SetConnection(piConnection);
    }

    /// AosNetTracker
    if (m_objNetTracker.GetIndexOfKey(pProfile->GetConnectionType()) < 0)
    {
        IAosNetTracker* piNetTracker = m_piBuilder->BuildNetTracker(piAppContext);
        piNetTracker->Init();
        m_objNetTracker.Add(pProfile->GetConnectionType(), piNetTracker);
        piAppContext->SetNetTracker(piNetTracker);
    }
    else
    {
        piAppContext->SetNetTracker(m_objNetTracker.GetValue(pProfile->GetConnectionType()));
    }

    /// AoSBlock
    piAppContext->SetBlock(m_piBuilder->BuildBlock(piAppContext));

    /// AoSRegistration
    IAosRegistration* piRegistration = m_piBuilder->BuildRegistration(piAppContext);
    piRegistration->Init();
    piAppContext->SetRegistration(piRegistration);

    /// AoSSubscriber
    IAosSubscriber* piSubscriber = m_piBuilder->BuildSubscriber(piAppContext);
    piSubscriber->Init();
    piAppContext->SetSubscriber(piSubscriber);

    /// AoSPcscf
    IAosPcscf* piPcscf = m_piBuilder->BuildPcscf(piAppContext);
    piPcscf->Init();
    piAppContext->SetPcscf(piPcscf);

    /// AoSApplication
    IAosApplication* piApp = m_piBuilder->BuildApp(piAppContext);
    piApp->Init();
    piAppContext->SetApp(piApp);

    /// AoSHandles
    const ImsList<AosServiceProfile*>& objProfiles = pProfile->GetServiceProfiles();
    for (IMS_UINT32 i = 0; i < objProfiles.GetSize(); i++)
    {
        AosServiceProfile* pServiceProfile = objProfiles.GetAt(i);
        if (pServiceProfile == IMS_NULL)
            continue;

        IAosHandle* piHandle = m_piBuilder->BuildHandle(
                piAppContext, pServiceProfile->GetAppId(), pServiceProfile->GetServiceId());

        piAppContext->AddHandle(pServiceProfile->GetServiceId(), piHandle);
        piHandle->Init();
    }

    m_objAppContext.Add(pProfile->GetId(), piAppContext);
    return piAppContext;
}

PUBLIC
void AosBuildDirector::ConstructProvider()
{
    IAosNConfiguration* piConfig = m_piBuilder->BuildNConfiguration();
    piConfig->Init(m_nSlotId);
    AosProvider::GetInstance()->SetNConfiguration(piConfig, m_nSlotId);

    AosProvider::GetInstance()->SetService(m_piBuilder->BuildService(m_nSlotId), m_nSlotId);
    AosProvider::GetInstance()->SetSubscriberManager(
            m_piBuilder->BuildSubscriberManager(m_nSlotId), m_nSlotId);
    AosProvider::GetInstance()->SetCallTracker(m_piBuilder->BuildCallTracker(m_nSlotId), m_nSlotId);
    AosProvider::GetInstance()->SetRegStateManager(m_piBuilder->BuildRegStateManager(), m_nSlotId);
    AosProvider::GetInstance()->SetRetryRepository(
            m_piBuilder->BuildRetryRepository(m_nSlotId), m_nSlotId);
    AosProvider::GetInstance()->SetTracer(m_piBuilder->BuildTracer(m_nSlotId), m_nSlotId);
    AosProvider::GetInstance()->SetTransaction(m_piBuilder->BuildTransaction(m_nSlotId), m_nSlotId);
}

PUBLIC
void AosBuildDirector::DestructAos()
{
    for (IMS_UINT32 i = 0; i < m_objAppContext.GetSize(); i++)
    {
        IAosAppContext* piContext = m_objAppContext.GetValueAt(i);
        if (piContext != IMS_NULL)
        {
            delete piContext;
        }
    }

    for (IMS_UINT32 i = 0; i < m_objNetTracker.GetSize(); i++)
    {
        IAosNetTracker* piTracker = m_objNetTracker.GetValueAt(i);
        if (piTracker != IMS_NULL)
        {
            delete piTracker;
        }
    }

    for (IMS_UINT32 i = 0; i < m_objConnection.GetSize(); i++)
    {
        IAosConnection* piConnection = m_objConnection.GetValueAt(i);
        if (piConnection != IMS_NULL)
        {
            delete piConnection;
        }
    }
}

PUBLIC
void AosBuildDirector::DestructProvider()
{
    IAosTransaction* piTransaction = AosProvider::GetInstance()->GetTransaction(m_nSlotId);
    if (piTransaction != IMS_NULL)
    {
        AosProvider::GetInstance()->SetTransaction(IMS_NULL, m_nSlotId);
        delete piTransaction;
    }

    IAosTracer* piTracer = AosProvider::GetInstance()->GetTracer(m_nSlotId);
    if (piTracer != IMS_NULL)
    {
        AosProvider::GetInstance()->SetTracer(IMS_NULL, m_nSlotId);
        delete piTracer;
    }

    IAosRetryRepository* piRetryRep = AosProvider::GetInstance()->GetRetryRepository(m_nSlotId);
    if (piRetryRep != IMS_NULL)
    {
        AosProvider::GetInstance()->SetRetryRepository(IMS_NULL, m_nSlotId);
        delete piRetryRep;
    }

    IAosRegStateManager* piRsm = AosProvider::GetInstance()->GetRegStateManager(m_nSlotId);
    if (piRsm != IMS_NULL)
    {
        AosProvider::GetInstance()->SetRegStateManager(IMS_NULL, m_nSlotId);
        delete piRsm;
    }

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL)
    {
        AosProvider::GetInstance()->SetCallTracker(IMS_NULL, m_nSlotId);
        delete piCallTracker;
    }

    IAosSubscriberManager* piSubscriberManager =
            AosProvider::GetInstance()->GetSubscriberManager(m_nSlotId);
    if (piSubscriberManager != IMS_NULL)
    {
        AosProvider::GetInstance()->SetSubscriberManager(IMS_NULL, m_nSlotId);
        delete piSubscriberManager;
    }

    IAosService* piService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piService != IMS_NULL)
    {
        AosProvider::GetInstance()->SetService(IMS_NULL, m_nSlotId);
        delete piService;
    }

    IAosNConfiguration* piNc = AosProvider::GetInstance()->GetNConfiguration(m_nSlotId);
    if (piNc != IMS_NULL)
    {
        AosProvider::GetInstance()->SetNConfiguration(IMS_NULL, m_nSlotId);
        delete piNc;
    }
}
