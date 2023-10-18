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
#include "ServiceMemory.h"
#include "SystemConfig.h"

#include "SipFactoryProxy.h"
#include "SipIpSecState.h"
#include "SipKeepAliveHelper.h"
#include "SipMessageTracker.h"
#include "SipPacketTracker.h"
#include "SipRoutingRejectNotifier.h"
#include "SipRtConfigHelper.h"
#include "SipTransportHelper.h"

PRIVATE GLOBAL SipFactoryProxy* SipFactoryProxy::s_pFactoryProxy = IMS_NULL;

class SipFactoryHolder
{
public:
    SipFactoryHolder();
    ~SipFactoryHolder();

    SipFactoryHolder(IN const SipFactoryHolder&) = delete;
    SipFactoryHolder& operator=(IN const SipFactoryHolder&) = delete;

public:
    inline SipIpSecState* GetIpSecState()
    {
        if (m_pIpSecState == IMS_NULL)
        {
            m_pIpSecState = new SipIpSecState();
        }

        return m_pIpSecState;
    }

    inline SipMessageTracker* GetMessageTracker()
    {
        if (m_pMessageTracker == IMS_NULL)
        {
            m_pMessageTracker = new SipMessageTracker();
        }

        return m_pMessageTracker;
    }

    inline SipPacketTracker* GetPacketTracker()
    {
        if (m_pPacketTracker == IMS_NULL)
        {
            m_pPacketTracker = new SipPacketTracker();
        }

        return m_pPacketTracker;
    }

    inline SipRoutingRejectNotifier* GetRoutingRejectNotifier()
    {
        if (m_pRoutingRejectNotifier == IMS_NULL)
        {
            m_pRoutingRejectNotifier = new SipRoutingRejectNotifier();
        }

        return m_pRoutingRejectNotifier;
    }

    inline SipRtConfigHelper* GetRtConfigHelper()
    {
        if (m_pRtConfigHelper == IMS_NULL)
        {
            m_pRtConfigHelper = new SipRtConfigHelper();
        }

        return m_pRtConfigHelper;
    }

    inline SipTransportHelper* GetTransportHelper()
    {
        if (m_pTransportHelper == IMS_NULL)
        {
            m_pTransportHelper = new SipTransportHelper();
        }

        return m_pTransportHelper;
    }

    inline void SetTokenGenerator(IN ISipTokenGenerator* piTokenGenerator)
    {
        m_piTokenGenerator = piTokenGenerator;
    }

    inline IMS_BOOL IsIpSecStateEnabled() const
    {
        return (m_pIpSecState != IMS_NULL) && m_pIpSecState->IsIpSecEnabled();
    }
    inline IMS_BOOL IsMessageTrackerEnabled() const
    {
        return (m_pMessageTracker != IMS_NULL) && m_pMessageTracker->IsMessageTrackerEnabled();
    }
    inline IMS_BOOL IsPacketTrackerEnabled() const
    {
        return (m_pPacketTracker != IMS_NULL) && m_pPacketTracker->IsPacketTrackerEnabled();
    }
    inline IMS_BOOL IsRoutingRejectNotifierEnabled() const
    {
        return (m_pRoutingRejectNotifier != IMS_NULL) &&
                m_pRoutingRejectNotifier->IsNotificationRequired();
    }

private:
    void Clear();

private:
    SipIpSecState* m_pIpSecState;
    SipMessageTracker* m_pMessageTracker;
    SipPacketTracker* m_pPacketTracker;
    SipRoutingRejectNotifier* m_pRoutingRejectNotifier;
    SipRtConfigHelper* m_pRtConfigHelper;
    SipTransportHelper* m_pTransportHelper;
    ISipTokenGenerator* m_piTokenGenerator;
};

PUBLIC
SipFactoryHolder::SipFactoryHolder() :
        m_pIpSecState(IMS_NULL),
        m_pMessageTracker(IMS_NULL),
        m_pPacketTracker(IMS_NULL),
        m_pRoutingRejectNotifier(IMS_NULL),
        m_pRtConfigHelper(IMS_NULL),
        m_pTransportHelper(IMS_NULL),
        m_piTokenGenerator(IMS_NULL)
{
}

PUBLIC
SipFactoryHolder::~SipFactoryHolder()
{
    Clear();
}

