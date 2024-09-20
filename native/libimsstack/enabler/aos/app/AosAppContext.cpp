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

#include "app/AosApplication.h"
#include "condition/AosBlock.h"
#include "provider/AosStaticProfile.h"
#include "handle/AosHandle.h"
#include "connection/AosPcscf.h"
#include "registration/AosRegistration.h"
#include "condition/AosSubscriber.h"

#include "app/AosAppContext.h"

__IMS_TRACE_TAG_AOS__;

PUBLIC
AosAppContext::AosAppContext(IN AosStaticProfile* pProfile) :
        m_nSlotId(IMS_SLOT_0),
        m_pStaticProfile(pProfile),
        objAosHandles(ImsMap<AString, IAosHandle*>()),
        m_piApp(IMS_NULL),
        m_piConnection(IMS_NULL),
        m_piRegistration(IMS_NULL),
        m_piNetTracker(IMS_NULL),
        m_piBlock(IMS_NULL),
        m_piSubscriber(IMS_NULL),
        m_piPcscf(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosAppContext = %" PFLS_u "/%" PFLS_x,
            pProfile->GetId().GetStr(), sizeof(AosAppContext), this);
}

PUBLIC VIRTUAL AosAppContext::~AosAppContext()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosAppContext = %" PFLS_u "/%" PFLS_x,
            m_pStaticProfile->GetId().GetStr(), sizeof(AosAppContext), this);

    for (IMS_UINT32 i = 0; i < objAosHandles.GetSize(); i++)
    {
        IAosHandle* piHandle = objAosHandles.GetValueAt(i);
        if (piHandle != IMS_NULL)
        {
            piHandle->CleanUp();
            delete piHandle;
        }
    }

    objAosHandles.Clear();

    if (m_piApp != IMS_NULL)
    {
        m_piApp->CleanUp();
        delete m_piApp;
        m_piApp = IMS_NULL;
    }

    if (m_piPcscf != IMS_NULL)
    {
        m_piPcscf->CleanUp();
        delete m_piPcscf;
        m_piPcscf = IMS_NULL;
    }

    if (m_piSubscriber != IMS_NULL)
    {
        m_piSubscriber->CleanUp();
        delete m_piSubscriber;
        m_piSubscriber = IMS_NULL;
    }

    if (m_piRegistration != IMS_NULL)
    {
        m_piRegistration->CleanUp();
        delete m_piRegistration;
        m_piRegistration = IMS_NULL;
    }

    if (m_piBlock != IMS_NULL)
    {
        delete m_piBlock;
        m_piBlock = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_SINT32 AosAppContext::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC VIRTUAL const AString& AosAppContext::GetProfileId() const
{
    return m_pStaticProfile->GetId();
}

PUBLIC VIRTUAL IAosHandle* AosAppContext::GetHandle(IN const AString& strSrvId) const
{
    if (objAosHandles.GetIndexOfKey(strSrvId) < 0)
    {
        IMS_TRACE_D("GetHandle :: service (%s) is not exist ", strSrvId.GetStr(), 0, 0);
        return IMS_NULL;
    }

    return objAosHandles.GetValue(strSrvId);
}

PUBLIC VIRTUAL IAosHandle* AosAppContext::GetHandle(IN IMS_UINT32 nServiceType)
{
    for (IMS_UINT32 nAt = 0; nAt < objAosHandles.GetSize(); ++nAt)
    {
        IAosHandle* piHandle = objAosHandles.GetValueAt(nAt);
        if (piHandle->GetServiceType() == nServiceType)
        {
            return piHandle;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL ImsMap<AString, IAosHandle*>& AosAppContext::GetHandles()
{
    return objAosHandles;
}

PUBLIC VIRTUAL IAosApplication* AosAppContext::GetApp() const
{
    return m_piApp;
}

PUBLIC VIRTUAL IAosConnection* AosAppContext::GetConnection() const
{
    return m_piConnection;
}

PUBLIC VIRTUAL IAosRegistration* AosAppContext::GetRegistration() const
{
    return m_piRegistration;
}

PUBLIC VIRTUAL IAosNetTracker* AosAppContext::GetNetTracker() const
{
    return m_piNetTracker;
}

PUBLIC VIRTUAL IAosBlock* AosAppContext::GetBlock() const
{
    return m_piBlock;
}

PUBLIC VIRTUAL IAosSubscriber* AosAppContext::GetSubscriber() const
{
    return m_piSubscriber;
}

PUBLIC VIRTUAL IAosPcscf* AosAppContext::GetPcscf() const
{
    return m_piPcscf;
}

PUBLIC VIRTUAL AosStaticProfile* AosAppContext::GetStaticProfile() const
{
    return m_pStaticProfile;
}

PRIVATE VIRTUAL void AosAppContext::SetSlotId(IN IMS_SINT32 nSlotId)
{
    m_nSlotId = nSlotId;
}

PRIVATE VIRTUAL void AosAppContext::AddHandle(IN const AString& strSrvId, IN IAosHandle* piHandle)
{
    objAosHandles.Add(strSrvId, piHandle);
}

PRIVATE VIRTUAL void AosAppContext::SetApp(IN IAosApplication* piApp)
{
    m_piApp = piApp;
}

PRIVATE VIRTUAL void AosAppContext::SetConnection(IN IAosConnection* piConnection)
{
    m_piConnection = piConnection;
}

PRIVATE VIRTUAL void AosAppContext::SetRegistration(IN IAosRegistration* piRegistration)
{
    m_piRegistration = piRegistration;
}

PRIVATE VIRTUAL void AosAppContext::SetNetTracker(IN IAosNetTracker* piNetTracker)
{
    m_piNetTracker = piNetTracker;
}

PRIVATE VIRTUAL void AosAppContext::SetBlock(IN IAosBlock* piBlock)
{
    m_piBlock = piBlock;
}

PRIVATE VIRTUAL void AosAppContext::SetSubscriber(IN IAosSubscriber* piSubscriber)
{
    m_piSubscriber = piSubscriber;
}

PRIVATE VIRTUAL void AosAppContext::SetPcscf(IN IAosPcscf* piPcscf)
{
    m_piPcscf = piPcscf;
}
