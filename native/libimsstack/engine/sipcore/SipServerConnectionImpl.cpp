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

#include "ISipErrorListener.h"
#include "SipDialogImpl.h"
#include "SipPrivate.h"
#include "SipServerConnection.h"
#include "SipServerConnectionImpl.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipServerConnectionImpl::SipServerConnectionImpl(IN SipServerConnection* pSsc) :
        m_pSsc(pSsc),
        m_pDialogImpl(IMS_NULL),
        m_piErrorListener(IMS_NULL)
{
    m_pSsc->SetErrorListener(this);

    SipDialog* pDialog = m_pSsc->GetDialog();

    if (pDialog != IMS_NULL)
    {
        m_pDialogImpl = new SipDialogImpl(new SipDialog(*pDialog));

        if (m_pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        }
    }
}

PUBLIC VIRTUAL SipServerConnectionImpl::~SipServerConnectionImpl()
{
    if (m_pDialogImpl != IMS_NULL)
    {
        m_pDialogImpl->Destroy();
    }

    if (m_pSsc != IMS_NULL)
    {
        m_pSsc->SetErrorListener(IMS_NULL);
        m_pSsc->Close();
    }
}

PRIVATE VIRTUAL void SipServerConnectionImpl::Close()
{
    if (m_pDialogImpl != IMS_NULL)
    {
        m_pDialogImpl->Destroy();
        m_pDialogImpl = IMS_NULL;
    }

    m_pSsc->SetErrorListener(IMS_NULL);
    m_pSsc->Close();
    m_pSsc = IMS_NULL;

    delete this;
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::AddHeader(
        IN const AString& strName, IN const AString& strValue)
{
    return m_pSsc->AddHeader(strName, strValue);
}

PRIVATE VIRTUAL ISipDialog* SipServerConnectionImpl::GetDialog() const
{
    return m_pDialogImpl;
}

PRIVATE VIRTUAL AString SipServerConnectionImpl::GetHeader(
        IN const AString& strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    return m_pSsc->GetHeader(strName, nIndex);
}

PRIVATE VIRTUAL ImsList<AString> SipServerConnectionImpl::GetHeaders(IN const AString& strName)
{
    return m_pSsc->GetHeaders(strName);
}

PRIVATE VIRTUAL const SipMethod& SipServerConnectionImpl::GetMethod() const
{
    return m_pSsc->GetMethod();
}

PRIVATE VIRTUAL const AString& SipServerConnectionImpl::GetReasonPhrase() const
{
    return m_pSsc->GetReasonPhrase();
}

PRIVATE VIRTUAL const AString& SipServerConnectionImpl::GetRequestUri() const
{
    return m_pSsc->GetRequestUri();
}

PRIVATE VIRTUAL IMS_SINT32 SipServerConnectionImpl::GetStatusCode() const
{
    return m_pSsc->GetStatusCode();
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::RemoveHeader(IN const AString& strName)
{
    return m_pSsc->RemoveHeader(strName);
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::Send()
{
    return m_pSsc->Send();
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::SetHeader(
        IN const AString& strName, IN const AString& strValue)
{
    return m_pSsc->SetHeader(strName, strValue);
}

PRIVATE VIRTUAL const ByteArray& SipServerConnectionImpl::GetContent() const
{
    return m_pSsc->GetContent();
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::SetContent(IN const ByteArray& objContent)
{
    return m_pSsc->SetContent(objContent);
}

PRIVATE VIRTUAL IMS_SINT32 SipServerConnectionImpl::GetHeaderCount(IN const AString& strName) const
{
    return m_pSsc->GetHeaderCount(strName);
}

PRIVATE VIRTUAL ISipMessage* SipServerConnectionImpl::GetMessage() const
{
    return m_pSsc->GetMessage();
}

PRIVATE VIRTUAL IMS_SINT32 SipServerConnectionImpl::GetSlotId() const
{
    return m_pSsc->GetSlotId();
}

PRIVATE VIRTUAL void SipServerConnectionImpl::SetSipProfile(IN SipProfile* pProfile)
{
    m_pSsc->SetSipProfile(pProfile);
}

PRIVATE VIRTUAL void SipServerConnectionImpl::SetTransactionTimerValues(
        IN const SipTimerValues& objTimerValues)
{
    m_pSsc->SetTransactionTimerValues(objTimerValues);
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::InitResponse(IN IMS_SINT32 nStatusCode)
{
    return m_pSsc->InitResponse(nStatusCode);
}

PRIVATE VIRTUAL IMS_RESULT SipServerConnectionImpl::SetReasonPhrase(
        IN const AString& strReasonPhrase)
{
    return m_pSsc->SetReasonPhrase(strReasonPhrase);
}

PRIVATE VIRTUAL IMS_BOOL SipServerConnectionImpl::IsSameTransaction(
        IN const ISipServerConnection* piOngoingSsc) const
{
    const SipServerConnectionImpl* pSscImpl =
            DYNAMIC_CAST(const SipServerConnectionImpl*, piOngoingSsc);

    if (pSscImpl == IMS_NULL)
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    if (pSscImpl->m_pSsc == IMS_NULL)
    {
        // Ignore the CANCEL request
        // because the ongoing transaction is already completed or terminated.
        return IMS_FALSE;
    }

    return m_pSsc->IsSameTransaction(pSscImpl->m_pSsc);
}

PRIVATE VIRTUAL void SipServerConnectionImpl::OnError_NotifyError(
        IN SipConnection* pSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    if (m_pSsc != pSc)
    {
        IMS_TRACE_E(0, "SSC MISMATCHED", 0, 0, 0);
        return;
    }

    if (m_piErrorListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    m_piErrorListener->Error_NotifyError(this, nCode, strMessage);
}
