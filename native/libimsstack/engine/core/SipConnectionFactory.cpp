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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ISipConnectionFactoryListener.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "Service.h"
#include "SipConnectionFactory.h"
#include "SipDebug.h"
#include "SipStatusCode.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
SipConnectionFactory::SipConnectionFactory(IN Service* pService) :
        m_pService(pService),
        m_piDialog(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piInitialSsc(IMS_NULL),
        m_piInviteSsc(IMS_NULL)
{
    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
}

PUBLIC
SipConnectionFactory::SipConnectionFactory(IN Service* pService, IN ISipServerConnection* piSsc) :
        m_pService(pService),
        m_piDialog(IMS_NULL),
        m_piListener(IMS_NULL),
        m_piInitialSsc(piSsc),
        m_piInviteSsc(IMS_NULL)
{
    DialogMethodManager::GetInstance()->AddMethod(GetName(), this);
    CancellableMethodManager::GetInstance()->AddMethod(GetName(), this);
}

PUBLIC VIRTUAL SipConnectionFactory::~SipConnectionFactory()
{
    if (m_piDialog != IMS_NULL)
    {
        m_piDialog->Destroy();
    }
}

PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SSC_FOR_MID_DIALOG_RECEIVED:
        {
            ISipServerConnection* piSsc = reinterpret_cast<ISipServerConnection*>(objMsg.nLparam);

            if (m_piListener == IMS_NULL)
            {
                if (m_pService != IMS_NULL)
                {
                    m_pService->SendResponse(piSsc, SipStatusCode::SC_480);
                }

                piSsc->Close();
                return IMS_TRUE;
            }

            m_piListener->ConnectionFactory_NotifyRequest(this, piSsc);
            return IMS_TRUE;
        }
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Dialog_Compare(IN ISipServerConnection* piSsc) const
{
    if (m_piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSsc->GetMethod();

    if (objMethod.Equals(SipMethod::REFER))
    {
        // If the server transaction has the same dialog identifier with a dialog of this session,
        // then it will be handled by this session.
        ISipDialog* piReferDialog = piSsc->GetDialog();

        if (piReferDialog == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strDialogId = m_piDialog->GetDialogId();
        AString strReferDialogId = piReferDialog->GetDialogId();

        if (strDialogId.Equals(strReferDialogId))
        {
            IMS_TRACE_D("SipConnectionFactory :: Dialog (%s), Refer's Dialog (%s)",
                    SipDebug::GetCharA1(strDialogId.GetStr(), 8, '@'),
                    SipDebug::GetCharA2(strReferDialogId.GetStr(), 8, '@'), 0);
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    if (!m_piDialog->IsSameDialog(piSsc))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Dialog_NotifyRequest(IN ISipServerConnection* piSsc)
{
    if (m_piListener == IMS_NULL)
    {
        if (m_pService != IMS_NULL)
        {
            m_pService->SendResponse(piSsc, SipStatusCode::SC_480);
        }

        piSsc->Close();
        return IMS_TRUE;
    }

    PostMessage(AMSG_SSC_FOR_MID_DIALOG_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSsc));

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Cancellable_Compare(
        IN ISipServerConnection* piSscCancel) const
{
    if (m_piInviteSsc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return piSscCancel->IsSameTransaction(m_piInviteSsc);
}

PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::Cancellable_NotifyRequest(
        IN ISipServerConnection* piSscCancel)
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->ConnectionFactory_NotifyRequest(this, piSscCancel);
    }
    else
    {
        CreateResponse(piSscCancel, SipStatusCode::SC_200);
        piSscCancel->Send();
        piSscCancel->Close();
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void SipConnectionFactory::Destroy()
{
    DialogMethodManager::GetInstance()->RemoveMethod(GetName());
    CancellableMethodManager::GetInstance()->RemoveMethod(GetName());

    PostMessage(AMSG_DESTROY, 0, 0);
}

PUBLIC VIRTUAL ISipClientConnection* SipConnectionFactory::CreateClientConnection(
        IN const SipMethod& objMethod, IN const SipAddress* pFrom, IN const SipAddress* pTo)
{
    if (m_pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_NULL;
    }

    const SipAddress* pTempFrom;
    const SipAddress* pTempTo;
    IMS_BOOL bFromDeleteRequired = IMS_FALSE;
    IMS_BOOL bToDeleteRequired = IMS_FALSE;

    if (pFrom == IMS_NULL)
    {
        bFromDeleteRequired = IMS_TRUE;
        pTempFrom = new SipAddress(m_pService->GetAuthorizedUserId());
    }
    else
    {
        pTempFrom = pFrom;
    }

    if (pTo == IMS_NULL)
    {
        bToDeleteRequired = IMS_TRUE;
        pTempTo = new SipAddress(ImsIdentity::GetAnonymousUserId());
    }
    else
    {
        pTempTo = pTo;
    }

    ISipClientConnection* piScc = m_pService->CreateConnection(pTempFrom, pTempTo, objMethod);

    if (bFromDeleteRequired)
    {
        delete pTempFrom;
    }

    if (bToDeleteRequired)
    {
        delete pTempTo;
    }

    return piScc;
}

PUBLIC VIRTUAL ISipClientConnection* SipConnectionFactory::CreateClientConnection(
        IN ISipDialog* piDialog, IN const SipMethod& objMethod)
{
    if (m_pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_NULL;
    }

    ISipDialog* piTempDialog = (piDialog != IMS_NULL) ? piDialog : m_piDialog;

    if (piTempDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "ISipDialog is null", 0, 0, 0);
        return IMS_NULL;
    }

    return m_pService->CreateConnection(piTempDialog, objMethod);
}

PUBLIC VIRTUAL IMS_BOOL SipConnectionFactory::CreateResponse(IN_OUT ISipServerConnection* piSsc,
        IN IMS_SINT32 nStatusCode, IN const AString& strPhrase /*= AString::ConstNull()*/)
{
    if (m_pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Service is null", 0, 0, 0);
        return IMS_FALSE;
    }

    return m_pService->CreateResponse(piSsc, nStatusCode, strPhrase);
}

PUBLIC VIRTUAL void SipConnectionFactory::SetDialog(IN ISipDialog* piDialog)
{
    if (m_piDialog != IMS_NULL)
    {
        m_piDialog->Destroy();
        m_piDialog = IMS_NULL;
    }

    if (piDialog != IMS_NULL)
    {
        m_piDialog = piDialog->Clone();
    }
}
