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

#include "IPublicationListener.h"
#include "PublicationImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PublicationImpl::PublicationImpl(IN Publication* pPublication) :
        m_pPublication(pPublication),
        m_piListener(IMS_NULL)
{
    m_pPublication->SetListener(this);
}

PUBLIC VIRTUAL PublicationImpl::~PublicationImpl()
{
    if (m_pPublication != IMS_NULL)
    {
        m_pPublication->SetListener(IMS_NULL);
        m_pPublication->Destroy();
    }
}

PRIVATE VIRTUAL void PublicationImpl::Destroy()
{
    if (m_pPublication != IMS_NULL)
    {
        m_pPublication->SetMessageMediator(IMS_NULL);
        m_pPublication->SetListener(IMS_NULL);
        m_pPublication->SetRefreshListener(IMS_NULL);
        m_pPublication->Destroy();
        m_pPublication = IMS_NULL;
    }

    delete this;
}

PRIVATE VIRTUAL ImsList<IMessage*> PublicationImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    ImsList<IMessage*> objIMessages;
    ImsList<Message*> objResponses = m_pPublication->GetPreviousResponses(nServiceMethod);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_Delivered(IN Publication* pPublication)
{
    if (m_pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PublicationDelivered(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_DeliveryFailed(IN Publication* pPublication)
{
    if (m_pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PublicationDeliveryFailed(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_Terminated(IN Publication* pPublication)
{
    if (m_pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PublicationTerminated(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_RefreshStarted(IN Publication* pPublication)
{
    if (m_pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PublicationRefreshStarted(this);
}

PRIVATE VIRTUAL void PublicationImpl::OnPublication_RefreshCompleted(IN Publication* pPublication)
{
    if (m_pPublication != pPublication)
    {
        IMS_TRACE_E(0, "PUBLICATION MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PublicationRefreshCompleted(this);
}
