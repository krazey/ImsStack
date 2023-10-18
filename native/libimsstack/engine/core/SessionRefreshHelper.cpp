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
#include "AStringArray.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "Service.h"
#include "SessionRefreshHelper.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_REFRESHER[] = "refresher";
PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_TIMER[] = "timer";
PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_UAC[] = "uac";
PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_UAS[] = "uas";

PUBLIC
SessionRefreshHelper::SessionRefreshHelper(IN Service* pService, IN IRefreshable* piRefreshable) :
        RefreshHelper(piRefreshable, IMS_FALSE),
        m_nMinSe(0),
        m_nSessionTimerDuration(0),
        m_nLocalSessionTimerDuration(0),
        m_nRefresher(REFRESHER_NONE),
        m_nRefreshRequest(SipMethod::INVALID),
        m_bUpdateMethodAllowed(IMS_FALSE),
        m_nTimerSupportedOnRemoteEnd(0),
        m_nSipHeaders(SipConfigV::SESSION_HEADER_SESSION_EXPIRES),
        m_pService(pService)
{
    const SipConfigV* pSipConfigV = pService->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        m_nSessionTimerDuration = pSipConfigV->GetSessionExpires();
        m_nRefresher = pSipConfigV->GetSessionRefresher();
        m_nSipHeaders = pSipConfigV->GetSessionHeaders();

        IMS_TRACE_I("SessionRefreshHelper: duration=%d, refresher=%d, sipHeaderOptions=%08x",
                m_nSessionTimerDuration, m_nRefresher, m_nSipHeaders);
    }
}

PUBLIC VIRTUAL SessionRefreshHelper::~SessionRefreshHelper()
{
#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SessionRefreshHelper", 0, 0, 0);
#endif
}

