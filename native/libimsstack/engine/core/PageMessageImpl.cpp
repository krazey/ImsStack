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

#include "IPageMessageListener.h"
#include "PageMessageImpl.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
PageMessageImpl::PageMessageImpl(IN PageMessage* pPageMessage) :
        m_pPageMessage(pPageMessage),
        m_piListener(IMS_NULL)
{
    m_pPageMessage->SetListener(this);
}

PUBLIC VIRTUAL PageMessageImpl::~PageMessageImpl()
{
    if (m_pPageMessage != IMS_NULL)
    {
        m_pPageMessage->SetListener(IMS_NULL);
        m_pPageMessage->Destroy();
    }
}

PRIVATE VIRTUAL void PageMessageImpl::Destroy()
{
    if (m_pPageMessage != IMS_NULL)
    {
        m_pPageMessage->SetMessageMediator(IMS_NULL);
        m_pPageMessage->SetListener(IMS_NULL);
        m_pPageMessage->Destroy();
        m_pPageMessage = IMS_NULL;
    }

    delete this;
}

PRIVATE VIRTUAL ImsList<IMessage*> PageMessageImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    ImsList<IMessage*> objIMessages;
    ImsList<Message*> objResponses = m_pPageMessage->GetPreviousResponses(nServiceMethod);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL void PageMessageImpl::OnPageMessage_Delivered(IN PageMessage* pPageMessage)
{
    if (m_pPageMessage != pPageMessage)
    {
        IMS_TRACE_E(0, "PAGE MESSAGE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PageMessageDelivered(this);
}

PRIVATE VIRTUAL void PageMessageImpl::OnPageMessage_DeliveryFailed(IN PageMessage* pPageMessage)
{
    if (m_pPageMessage != pPageMessage)
    {
        IMS_TRACE_E(0, "PAGE MESSAGE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->PageMessageDeliveryFailed(this);
}
