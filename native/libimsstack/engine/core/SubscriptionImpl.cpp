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
#include "ServiceTrace.h"

#include "ISubscriptionListener.h"
#include "SubscriptionImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SubscriptionImpl::SubscriptionImpl(IN Subscription* pSubscription) :
        m_pSubscription(pSubscription),
        m_piListener(IMS_NULL)
{
    m_pSubscription->SetListener(this);
}

PUBLIC VIRTUAL SubscriptionImpl::~SubscriptionImpl()
{
    if (m_pSubscription != IMS_NULL)
    {
        m_pSubscription->SetListener(IMS_NULL);
        m_pSubscription->Destroy();
    }
}

PRIVATE VIRTUAL void SubscriptionImpl::Destroy()
{
    if (m_pSubscription != IMS_NULL)
    {
        m_pSubscription->SetMessageMediator(IMS_NULL);
        m_pSubscription->SetListener(IMS_NULL);
        m_pSubscription->SetRefreshListener(IMS_NULL);
        m_pSubscription->Destroy();
        m_pSubscription = IMS_NULL;
    }

    delete this;
}

PRIVATE VIRTUAL IMSList<IMessage*> SubscriptionImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = m_pSubscription->GetPreviousResponses(nServiceMethod);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMS_BOOL SubscriptionImpl::OnSubscription_ForkedNotifyReceived(
        IN Subscription* pSubscription, IN Subscription* pForkedSubscription)
{
    if (m_pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return IMS_FALSE;
    }

    if (pForkedSubscription == IMS_NULL)
    {
        IMS_TRACE_E(0, "No forked subscription", 0, 0, 0);
        return IMS_FALSE;
    }

    SubscriptionImpl* pSubscriptionImpl = new SubscriptionImpl(pForkedSubscription);

    if (pSubscriptionImpl == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a forked SubscriptionImpl failed", 0, 0, 0);
        return IMS_FALSE;
    }

    m_piListener->SubscriptionForkedNotify(this, pSubscriptionImpl);

    return IMS_TRUE;
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_NotifyReceived(
        IN Subscription* pSubscription, IN Message* pNotify, OUT IMS_BOOL& /*bDestroyNotify*/)
{
    if (m_pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SubscriptionNotify(this, pNotify);
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_Started(IN Subscription* pSubscription)
{
    if (m_pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SubscriptionStarted(this);
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_StartFailed(IN Subscription* pSubscription)
{
    if (m_pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SubscriptionStartFailed(this);
}

PRIVATE VIRTUAL void SubscriptionImpl::OnSubscription_Terminated(IN Subscription* pSubscription)
{
    if (m_pSubscription != pSubscription)
    {
        IMS_TRACE_E(0, "SUBSCRIPTION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->SubscriptionTerminated(this);
}
