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

#include "CapabilitiesImpl.h"
#include "ICapabilitiesListener.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CapabilitiesImpl::CapabilitiesImpl(IN Capabilities* pCapabilities) :
        m_pCapabilities(pCapabilities),
        m_piListener(IMS_NULL)
{
    m_pCapabilities->SetListener(this);
}

PUBLIC VIRTUAL CapabilitiesImpl::~CapabilitiesImpl()
{
    if (m_pCapabilities != IMS_NULL)
    {
        m_pCapabilities->SetListener(IMS_NULL);
        m_pCapabilities->Destroy();
    }
}

PRIVATE VIRTUAL void CapabilitiesImpl::Destroy()
{
    if (m_pCapabilities != IMS_NULL)
    {
        m_pCapabilities->SetMessageMediator(IMS_NULL);
        m_pCapabilities->SetListener(IMS_NULL);
        m_pCapabilities->Destroy();
        m_pCapabilities = IMS_NULL;
    }

    delete this;
}

PRIVATE VIRTUAL IMSList<IMessage*> CapabilitiesImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    IMSList<IMessage*> objIMessages;
    IMSList<Message*> objResponses = m_pCapabilities->GetPreviousResponses(nServiceMethod);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL void CapabilitiesImpl::OnCapabilities_QueryDelivered(IN Capabilities* pCapabilities)
{
    if (m_pCapabilities != pCapabilities)
    {
        IMS_TRACE_E(0, "CAPABILITIES MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->CapabilityQueryDelivered(this);
}

PRIVATE VIRTUAL void CapabilitiesImpl::OnCapabilities_QueryDeliveryFailed(
        IN Capabilities* pCapabilities)
{
    if (m_pCapabilities != pCapabilities)
    {
        IMS_TRACE_E(0, "CAPABILITIES MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->CapabilityQueryDeliveryFailed(this);
}
