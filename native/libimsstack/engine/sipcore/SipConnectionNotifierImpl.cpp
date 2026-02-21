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

#include "ISipConnectionNotifierErrorListener.h"
#include "ISipServerConnectionListener.h"
#include "SipConnectionNotifier.h"
#include "SipConnectionNotifierImpl.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipConnectionNotifierImpl::SipConnectionNotifierImpl(IN SipConnectionNotifier* pScn) :
        m_pScn(pScn),
        m_piListener(IMS_NULL)
{
    m_pScn->SetListener(this);
    m_pScn->SetErrorListener(this);
}

PUBLIC VIRTUAL SipConnectionNotifierImpl::~SipConnectionNotifierImpl()
{
    if (m_pScn != IMS_NULL)
    {
        m_pScn->SetListener(IMS_NULL);
        m_pScn->SetErrorListener(IMS_NULL);
        m_pScn->Close();
    }
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::Close()
{
    m_objErrorListeners.Clear();

    if (m_pScn != IMS_NULL)
    {
        m_pScn->SetListener(IMS_NULL);
        m_pScn->SetErrorListener(IMS_NULL);
        m_pScn->Close();
        m_pScn = IMS_NULL;
    }

    delete this;
}

PRIVATE VIRTUAL ISipServerConnection* SipConnectionNotifierImpl::AcceptAndOpen()
{
    return m_pScn->AcceptAndOpen();
}

PRIVATE VIRTUAL const IpAddress& SipConnectionNotifierImpl::GetLocalAddress() const
{
    return m_pScn->GetLocalAddress();
}

PRIVATE VIRTUAL IMS_SINT32 SipConnectionNotifierImpl::GetLocalPort() const
{
    return m_pScn->GetLocalPort();
}

PRIVATE VIRTUAL ISipServerConnection* SipConnectionNotifierImpl::AcceptAndOpen(
        OUT ISipDialog*& piOrigDialog)
{
    return m_pScn->AcceptAndOpen(piOrigDialog);
}

PRIVATE VIRTUAL AString SipConnectionNotifierImpl::GetContactAddress() const
{
    return m_pScn->GetContactAddress();
}

PRIVATE VIRTUAL SipProfile* SipConnectionNotifierImpl::GetSipProfile() const
{
    return m_pScn->GetSipProfile();
}

PRIVATE VIRTUAL IMS_SINT32 SipConnectionNotifierImpl::GetSlotId() const
{
    return m_pScn->GetSlotId();
}

PRIVATE VIRTUAL IMS_BOOL SipConnectionNotifierImpl::IsTransportResourceReserved(
        IN IMS_SINT32 nType /*= TRANSPORT_ALL*/) const
{
    return m_pScn->IsTransportResourceReserved(nType);
}

PRIVATE VIRTUAL IMS_RESULT SipConnectionNotifierImpl::ReserveTransportResource(
        IN const IpAddress& objIp, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
        IN IMS_SINT32 nPortFlowControl)
{
    return m_pScn->ReserveTransportResource(objIp, nPortS, nPortC, nPortFlowControl);
}

PRIVATE VIRTUAL IMS_RESULT SipConnectionNotifierImpl::RestoreTransportResource(
        IN IMS_SINT32 nType, IN const IpAddress& objPeerIp, IN IMS_SINT32 nPeerPort)
{
    return m_pScn->RestoreTransportResource(nType, objPeerIp, nPeerPort);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::SetFromAndContact(
        IN const AString& strFrom, IN const AString& strDisplayName, IN const AString& strUserInfo)
{
    m_pScn->SetFromAndContact(strFrom, strDisplayName, strUserInfo);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::SetSipProfile(IN SipProfile* pProfile)
{
    m_pScn->SetSipProfile(pProfile);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::UpdatePortFlowControl(IN IMS_SINT32 nPort)
{
    m_pScn->UpdatePortFlowControl(nPort);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::UpdatePortUc(IN IMS_SINT32 nPort)
{
    m_pScn->UpdatePortUc(nPort);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::AddErrorListener(
        IN ISipConnectionNotifierErrorListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objErrorListeners.GetSize(); ++i)
    {
        ISipConnectionNotifierErrorListener* piErrorListener = m_objErrorListeners.GetAt(i);

        if (piErrorListener == piListener)
        {
            IMS_TRACE_D("SCNImpl: ErrorListener(%p) is already added", piListener, 0, 0);
            return;
        }
    }

    m_objErrorListeners.Append(piListener);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::RemoveErrorListener(
        IN ISipConnectionNotifierErrorListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objErrorListeners.GetSize(); ++i)
    {
        const ISipConnectionNotifierErrorListener* piErrorListener = m_objErrorListeners.GetAt(i);

        if (piErrorListener == piListener)
        {
            m_objErrorListeners.RemoveAt(i);
            break;
        }
    }

    if (m_objErrorListeners.IsEmpty())
    {
        IMS_TRACE_D("SCNImpl: No error listeners", 0, 0, 0);
    }
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::OnServerConnection_NotifyRequest(
        IN SipConnectionNotifier* pScn)
{
    if (m_pScn != pScn)
    {
        IMS_TRACE_E(0, "SCN MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ServerConnection_NotifyRequest(this);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::OnServerConnection_NotifyForkedRequest(
        IN SipConnectionNotifier* pScn)
{
    if (m_pScn != pScn)
    {
        IMS_TRACE_E(0, "SCN MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ServerConnection_NotifyRequest(this, IMS_TRUE);
}

PRIVATE VIRTUAL void SipConnectionNotifierImpl::OnConnectionNotifierError_NotifyError(
        IN SipConnectionNotifier* pScn, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    if (m_pScn != pScn)
    {
        IMS_TRACE_E(0, "SCN MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_objErrorListeners.IsEmpty())
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    ImsList<ISipConnectionNotifierErrorListener*> objTempListeners = m_objErrorListeners;

    for (IMS_UINT32 i = 0; i < objTempListeners.GetSize(); ++i)
    {
        ISipConnectionNotifierErrorListener* piErrorListener = objTempListeners.GetAt(i);

        if (piErrorListener != IMS_NULL)
        {
            piErrorListener->ConnectionNotifierError_NotifyError(this, nCode, strMessage);
        }
    }
}
