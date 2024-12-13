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

#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "SipClientConnection.h"
#include "SipClientConnectionImpl.h"
#include "SipConnectionNotifierImpl.h"
#include "SipDialogImpl.h"
#include "SipPrivate.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipClientConnectionImpl::SipClientConnectionImpl(IN SipClientConnection* pScc) :
        m_pScc(pScc),
        m_pDialogImpl(IMS_NULL),
        m_piErrorListener(IMS_NULL),
        m_piListener(IMS_NULL)
{
    m_pScc->SetErrorListener(this);
    m_pScc->SetListener(this);
}

PUBLIC VIRTUAL SipClientConnectionImpl::~SipClientConnectionImpl()
{
    if (m_pDialogImpl != IMS_NULL)
    {
        m_pDialogImpl->Destroy();
    }

    if (m_pScc != IMS_NULL)
    {
        m_pScc->SetErrorListener(IMS_NULL);
        m_pScc->SetListener(IMS_NULL);
        m_pScc->Close();
    }
}

PUBLIC
IMS_RESULT SipClientConnectionImpl::InitDialogRequest()
{
    if (m_pScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (m_pDialogImpl == IMS_NULL)
    {
        SipDialog* pDialog = m_pScc->GetDialog();

        if (pDialog == IMS_NULL)
        {
            IMS_TRACE_E(0, "Dialog is null", 0, 0, 0);
            return IMS_FAILURE;
        }

        m_pDialogImpl = new SipDialogImpl(new SipDialog(*pDialog));

        if (m_pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating SipDialogImpl failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL void SipClientConnectionImpl::Close()
{
    if (m_pDialogImpl != IMS_NULL)
    {
        m_pDialogImpl->Destroy();
        m_pDialogImpl = IMS_NULL;
    }

    m_pScc->SetErrorListener(IMS_NULL);
    m_pScc->SetListener(IMS_NULL);
    m_pScc->Close();
    m_pScc = IMS_NULL;

    delete this;
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::AddHeader(
        IN const AString& strName, IN const AString& strValue)
{
    return m_pScc->AddHeader(strName, strValue);
}

PRIVATE VIRTUAL ISipDialog* SipClientConnectionImpl::GetDialog() const
{
    return m_pDialogImpl;
}

PRIVATE VIRTUAL AString SipClientConnectionImpl::GetHeader(
        IN const AString& strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    return m_pScc->GetHeader(strName, nIndex);
}

PRIVATE VIRTUAL ImsList<AString> SipClientConnectionImpl::GetHeaders(IN const AString& strName)
{
    return m_pScc->GetHeaders(strName);
}

PRIVATE VIRTUAL const SipMethod& SipClientConnectionImpl::GetMethod() const
{
    return m_pScc->GetMethod();
}

PRIVATE VIRTUAL const AString& SipClientConnectionImpl::GetReasonPhrase() const
{
    return m_pScc->GetReasonPhrase();
}

PRIVATE VIRTUAL const AString& SipClientConnectionImpl::GetRequestUri() const
{
    return m_pScc->GetRequestUri();
}

PRIVATE VIRTUAL IMS_SINT32 SipClientConnectionImpl::GetStatusCode() const
{
    return m_pScc->GetStatusCode();
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::RemoveHeader(IN const AString& strName)
{
    return m_pScc->RemoveHeader(strName);
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::Send()
{
    IMS_RESULT nResult = m_pScc->Send();

    // in-dialog & dialogUsage used
    if (nResult == IMS_SUCCESS)
    {
        SipDialog* pDialog = (m_pDialogImpl != IMS_NULL) ? m_pDialogImpl->GetDialog() : IMS_NULL;
        SipDialog* pSccDialog = m_pScc->GetDialog();

        if ((pSccDialog != IMS_NULL) && (pDialog != IMS_NULL) &&
                (pDialog->GetState() == SipDialog::STATE_CONFIRMED))
        {
            const SipMethod& objMethod = GetMethod();

            if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER))
            {
                *pDialog = *pSccDialog;
            }
        }
    }

    return nResult;
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::SetHeader(
        IN const AString& strName, IN const AString& strValue)
{
    return m_pScc->SetHeader(strName, strValue);
}

PRIVATE VIRTUAL const ByteArray& SipClientConnectionImpl::GetContent() const
{
    return m_pScc->GetContent();
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::SetContent(IN const ByteArray& objContent)
{
    return m_pScc->SetContent(objContent);
}

PRIVATE VIRTUAL IMS_SINT32 SipClientConnectionImpl::GetHeaderCount(IN const AString& strName) const
{
    return m_pScc->GetHeaderCount(strName);
}

PRIVATE VIRTUAL ISipMessage* SipClientConnectionImpl::GetMessage() const
{
    return m_pScc->GetMessage();
}

PRIVATE VIRTUAL IMS_SINT32 SipClientConnectionImpl::GetSlotId() const
{
    return m_pScc->GetSlotId();
}

PRIVATE VIRTUAL void SipClientConnectionImpl::SetSipProfile(IN SipProfile* pProfile)
{
    m_pScc->SetSipProfile(pProfile);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::SetTransactionTimerValues(
        IN const SipTimerValues& objTimerValues)
{
    m_pScc->SetTransactionTimerValues(objTimerValues);
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::InitAck()
{
    return m_pScc->InitAck();
}

PRIVATE VIRTUAL ISipClientConnection* SipClientConnectionImpl::InitCancel()
{
    // 3 To-Tag removal needs to be handled by the user because the re-INVITE may be cancelled
    // 3 Session implementation has the responsibility of the to-tag removal.

    SipClientConnection* pCancel = m_pScc->InitCancel();

    if (pCancel == IMS_NULL)
    {
        return IMS_NULL;
    }

    SipClientConnectionImpl* pCancelImpl = new SipClientConnectionImpl(pCancel);

    if (pCancelImpl == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::CONNECTION_NOT_FOUND);
        return IMS_NULL;
    }

    return pCancelImpl;
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::InitRequest(
        IN const AString& strMethod, IN ISipConnectionNotifier* piScn)
{
    SipConnectionNotifierImpl* pScnImpl = DYNAMIC_CAST(SipConnectionNotifierImpl*, piScn);
    SipConnectionNotifier* pScn = IMS_NULL;

    if (pScnImpl != IMS_NULL)
    {
        pScn = pScnImpl->GetConnectionNotifier();
    }

    if (m_pScc->InitRequest(strMethod, pScn) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    SipDialog* pDialog = m_pScc->GetDialog();

    if (pDialog != IMS_NULL)
    {
        m_pDialogImpl = new SipDialogImpl(new SipDialog(*pDialog));

        if (m_pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        }
    }

    return IMS_SUCCESS;
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::Receive(IN IMS_SLONG /* nTimeout = 0 */)
{
    return m_pScc->Receive();
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::SetCredentials(
        IN ImsList<Credential>& objCredentials)
{
    return m_pScc->SetCredentials(objCredentials);
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::SetCredentials(
        IN const Credential& objCredential)
{
    return m_pScc->SetCredentials(objCredential);
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::SetRequestUri(IN const AString& strUri)
{
    return m_pScc->SetRequestUri(strUri);
}

PRIVATE VIRTUAL ISipGenericChallenge* SipClientConnectionImpl::GetAuthenticationChallenge(
        IN IMS_SINT32 nIndex /* = 0 */) const
{
    return m_pScc->GetAuthenticationChallenge(nIndex);
}

PRIVATE VIRTUAL ISipAckPackage* SipClientConnectionImpl::GrabAck()
{
    return m_pScc->GrabAck();
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::InitResubmissionRequest()
{
    return m_pScc->InitResubmissionRequest();
}

PRIVATE VIRTUAL void SipClientConnectionImpl::RemoveAllChallenges()
{
    m_pScc->RemoveAllChallenges();
}

PRIVATE VIRTUAL void SipClientConnectionImpl::RemoveAllCredentials()
{
    m_pScc->RemoveAllCredentials();
}

PRIVATE VIRTUAL IMS_RESULT SipClientConnectionImpl::SetAuthenticationChallenge(
        IN ISipGenericChallenge* piChallenge)
{
    return m_pScc->SetAuthenticationChallenge(piChallenge);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::SetExtensionTokenForViaBranch(
        IN const AString& strToken)
{
    m_pScc->SetExtensionTokenForViaBranch(strToken);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::SetImplicitRouteHeader(
        IN const AString& strRouteHeader)
{
    m_pScc->SetImplicitRouteHeader(strRouteHeader);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::SetTransportTuple(IN const IpAddress& objIp,
        IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
        IN IMS_SINT32 nPortFc /*= Sip::PORT_UNSPECIFIED*/,
        IN IMS_SINT32 nTransportExt /*= Sip::TRANSPORT_EXT_ANY*/)
{
    m_pScc->SetTransportTuple(objIp, nPortS, nPortC, nPortFc, nTransportExt);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::OnError_NotifyError(
        IN SipConnection* pSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    if (m_pScc != pSc)
    {
        IMS_TRACE_E(0, "SCC MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piErrorListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piErrorListener->Error_NotifyError(this, nCode, strMessage);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::OnClientConnection_NotifyResponse(
        IN SipClientConnection* pScc)
{
    if (m_pScc != pScc)
    {
        IMS_TRACE_E(0, "SCC MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piListener->ClientConnection_NotifyResponse(this);
}

PRIVATE VIRTUAL void SipClientConnectionImpl::OnClientConnection_NotifyForkedResponse(
        IN SipClientConnection* pScc, IN SipClientConnection* pForkedScc)
{
    if (m_pScc != pScc)
    {
        IMS_TRACE_E(0, "SCC MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piListener == IMS_NULL)
    {
        pForkedScc->Close();

        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    SipClientConnectionImpl* pSccImpl = new SipClientConnectionImpl(pForkedScc);

    if (pSccImpl == IMS_NULL)
    {
        pForkedScc->Close();
        return;
    }

    SipDialog* pDialog = pForkedScc->GetDialog();

    if (pDialog != IMS_NULL)
    {
        pSccImpl->m_pDialogImpl = new SipDialogImpl(new SipDialog(*pDialog));

        if (pSccImpl->m_pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        }
    }

    m_piListener->ClientConnection_NotifyResponse(this, pSccImpl);
}
