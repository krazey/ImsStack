/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170621  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SystemConfig.h"
#include "SipIpSecState.h"
#include "SipKeepAliveHelper.h"
#include "SipMessageTracker.h"
#include "SipPacketTracker.h"
#include "SipRoutingRejectNotifier.h"
#include "SipRtConfigHelper.h"
#include "SipTransportHelper.h"
#include "SipFactoryProxy.h"

PRIVATE GLOBAL SIPFactoryProxy* SIPFactoryProxy::pFactoryProxy = IMS_NULL;

class SIPFactoryHolder
{
public:
    SIPFactoryHolder();
    ~SIPFactoryHolder();

private:
    SIPFactoryHolder(IN const SIPFactoryHolder& objRHS);
    SIPFactoryHolder& operator=(IN const SIPFactoryHolder& objRHS);

public:
    inline SIPIPSecState* GetIPSecState()
    {
        if (pIPSecState == IMS_NULL)
        {
            pIPSecState = new SIPIPSecState();
        }

        return pIPSecState;
    }

    inline SIPMessageTracker* GetMessageTracker()
    {
        if (pMessageTracker == IMS_NULL)
        {
            pMessageTracker = new SIPMessageTracker();
        }

        return pMessageTracker;
    }

    inline SIPPacketTracker* GetPacketTracker()
    {
        if (pPacketTracker == IMS_NULL)
        {
            pPacketTracker = new SIPPacketTracker();
        }

        return pPacketTracker;
    }

    inline SipRoutingRejectNotifier* GetRoutingRejectNotifier()
    {
        if (pRoutingRejectNotifier == IMS_NULL)
        {
            pRoutingRejectNotifier = new SipRoutingRejectNotifier();
        }

        return pRoutingRejectNotifier;
    }

    inline SIPRTConfigHelper* GetRtConfigHelper()
    {
        if (pRTConfigHelper == IMS_NULL)
        {
            pRTConfigHelper = new SIPRTConfigHelper();
        }

        return pRTConfigHelper;
    }

    inline SIPTransportHelper* GetTransportHelper()
    {
        if (pTransportHelper == IMS_NULL)
        {
            pTransportHelper = new SIPTransportHelper();
        }

        return pTransportHelper;
    }

    inline void SetTokenGenerator(IN ISipTokenGenerator* piTokenGenerator)
    {
        this->piTokenGenerator = piTokenGenerator;
    }

    inline IMS_BOOL IsIPSecStateEnabled() const
    {
        return (pIPSecState != IMS_NULL) && pIPSecState->IsIPSecEnabled();
    }
    inline IMS_BOOL IsMessageTrackerEnabled() const
    {
        return (pMessageTracker != IMS_NULL) && pMessageTracker->IsMessageTrackerEnabled();
    }
    inline IMS_BOOL IsPacketTrackerEnabled() const
    {
        return (pPacketTracker != IMS_NULL) && pPacketTracker->IsPacketTrackerEnabled();
    }
    inline IMS_BOOL IsRoutingRejectNotifierEnabled() const
    {
        return (pRoutingRejectNotifier != IMS_NULL) &&
                pRoutingRejectNotifier->IsNotificationRequired();
    }

private:
    void Clear();

private:
    SIPIPSecState* pIPSecState;
    SIPMessageTracker* pMessageTracker;
    SIPPacketTracker* pPacketTracker;
    SipRoutingRejectNotifier* pRoutingRejectNotifier;
    SIPRTConfigHelper* pRTConfigHelper;
    SIPTransportHelper* pTransportHelper;
    ISipTokenGenerator* piTokenGenerator;
};

PUBLIC
SIPFactoryHolder::SIPFactoryHolder() :
        pIPSecState(IMS_NULL),
        pMessageTracker(IMS_NULL),
        pPacketTracker(IMS_NULL),
        pRoutingRejectNotifier(IMS_NULL),
        pRTConfigHelper(IMS_NULL),
        pTransportHelper(IMS_NULL),
        piTokenGenerator(IMS_NULL)
{
}

PUBLIC
SIPFactoryHolder::~SIPFactoryHolder()
{
    Clear();
}

PRIVATE
void SIPFactoryHolder::Clear()
{
    if (pIPSecState != IMS_NULL)
    {
        delete pIPSecState;
        pIPSecState = IMS_NULL;
    }

    if (pMessageTracker != IMS_NULL)
    {
        delete pMessageTracker;
        pMessageTracker = IMS_NULL;
    }

    if (pPacketTracker != IMS_NULL)
    {
        delete pPacketTracker;
        pPacketTracker = IMS_NULL;
    }

    if (pRoutingRejectNotifier != IMS_NULL)
    {
        delete pRoutingRejectNotifier;
        pRoutingRejectNotifier = IMS_NULL;
    }

    if (pTransportHelper != IMS_NULL)
    {
        delete pTransportHelper;
        pTransportHelper = IMS_NULL;
    }

    if (pRTConfigHelper != IMS_NULL)
    {
        delete pRTConfigHelper;
        pRTConfigHelper = IMS_NULL;
    }

    piTokenGenerator = IMS_NULL;
}

