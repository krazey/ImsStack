/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090826  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "AStringArray.h"
#include "private/SipConfigV.h"
#include "Sip.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "SipStatusCode.h"
#include "SipParsingHelper.h"
#include "SipParameter.h"
#include "SipConfigProxy.h"
#include "base/IMS.h"
#include "Service.h"
#include "SessionRefreshHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_REFRESHER[] = "refresher";
PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_TIMER[] = "timer";
PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_UAC[] = "uac";
PRIVATE GLOBAL const IMS_CHAR SessionRefreshHelper::STR_UAS[] = "uas";

PUBLIC
SessionRefreshHelper::SessionRefreshHelper(IN Service* pService_, IN IRefreshable* piRefreshable_) :
        RefreshHelper(piRefreshable_, IMS_FALSE),
        nMinSE(0),
        nSessionInterval(0),
        nRefresher(REFRESHER_NONE),
        nRefreshRequest(SipMethod::INVALID),
        bUPDATEAllowed(IMS_FALSE),
        nTimerSupportedOnRemoteEnd(0),
        nSIPHeaders(SipConfigV::SESSION_HEADER_SESSION_EXPIRES),
        pService(pService_)
{
    const SipConfigV* pSipConfigV = pService->GetSipConfigV();

    if (pSipConfigV != IMS_NULL)
    {
        nSessionInterval = pSipConfigV->GetSessionExpires();
        nRefresher = pSipConfigV->GetSessionRefresher();

        nSIPHeaders = pSipConfigV->GetSessionHeaders();
    }
}

PUBLIC VIRTUAL SessionRefreshHelper::~SessionRefreshHelper()
{
    //---------------------------------------------------------------------------------------------

#ifdef __IMS_SIP_DEBUG__
    IMS_TRACE_D("Destructor :: SessionRefreshHelper", 0, 0, 0);
#endif
}

