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
#include "Credential.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"

#include "ISipDialog.h"
#include "ISipGenericChallenge.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipError.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "base/Ims.h"
#include "base/Method.h"
#include "base/SubscriberTracker.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
Method::Method() :
        EngineActivity(),
        m_bMobileOriginated(IMS_TRUE),
        m_pUserAor(IMS_NULL),
        m_pRemoteUserAor(IMS_NULL),
        m_objRemoteUserIds(ImsList<AString>()),
        m_piDialog(IMS_NULL),
        m_piAuthChallenge(IMS_NULL),
        m_objAuthChallengeMap(ImsMap<IMS_SINT32, IMS_SINT32>()),
        m_piMessageMediator(IMS_NULL)
{
    // AUTH_SIP_DIGEST {
    m_objAuthChallengeMap.Add(SipMethod::BYE, 0);
    m_objAuthChallengeMap.Add(SipMethod::CANCEL, 0);
    m_objAuthChallengeMap.Add(SipMethod::INVITE, 0);
    m_objAuthChallengeMap.Add(SipMethod::OPTIONS, 0);
    m_objAuthChallengeMap.Add(SipMethod::PRACK, 0);
    m_objAuthChallengeMap.Add(SipMethod::SUBSCRIBE, 0);
    m_objAuthChallengeMap.Add(SipMethod::NOTIFY, 0);
    m_objAuthChallengeMap.Add(SipMethod::UPDATE, 0);
    m_objAuthChallengeMap.Add(SipMethod::MESSAGE, 0);
    m_objAuthChallengeMap.Add(SipMethod::REFER, 0);
    m_objAuthChallengeMap.Add(SipMethod::PUBLISH, 0);
    m_objAuthChallengeMap.Add(SipMethod::INFO, 0);
    // }
}

PUBLIC VIRTUAL Method::~Method()
{
    if (m_pUserAor != IMS_NULL)
    {
        delete m_pUserAor;
    }

    if (m_pRemoteUserAor != IMS_NULL)
    {
        delete m_pRemoteUserAor;
    }

    DestroyDialog();

    if (m_piAuthChallenge != IMS_NULL)
    {
        m_piAuthChallenge->Destroy();
    }
}

PUBLIC VIRTUAL void Method::Destroy()
{
    PostMessage(AMSG_DESTROY, 0, 0);
}