PUBLIC VIRTUAL IMS_BOOL SessionRefreshHelper::AddSpecificHeader(IN ISipConnection* piSc)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISipDialog* piDialog = piSc->GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE))
    {
        return IMS_TRUE;
    }

    IMS_BOOL bUpdateOnConfirmedDialog = IMS_FALSE;

    if (objMethod.Equals(SipMethod::UPDATE) &&
            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED))
    {
        bUpdateOnConfirmedDialog = IMS_TRUE;
    }

    // Updates the session timer information which is set by the application
    if ((piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) && objMethod.Equals(SipMethod::INVITE) &&
            (piDialog->GetState() != ISipDialog::STATE_CONFIRMED))
    {
        IMS_SINT32 nTmpRefresher = m_nRefresher;

        UpdateProperties(piSc, IMS_TRUE, IMS_TRUE);

        // If the refresher is not specified, just use the refresher, read from the configuration
        if (m_nRefresher == REFRESHER_NONE)
        {
            m_nRefresher = nTmpRefresher;
        }
    }

    const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

    if (pSipConfigV == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nConfigMinSe = pSipConfigV->GetSessionMinSe();

    // Set Supported header field if the application wants to use the session timer
    if (pSipConfigV->IsSessionTimerSupported() && !piSipMsg->IsOptionSupported(STR_TIMER))
    {
        if (piSipMsg->AddHeader(ISipHeader::SUPPORTED, STR_TIMER) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding Supported header failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if ((piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            pSipConfigV->IsSessionTimerSupported())
    {
        if (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE))
        {
            // Set a Min-SE header field
            if (((m_nMinSe > 0) || (nConfigMinSe > 0)) && IsMinSeHeaderRequired())
            {
                AString strMinSe;

                if (m_nMinSe > nConfigMinSe)
                {
                    strMinSe.SetNumber(m_nMinSe);
                }
                else
                {
                    strMinSe.SetNumber(nConfigMinSe);
                }

                if (piSipMsg->SetHeader(ISipHeader::MIN_SE, strMinSe) != IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }

            // At the time of making the request, if the request is INVITE or UPDATE
            // (in established state), then set the session refresh request.
            if (piDialog->GetState() == ISipDialog::STATE_CONFIRMED)
            {
                if (objMethod.Equals(SipMethod::INVITE))
                {
                    m_nRefreshRequest = SipMethod::INVITE;
                }
                else if (objMethod.Equals(SipMethod::UPDATE))
                {
                    m_nRefreshRequest = SipMethod::UPDATE;
                }
            }

            // Set a Session-Expires header field
            if (HasSessionTimerDuration() && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(GetSessionTimerDuration());

                // Set the refresher parameter in the Session-Expires header field
                if ((m_nRefresher == REFRESHER_LOCAL) || (m_nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (m_nRefresher == REFRESHER_LOCAL)
                    {
                        strSessionExpires.Append(STR_UAC);
                    }
                    else
                    {
                        strSessionExpires.Append(STR_UAS);
                    }
                }

                if (piSipMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }
    else if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if ((nStatusCode == SipStatusCode::SC_422) &&
                (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE)))
        {
            AString strMinSe;

            // Set a Min-SE header field
            if (m_nMinSe > nConfigMinSe)
            {
                strMinSe.SetNumber(m_nMinSe);
            }
            else
            {
                strMinSe.SetNumber(nConfigMinSe);
            }

            if (piSipMsg->SetHeader(ISipHeader::MIN_SE, strMinSe) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (m_nSessionTimerDuration > 0)
            {
                // Require header SHALL include the timer option tag
                if (IsRequireHeaderRequired() || (m_nRefresher == REFRESHER_REMOTE) ||
                        ((m_nRefresher == REFRESHER_LOCAL) && IsSessionTimerSupportedOnUac()))
                {
                    // Set a Require header field
                    if (piSipMsg->AddHeader(ISipHeader::REQUIRE, STR_TIMER) != IMS_SUCCESS)
                    {
                        IMS_TRACE_E(0, "Adding Require header failed", 0, 0, 0);
                        return IMS_FALSE;
                    }
                }
            }
            else if (m_nLocalSessionTimerDuration > 0 && !piSipMsg->IsOptionSupported(STR_TIMER))
            {
                // Set a Supported header field
                if (piSipMsg->AddHeader(ISipHeader::SUPPORTED, STR_TIMER) != IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Adding Supported header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }

            // Set a Session-Expires header field
            if (HasSessionTimerDuration() && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(GetSessionTimerDuration());

                if ((m_nRefresher == REFRESHER_LOCAL) || (m_nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (m_nRefresher == REFRESHER_LOCAL)
                    {
                        strSessionExpires.Append(STR_UAS);
                    }
                    else
                    {
                        strSessionExpires.Append(STR_UAC);
                    }
                }

                if (piSipMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL SessionRefreshHelper::AddSpecificHeaderWithoutParameterChange(
        IN ISipConnection* piSc)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISipDialog* piDialog = piSc->GetDialog();

    if (piDialog == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE))
    {
        return IMS_TRUE;
    }

    IMS_BOOL bUpdateOnConfirmedDialog = IMS_FALSE;
    IMS_SINT32 nBackupRefresher = m_nRefresher;
    IMS_SINT32 nBackupRefreshRequest = m_nRefreshRequest;
    IMS_SINT32 nBackupMinSe = m_nMinSe;
    IMS_SINT32 nBackupSessionTimerDuration = m_nSessionTimerDuration;
    IMS_SINT32 nBackupLocalSessionTimerDuration = m_nLocalSessionTimerDuration;

    if (objMethod.Equals(SipMethod::UPDATE) &&
            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED))
    {
        bUpdateOnConfirmedDialog = IMS_TRUE;
    }

    // Updates the session timer information which is set by the application
    if ((piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) && objMethod.Equals(SipMethod::INVITE) &&
            (piDialog->GetState() != ISipDialog::STATE_CONFIRMED))
    {
        IMS_SINT32 nTmpRefresher = m_nRefresher;

        UpdateProperties(piSc, IMS_TRUE, IMS_TRUE);

        // If the refresher is not specified, just use the refresher, read from the configuration
        if (m_nRefresher == REFRESHER_NONE)
        {
            m_nRefresher = nTmpRefresher;
        }
    }

    const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

    if (pSipConfigV == IMS_NULL)
    {
        // Restore the parameter values
        SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest, nBackupMinSe,
                nBackupSessionTimerDuration, nBackupLocalSessionTimerDuration);
        return IMS_FALSE;
    }

    IMS_SINT32 nConfigMinSe = pSipConfigV->GetSessionMinSe();

    // Set Supported header field if the application wants to use the session timer
    if (pSipConfigV->IsSessionTimerSupported() && !piSipMsg->IsOptionSupported(STR_TIMER))
    {
        if (piSipMsg->AddHeader(ISipHeader::SUPPORTED, STR_TIMER) != IMS_SUCCESS)
        {
            // Restore the parameter values
            SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest, nBackupMinSe,
                    nBackupSessionTimerDuration, nBackupLocalSessionTimerDuration);

            IMS_TRACE_E(0, "Adding Supported header failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if ((piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            pSipConfigV->IsSessionTimerSupported())
    {
        if (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE))
        {
            // Set a Min-SE header field
            if (((m_nMinSe > 0) || (nConfigMinSe > 0)) && IsMinSeHeaderRequired())
            {
                AString strMinSe;

                if (m_nMinSe > nConfigMinSe)
                {
                    strMinSe.SetNumber(m_nMinSe);
                }
                else
                {
                    strMinSe.SetNumber(nConfigMinSe);
                }

                if (piSipMsg->SetHeader(ISipHeader::MIN_SE, strMinSe) != IMS_SUCCESS)
                {
                    // Restore the parameter values
                    SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest,
                            nBackupMinSe, nBackupSessionTimerDuration,
                            nBackupLocalSessionTimerDuration);

                    IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }

            // At the time of making the request, if the request is INVITE or UPDATE
            // (in established state), then set the session refresh request.
            if (piDialog->GetState() == ISipDialog::STATE_CONFIRMED)
            {
                if (objMethod.Equals(SipMethod::INVITE))
                {
                    m_nRefreshRequest = SipMethod::INVITE;
                }
                else if (objMethod.Equals(SipMethod::UPDATE))
                {
                    m_nRefreshRequest = SipMethod::UPDATE;
                }
            }

            // Set a Session-Expires header field
            if (HasSessionTimerDuration() && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(GetSessionTimerDuration());

                // Set the refresher parameter in the Session-Expires header field
                if ((m_nRefresher == REFRESHER_LOCAL) || (m_nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (m_nRefresher == REFRESHER_LOCAL)
                    {
                        strSessionExpires.Append(STR_UAC);
                    }
                    else
                    {
                        strSessionExpires.Append(STR_UAS);
                    }
                }

                if (piSipMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    // Restore the parameter values
                    SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest,
                            nBackupMinSe, nBackupSessionTimerDuration,
                            nBackupLocalSessionTimerDuration);

                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }
    else if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if ((nStatusCode == SipStatusCode::SC_422) &&
                (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE)))
        {
            AString strMinSe;

            // Set a Min-SE header field
            if (m_nMinSe > nConfigMinSe)
            {
                strMinSe.SetNumber(m_nMinSe);
            }
            else
            {
                strMinSe.SetNumber(nConfigMinSe);
            }

            if (piSipMsg->SetHeader(ISipHeader::MIN_SE, strMinSe) != IMS_SUCCESS)
            {
                // Restore the parameter values
                SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest, nBackupMinSe,
                        nBackupSessionTimerDuration, nBackupLocalSessionTimerDuration);

                IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            if (m_nSessionTimerDuration > 0)
            {
                // Require header SHALL include the timer option tag
                if (IsRequireHeaderRequired() || (m_nRefresher == REFRESHER_REMOTE) ||
                        ((m_nRefresher == REFRESHER_LOCAL) && IsSessionTimerSupportedOnUac()))
                {
                    // Set a Require header field
                    if (piSipMsg->AddHeader(ISipHeader::REQUIRE, STR_TIMER) != IMS_SUCCESS)
                    {
                        // Restore the parameter values
                        SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest,
                                nBackupMinSe, nBackupSessionTimerDuration,
                                nBackupLocalSessionTimerDuration);

                        IMS_TRACE_E(0, "Adding Require header failed", 0, 0, 0);
                        return IMS_FALSE;
                    }
                }
            }
            else if (m_nLocalSessionTimerDuration > 0 && !piSipMsg->IsOptionSupported(STR_TIMER))
            {
                // Set a Supported header field
                if (piSipMsg->AddHeader(ISipHeader::SUPPORTED, STR_TIMER) != IMS_SUCCESS)
                {
                    // Restore the parameter values
                    SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest,
                            nBackupMinSe, nBackupSessionTimerDuration,
                            nBackupLocalSessionTimerDuration);

                    IMS_TRACE_E(0, "Adding Supported header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }

            // Set a Session-Expires header field
            if (HasSessionTimerDuration() && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(GetSessionTimerDuration());

                if ((m_nRefresher == REFRESHER_LOCAL) || (m_nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (m_nRefresher == REFRESHER_LOCAL)
                    {
                        strSessionExpires.Append(STR_UAS);
                    }
                    else
                    {
                        strSessionExpires.Append(STR_UAC);
                    }
                }

                if (piSipMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    // Restore the parameter values
                    SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest,
                            nBackupMinSe, nBackupSessionTimerDuration,
                            nBackupLocalSessionTimerDuration);

                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }

    // Restore the parameter values
    SetSessionRefreshParameters(nBackupRefresher, nBackupRefreshRequest, nBackupMinSe,
            nBackupSessionTimerDuration, nBackupLocalSessionTimerDuration);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_RESULT SessionRefreshHelper::SendRefreshRequest(IN ISipClientConnection* piScc)
{
    if (piScc == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piScc))
    {
        IMS_TRACE_E(0, "Adding the specific headers for a session refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    StopSessionTimer(piScc);

    if (RefreshHelper::SendRefreshRequest(piScc) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    UpdateOnMessageSent(piScc);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SessionRefreshHelper::UpdateOnMessageReceived(
        IN const ISipConnection* piSc)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return RESULT_ERROR;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    // Currently, the session refresh parameters are effected only for INVITE/UPDATE/BYE.
    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::BYE) &&
            !objMethod.Equals(SipMethod::UPDATE))
    {
        return RESULT_SUCCESS;
    }

    // BYE message processing stops here.
    if (objMethod.Equals(SipMethod::BYE) && IsTimerActive())
    {
        StopRefresh();
        return RESULT_SUCCESS;
    }

    ISipDialog* piDialog = piSc->GetDialog();

    // Any session refresh info. which comes in an early UPDATE SHOULD be ignored.
    if (piDialog != IMS_NULL)
    {
        if (objMethod.Equals(SipMethod::UPDATE) &&
                piDialog->GetState() != ISipDialog::STATE_CONFIRMED)
        {
            // do nothing...
            return RESULT_SUCCESS;
        }

        // CallEstablished -> Confirmed
        if ((piDialog->GetState() == ISipDialog::STATE_CONFIRMED) &&
                (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) && IsTimerActive())
        {
            StopRefresh();
        }
    }

    // The rest of the code is valid only for INVITE/UPDATE and responses to it.
    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE))
    {
        // do nothing...
        return RESULT_SUCCESS;
    }

    // Check if the peer UA supports the session timer
    IMS_BOOL bCheckSePresentity = IsSessionTimerSupportedBySessionExpires();
    IMS_BOOL bTimerOptionSupported = IsSessionTimerSupported(piSc, IMS_FALSE, bCheckSePresentity);

    if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::UPDATE) && (m_nRefreshRequest != SipMethod::INVALID) &&
                (piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES)))
        {
            // An UPDATE/INVITE has come to update session timer,
            // while it is already being updated by another re-INVITE or UPDATE.
            return RESULT_REJECT_500;
        }

        // Checks if timer option tag is specified in the incoming request message
        UpdateTimerOptionOnRequestReceived(piSc);

        if (piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
        {
            // In this state (STATE_ESTABLISHED), we have received a (re)fresh request,
            // set the session refresh method to indicate that either a (re)INVITE / UPDATE
            // has come
            if (objMethod.Equals(SipMethod::INVITE))
            {
                m_nRefreshRequest = SipMethod::INVITE;
            }
            else if (objMethod.Equals(SipMethod::UPDATE))
            {
                m_nRefreshRequest = SipMethod::UPDATE;
            }
        }
        else
        {
            // If "Session-Expires" header didn't come in Request:
            // Check if the session timer was in use (we come to know whether or not
            // session timer was in use through the value of session-interval & Min-SE).
            // If yes, reset the value of these two parameters, so that further requests
            // which originate from this UA goes without "Session-Expires" header.
            m_nSessionTimerDuration = 0;
            m_nMinSe = 0;
            m_nRefresher = REFRESHER_NONE;
        }

        UpdateProperties(piSc, bTimerOptionSupported, IMS_FALSE);

        // Negotiates the session refresher
        // Checks whether incoming message has a Supported header with value "timer"
        // if it's absent, then set the refresher as defined in the session timer spec.
        NegotiateRefresher(bTimerOptionSupported);

        const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

        if (pSipConfigV == IMS_NULL)
        {
            return RESULT_ERROR;
        }

        IMS_SINT32 nConfigMinSe = pSipConfigV->GetSessionMinSe();

        if ((m_nSessionTimerDuration != 0) &&
                ((nConfigMinSe > m_nSessionTimerDuration) || (m_nMinSe > m_nSessionTimerDuration)))
        {
            if (bTimerOptionSupported)
            {
                // Once a 422 response is sent, the session timer should be turned off
                m_nSessionTimerDuration = 0;
                SetDuration(0);

                return RESULT_REJECT_422;
            }
            else
            {
                m_nSessionTimerDuration = nConfigMinSe;

                if (m_nMinSe > nConfigMinSe)
                {
                    m_nMinSe = nConfigMinSe;
                }
            }
        }

        SetDuration(GetSessionTimerDuration());

        // Checks if Allow header contains UPDATE method
        if (objMethod.Equals(SipMethod::INVITE))
        {
            m_bUpdateMethodAllowed = IsUpdateMethodSupported(piSipMsg);
        }

        IMS_SINT32 nFlag = objMethod.Equals(SipMethod::INVITE)
                ? TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_INVITE
                : TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_UPDATE;

        SetOrClearTimerSupportedOnRemoteEnd(bTimerOptionSupported, nFlag);
    }
    else
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        if (nStatusCode == SipStatusCode::SC_422)
        {
            // Once a 422 response is sent, the session timer should be turned off
            m_nSessionTimerDuration = 0;
            UpdateProperties(piSc, bTimerOptionSupported, IMS_FALSE);

            SetDuration(GetSessionTimerDuration());
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            SetOrClearTimerSupportedOnRemoteEnd(
                    bTimerOptionSupported, TIMER_SUPPORTED_ON_REMOTE_UA);

            UpdateProperties(piSc, bTimerOptionSupported, IMS_FALSE);

            SetDuration(GetSessionTimerDuration());

            // If the response being sent is 200 OK, then start the session timer
            // after modifying the session timer state.
            if ((GetDuration() > 0) && IsSessionRefreshRequired(piSc))
            {
                // Session timer negotiation is done, so reset the request method
                m_nRefreshRequest = SipMethod::INVALID;

                if (!StartRefresh())
                {
                    IMS_TRACE_E(0, "Starting the session timer failed", 0, 0, 0);
                    return RESULT_ERROR;
                }
            }
            else if (IsSessionTimerUpdateRequiredByReInvite())
            {
                if (IsTimerActive() && !IsSessionTimerSupportedOnRemoteUa())
                {
                    IMS_TRACE_I("Session timer is stopped by remote UA", 0, 0, 0);
                    StopRefresh();
                }
            }

            // Checks if Allow header contains UPDATE method
            if (objMethod.Equals(SipMethod::INVITE))
            {
                m_bUpdateMethodAllowed = IsUpdateMethodSupported(piSipMsg);
            }

            IMS_TRACE_D("Session timer of remote UA: %s",
                    IsSessionTimerSupportedOnRemoteUa() ? "support" : "not-support", 0, 0);
        }
        else
        {
            if (IsSessionTimerUpdateRequiredByReInvite())
            {
                // Re-start a session refresh timer with the previous negotiated session interval
                if ((piDialog != IMS_NULL) &&
                        (piDialog->GetState() == ISipDialog::STATE_CONFIRMED) &&
                        (nStatusCode != SipStatusCode::SC_100))
                {
                    IMS_TRACE_D("Session refresh is resumed using the previous session interval", 0,
                            0, 0);

                    m_nRefreshRequest = SipMethod::INVALID;

                    if ((GetDuration() > 0) && IsSessionTimerSupportedOnRemoteUa())
                    {
                        if (!StartRefresh())
                        {
                            IMS_TRACE_E(0, "Re-starting the session timer failed", 0, 0, 0);
                            return RESULT_ERROR;
                        }
                    }
                }
            }
        }
    }

    return RESULT_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SessionRefreshHelper::UpdateOnMessageSent(IN const ISipConnection* piSc)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return RESULT_ERROR;
    }

    ISipDialog* piDialog = piSc->GetDialog();

    if (piDialog == IMS_NULL)
    {
        return RESULT_ERROR;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSipMsg->GetStatusCode();

        // Only one ongoing transaction for each method exists.
        if (SipStatusCode::IsFinal(nStatusCode))
        {
            IMS_SINT32 nFlag = objMethod.Equals(SipMethod::INVITE)
                    ? TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_INVITE
                    : TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_UPDATE;

            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
                SetOrClearTimerSupportedOnRemoteEnd(
                        IsTimerSupportedOnRemoteEnd(nFlag), TIMER_SUPPORTED_ON_REMOTE_UA);

                IMS_TRACE_D("Session timer of remote UA: %s",
                        IsSessionTimerSupportedOnRemoteUa() ? "support" : "not-support", 0, 0);
            }

            ClearTimerSupportedOnRemoteEnd(nFlag);
        }

        if (SipStatusCode::IsFinalSuccess(nStatusCode) &&
                (objMethod.Equals(SipMethod::INVITE) ||
                        (objMethod.Equals(SipMethod::UPDATE) &&
                                (piDialog->GetState() == ISipDialog::STATE_CONFIRMED))))
        {
            // Session timer negotiation is complete by now.
            // Hence, reset the refresh request parameter of the session timer.
            m_nRefreshRequest = SipMethod::INVALID;

            IMS_BOOL bCheckSePresentity = IsSessionTimerSupportedBySessionExpires();
            UpdateProperties(
                    piSc, IsSessionTimerSupported(piSc, IMS_TRUE, bCheckSePresentity), IMS_TRUE);

            if ((GetDuration() > 0) && IsSessionRefreshRequired(piSc))
            {
                if (!StartRefresh())
                {
                    return RESULT_ERROR;
                }
            }
            else if (IsSessionTimerUpdateRequiredByReInvite())
            {
                if (IsTimerActive() && !IsSessionTimerSupportedOnRemoteUa())
                {
                    IMS_TRACE_I("Session timer is stopped by remote UA", 0, 0, 0);
                    StopRefresh();
                }
            }
        }
        // Session timer negotiation is complete by now.
        // Hence, reset the refresh request parameter of the session timer.
        else if (nStatusCode == SipStatusCode::SC_422)
        {
            m_nRefreshRequest = SipMethod::INVALID;
        }
        else
        {
            if (IsSessionTimerUpdateRequiredByReInvite())
            {
                // Re-start a session refresh timer with the previous negotiated session interval
                if ((piDialog->GetState() == ISipDialog::STATE_CONFIRMED) &&
                        (objMethod.Equals(SipMethod::INVITE) ||
                                objMethod.Equals(SipMethod::UPDATE)) &&
                        (nStatusCode != SipStatusCode::SC_100))
                {
                    IMS_TRACE_D("Session refresh is resumed using the previous session interval", 0,
                            0, 0);

                    m_nRefreshRequest = SipMethod::INVALID;

                    if ((GetDuration() > 0) && IsSessionTimerSupportedOnRemoteUa())
                    {
                        if (!StartRefresh())
                        {
                            IMS_TRACE_E(0, "Re-starting a session refresh timer failed", 0, 0, 0);
                            return RESULT_ERROR;
                        }
                    }
                }
            }
        }
    }

    return RESULT_SUCCESS;
}

PUBLIC
IMS_BOOL SessionRefreshHelper::AddSpecificHeaderOnEarlyUpdate(
        IN ISipConnection* piSc, IN IMS_BOOL bTimerOptionSupported)
{
    // Store the current refresher parameter
    IMS_SINT32 nTmpRefresher = m_nRefresher;

    if (IsRefresherParameterControlledOnEarlyUpdate() && (m_nRefresher == REFRESHER_NONE))
    {
        NegotiateRefresher(bTimerOptionSupported);
    }

    IMS_BOOL bResult = AddSpecificHeader(piSc);

    // Rollback the refresher parameter
    m_nRefresher = nTmpRefresher;

    return bResult;
}

PUBLIC
IMS_SINT32 SessionRefreshHelper::GetRefreshMethod() const
{
    const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

    if (pSipConfigV == IMS_NULL)
    {
        return SipMethod::INVALID;
    }

    IMS_SINT32 nRefreshMethod = pSipConfigV->GetSessionMethod();

    if (nRefreshMethod == SipConfigV::SESSION_REFRESH_INVITE)
    {
        return SipMethod::INVITE;
    }
    else if (nRefreshMethod == SipConfigV::SESSION_REFRESH_UPDATE)
    {
        return SipMethod::UPDATE;
    }
    else
    {
        if (m_bUpdateMethodAllowed)
        {
            return SipMethod::UPDATE;
        }
        else
        {
            return SipMethod::INVITE;
        }
    }
}

PUBLIC
IMS_BOOL SessionRefreshHelper::IsSessionTimerSupported(IN const ISipConnection* piSc,
        IN IMS_BOOL bSent, IN IMS_BOOL bCheckSePresentity /*= IMS_TRUE*/)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // Get Supported header field values
    AStringArray objSupported = piSipMsg->GetHeaders(ISipHeader::SUPPORTED);

    if (!objSupported.IsEmpty())
    {
        // Checks if the session timer is supported
        if (objSupported.Contains(TextParser::STR_ASTERISK, IMS_FALSE) ||
                objSupported.Contains(STR_TIMER, IMS_FALSE))
        {
            // UAC
            // Remote: "timer" option-tag in Supported, but no Session-Expires header
            //  => Session timer is turned off by the remote end.
            if (!bSent && piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE &&
                    !piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
            {
                return IsLocalSessionTimerRequired();
            }

            return IMS_TRUE;
        }
    }

    // Check the additional headers: Require, Session-Expires
    AStringArray objRequire = piSipMsg->GetHeaders(ISipHeader::REQUIRE);

    if (!objRequire.IsEmpty())
    {
        // Checks if the session timer is required
        if (objRequire.Contains(STR_TIMER, IMS_FALSE))
        {
            IMS_TRACE_D("Session timer is supported by Require header", 0, 0, 0);
            return IMS_TRUE;
        }
    }

    // NON-STANDARD :: for IMS client's flexibility
    if (bCheckSePresentity && piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
    {
        IMS_TRACE_D("Session timer is supported by Session-Expires header", 0, 0, 0);
        return IMS_TRUE;
    }

    if (IsLocalSessionTimerRequired())
    {
        IMS_TRACE_D("Session timer is required by the local endpoint", 0, 0, 0);
        return IMS_TRUE;
    }

    // UAC
    // Remote: no "timer" option-tag in Supported and Session-Expires header
    //  => Session timer may be supported by the local end.
    return !bSent;
}

PUBLIC
void SessionRefreshHelper::StopSessionTimer(IN const ISipConnection* piSc)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    ISipDialog* piDialog = piSc->GetDialog();

    if (piDialog == IMS_NULL)
    {
        return;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    // Stop the session timer for the messages of type BYE, UPDATE and re-INVITE.
    if ((piSipMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED) && IsTimerActive() &&
            (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::BYE) ||
                    objMethod.Equals(SipMethod::UPDATE)))
    {
        StopRefresh();
    }
}

PUBLIC
void SessionRefreshHelper::UpdateTimerOptionOnRequestReceived(IN const ISipConnection* piSc)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    if (piSipMsg->GetType() != ISipMessage::TYPE_REQUEST)
    {
        return;
    }

    ISipDialog* piDialog = piSc->GetDialog();
    const SipMethod& objMethod = piSipMsg->GetMethod();

    // Checks if the initial INVITE/re-INVITE/UPDATE request supports
    // the session timer or not... (Supported header)
    if (objMethod.Equals(SipMethod::INVITE) &&
            ((piDialog == IMS_NULL) || (piDialog->GetState() != ISipDialog::STATE_CONFIRMED)))
    {
        if (piSipMsg->IsOptionSupported(STR_TIMER))
        {
            SetTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_INITIAL_INVITE);
            IMS_TRACE_D("Session timer is supported in INVITE", 0, 0, 0);
        }

        return;
    }

    if ((objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::UPDATE)) &&
            ((piDialog != IMS_NULL) && (piDialog->GetState() == ISipDialog::STATE_CONFIRMED)))
    {
        if (piSipMsg->IsOptionSupported(STR_TIMER))
        {
            SetTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_SESSION_UPDATE);
            IMS_TRACE_D("Session timer is supported in %s", objMethod.ToString().GetStr(), 0, 0);
        }
        else
        {
            ClearTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_SESSION_UPDATE);
        }
    }
}

PROTECTED VIRTUAL IMS_SINT32 SessionRefreshHelper::GetTimerInterval() const
{
    IMS_SINT32 nTimerInterval = GetDuration();

    if (m_nRefresher == REFRESHER_LOCAL)
    {
        nTimerInterval = static_cast<IMS_SINT32>(nTimerInterval / 2);
    }
    else if (m_nRefresher == REFRESHER_REMOTE)
    {
        if (GetPolicy() != POLICY_SPEC)
        {
            return RefreshHelper::GetTimerInterval();
        }

        const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();
        // Unit : seconds
        IMS_SINT32 nRefreshMethodTxnTv = (-1);

        if (pSipConfigV != IMS_NULL)
        {
            IMS_SINT32 nRefreshMethod = pSipConfigV->GetSessionMethod();

            if (nRefreshMethod == SipConfigV::SESSION_REFRESH_INVITE)
            {
                nRefreshMethodTxnTv = SipConfigProxy::GetTimerValueB(
                        m_pService->GetSlotId(), m_pService->GetSipProfile(), pSipConfigV);
            }
            else if (nRefreshMethod == SipConfigV::SESSION_REFRESH_UPDATE)
            {
                nRefreshMethodTxnTv = SipConfigProxy::GetTimerValueF(
                        m_pService->GetSlotId(), m_pService->GetSipProfile(), pSipConfigV);
            }
        }

        if (nRefreshMethodTxnTv < 0)
        {
            // (T1 * 64) seconds
            IMS_SINT32 nT1 = SipConfigProxy::GetTimerValueT1(
                    m_pService->GetSlotId(), m_pService->GetSipProfile(), pSipConfigV);
            nRefreshMethodTxnTv = nT1 * 64;
        }

        // Converts milli-seconds to seconds
        nRefreshMethodTxnTv = nRefreshMethodTxnTv / 1000;

        // 4 from configuration, start point of the session refresh ?
        IMS_SINT32 nMinInterval = static_cast<IMS_SINT32>(nTimerInterval / 3);
        IMS_SINT32 nMaxInterval =
                (nMinInterval > nRefreshMethodTxnTv) ? nMinInterval : nRefreshMethodTxnTv;

        // Re-calculate minimum interval
        nMinInterval = (nMinInterval < nRefreshMethodTxnTv) ? nMinInterval : nRefreshMethodTxnTv;

        // two third of session interval
        IMS_SINT32 nNonRefresherInterval1 = nTimerInterval - nMinInterval;
        // half of session interval + (txn timer value or one third of session interval)
        IMS_SINT32 nNonRefresherInterval2 =
                static_cast<IMS_SINT32>(nTimerInterval / 2) + nMaxInterval;

        if (nNonRefresherInterval2 > nTimerInterval)
        {
            nTimerInterval = nNonRefresherInterval1;
        }
        else
        {
            if (nNonRefresherInterval1 > nNonRefresherInterval2)
            {
                nTimerInterval = nNonRefresherInterval1;
            }
            else
            {
                nTimerInterval = nNonRefresherInterval2;
            }
        }
    }

    return nTimerInterval;
}

PROTECTED VIRTUAL void SessionRefreshHelper::RefreshCompleted(
        IN ISipClientConnection* piScc, IN IMS_SINT32 nCode /*= 0*/)
{
    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piScc->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            StopRefresh();
        }

        UpdateOnMessageReceived(piScc);
    }
    else if (nCode == TRANSACTION_TIMEOUT)
    {
        // Behaves as if the 408 request timeout response received
        StopRefresh();
    }

    Refreshable_RefreshCompleted(piScc, nCode);
}

PROTECTED VIRTUAL void SessionRefreshHelper::RefreshStarted()
{
    if (m_nRefreshRequest != SipMethod::INVALID)
    {
        return;
    }

    if (m_nRefresher == REFRESHER_REMOTE)
    {
        Refreshable_RefreshTerminated();
    }
    else
    {
        if (Refreshable_RefreshStarted() != IMS_TRUE)
        {
            // Clean up the refresh timer related resources
            Refreshable_RefreshTerminated();
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL SessionRefreshHelper::IsSessionTimerUpdateRequiredByReInvite() const
{
    return SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            m_pService->GetSlotId(), m_pService->GetSipProfile());
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsSessionRefreshRequired(IN const ISipConnection* piSc) const
{
    if (piSc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISipMessage* piSipMsg = piSc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
    {
        IMS_TRACE_D("The message had no Session-Expires header field;"
                    " no refreshes have to be performed",
                0, 0, 0);

        // Remote: "timer" option-tag in Supported, but no Session-Expires header
        //  => Session timer is turned off by the remote end.
        // Remote: no "timer" option-tag in Supported and Session-Expires header
        //  => Session timer may be supported by the local end.
        return !piSipMsg->IsOptionSupported(STR_TIMER);
    }

    return IMS_TRUE;
}

PRIVATE
void SessionRefreshHelper::NegotiateRefresher(IN IMS_BOOL bTimerOptionSupported)
{
    //
    //    UAC supports ?    Request            Response
    //    N                 none               uas
    //    N                 uac                NA
    //    N                 uas                NA
    //    Y                 none               uas or uac
    //    Y                 uac                uac
    //    Y                 uas                uas
    //

    if (bTimerOptionSupported)
    {
        if (m_nRefresher == REFRESHER_NONE)
        {
            m_nRefresher = REFRESHER_REMOTE;
        }
    }
    else
    {
        if (m_nRefresher == REFRESHER_NONE)
        {
            m_nRefresher = REFRESHER_LOCAL;
        }
        else
        {
            m_nRefresher = REFRESHER_NONE;
        }
    }
}

PRIVATE
void SessionRefreshHelper::UpdateProperties(IN const ISipConnection* piSc,
        IN IMS_BOOL bTimerOptionSupported, IN IMS_BOOL bSent /*= IMS_FALSE*/)
{
    ISipMessage* piSipMsg = piSc->GetMessage();

    // Gets the Session-Expires header field from SIP message
    // and sets the session interval and refresher.
    if (piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
    {
        AString strHeader = piSipMsg->GetHeader(ISipHeader::SESSION_EXPIRES);
        ISipHeader* piHeader =
                SipParsingHelper::CreateHeader(ISipHeader::SESSION_EXPIRES, strHeader);

        if (piHeader == IMS_NULL)
        {
            return;
        }

        IMS_BOOL bOk = IMS_FALSE;
        m_nSessionTimerDuration = piHeader->GetValue().ToInt32(&bOk);

        if (!bOk)
        {
            piHeader->Destroy();
            return;
        }

        const SipParameter* pParameter = piHeader->GetParameter(STR_REFRESHER);

        if (pParameter == IMS_NULL)
        {
            m_nRefresher = REFRESHER_NONE;

            // Overwrite the refresher according to the configuration
            if (IsLocalSessionTimerRequired())
            {
                if (bSent)
                {
                    if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
                    {
                        m_nRefresher = REFRESHER_LOCAL;
                    }
                }
                else
                {
                    m_nRefresher = REFRESHER_LOCAL;
                }
            }
        }
        else
        {
            const AString& strRefresher = pParameter->GetValue();

            if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
            {
                if (bSent)
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                    {
                        m_nRefresher = REFRESHER_REMOTE;
                    }
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                    {
                        m_nRefresher = REFRESHER_LOCAL;
                    }
                    else
                    {
                        m_nRefresher = REFRESHER_NONE;

                        // Overwrite the refresher according to the configuration
                        if (IsLocalSessionTimerRequired())
                        {
                            m_nRefresher = REFRESHER_LOCAL;
                        }
                    }
                }
                else
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                    {
                        m_nRefresher = REFRESHER_LOCAL;
                    }
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                    {
                        m_nRefresher = REFRESHER_REMOTE;
                    }
                    else
                    {
                        m_nRefresher = REFRESHER_NONE;

                        // Overwrite the refresher according to the configuration
                        if (IsLocalSessionTimerRequired())
                        {
                            m_nRefresher = REFRESHER_LOCAL;
                        }
                    }
                }
            }
            else if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
            {
                if (bSent)
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                    {
                        m_nRefresher = REFRESHER_LOCAL;
                    }
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                    {
                        m_nRefresher = REFRESHER_REMOTE;
                    }
                    else
                    {
                        m_nRefresher = REFRESHER_NONE;
                    }
                }
                else
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                    {
                        m_nRefresher = REFRESHER_REMOTE;
                    }
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                    {
                        m_nRefresher = REFRESHER_LOCAL;
                    }
                    else
                    {
                        m_nRefresher = REFRESHER_NONE;

                        // Overwrite the refresher according to the configuration
                        if (IsLocalSessionTimerRequired())
                        {
                            m_nRefresher = REFRESHER_LOCAL;
                        }
                    }
                }
            }
        }

        piHeader->Destroy();
    }

    // Gets Min-SE header from the SIP message and sets the minimum session expiration duration.
    if (piSipMsg->IsHeaderPresent(ISipHeader::MIN_SE))
    {
        AString strHeader = piSipMsg->GetHeader(ISipHeader::MIN_SE);
        ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::MIN_SE, strHeader);

        if (piHeader == IMS_NULL)
        {
            return;
        }

        IMS_BOOL bOk = IMS_FALSE;
        IMS_SINT32 nTempMinSe = piHeader->GetValue().ToInt32(&bOk);

        if (!bOk)
        {
            piHeader->Destroy();
            return;
        }

        // The largest value among all Min-SE header field values
        // returned in all 422 responses or received in session refresh requests,
        // on the same dialog.
        if (nTempMinSe > m_nMinSe)
        {
            m_nMinSe = nTempMinSe;
        }

        piHeader->Destroy();
    }

    // If the session timer is supported by the peer entity, then the request should come
    // with Session-Expires > Min-SE.
    // Otherwise, we should increase the value of Session-Expires equal to Min-SE.
    IMS_BOOL bSessionIntervalChangeable = !bTimerOptionSupported ||
            (!bSent && (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE) &&
                    (piSipMsg->GetStatusCode() == SipStatusCode::SC_422));

    if (bSessionIntervalChangeable && (m_nMinSe > m_nSessionTimerDuration))
    {
        IMS_TRACE_I("SessionTimer(notSupport or 422) :: %d >> %d", m_nSessionTimerDuration,
                m_nMinSe, 0);
        m_nSessionTimerDuration = m_nMinSe;
    }

    m_nLocalSessionTimerDuration = 0;

    //// To handle other optional cases for session refresh...
    if (!bSent && !piSipMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES) && bTimerOptionSupported)
    {
        // According to RFC 4028,
        // UAC
        // It is possible that the UAC requested the session timer (and thus included
        // a Session-Expires header field in the request) and that there was no Require
        // or Session-Expires header field in the 2xx response. This will happen when the
        // UAS doesn't support the session timer extension and only the UAC has asked for
        // a session timer (no proxies have requested it). In this case, if the UAC still wishes
        // to use the session timer (which is purely for its benefit alone), it has to perform
        // them.
        //
        // UAS
        // If the incoming request contains a Supported header field with a value 'timer'
        // but does not contain a Session-Expires header,
        // it means that the UAS is indicating support for timers but is not requesting one.
        // UAS may request a session timer in the 2XX response by including
        // a Session-Expires header field.
        if (bSessionIntervalChangeable)
        {
            // 422 response: do not update session-interval/refresher.
        }
        else
        {
            if (m_nRefresher == REFRESHER_NONE && !piSipMsg->IsOptionSupported(STR_TIMER))
            {
                m_nRefresher = REFRESHER_LOCAL;
            }

            if (IsLocalSessionTimerRequired())
            {
                m_nRefresher = REFRESHER_LOCAL;

                const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

                if (pSipConfigV != IMS_NULL)
                {
                    m_nLocalSessionTimerDuration = pSipConfigV->GetSessionExpires();

                    if ((m_nMinSe > 0) && (m_nLocalSessionTimerDuration < m_nMinSe))
                    {
                        m_nLocalSessionTimerDuration = m_nMinSe;
                    }

                    IMS_TRACE_D("Remote endpoint is not requesting the session timer,"
                                " but local endpoint supports it (%d:%d)",
                            m_nLocalSessionTimerDuration, m_nMinSe, 0);
                }
            }
            else if (m_nSessionTimerDuration == 0)
            {
                const SipConfigV* pSipConfigV = m_pService->GetSipConfigV();

                if (pSipConfigV != IMS_NULL)
                {
                    m_nSessionTimerDuration = pSipConfigV->GetSessionExpires();

                    if ((m_nMinSe > 0) && (m_nSessionTimerDuration < m_nMinSe))
                    {
                        m_nSessionTimerDuration = m_nMinSe;
                    }

                    IMS_TRACE_I("Remote endpoint does not have Session-Expires header,"
                                " but it supports \"timer\" option tag(%d:%d).",
                            m_nSessionTimerDuration, m_nMinSe, 0);
                }
            }
        }
    }
}

PRIVATE
void SessionRefreshHelper::SetSessionRefreshParameters(IN IMS_SINT32 nRefresher,
        IN IMS_SINT32 nRefreshRequest, IN IMS_SINT32 nMinSe, IN IMS_SINT32 nSessionTimerDuration,
        IN IMS_SINT32 nLocalSessionTimerDuration)
{
    m_nRefresher = nRefresher;
    m_nRefreshRequest = nRefreshRequest;
    m_nMinSe = nMinSe;
    m_nSessionTimerDuration = nSessionTimerDuration;
    m_nLocalSessionTimerDuration = nLocalSessionTimerDuration;
}

PRIVATE GLOBAL IMS_BOOL SessionRefreshHelper::IsUpdateMethodSupported(
        IN const ISipMessage* piSipMsg)
{
    ImsList<AString> objHeaders = piSipMsg->GetHeaders(ISipHeader::ALLOW);

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const AString& strHeader = objHeaders.GetAt(i);

        if (strHeader.Equals(SipMethod::ToName(SipMethod::UPDATE)))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
