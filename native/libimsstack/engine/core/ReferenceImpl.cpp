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

#include "INotificationListener.h"
#include "IReferenceListener.h"
#include "ReferenceImpl.h"
#include "SessionImpl.h"
#include "base/Ims.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
ReferenceImpl::ReferenceImpl(IN Reference* pReference) :
        m_pReference(pReference),
        m_piListener(IMS_NULL),
        m_piNotificationListener(IMS_NULL)
{
    m_pReference->SetListener(this);
}

PUBLIC VIRTUAL ReferenceImpl::~ReferenceImpl()
{
    if (m_pReference != IMS_NULL)
    {
        m_pReference->SetListener(IMS_NULL);
        m_pReference->Destroy();
    }
}

PRIVATE VIRTUAL void ReferenceImpl::Destroy()
{
    if (m_pReference != IMS_NULL)
    {
        m_pReference->SetMessageMediator(IMS_NULL);
        m_pReference->SetNotificationListener(IMS_NULL);
        m_pReference->SetListener(IMS_NULL);
        m_pReference->Destroy();
        m_pReference = IMS_NULL;
    }

    delete this;
}

PRIVATE VIRTUAL ImsList<IMessage*> ReferenceImpl::GetPreviousResponses(
        IN IMS_SINT32 nServiceMethod) const
{
    ImsList<IMessage*> objIMessages;
    ImsList<Message*> objResponses = m_pReference->GetPreviousResponses(nServiceMethod);

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        objIMessages.Append(objResponses.GetAt(i));
    }

    return objIMessages;
}

PRIVATE VIRTUAL IMS_RESULT ReferenceImpl::ConnectReferMethod(IN IServiceMethod* piServiceMethod)
{
    if (piServiceMethod == IMS_NULL)
    {
        Ims::SetLastError(ImsError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    const SipMethod& objMethod = m_pReference->GetReferMethod();
    Method* pMethod = IMS_NULL;

    if (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::BYE))
    {
        const SessionImpl* pSessionImpl = DYNAMIC_CAST(SessionImpl*, piServiceMethod);

        pMethod = pSessionImpl->GetSession();
    }

    // FIXME: If any other method needs to be handled, then add the code below...

    return m_pReference->ConnectReferMethod(pMethod);
}

PRIVATE VIRTUAL void ReferenceImpl::SetNotificationListener(IN INotificationListener* piListener)
{
    m_piNotificationListener = piListener;

    if (m_piNotificationListener != IMS_NULL)
    {
        m_pReference->SetNotificationListener(this);
    }
    else
    {
        m_pReference->SetNotificationListener(IMS_NULL);
    }
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_Delivered(IN Reference* pReference)
{
    if (m_pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ReferenceDelivered(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_DeliveryFailed(IN Reference* pReference)
{
    if (m_pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ReferenceDeliveryFailed(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_NotifyReceived(
        IN Reference* pReference, IN Message* pNotify)
{
    if (m_pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ReferenceNotify(this, pNotify);
}

PRIVATE VIRTUAL void ReferenceImpl::OnReference_Terminated(IN Reference* pReference)
{
    if (m_pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ReferenceTerminated(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnNotification_Delivered(IN ServiceMethod* pMethod)
{
    const Reference* pReference = DYNAMIC_CAST(Reference*, pMethod);

    if (m_pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piNotificationListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER (INotificationListener)", 0, 0, 0);
        return;
    }

    m_piNotificationListener->NotificationDelivered(this);
}

PRIVATE VIRTUAL void ReferenceImpl::OnNotification_DeliveryFailed(
        IN ServiceMethod* pMethod, IN IMS_SINT32 nStatusCode)
{
    const Reference* pReference = DYNAMIC_CAST(Reference*, pMethod);

    if (m_pReference != pReference)
    {
        IMS_TRACE_E(0, "REFERENCE MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piNotificationListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER (INotificationListener)", 0, 0, 0);
        return;
    }

    m_piNotificationListener->NotificationDeliveryFailed(this, nStatusCode);
}
