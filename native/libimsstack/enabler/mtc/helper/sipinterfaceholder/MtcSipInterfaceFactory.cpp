#include "ServiceTrace.h"

#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "helper/sipinterfaceholder/ReferenceInterfaceHolder.h"
#include "helper/sipinterfaceholder/SubscriptionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcSipInterfaceFactory::MtcSipInterfaceFactory() :
        m_piSessionHolder(IMS_NULL),
        m_piReferenceHolder(IMS_NULL),
        m_piSubscriptionHolder(IMS_NULL)
{
    IMS_TRACE_D("+MtcSipInterfaceFactory", 0, 0, 0);
}

PUBLIC
MtcSipInterfaceFactory::~MtcSipInterfaceFactory()
{
    IMS_TRACE_D("~MtcSipInterfaceFactory", 0, 0, 0);

    delete m_piSessionHolder;
    delete m_piReferenceHolder;
    delete m_piSubscriptionHolder;
}

PUBLIC VIRTUAL void MtcSipInterfaceFactory::OnSessionInterfaceCleared()
{
    delete m_piSessionHolder;
    m_piSessionHolder = IMS_NULL;
}

PUBLIC VIRTUAL void MtcSipInterfaceFactory::OnReferenceInterfaceCleared()
{
    delete m_piReferenceHolder;
    m_piReferenceHolder = IMS_NULL;
}

PUBLIC VIRTUAL void MtcSipInterfaceFactory::OnSubscriptionInterfaceCleared()
{
    delete m_piSubscriptionHolder;
    m_piSubscriptionHolder = IMS_NULL;
}

PUBLIC
SessionInterfaceHolder* MtcSipInterfaceFactory::GetISessionHolder()
{
    if (m_piSessionHolder == IMS_NULL)
    {
        m_piSessionHolder = new SessionInterfaceHolder(*this);
    }
    return m_piSessionHolder;
}

PUBLIC
ReferenceInterfaceHolder* MtcSipInterfaceFactory::GetIReferenceHolder()
{
    if (m_piReferenceHolder == IMS_NULL)
    {
        m_piReferenceHolder = new ReferenceInterfaceHolder(*this);
    }
    return m_piReferenceHolder;
}

PUBLIC
SubscriptionInterfaceHolder* MtcSipInterfaceFactory::GetISubscriptionHolder()
{
    if (m_piSubscriptionHolder == IMS_NULL)
    {
        m_piSubscriptionHolder = new SubscriptionInterfaceHolder(*this);
    }
    return m_piSubscriptionHolder;
}
