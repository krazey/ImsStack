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
#include "ImsIdentity.h"
#include "ServiceTrace.h"
#include "ISubscriberConfig.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriberListener.h"
#include "interface/IAosSubscriberManager.h"
#include "provider/AosProvider.h"
#include "condition/AosSubscriber.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosSubscriber::AosSubscriber(IN IAosAppContext* piAppContext) :
        m_piAppContext(piAppContext),
        m_piSubscriberManager(IMS_NULL),
        m_nSlotId(m_piAppContext->GetSlotId()),
        m_piListener(IMS_NULL),
        m_eRegType(AosRegistrationType::NORMAL),
        m_strTempPuidForGiba(AString::ConstNull())
{
    m_strTag.Sprintf("%d:%s", m_nSlotId, m_piAppContext->GetProfileId().GetStr());
    m_piSubscriberManager = AosProvider::GetInstance()->GetSubscriberManager(m_nSlotId);

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosSubscriber = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosSubscriber), this);
}

PUBLIC VIRTUAL AosSubscriber::~AosSubscriber()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosSubscriber = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosSubscriber), this);
}

PUBLIC VIRTUAL IMS_BOOL AosSubscriber::IsReady() const
{
    return (m_piSubscriberManager != IMS_NULL)
            ? m_piSubscriberManager->IsReady(m_eRegType == AosRegistrationType::FAKE)
            : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosSubscriber::IsIsim() const
{
    return (m_piSubscriberManager != IMS_NULL) ? m_piSubscriberManager->IsIsim() : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosSubscriber::IsUsim() const
{
    return (m_piSubscriberManager != IMS_NULL) ? m_piSubscriberManager->IsUsim() : IMS_FALSE;
}

PUBLIC VIRTUAL void AosSubscriber::SetListener(IN IAosSubscriberListener* piListener)
{
    A_IMS_TRACE_D(APPPROFILE, "SetListener :: (%" PFLS_x ") is set", piListener, 0, 0);
    m_piListener = piListener;

    if (IsReady())
    {
        Notify(READY);
    }
    else
    {
        Notify(NOT_READY);
    }
}

PUBLIC VIRTUAL const AStringArray& AosSubscriber::GetConfiguredImpus() const
{
    if (m_piSubscriberManager == IMS_NULL)
    {
        return AStringArray::ConstNull();
    }

    if (m_eRegType == AosRegistrationType::FAKE)
    {
        A_IMS_TRACE_D(APPPROFILE, "GetConfiguredImpus: Configured IMPU for fake", 0, 0, 0);
        return m_piSubscriberManager->GetConfiguredImpusForFake();
    }

    if (m_eRegType == AosRegistrationType::EMERGENCY &&
            GET_N_CONFIG(m_nSlotId)->IsERegUsingFirstImpuInIsim())
    {
        A_IMS_TRACE_D(APPPROFILE, "GetConfiguredImpus: Ordered IMPUs", 0, 0, 0);
        return m_piSubscriberManager->GetOrderedImpus();
    }

    A_IMS_TRACE_D(APPPROFILE, "GetConfiguredImpus: Configured IMPU", 0, 0, 0);
    return m_piSubscriberManager->GetConfiguredImpus();
}

PUBLIC VIRTUAL const AStringArray& AosSubscriber::GetFakeImpus() const
{
    return (m_piSubscriberManager != IMS_NULL) ? m_piSubscriberManager->GetFakeImpus()
                                               : AStringArray::ConstNull();
}

PUBLIC VIRTUAL const ISubscriberConfig* AosSubscriber::GetSubscriberConfig(
        IN IMS_SINT32 nType /*= NORMAL*/) const
{
    return (m_piSubscriberManager != IMS_NULL) ? m_piSubscriberManager->GetSubscriberConfig(nType)
                                               : IMS_NULL;
}

PROTECTED VIRTUAL void AosSubscriber::CreateTemporaryPublicUserIdForGiba()
{
    m_strTempPuidForGiba = AString(ImsIdentity::CreateTemporaryPublicUserId(m_nSlotId));
}

PROTECTED VIRTUAL void AosSubscriber::ClearTemporaryPublicUserIdForGiba()
{
    m_strTempPuidForGiba = AString::ConstNull();
}

PROTECTED VIRTUAL IMS_BOOL AosSubscriber::HasValidTemporaryPublicUserIdForGiba() const
{
    return m_strTempPuidForGiba.GetLength() != 0;
}

PROTECTED VIRTUAL const AString& AosSubscriber::GetTemporaryPublicUserIdForGiba() const
{
    return m_strTempPuidForGiba;
}

PROTECTED
IMS_BOOL AosSubscriber::Init()
{
    m_eRegType = m_piAppContext->GetRegistration()->GetRegType();

    const ISubscriberConfig* piSubsConfig = GetSubscriberConfig();

    // check IMS Service ON/OFF
    if (piSubsConfig != IMS_NULL && !piSubsConfig->IsServiceAllowed())
    {
        m_piAppContext->GetBlock()->SetBlockReason(BLOCK_IMS_DISABLED);
    }

    if (m_piSubscriberManager == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_eRegType == AosRegistrationType::FAKE)
    {
        m_piSubscriberManager->AddListenerForMonitor(this);
    }
    else
    {
        m_piSubscriberManager->AddListener(this);
    }
    A_IMS_TRACE_D(APPPROFILE, "Init - AddListener :: (%" PFLS_X ")", this, 0, 0);

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosSubscriber::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    if (m_piSubscriberManager == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_eRegType == AosRegistrationType::FAKE)
    {
        m_piSubscriberManager->RemoveListenerForMonitor(this);
    }
    else
    {
        m_piSubscriberManager->RemoveListener(this);
    }

    return IMS_TRUE;
}

PROTECTED
void AosSubscriber::Notify(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_D(APPPROFILE, "Notify (%d)", nState, 0, 0);
    if (m_piListener != IMS_NULL)
    {
        m_piListener->Subscriber_StateChanged(nState);
    }
}

PROTECTED
void AosSubscriber::AosSubscriberManager_NotifyState(IN IMS_UINT32 nState)
{
    Notify(nState);
}