PUBLIC VIRTUAL IMS_BOOL SessionRefreshHelper::AddSpecificHeader(IN ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
        return IMS_FALSE;

    ISipDialog* piDialog = piSC->GetDialog();

    if (piDialog == IMS_NULL)
        return IMS_FALSE;

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE))
        return IMS_TRUE;

    IMS_BOOL bUpdateOnConfirmedDialog = IMS_FALSE;

    if (objMethod.Equals(SipMethod::UPDATE) &&
            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED))
    {
        bUpdateOnConfirmedDialog = IMS_TRUE;
    }

    // Updates the session timer information which is set by the application
    if ((piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST) && objMethod.Equals(SipMethod::INVITE) &&
            (piDialog->GetState() != ISipDialog::STATE_CONFIRMED))
    {
        IMS_SINT32 nTmpRefresher = nRefresher;

        UpdateProperties(piSC, IMS_TRUE, IMS_TRUE);

        // If the refresher is not specified, just use the refresher, read from the configuration
        if (nRefresher == REFRESHER_NONE)
        {
            nRefresher = nTmpRefresher;
        }
    }

    const SipConfigV* pSipConfigV = pService->GetSipConfigV();

    if (pSipConfigV == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nConfig_MinSE = pSipConfigV->GetSessionMinSE();

    // Set Supported header field if the application wants to use the session timer
    if (pSipConfigV->IsSessionTimerSupported() && !piSIPMsg->IsOptionSupported(STR_TIMER))
    {
        if (piSIPMsg->AddHeader(ISipHeader::SUPPORTED, STR_TIMER) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Adding Supported header failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if ((piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            pSipConfigV->IsSessionTimerSupported())
    {
        if (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE))
        {
            // Set a Min-SE header field
            if (((nMinSE > 0) || (nConfig_MinSE > 0)) && IsMinSEHeaderRequired())
            {
                AString strMinSE;

                if (nMinSE > nConfig_MinSE)
                    strMinSE.SetNumber(nMinSE);
                else
                    strMinSE.SetNumber(nConfig_MinSE);

                if (piSIPMsg->SetHeader(ISipHeader::MIN_SE, strMinSE) != IMS_SUCCESS)
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
                    nRefreshRequest = SipMethod::INVITE;
                else if (objMethod.Equals(SipMethod::UPDATE))
                    nRefreshRequest = SipMethod::UPDATE;
            }

            // Set a Session-Expires header field
            if ((nSessionInterval > 0) && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(nSessionInterval);

                // Set the refresher parameter in the Session-Expires header field
                if ((nRefresher == REFRESHER_LOCAL) || (nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (nRefresher == REFRESHER_LOCAL)
                        strSessionExpires.Append(STR_UAC);
                    else
                        strSessionExpires.Append(STR_UAS);
                }

                if (piSIPMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }
    else if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if ((nStatusCode == SipStatusCode::SC_422) &&
                (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE)))
        {
            AString strMinSE;

            // Set a Min-SE header field
            if (nMinSE > nConfig_MinSE)
                strMinSE.SetNumber(nMinSE);
            else
                strMinSE.SetNumber(nConfig_MinSE);

            if (piSIPMsg->SetHeader(ISipHeader::MIN_SE, strMinSE) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            // Set a Session-Expires header field
            if (nSessionInterval > 0)
            {
                // Require header SHALL include the timer option tag
                if (IsRequireHeaderRequired() || (nRefresher == REFRESHER_REMOTE) ||
                        ((nRefresher == REFRESHER_LOCAL) && IsSessionTimerSupportedOnUAC()))
                {
                    // Set a Require header field
                    if (piSIPMsg->AddHeader(ISipHeader::REQUIRE, STR_TIMER) != IMS_SUCCESS)
                    {
                        IMS_TRACE_E(0, "Adding Require header failed", 0, 0, 0);
                        return IMS_FALSE;
                    }
                }
            }

            if ((nSessionInterval > 0) && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(nSessionInterval);

                if ((nRefresher == REFRESHER_LOCAL) || (nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (nRefresher == REFRESHER_LOCAL)
                        strSessionExpires.Append(STR_UAS);
                    else
                        strSessionExpires.Append(STR_UAC);
                }

                if (piSIPMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
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
        IN ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
        return IMS_FALSE;

    ISipDialog* piDialog = piSC->GetDialog();

    if (piDialog == IMS_NULL)
        return IMS_FALSE;

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    if (!objMethod.Equals(SipMethod::INVITE) && !objMethod.Equals(SipMethod::UPDATE))
        return IMS_TRUE;

    IMS_BOOL bUpdateOnConfirmedDialog = IMS_FALSE;
    IMS_SINT32 nBackupRefresher = nRefresher;
    IMS_SINT32 nBackupRefreshRequest = nRefreshRequest;
    IMS_SINT32 nBackupMinSE = nMinSE;
    IMS_SINT32 nBackupSessionInterval = nSessionInterval;

    if (objMethod.Equals(SipMethod::UPDATE) &&
            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED))
    {
        bUpdateOnConfirmedDialog = IMS_TRUE;
    }

    // Updates the session timer information which is set by the application
    if ((piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST) && objMethod.Equals(SipMethod::INVITE) &&
            (piDialog->GetState() != ISipDialog::STATE_CONFIRMED))
    {
        IMS_SINT32 nTmpRefresher = nRefresher;

        UpdateProperties(piSC, IMS_TRUE, IMS_TRUE);

        // If the refresher is not specified, just use the refresher, read from the configuration
        if (nRefresher == REFRESHER_NONE)
        {
            nRefresher = nTmpRefresher;
        }
    }

    const SipConfigV* pSipConfigV = pService->GetSipConfigV();

    if (pSipConfigV == IMS_NULL)
    {
        // Restore the parameter values
        nRefresher = nBackupRefresher;
        nRefreshRequest = nBackupRefreshRequest;
        nMinSE = nBackupMinSE;
        nSessionInterval = nBackupSessionInterval;
        return IMS_FALSE;
    }

    IMS_SINT32 nConfig_MinSE = pSipConfigV->GetSessionMinSE();

    // Set Supported header field if the application wants to use the session timer
    if (pSipConfigV->IsSessionTimerSupported() && !piSIPMsg->IsOptionSupported(STR_TIMER))
    {
        if (piSIPMsg->AddHeader(ISipHeader::SUPPORTED, STR_TIMER) != IMS_SUCCESS)
        {
            // Restore the parameter values
            nRefresher = nBackupRefresher;
            nRefreshRequest = nBackupRefreshRequest;
            nMinSE = nBackupMinSE;
            nSessionInterval = nBackupSessionInterval;

            IMS_TRACE_E(0, "Adding Supported header failed", 0, 0, 0);
            return IMS_FALSE;
        }
    }

    if ((piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            pSipConfigV->IsSessionTimerSupported())
    {
        if (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE))
        {
            // Set a Min-SE header field
            if (((nMinSE > 0) || (nConfig_MinSE > 0)) && IsMinSEHeaderRequired())
            {
                AString strMinSE;

                if (nMinSE > nConfig_MinSE)
                    strMinSE.SetNumber(nMinSE);
                else
                    strMinSE.SetNumber(nConfig_MinSE);

                if (piSIPMsg->SetHeader(ISipHeader::MIN_SE, strMinSE) != IMS_SUCCESS)
                {
                    // Restore the parameter values
                    nRefresher = nBackupRefresher;
                    nRefreshRequest = nBackupRefreshRequest;
                    nMinSE = nBackupMinSE;
                    nSessionInterval = nBackupSessionInterval;

                    IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }

            // At the time of making the request, if the request is INVITE or UPDATE
            // (in established state), then set the session refresh request.
            if (piDialog->GetState() == ISipDialog::STATE_CONFIRMED)
            {
                if (objMethod.Equals(SipMethod::INVITE))
                    nRefreshRequest = SipMethod::INVITE;
                else if (objMethod.Equals(SipMethod::UPDATE))
                    nRefreshRequest = SipMethod::UPDATE;
            }

            // Set a Session-Expires header field
            if ((nSessionInterval > 0) && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(nSessionInterval);

                // Set the refresher parameter in the Session-Expires header field
                if ((nRefresher == REFRESHER_LOCAL) || (nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (nRefresher == REFRESHER_LOCAL)
                        strSessionExpires.Append(STR_UAC);
                    else
                        strSessionExpires.Append(STR_UAS);
                }

                if (piSIPMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    // Restore the parameter values
                    nRefresher = nBackupRefresher;
                    nRefreshRequest = nBackupRefreshRequest;
                    nMinSE = nBackupMinSE;
                    nSessionInterval = nBackupSessionInterval;

                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }
    else if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if ((nStatusCode == SipStatusCode::SC_422) &&
                (objMethod.Equals(SipMethod::INVITE) || (bUpdateOnConfirmedDialog == IMS_TRUE)))
        {
            AString strMinSE;

            // Set a Min-SE header field
            if (nMinSE > nConfig_MinSE)
                strMinSE.SetNumber(nMinSE);
            else
                strMinSE.SetNumber(nConfig_MinSE);

            if (piSIPMsg->SetHeader(ISipHeader::MIN_SE, strMinSE) != IMS_SUCCESS)
            {
                // Restore the parameter values
                nRefresher = nBackupRefresher;
                nRefreshRequest = nBackupRefreshRequest;
                nMinSE = nBackupMinSE;
                nSessionInterval = nBackupSessionInterval;

                IMS_TRACE_E(0, "Setting Min-SE header failed", 0, 0, 0);
                return IMS_FALSE;
            }
        }

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            // Set a Session-Expires header field
            if (nSessionInterval > 0)
            {
                // Require header SHALL include the timer option tag
                if (IsRequireHeaderRequired() || (nRefresher == REFRESHER_REMOTE) ||
                        ((nRefresher == REFRESHER_LOCAL) && IsSessionTimerSupportedOnUAC()))
                {
                    // Set a Require header field
                    if (piSIPMsg->AddHeader(ISipHeader::REQUIRE, STR_TIMER) != IMS_SUCCESS)
                    {
                        // Restore the parameter values
                        nRefresher = nBackupRefresher;
                        nRefreshRequest = nBackupRefreshRequest;
                        nMinSE = nBackupMinSE;
                        nSessionInterval = nBackupSessionInterval;

                        IMS_TRACE_E(0, "Adding Require header failed", 0, 0, 0);
                        return IMS_FALSE;
                    }
                }
            }

            if ((nSessionInterval > 0) && IsSessionExpiresHeaderRequired())
            {
                AString strSessionExpires;

                strSessionExpires.SetNumber(nSessionInterval);

                if ((nRefresher == REFRESHER_LOCAL) || (nRefresher == REFRESHER_REMOTE))
                {
                    strSessionExpires.Append(TextParser::CHAR_SEMICOLON);
                    strSessionExpires.Append(STR_REFRESHER);
                    strSessionExpires.Append(TextParser::CHAR_EQUAL);

                    if (nRefresher == REFRESHER_LOCAL)
                        strSessionExpires.Append(STR_UAS);
                    else
                        strSessionExpires.Append(STR_UAC);
                }

                if (piSIPMsg->SetHeader(ISipHeader::SESSION_EXPIRES, strSessionExpires) !=
                        IMS_SUCCESS)
                {
                    // Restore the parameter values
                    nRefresher = nBackupRefresher;
                    nRefreshRequest = nBackupRefreshRequest;
                    nMinSE = nBackupMinSE;
                    nSessionInterval = nBackupSessionInterval;

                    IMS_TRACE_E(0, "Setting Session-Expires header failed", 0, 0, 0);
                    return IMS_FALSE;
                }
            }
        }
    }

    // Restore the parameter values
    nRefresher = nBackupRefresher;
    nRefreshRequest = nBackupRefreshRequest;
    nMinSE = nBackupMinSE;
    nSessionInterval = nBackupSessionInterval;

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_RESULT SessionRefreshHelper::SendRefreshRequest(IN ISipClientConnection* piSCC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCC == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!AddSpecificHeader(piSCC))
    {
        IMS_TRACE_E(0, "Adding the specific headers for a session refresh request failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    StopSessionTimer(piSCC);

    if (RefreshHelper::SendRefreshRequest(piSCC) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    UpdateOnMessageSent(piSCC);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT SessionRefreshHelper::UpdateOnMessageReceived(
        IN CONST ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return RESULT_ERROR;
    }

    const SipMethod& objMethod = piSIPMsg->GetMethod();

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

    ISipDialog* piDialog = piSC->GetDialog();

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
                (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST) && IsTimerActive())
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
    IMS_BOOL bCheckSEPresentity = IsSessionTimerSupportedBySessionExpires();
    IMS_BOOL bTimerOptionSupported = IsSessionTimerSupported(piSC, bCheckSEPresentity);

    if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        if (objMethod.Equals(SipMethod::UPDATE) && (nRefreshRequest != SipMethod::INVALID) &&
                (piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES)))
        {
            // An UPDATE/INVITE has come to update session timer,
            // while it is already being updated by another re-INVITE or UPDATE.
            return RESULT_REJECT_500;
        }

        // Checks if timer option tag is specified in the incoming request message
        UpdateTimerOptionOnRequestReceived(piSC);

        if (piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
        {
            // In this state (STATE_ESTABLISHED), we have received a (re)fresh request,
            // set the session refresh method to indicate that either a (re)INVITE / UPDATE
            // has come
            if (objMethod.Equals(SipMethod::INVITE))
                nRefreshRequest = SipMethod::INVITE;
            else if (objMethod.Equals(SipMethod::UPDATE))
                nRefreshRequest = SipMethod::UPDATE;
        }
        else
        {
            // If "Session-Expires" header didn't come in Request:
            // Check if the session timer was in use (we come to know whether or not
            // session timer was in use through the value of session-interval & Min-SE).
            // If yes, reset the value of these two parameters, so that further requests
            // which originate from this UA goes without "Session-Expires" header.
            if (nSessionInterval != 0)
                nSessionInterval = 0;

            if (nMinSE != 0)
                nMinSE = 0;
        }

        UpdateProperties(piSC, bTimerOptionSupported, IMS_FALSE);

        // Negotiates the session refresher
        // Checks whether incoming message has a Supported header with value "timer"
        // if it's absent, then set the refresher as defined in the session timer spec.
        NegotiateRefresher(bTimerOptionSupported);

        const SipConfigV* pSipConfigV = pService->GetSipConfigV();

        if (pSipConfigV == IMS_NULL)
        {
            return RESULT_ERROR;
        }

        IMS_SINT32 nConfig_MinSE = pSipConfigV->GetSessionMinSE();

        if ((nSessionInterval != 0) &&
                ((nConfig_MinSE > nSessionInterval) || (nMinSE > nSessionInterval)))
        {
            if (bTimerOptionSupported)
            {
                // Once a 422 response is sent, the session timer should be turned off
                nSessionInterval = 0;
                SetDuration(nSessionInterval);

                return RESULT_REJECT_422;
            }
            else
            {
                nSessionInterval = nConfig_MinSE;

                if (nMinSE > nConfig_MinSE)
                    nMinSE = nConfig_MinSE;
            }
        }

        SetDuration(nSessionInterval);

        // Checks if Allow header contains UPDATE method
        bUPDATEAllowed = IMS_FALSE;

        IMSList<AString> objHeaders = piSIPMsg->GetHeaders(ISipHeader::ALLOW);

        for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
        {
            const AString& strHeader = objHeaders.GetAt(i);

            if (strHeader.Equals(SipMethod::ToName(SipMethod::UPDATE)))
            {
                bUPDATEAllowed = IMS_TRUE;
                break;
            }
        }

        IMS_SINT32 nFlag = objMethod.Equals(SipMethod::INVITE)
                ? TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_INVITE
                : TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_UPDATE;

        SetOrClearTimerSupportedOnRemoteEnd(bTimerOptionSupported, nFlag);
    }
    else
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

        if (nStatusCode == SipStatusCode::SC_422)
        {
            // Once a 422 response is sent, the session timer should be turned off
            nSessionInterval = 0;
            UpdateProperties(piSC, bTimerOptionSupported, IMS_FALSE);

            SetDuration(nSessionInterval);
        }
        else if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            SetOrClearTimerSupportedOnRemoteEnd(
                    bTimerOptionSupported, TIMER_SUPPORTED_ON_REMOTE_UA);

            UpdateProperties(piSC, bTimerOptionSupported, IMS_FALSE);

            SetDuration(nSessionInterval);

            // If the response being sent is 200 OK, then start the session timer
            // after modifying the session timer state.
            if ((GetDuration() > 0) && IsSessionRefreshRequired(piSC))
            {
                // Session timer negotiation is done, so reset the request method
                nRefreshRequest = SipMethod::INVALID;

                if (!StartRefresh())
                {
                    IMS_TRACE_E(0, "Starting the session timer failed", 0, 0, 0);
                    return RESULT_ERROR;
                }
            }
            else if (IsSessionTimerUpdateRequiredByReInvite())
            {
                if (IsTimerActive() && !IsSessionTimerSupportedOnRemoteUA())
                {
                    IMS_TRACE_I("Session timer is stopped by remote UA", 0, 0, 0);
                    StopRefresh();
                }
            }

            // Checks if Allow header contains UPDATE method
            bUPDATEAllowed = IMS_FALSE;

            IMSList<AString> objHeaders = piSIPMsg->GetHeaders(ISipHeader::ALLOW);

            for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
            {
                const AString& strHeader = objHeaders.GetAt(i);

                if (strHeader.Equals(SipMethod::ToName(SipMethod::UPDATE)))
                {
                    bUPDATEAllowed = IMS_TRUE;
                    break;
                }
            }

            IMS_TRACE_D("Session timer of remote UA: %s",
                    IsSessionTimerSupportedOnRemoteUA() ? "support" : "not-support", 0, 0);
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

                    nRefreshRequest = SipMethod::INVALID;

                    if ((GetDuration() > 0) && IsSessionTimerSupportedOnRemoteUA())
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

PUBLIC VIRTUAL IMS_RESULT SessionRefreshHelper::UpdateOnMessageSent(IN CONST ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
        return RESULT_ERROR;

    ISipDialog* piDialog = piSC->GetDialog();

    if (piDialog == IMS_NULL)
        return RESULT_ERROR;

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
    {
        IMS_SINT32 nStatusCode = piSIPMsg->GetStatusCode();

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
                        IsSessionTimerSupportedOnRemoteUA() ? "support" : "not-support", 0, 0);
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
            nRefreshRequest = SipMethod::INVALID;

            IMS_BOOL bCheckSEPresentity = IsSessionTimerSupportedBySessionExpires();
            UpdateProperties(piSC, IsSessionTimerSupported(piSC, bCheckSEPresentity), IMS_TRUE);

            if ((GetDuration() > 0) && IsSessionRefreshRequired(piSC))
            {
                if (!StartRefresh())
                {
                    return RESULT_ERROR;
                }
            }
            else if (IsSessionTimerUpdateRequiredByReInvite())
            {
                if (IsTimerActive() && !IsSessionTimerSupportedOnRemoteUA())
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
            nRefreshRequest = SipMethod::INVALID;
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

                    nRefreshRequest = SipMethod::INVALID;

                    if ((GetDuration() > 0) && IsSessionTimerSupportedOnRemoteUA())
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
IMS_BOOL SessionRefreshHelper::AddSpecificHeaderOnEarlyUPDATE(
        IN ISipConnection* piSC, IN IMS_BOOL bTimerOptionSupported)
{
    // Store the current refresher parameter
    IMS_SINT32 nTmpRefresher = nRefresher;

    //---------------------------------------------------------------------------------------------

    if (IsRefresherParameterControlledOnEarlyUPDATE() && (nRefresher == REFRESHER_NONE))
    {
        NegotiateRefresher(bTimerOptionSupported);
    }

    IMS_BOOL bResult = AddSpecificHeader(piSC);

    // Rollback the refresher parameter
    nRefresher = nTmpRefresher;

    return bResult;
}

PUBLIC
IMS_SINT32 SessionRefreshHelper::GetRefreshMethod() const
{
    const SipConfigV* pSipConfigV = pService->GetSipConfigV();

    //---------------------------------------------------------------------------------------------

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
        if (bUPDATEAllowed)
            return SipMethod::UPDATE;
        else
            return SipMethod::INVITE;
    }
}

PUBLIC
IMS_BOOL SessionRefreshHelper::IsSessionTimerSupported(
        IN CONST ISipConnection* piSC, IN IMS_BOOL bCheckSEPresentity /* = IMS_TRUE */)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
        return IMS_FALSE;

    // Get Supported header field values
    AStringArray objSupported = piSIPMsg->GetHeaders(ISipHeader::SUPPORTED);

    if (!objSupported.IsEmpty())
    {
        // Checks if the session timer is supported
        if (objSupported.Contains(TextParser::STR_ASTERISK, IMS_FALSE) ||
                objSupported.Contains(STR_TIMER, IMS_FALSE))
        {
            return IMS_TRUE;
        }
    }

    // Check the additional headers: Require, Session-Expires
    AStringArray objRequire = piSIPMsg->GetHeaders(ISipHeader::REQUIRE);

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
    if (bCheckSEPresentity && piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
    {
        IMS_TRACE_D("Session timer is supported by Session-Expires header", 0, 0, 0);
        return IMS_TRUE;
    }

    if (IsLocalSessionTimerRequired())
    {
        IMS_TRACE_D("Session timer is required by the local endpoint", 0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;

#if 0
    if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        // Get Supported header field values
        AStringArray objSupported = piSIPMsg->GetHeaders(ISipHeader::SUPPORTED);

        if (!objSupported.IsEmpty())
        {
            if (objSupported.Contains("*", IMS_FALSE) || objSupported.Contains("timer", IMS_FALSE))
            {
                return IMS_TRUE;
            }
        }

        return IMS_FALSE;
    }
    else
    {
        // Session timer is not supported by the remote end
        // if there was no Require or Session-Expires header field.
        if (!piSIPMsg->IsHeaderPresent(ISipHeader::REQUIRE)
                || !piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
        {
            return IMS_FALSE;
        }

        // Get Require header field values
        AStringArray objRequire = piSIPMsg->GetHeaders(ISipHeader::REQUIRE);

        if (!objRequire.IsEmpty())
        {
            if (!objRequire.Contains("timer", IMS_FALSE))
                return IMS_FALSE;
        }

        return IMS_TRUE;
    }
#endif
}

PUBLIC
IMS_BOOL SessionRefreshHelper::IsSessionTimerSupportedBySessionExpires() const
{
    //---------------------------------------------------------------------------------------------

    return ((nSIPHeaders & SipConfigV::SESSION_HEADER_CHECK_SESSION_EXPIRES) != 0);
}

PUBLIC
void SessionRefreshHelper::StopSessionTimer(IN CONST ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
        return;

    ISipDialog* piDialog = piSC->GetDialog();

    if (piDialog == IMS_NULL)
        return;

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    // Stop the session timer for the messages of type BYE, UPDATE and re-INVITE.
    if ((piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST) &&
            (piDialog->GetState() == ISipDialog::STATE_CONFIRMED) && IsTimerActive() &&
            (objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::BYE) ||
                    objMethod.Equals(SipMethod::UPDATE)))
    {
        StopRefresh();
    }
}

PUBLIC
void SessionRefreshHelper::UpdateTimerOptionOnRequestReceived(IN CONST ISipConnection* piSC)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    if (piSIPMsg->GetType() != ISipMessage::TYPE_REQUEST)
    {
        return;
    }

    ISipDialog* piDialog = piSC->GetDialog();
    const SipMethod& objMethod = piSIPMsg->GetMethod();

    // Checks if the initial INVITE/re-INVITE/UPDATE request supports
    // the session timer or not... (Supported header)
    if (objMethod.Equals(SipMethod::INVITE) &&
            ((piDialog == IMS_NULL) ||
                    ((piDialog != IMS_NULL) &&
                            (piDialog->GetState() != ISipDialog::STATE_CONFIRMED))))
    {
        if (piSIPMsg->IsOptionSupported(STR_TIMER))
        {
            SetTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_INITIAL_INVITE);
            IMS_TRACE_D("Session timer is supported in INVITE", 0, 0, 0);
        }

        return;
    }

    if ((objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::UPDATE)) &&
            ((piDialog != IMS_NULL) && (piDialog->GetState() == ISipDialog::STATE_CONFIRMED)))
    {
        if (piSIPMsg->IsOptionSupported(STR_TIMER))
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

    //---------------------------------------------------------------------------------------------

    if (nRefresher == REFRESHER_LOCAL)
    {
        nTimerInterval = static_cast<IMS_SINT32>(nTimerInterval / 2);
    }
    else if (nRefresher == REFRESHER_REMOTE)
    {
        if (GetPolicy() != POLICY_SPEC)
        {
            return RefreshHelper::GetTimerInterval();
        }

        const SipConfigV* pSipConfigV = pService->GetSipConfigV();
        // Unit : seconds
        IMS_SINT32 nRefreshMethodTxnTV = (-1);

        if (pSipConfigV != IMS_NULL)
        {
            IMS_SINT32 nRefreshMethod = pSipConfigV->GetSessionMethod();

            if (nRefreshMethod == SipConfigV::SESSION_REFRESH_INVITE)
            {
                nRefreshMethodTxnTV = SipConfigProxy::GetTimerValueB(
                        pService->GetSlotId(), pService->GetSIPProfile(), pSipConfigV);
            }
            else if (nRefreshMethod == SipConfigV::SESSION_REFRESH_UPDATE)
            {
                nRefreshMethodTxnTV = SipConfigProxy::GetTimerValueF(
                        pService->GetSlotId(), pService->GetSIPProfile(), pSipConfigV);
            }
        }

        if (nRefreshMethodTxnTV < 0)
        {
            // (T1 * 64) seconds
            nRefreshMethodTxnTV = SipConfigProxy::GetTimerValueT1(pService->GetSlotId(),
                                          pService->GetSIPProfile(), pSipConfigV) *
                    64;
        }

        // Converts milli-seconds to seconds
        nRefreshMethodTxnTV = nRefreshMethodTxnTV / 1000;

        // 4 from configuration, start point of the session refresh ?
        IMS_SINT32 nMinInterval = static_cast<IMS_SINT32>(nTimerInterval / 3);
        IMS_SINT32 nMaxInterval =
                (nMinInterval > nRefreshMethodTxnTV) ? nMinInterval : nRefreshMethodTxnTV;

        // Re-calculate minimum interval
        nMinInterval = (nMinInterval < nRefreshMethodTxnTV) ? nMinInterval : nRefreshMethodTxnTV;

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
                nTimerInterval = nNonRefresherInterval1;
            else
                nTimerInterval = nNonRefresherInterval2;
        }
    }

    return nTimerInterval;
}

PROTECTED VIRTUAL void SessionRefreshHelper::RefreshCompleted(
        IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    // do something ...
    if (nCode == 0)
    {
        IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

        if (SipStatusCode::IsFinalSuccess(nStatusCode))
        {
            StopRefresh();
        }

        UpdateOnMessageReceived(piSCC);
    }
    else if (nCode == TRANSACTION_TIMEOUT)
    {
        // Behaves as if the 408 request timeout response received
        StopRefresh();
    }

    Refreshable_RefreshCompleted(piSCC, nCode);
}

PROTECTED VIRTUAL void SessionRefreshHelper::RefreshStarted()
{
    //---------------------------------------------------------------------------------------------

    if (nRefreshRequest != SipMethod::INVALID)
        return;

    if (nRefresher == REFRESHER_REMOTE)
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

PROTECTED VIRTUAL void SessionRefreshHelper::RefreshTerminated()
{
    //---------------------------------------------------------------------------------------------

    Refreshable_RefreshTerminated();
}

PROTECTED VIRTUAL IMS_BOOL SessionRefreshHelper::IsSessionTimerUpdateRequiredByReInvite() const
{
    return SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(
            pService->GetSlotId(), pService->GetSIPProfile());
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsLocalSessionTimerRequired() const
{
    //---------------------------------------------------------------------------------------------

    return ((nSIPHeaders & SipConfigV::SESSION_HEADER_LOCAL_TIMER_REQUIRED) != 0);
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsMinSEHeaderRequired() const
{
    //---------------------------------------------------------------------------------------------

    return ((nSIPHeaders & SipConfigV::SESSION_HEADER_MIN_SE) != 0);
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsRefresherParameterControlledOnEarlyUPDATE() const
{
    //---------------------------------------------------------------------------------------------

    return ((nSIPHeaders & SipConfigV::SESSION_HEADER_NO_REFRESHER_CONTROLLED_ON_EARLY_UPDATE) ==
            0);
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsRequireHeaderRequired() const
{
    //---------------------------------------------------------------------------------------------

    return ((nSIPHeaders & SipConfigV::SESSION_HEADER_TIMER_OPTION) != 0);
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsSessionExpiresHeaderRequired() const
{
    //---------------------------------------------------------------------------------------------

    return ((nSIPHeaders & SipConfigV::SESSION_HEADER_SESSION_EXPIRES) != 0);
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsSessionRefreshRequired(IN CONST ISipConnection* piSC) const
{
    //---------------------------------------------------------------------------------------------

    if (piSC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    ISipMessage* piSIPMsg = piSC->GetMessage();

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
    {
        IMS_TRACE_D("The message had no Session-Expires header field;"
                    " no refreshes have to be performed",
                0, 0, 0);

        if (IsLocalSessionTimerRequired())
        {
            IMS_TRACE_D("Session refresh is required by the local endpoint", 0, 0, 0);
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsSessionTimerSupportedOnRemoteUA() const
{
    return IsTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_REMOTE_UA);
}

PRIVATE
IMS_BOOL SessionRefreshHelper::IsSessionTimerSupportedOnUAC() const
{
    return IsTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_INITIAL_INVITE) ||
            IsTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_SESSION_UPDATE);
}

PRIVATE
void SessionRefreshHelper::NegotiateRefresher(IN IMS_BOOL bTimerOptionSupported)
{
    //---------------------------------------------------------------------------------------------

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
        if (nRefresher == REFRESHER_NONE)
            nRefresher = REFRESHER_REMOTE;
    }
    else
    {
        if (nRefresher == REFRESHER_NONE)
            nRefresher = REFRESHER_LOCAL;
        else
            nRefresher = REFRESHER_NONE;
    }
}

PRIVATE
void SessionRefreshHelper::UpdateProperties(IN CONST ISipConnection* piSC,
        IN IMS_BOOL bTimerOptionSupported, IN IMS_BOOL bSent /* = IMS_FALSE */)
{
    ISipMessage* piSIPMsg = piSC->GetMessage();

    //---------------------------------------------------------------------------------------------

    // Gets the Session-Expires header field from SIP message
    // and sets the session interval and refresher.
    if (piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES))
    {
        AString strHeader = piSIPMsg->GetHeader(ISipHeader::SESSION_EXPIRES);
        ISipHeader* piHeader =
                SipParsingHelper::CreateHeader(ISipHeader::SESSION_EXPIRES, strHeader);

        if (piHeader == IMS_NULL)
        {
            return;
        }

        IMS_BOOL bOK = IMS_FALSE;
        nSessionInterval = piHeader->GetValue().ToInt32(&bOK);

        if (!bOK)
        {
            piHeader->Destroy();
            return;
        }

        const SipParameter* pParameter = piHeader->GetParameter(STR_REFRESHER);

        if (pParameter == IMS_NULL)
        {
            nRefresher = REFRESHER_NONE;

            // Overwrite the refresher according to the configuration
            if (IsLocalSessionTimerRequired())
            {
                if (bSent)
                {
                    if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
                        nRefresher = REFRESHER_LOCAL;
                }
                else
                {
                    nRefresher = REFRESHER_LOCAL;
                }
            }
        }
        else
        {
            const AString& strRefresher = pParameter->GetValue();

            if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
            {
                if (bSent)
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                        nRefresher = REFRESHER_REMOTE;
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                        nRefresher = REFRESHER_LOCAL;
                    else
                    {
                        nRefresher = REFRESHER_NONE;

                        // Overwrite the refresher according to the configuration
                        if (IsLocalSessionTimerRequired())
                        {
                            nRefresher = REFRESHER_LOCAL;
                        }
                    }
                }
                else
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                        nRefresher = REFRESHER_LOCAL;
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                        nRefresher = REFRESHER_REMOTE;
                    else
                    {
                        nRefresher = REFRESHER_NONE;

                        // Overwrite the refresher according to the configuration
                        if (IsLocalSessionTimerRequired())
                        {
                            nRefresher = REFRESHER_LOCAL;
                        }
                    }
                }
            }
            else if (piSIPMsg->GetType() == ISipMessage::TYPE_REQUEST)
            {
                if (bSent)
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                        nRefresher = REFRESHER_LOCAL;
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                        nRefresher = REFRESHER_REMOTE;
                    else
                        nRefresher = REFRESHER_NONE;
                }
                else
                {
                    if (strRefresher.EqualsIgnoreCase(STR_UAC))
                        nRefresher = REFRESHER_REMOTE;
                    else if (strRefresher.EqualsIgnoreCase(STR_UAS))
                        nRefresher = REFRESHER_LOCAL;
                    else
                    {
                        nRefresher = REFRESHER_NONE;

                        // Overwrite the refresher according to the configuration
                        if (IsLocalSessionTimerRequired())
                        {
                            nRefresher = REFRESHER_LOCAL;
                        }
                    }
                }
            }
        }

        piHeader->Destroy();
    }

    // Gets Min-SE header from the SIP message and sets the minimum session expiration duration.
    if (piSIPMsg->IsHeaderPresent(ISipHeader::MIN_SE))
    {
        AString strHeader = piSIPMsg->GetHeader(ISipHeader::MIN_SE);
        ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::MIN_SE, strHeader);

        if (piHeader == IMS_NULL)
        {
            return;
        }

        IMS_BOOL bOK = IMS_FALSE;
        IMS_SINT32 nTempMinSE = piHeader->GetValue().ToInt32(&bOK);

        if (!bOK)
        {
            piHeader->Destroy();
            return;
        }

        // The largest value among all Min-SE header field values
        // returned in all 422 responses or received in session refresh requests,
        // on the same dialog.
        if (nTempMinSE > nMinSE)
            nMinSE = nTempMinSE;

        piHeader->Destroy();
    }

    // If the session timer is supported by the peer entity, then the request should come
    // with Session-Expires > Min-SE.
    // Otherwise, we should increase the value of Session-Expires equal to Min-SE.
    IMS_BOOL bSessionIntervalChangeable = !bTimerOptionSupported ||
            (!bSent && (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE) &&
                    (piSIPMsg->GetStatusCode() == SipStatusCode::SC_422));

    if (bSessionIntervalChangeable && (nMinSE > nSessionInterval))
    {
        IMS_TRACE_I("SessionTimer(notSupport or 422) :: %d >> %d", nSessionInterval, nMinSE, 0);
        nSessionInterval = nMinSE;
    }

    //// To handle other optional cases for session refresh...
    if (IsLocalSessionTimerRequired() && !bSent &&
            !piSIPMsg->IsHeaderPresent(ISipHeader::SESSION_EXPIRES) && bTimerOptionSupported)
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
            nRefresher = REFRESHER_LOCAL;

            const SipConfigV* pSipConfigV = pService->GetSipConfigV();

            if (pSipConfigV != IMS_NULL)
            {
                nSessionInterval = pSipConfigV->GetSessionExpires();

                if ((nMinSE > 0) && (nSessionInterval > nMinSE))
                {
                    nSessionInterval = nMinSE;
                }

                IMS_TRACE_D("Remote endpoint is not requesting the session timer,"
                            " but local endpoint supports it (%d:%d)",
                        nSessionInterval, nMinSE, 0);
            }
        }
    }
}
