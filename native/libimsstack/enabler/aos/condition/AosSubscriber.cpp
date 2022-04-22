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
#include "ISubscriberConfig.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosRegistration.h"
#include "interface/IAosSubscriberListener.h"
#include "interface/IAosSubscriberManager.h"
#include "provider/AosProvider.h"
#include "condition/AosSubscriber.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define GET_MANAGER(SLOT) (AosProvider::GetInstance()->GetSubscriberManager(SLOT))
#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosSubscriber::AosSubscriber(IN IAosAppContext* piAppContext)
    : m_piAppContext(piAppContext)
    , m_nSlotId(m_piAppContext->GetSlotId())
    , m_piListener(IMS_NULL)
    , m_eRegType(AosRegistrationType::NORMAL)
{
    m_strTag.Sprintf("%d:%s", m_nSlotId,
            m_piAppContext->GetProfileId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosSubscriber = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosSubscriber), this);
}

PUBLIC VIRTUAL
AosSubscriber::~AosSubscriber()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosSubscriber = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosSubscriber), this);
}

PUBLIC VIRTUAL
IMS_BOOL AosSubscriber::IsReady() const
{
    return GET_MANAGER(m_nSlotId)->IsReady(m_eRegType == AosRegistrationType::FAKE);
}

PUBLIC VIRTUAL
void AosSubscriber::SetListener(IN IAosSubscriberListener* piListener)
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

PUBLIC VIRTUAL
const AStringArray& AosSubscriber::GetConfiguredImpus() const
{
    return GET_MANAGER(m_nSlotId)->GetConfiguredImpus(m_eRegType == AosRegistrationType::FAKE);
}

PUBLIC VIRTUAL
const AStringArray& AosSubscriber::GetFakeImpus() const
{
    return GET_MANAGER(m_nSlotId)->GetFakeImpus();
}

PUBLIC VIRTUAL
const ISubscriberConfig* AosSubscriber::GetSubscriberConfig(IN IMS_SINT32 nType /*= NORMAL*/) const
{
    return GET_MANAGER(m_nSlotId)->GetSubscriberConfig(nType);
}

PRIVATE
void AosSubscriber::Init()
{
    m_eRegType = m_piAppContext->GetRegistration()->GetRegType();

    const ISubscriberConfig* piSubsConfig = GetSubscriberConfig();

    // check IMS Service ON/OFF
    if (!piSubsConfig->IsServiceAllowed())
    {
        m_piAppContext->GetBlock()->SetBlockReason(BLOCK_IMS_DISABLED);
    }

    if (m_eRegType == AosRegistrationType::FAKE)
    {
        GET_MANAGER(m_nSlotId)->AddListenerForMonitor(this);
    }
    else
    {
        GET_MANAGER(m_nSlotId)->AddListener(this);
    }
    A_IMS_TRACE_D(APPPROFILE, "Init - AddListener :: (%" PFLS_X ")", this, 0, 0);

}

PRIVATE
void AosSubscriber::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    if (m_eRegType == AosRegistrationType::FAKE)
    {
        GET_MANAGER(m_nSlotId)->RemoveListenerForMonitor(this);
    }
    else
    {
        GET_MANAGER(m_nSlotId)->RemoveListener(this);
    }
}

PRIVATE
void AosSubscriber::Notify(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_D(APPPROFILE, "Notify (%d)", nState, 0, 0);
    if (m_piListener != IMS_NULL)
    {
        m_piListener->Subscriber_StateChanged(nState);
    }
}

PRIVATE
void AosSubscriber::AosSubscriberManager_NotifyState(IN IMS_UINT32 nState)
{
    Notify(nState);
}