PRIVATE
void SipFactoryHolder::Clear()
{
    if (m_pIpSecState != IMS_NULL)
    {
        delete m_pIpSecState;
        m_pIpSecState = IMS_NULL;
    }

    if (m_pMessageTracker != IMS_NULL)
    {
        delete m_pMessageTracker;
        m_pMessageTracker = IMS_NULL;
    }

    if (m_pPacketTracker != IMS_NULL)
    {
        delete m_pPacketTracker;
        m_pPacketTracker = IMS_NULL;
    }

    if (m_pRoutingRejectNotifier != IMS_NULL)
    {
        delete m_pRoutingRejectNotifier;
        m_pRoutingRejectNotifier = IMS_NULL;
    }

    if (m_pTransportHelper != IMS_NULL)
    {
        delete m_pTransportHelper;
        m_pTransportHelper = IMS_NULL;
    }

    if (m_pRtConfigHelper != IMS_NULL)
    {
        delete m_pRtConfigHelper;
        m_pRtConfigHelper = IMS_NULL;
    }

    m_piTokenGenerator = IMS_NULL;
}

class SipFactoryProxyPrivate
{
public:
    SipFactoryProxyPrivate();
    ~SipFactoryProxyPrivate();

    SipFactoryProxyPrivate(IN const SipFactoryProxyPrivate&) = delete;
    SipFactoryProxyPrivate& operator=(IN const SipFactoryProxyPrivate&) = delete;

public:
    inline SipFactoryHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return m_ppHolder[nSlotId];
    }

private:
    SipFactoryHolder** m_ppHolder;
};

PUBLIC
SipFactoryProxyPrivate::SipFactoryProxyPrivate() :
        m_ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppHolder = new SipFactoryHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppHolder[i] = new SipFactoryHolder();
    }
}

PUBLIC
SipFactoryProxyPrivate::~SipFactoryProxyPrivate()
{
    if (m_ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppHolder[i] != IMS_NULL)
            {
                delete m_ppHolder[i];
            }
        }

        delete[] m_ppHolder;
    }
}

PRIVATE
SipFactoryProxy::SipFactoryProxy() :
        m_pPrivate(new SipFactoryProxyPrivate())
{
}

PRIVATE
SipFactoryProxy::~SipFactoryProxy()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
SipIpSecState* SipFactoryProxy::GetIpSecState(IN IMS_SINT32 nSlotId)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetIpSecState();
}

PUBLIC
SipMessageTracker* SipFactoryProxy::GetMessageTracker(IN IMS_SINT32 nSlotId)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetMessageTracker();
}

PUBLIC
SipPacketTracker* SipFactoryProxy::GetPacketTracker(IN IMS_SINT32 nSlotId)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetPacketTracker();
}

PUBLIC
SipRoutingRejectNotifier* SipFactoryProxy::GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetRoutingRejectNotifier();
}

PUBLIC
SipRtConfigHelper* SipFactoryProxy::GetRtConfigHelper(IN IMS_SINT32 nSlotId)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetRtConfigHelper();
}

PUBLIC
SipTransportHelper* SipFactoryProxy::GetTransportHelper(IN IMS_SINT32 nSlotId)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetTransportHelper();
}

PUBLIC
void SipFactoryProxy::SetTokenGenerator(
        IN IMS_SINT32 nSlotId, IN ISipTokenGenerator* piTokenGenerator)
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    pHolder->SetTokenGenerator(piTokenGenerator);
}

PUBLIC
IMS_BOOL SipFactoryProxy::IsIpSecStateEnabled(IN IMS_SINT32 nSlotId) const
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->IsIpSecStateEnabled();
}

PUBLIC
IMS_BOOL SipFactoryProxy::IsMessageTrackerEnabled(IN IMS_SINT32 nSlotId) const
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->IsMessageTrackerEnabled();
}

PUBLIC
IMS_BOOL SipFactoryProxy::IsPacketTrackerEnabled(IN IMS_SINT32 nSlotId) const
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->IsPacketTrackerEnabled();
}

PUBLIC
IMS_BOOL SipFactoryProxy::IsRoutingRejectNotifierEnabled(IN IMS_SINT32 nSlotId) const
{
    SipFactoryHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->IsRoutingRejectNotifierEnabled();
}

PUBLIC GLOBAL void SipFactoryProxy::CreateInstance()
{
    if (s_pFactoryProxy == IMS_NULL)
    {
        s_pFactoryProxy = new SipFactoryProxy();
    }
}

PUBLIC GLOBAL void SipFactoryProxy::DestroyInstance()
{
    if (s_pFactoryProxy != IMS_NULL)
    {
        delete s_pFactoryProxy;
        s_pFactoryProxy = IMS_NULL;
    }
}

PUBLIC GLOBAL SipFactoryProxy* SipFactoryProxy::GetInstance()
{
    if (s_pFactoryProxy == IMS_NULL)
    {
        CreateInstance();
    }

    return s_pFactoryProxy;
}