PUBLIC
IMS_BOOL Method::Equals(IN const Method* pMethod) const
{
    if (pMethod == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!GetName().Equals(pMethod->GetName()))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL Method::InitMethod(IN const AString& strFrom, IN const AString& strTo,
        IN const SipAddress& objUserAor, IN IMS_BOOL bMobileOriginated /*= IMS_TRUE*/)
{
    m_bMobileOriginated = bMobileOriginated;

    // From header
    if (strFrom.IsNULL())
    {
        m_pUserAor = new SipAddress(objUserAor);

        if (m_pUserAor == IMS_NULL)
        {
            Ims::SetLastError(ImsError::NO_MEMORY);
            return IMS_FALSE;
        }
    }
    else
    {
        m_pUserAor = new SipAddress();

        if (m_pUserAor == IMS_NULL)
        {
            Ims::SetLastError(ImsError::NO_MEMORY);
            return IMS_FALSE;
        }

        if (!m_pUserAor->Create(strFrom))
        {
            Ims::SetLastError(ImsError::PARSING_ERROR);
            return IMS_FALSE;
        }
    }

    // To header
    if (strTo.IsNULL())
    {
        m_pRemoteUserAor = new SipAddress(objUserAor);

        if (m_pRemoteUserAor == IMS_NULL)
        {
            Ims::SetLastError(ImsError::NO_MEMORY);
            return IMS_FALSE;
        }
    }
    else
    {
        m_pRemoteUserAor = new SipAddress();

        if (m_pRemoteUserAor == IMS_NULL)
        {
            Ims::SetLastError(ImsError::NO_MEMORY);
            return IMS_FALSE;
        }

        if (!m_pRemoteUserAor->Create(strTo))
        {
            Ims::SetLastError(ImsError::PARSING_ERROR);
            return IMS_FALSE;
        }
    }

    return InitInstance();
}

PUBLIC
IMS_BOOL Method::InitMethod(IN const Method* pMethod, IN IMS_BOOL bMobileOriginated /*= IMS_TRUE*/)
{
    if (pMethod == IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_bMobileOriginated = bMobileOriginated;

    // From header
    m_pUserAor = new SipAddress(*(pMethod->m_pUserAor));

    if (m_pUserAor == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_FALSE;
    }

    // To header
    m_pRemoteUserAor = new SipAddress(*(pMethod->m_pRemoteUserAor));

    if (m_pRemoteUserAor == IMS_NULL)
    {
        Ims::SetLastError(ImsError::NO_MEMORY);
        return IMS_FALSE;
    }

    // To send/receive a request inside of a dialog
    if (pMethod->m_piDialog != IMS_NULL)
    {
        if (m_piDialog != IMS_NULL)
        {
            m_piDialog->Destroy();
        }

        m_piDialog = pMethod->m_piDialog->Clone();
    }

    return InitInstance();
}

PUBLIC
IMS_BOOL Method::ServerConnection_NotifyRequest(IN ISipServerConnection* piSsc)
{
    if (piSsc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("Method - \"%s\" REQUEST RECEIVED", piSsc->GetMethod().ToString().GetStr(), 0, 0);

    piSsc->SetErrorListener(this);

    // Update a remote user identities
    UpdateRemoteUserIds(piSsc);

    return NotifySipRequest(piSsc);
}

PROTECTED VIRTUAL IMS_BOOL Method::NotifySipForkedResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc)
{
    (void)piScc;

    // The subclass MUST implement this method if a SIP forked response needs to be handled.

    IMS_TRACE_E(0, "SIP forked response (%s) is not handled",
            piForkedScc->GetMethod().ToString().GetStr(), 0, 0);

    piForkedScc->Close();

    return IMS_FALSE;
}

PROTECTED VIRTUAL IMS_BOOL Method::SendRequestToChallenge(IN ISipClientConnection* piScc)
{
    // The subclass MAY insert the specific headers for each SIP method

    (void)AdjustMessage(piScc->GetMessage(), MESSAGE_CLASS_RESUBMIT);

    if (piScc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending the resubmission request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/**
 * @brief This method is invoked before sending the SIP message to the network\
 *        too adjust the specified message.
 */
PROTECTED
IMS_RESULT Method::AdjustMessage(
        IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage /*= MESSAGE_CLASS_NORMAL*/)
{
    if (m_piMessageMediator == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    return m_piMessageMediator->MessageMediator_AdjustMessage(piSipMsg, nMessage);
}

PROTECTED
void Method::CheckNCreateDialog(IN const ISipConnection* piSc, IN IMS_BOOL bDestroy /*= IMS_FALSE*/,
        IN IMS_BOOL bTerminatedDialogRequired /*= IMS_FALSE*/)
{
    if (bDestroy)
    {
        DestroyDialog();
    }

    if (m_piDialog == IMS_NULL)
    {
        const ISipDialog* piTmpDlg = piSc->GetDialog();

        if ((piTmpDlg->GetState() == ISipDialog::STATE_EARLY) ||
                (piTmpDlg->GetState() == ISipDialog::STATE_CONFIRMED))
        {
            m_piDialog = DYNAMIC_CAST(ISipDialog*, piTmpDlg->Clone());

            IMS_TRACE_D(
                    "___ DIALOG IS CREATED BY %s ___", piSc->GetMethod().ToString().GetStr(), 0, 0);
        }
        else if (bTerminatedDialogRequired &&
                (piTmpDlg->GetState() == ISipDialog::STATE_TERMINATED))
        {
            m_piDialog = DYNAMIC_CAST(ISipDialog*, piTmpDlg->Clone());

            IMS_TRACE_D("___ DIALOG IS CREATED BY %s IN TERMINATED STATE ___",
                    piSc->GetMethod().ToString().GetStr(), 0, 0);
        }
    }
}

PROTECTED
void Method::DestroyDialog()
{
    if (m_piDialog != IMS_NULL)
    {
        m_piDialog->Destroy();
        m_piDialog = IMS_NULL;
    }
}

PROTECTED
IMS_BOOL Method::HandleAllSipResponse(IN ISipClientConnection* piScc)
{
    do
    {
        if (piScc->Receive() != IMS_SUCCESS)
        {
            if (SipError::GetLastError() == SipError::NO_MESSAGE)
            {
                IMS_TRACE_I("No more messages in this client transaction", 0, 0, 0);
                break;
            }

            IMS_TRACE_E(0, "Receive() :: SipError (%d)", SipError::GetLastError(), 0, 0);
            return IMS_FALSE;
        }

        ISipMessage* piSipMsg = piScc->GetMessage();

        if (piSipMsg == IMS_NULL)
        {
            IMS_TRACE_E(0, "SIP message does not exist", 0, 0, 0);
            return IMS_FALSE;
        }

        // Parse the message body if it is a multipart body
        if (!SipParsingHelper::CreateMessageBodyParts(piSipMsg))
        {
            IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

            Error_NotifyError(
                    piScc, SipError::PARSING_ERROR, AString("Parsing Error :: message body part"));
            return IMS_FALSE;
        }

        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        IMS_TRACE_I("___ %d response to %s request is received ...", nStatusCode,
                piSipMsg->GetMethod().ToString().GetStr(), 0);

        // Update the remote user identities if present
        if (SipStatusCode::IsProvisional(nStatusCode) || SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            UpdateRemoteUserIds(piScc);
        }

        // All the subclass of this class MUST implement the below method.
        NotifySipResponse(piScc);

        if (nStatusCode >= SipStatusCode::SC_200)
        {
            // After the final response is received and handled,
            // the SCC does not contain any response messages in the queue.
            break;
        }
    } while (IMS_TRUE);

    return IMS_TRUE;
}

PROTECTED
void Method::ResetChallengeCount(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "No SIP message", 0, 0, 0);
        return;
    }

    if (SipStatusCode::IsFinalSuccess(piSipMsg->GetStatusCode()))
    {
        const SipMethod& objMethod = piSipMsg->GetMethod();
        IMS_SLONG nIndex = m_objAuthChallengeMap.GetIndexOfKey(objMethod.ToInt());

        if (nIndex >= 0)
        {
            IMS_SINT32& nAuthChallengeCount = m_objAuthChallengeMap.GetValueAt(nIndex);

            if (nAuthChallengeCount > 0)
            {
                IMS_TRACE_I("Authentication challenge count (%d) will be reset",
                        nAuthChallengeCount, 0, 0);
                nAuthChallengeCount = 0;
            }
        }
    }
}

PROTECTED
IMS_BOOL Method::RespondToChallenge(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piScc->GetMethod();
    IMS_SLONG nIndex = m_objAuthChallengeMap.GetIndexOfKey(objMethod.ToInt());

    // If any needs, we MAY change the scheme to compare with the configuration value
    // of each services.
    if (nIndex >= 0)
    {
        IMS_SINT32 nAuthChallengeCount = m_objAuthChallengeMap.GetValueAt(nIndex);

        if (nAuthChallengeCount >= MAX_CHALLENGE_COUNT)
        {
            IMS_TRACE_I("Authentication challenge count (%d) is over", nAuthChallengeCount, 0, 0);
            return IMS_FALSE;
        }
    }

    ISipGenericChallenge* piChallenge = piScc->GetAuthenticationChallenge();

    if (piChallenge == IMS_NULL)
    {
        IMS_TRACE_E(0, "No authentication challenge", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_SINT32 nAlgorithm = Credential::TranslateAlgorithm(piChallenge->GetAlgorithm());

    if (nAlgorithm != Credential::TYPE_MD5)
    {
        IMS_TRACE_E(0, "Authentication algorithm (%s) is not supported for non-REGISTER request",
                piChallenge->GetAlgorithm().GetStr(), 0, 0);
        return IMS_FALSE;
    }

    const SubscriberConfig* pSubscriberConfig = IMS_NULL;
    const AString& strSubsId = GetSubscriberId();
    ConfigurationManager* pConfigMngr = ConfigurationManager::GetInstance();

    if (strSubsId.GetLength() > 0)
    {
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strSubsId, GetSlotId());
    }
    else
    {
        const AString& strId =
                SubscriberTracker::GetInstance()->GetSubscriberId(GetSlotId(), GetUserAor());
        pSubscriberConfig = pConfigMngr->GetSubscriberConfig(strId, GetSlotId());
    }

    if (pSubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "SubscriberConfig is null; subsId=%s", strSubsId.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    Credential objCredential = pSubscriberConfig->GetCredential();

    objCredential.SetType(nAlgorithm);

    // If the username field is empty, then sets it to the private user identity.
    if (objCredential.GetUsername().GetLength() == 0)
    {
        objCredential.SetUsername(pSubscriberConfig->GetPrivateUserId());
    }

    // Overwrite the realm parameter if it required
    if (pSubscriberConfig->IsAuthRealmLenient() &&
            !objCredential.GetRealm().Equals(piChallenge->GetRealm()))
    {
        IMS_TRACE_D("auth_realm_leniency is true; %s -> %s",
                SipDebug::GetCharA1(objCredential.GetRealm().GetStr(), 4),
                SipDebug::GetCharA2(piChallenge->GetRealm().GetStr(), 4), 0);

        objCredential.SetRealm(piChallenge->GetRealm());
    }

    IMS_TRACE_D("Respond to the authentication challenge ...", 0, 0, 0);

    if (piScc->InitResubmissionRequest() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing resubmission request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Authentication algorithm will be MD5
    if (piScc->SetCredentials(objCredential) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting a credential information failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!SendRequestToChallenge(piScc))
    {
        // Remove the previous credential information for re-authentication
        piScc->RemoveAllCredentials();

        IMS_TRACE_E(0, "Sending the resubmission request failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Updates the authentication challenge information ...
    if (m_piAuthChallenge != IMS_NULL)
    {
        m_piAuthChallenge->Destroy();
        m_piAuthChallenge = IMS_NULL;
    }

    piChallenge = piScc->GetAuthenticationChallenge();

    if (piChallenge != IMS_NULL)
    {
        m_piAuthChallenge = piChallenge->Clone();

        IMS_TRACE_D("Authentication challenge is updated...", 0, 0, 0);
    }

    // Remove the previous credential information for re-authentication
    piScc->RemoveAllCredentials();

    // Increments the authentication challenge
    if (nIndex >= 0)
    {
        IMS_SINT32& nAuthChallengeCount = m_objAuthChallengeMap.GetValueAt(nIndex);

        ++nAuthChallengeCount;

        IMS_TRACE_I("Authentication challenge :: %d >> %d ", nAuthChallengeCount - 1,
                nAuthChallengeCount, 0);
    }

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL Method::SetChallengeNCredentials(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piAuthChallenge == IMS_NULL)
    {
        IMS_TRACE_D("No authentication challenge", 0, 0, 0);
        return IMS_TRUE;
    }

    const AString& strId =
            SubscriberTracker::GetInstance()->GetSubscriberId(GetSlotId(), GetUserAor());
    const SubscriberConfig* pSubscriberConfig =
            ConfigurationManager::GetInstance()->GetSubscriberConfig(strId, GetSlotId());

    if (pSubscriberConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "SubscriberConfig is null", 0, 0, 0);
        return IMS_FALSE;
    }

    Credential objCredential = pSubscriberConfig->GetCredential();

    objCredential.SetType(Credential::TYPE_MD5);

    // If the username field is empty, then sets it to the private user identity.
    if (objCredential.GetUsername().GetLength() == 0)
    {
        objCredential.SetUsername(pSubscriberConfig->GetPrivateUserId());
    }

    // Overwrite the realm parameter if it required
    if (pSubscriberConfig->IsAuthRealmLenient() &&
            !objCredential.GetRealm().Equals(m_piAuthChallenge->GetRealm()))
    {
        IMS_TRACE_D("auth_realm_leniency is true; %s -> %s",
                SipDebug::GetCharA1(objCredential.GetRealm().GetStr(), 4),
                SipDebug::GetCharA2(m_piAuthChallenge->GetRealm().GetStr(), 4), 0);

        objCredential.SetRealm(m_piAuthChallenge->GetRealm());
    }

    // Authentication algorithm will be MD5
    if (piScc->SetCredentials(objCredential) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting a credential information failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Updates the nonce count (usage will be increased)
    m_piAuthChallenge->IncreaseNonceCount();

    // Set a previous authentication challenge
    piScc->SetAuthenticationChallenge(m_piAuthChallenge);

    return IMS_TRUE;
}

PROTECTED
void Method::UpdateRemoteUserIds(IN const ISipConnection* piSc)
{
    const ISipMessage* piSipMsg = (piSc != IMS_NULL) ? piSc->GetMessage() : IMS_NULL;

    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    if (m_objRemoteUserIds.IsEmpty())
    {
        m_objRemoteUserIds = piSipMsg->GetHeaders(ISipHeader::P_ASSERTED_IDENTITY);
    }
    else
    {
        ImsList<AString> objLatestPaids = piSipMsg->GetHeaders(ISipHeader::P_ASSERTED_IDENTITY);

        if (!objLatestPaids.IsEmpty())
        {
            m_objRemoteUserIds.Clear();
            m_objRemoteUserIds = objLatestPaids;
        }
    }
}

PRIVATE VIRTUAL void Method::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc /*= IMS_NULL*/)
{
    if (piForkedScc != IMS_NULL)
    {
        if (!NotifySipForkedResponse(piScc, piForkedScc))
        {
            IMS_TRACE_E(0, "Handling a forked response failed", 0, 0, 0);
        }

        return;
    }

    if (piScc->Receive() != IMS_SUCCESS)
    {
        if (SipError::GetLastError() == SipError::NO_MESSAGE)
        {
            IMS_TRACE_I("No more messages in this client transaction", 0, 0, 0);
        }

        IMS_TRACE_E(0, "Receive() :: SipError (%d)", SipError::GetLastError(), 0, 0);
        return;
    }

    ISipMessage* piSipMsg = piScc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        IMS_TRACE_E(0, "No SIP message", 0, 0, 0);
        return;
    }

    // Parse the message body if it is a multipart body
    if (!SipParsingHelper::CreateMessageBodyParts(piSipMsg))
    {
        IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

        Error_NotifyError(
                piScc, SipError::PARSING_ERROR, AString("Parsing Error :: message body part"));
        return;
    }

    IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

    IMS_TRACE_I("___ %d response to %s request is received ...", nStatusCode,
            piSipMsg->GetMethod().ToString().GetStr(), 0);

    // Update the remote user identities if present
    if (SipStatusCode::IsProvisional(nStatusCode) || SipStatusCode::IsFinalSuccess(nStatusCode))
    {
        UpdateRemoteUserIds(piScc);
    }

    // AUTH_SIP_DIGEST {
    // RESET the authentication challenge count
    ResetChallengeCount(piScc);
    // }

    // Handle the SIP response.
    // All the subclass of this class MUST implement the below method.
    NotifySipResponse(piScc);
}

PRIVATE VIRTUAL void Method::Error_NotifyError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    IMS_TRACE_I("Error_NotifyError - %s : %d : %s", piSc->GetMethod().ToString().GetStr(), nCode,
            strMessage.GetStr());

    // SipError::TRANSACTION_TIMER_EXPIRED
    // SipError::AUTHENTICATION_FAILED
    // SipError::PARSING_ERROR

    // AUTH_SIP_DIGEST {
    if (m_piAuthChallenge != IMS_NULL)
    {
        m_piAuthChallenge->Destroy();
        m_piAuthChallenge = IMS_NULL;
    }
    // }

    // Handle the transaction failure.
    // All the subclass of this class MUST implement the below method.
    NotifySipError(piSc, nCode, strMessage);
}

PUBLIC
Method::SccListener::SccListener()
{
    IMS_TRACE_D("Constructor :: SccListener", 0, 0, 0);
}

PUBLIC VIRTUAL Method::SccListener::~SccListener()
{
    IMS_TRACE_D("Destructor :: SccListener", 0, 0, 0);
}

PROTECTED VIRTUAL void Method::SccListener::Error_NotifyError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    if (piSc == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("SCCListener :: Error_NotifyError - %s : %d : %s",
            piSc->GetMethod().ToString().GetStr(), nCode, strMessage.GetStr());

    piSc->Close();

    delete this;
}

PROTECTED VIRTUAL void Method::SccListener::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc /*= IMS_NULL*/)
{
    if (piForkedScc != IMS_NULL)
    {
        piForkedScc->Close();
    }

    if (piScc == IMS_NULL)
    {
        return;
    }

    if (piScc->Receive() != IMS_SUCCESS)
    {
        if (SipError::GetLastError() == SipError::NO_MESSAGE)
        {
            IMS_TRACE_I("No more messages in this client transaction", 0, 0, 0);
        }

        IMS_TRACE_E(0, "Receive() :: SipError (%d)", SipError::GetLastError(), 0, 0);

        piScc->Close();

        delete this;
        return;
    }

    IMS_TRACE_I("SCCListener :: ___ %d response to %s request is received ...",
            piScc->GetStatusCode(), piScc->GetMethod().ToString().GetStr(), 0);

    piScc->Close();

    delete this;
}