class SIPFactoryProxyPrivate
{
public:
    SIPFactoryProxyPrivate();
    ~SIPFactoryProxyPrivate();

private:
    SIPFactoryProxyPrivate(IN const SIPFactoryProxyPrivate& objRHS);
    SIPFactoryProxyPrivate& operator=(IN const SIPFactoryProxyPrivate& objRHS);

public:
    inline SIPFactoryHolder* GetHolder(IN IMS_SINT32 nSlotId) const
    {
        if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
        {
            nSlotId = IMS_SLOT_0;
        }

        return ppHolder[nSlotId];
    }

private:
    SIPFactoryHolder** ppHolder;
};

PUBLIC
SIPFactoryProxyPrivate::SIPFactoryProxyPrivate() :
        ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppHolder = new SIPFactoryHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ppHolder[i] = new SIPFactoryHolder();
    }
}

PUBLIC
SIPFactoryProxyPrivate::~SIPFactoryProxyPrivate()
{
    if (ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppHolder[i] != IMS_NULL)
            {
                delete ppHolder[i];
            }
        }

        delete[] ppHolder;
    }
}

PRIVATE
SIPFactoryProxy::SIPFactoryProxy() :
        pPrivate(new SIPFactoryProxyPrivate())
{
}

PRIVATE
SIPFactoryProxy::~SIPFactoryProxy()
{
    if (pPrivate != IMS_NULL)
    {
        delete pPrivate;
    }
}

/*

Remarks

*/
PUBLIC
SIPIPSecState* SIPFactoryProxy::GetIPSecState(IN IMS_SINT32 nSlotId)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetIPSecState();
}

/*

Remarks

*/
PUBLIC
SIPMessageTracker* SIPFactoryProxy::GetMessageTracker(IN IMS_SINT32 nSlotId)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetMessageTracker();
}

/*

Remarks

*/
PUBLIC
SIPPacketTracker* SIPFactoryProxy::GetPacketTracker(IN IMS_SINT32 nSlotId)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetPacketTracker();
}

/*

Remarks

*/
PUBLIC
SipRoutingRejectNotifier* SIPFactoryProxy::GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetRoutingRejectNotifier();
}

/*

Remarks

*/
PUBLIC
SIPRTConfigHelper* SIPFactoryProxy::GetRtConfigHelper(IN IMS_SINT32 nSlotId)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetRtConfigHelper();
}

/*

Remarks

*/
PUBLIC
SIPTransportHelper* SIPFactoryProxy::GetTransportHelper(IN IMS_SINT32 nSlotId)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->GetTransportHelper();
}

/*

Remarks

*/
PUBLIC
void SIPFactoryProxy::SetTokenGenerator(
        IN IMS_SINT32 nSlotId, IN ISipTokenGenerator* piTokenGenerator)
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    pHolder->SetTokenGenerator(piTokenGenerator);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPFactoryProxy::IsIPSecStateEnabled(IN IMS_SINT32 nSlotId) const
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->IsIPSecStateEnabled();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPFactoryProxy::IsMessageTrackerEnabled(IN IMS_SINT32 nSlotId) const
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->IsMessageTrackerEnabled();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPFactoryProxy::IsPacketTrackerEnabled(IN IMS_SINT32 nSlotId) const
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->IsPacketTrackerEnabled();
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPFactoryProxy::IsRoutingRejectNotifierEnabled(IN IMS_SINT32 nSlotId) const
{
    SIPFactoryHolder* pHolder = pPrivate->GetHolder(nSlotId);
    return pHolder->IsRoutingRejectNotifierEnabled();
}

PUBLIC GLOBAL void SIPFactoryProxy::CreateInstance()
{
    if (pFactoryProxy == IMS_NULL)
    {
        pFactoryProxy = new SIPFactoryProxy();
    }
}

PUBLIC GLOBAL void SIPFactoryProxy::DestroyInstance()
{
    if (pFactoryProxy != IMS_NULL)
    {
        delete pFactoryProxy;
        pFactoryProxy = IMS_NULL;
    }
}

PUBLIC GLOBAL SIPFactoryProxy* SIPFactoryProxy::GetInstance()
{
    if (pFactoryProxy == IMS_NULL)
    {
        CreateInstance();
    }

    return pFactoryProxy;
}
