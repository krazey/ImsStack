#include "ServiceTrace.h"

#include "Sip.h"
#include "SipAddress.h"
#include "TextParser.h"

#include "utility/MessageUtil.h"
#include "dialingplan/MtcDialingPlan.h"
#include "IMtcApp.h"

#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/ParticipantInfo.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/ConferenceConst.h"
#include "conferencecall/ConferenceConfigurationWrapper.h"
#include "conferencecall/UriFormatter.h"

__IMS_TRACE_TAG_COM_MTC__;

const IMS_CHAR UriFormatter::STR_USER_PHONE[] = ";user=phone";

PUBLIC GLOBAL AString& UriFormatter::GetReferToForInvite(
        OUT AString& strUri, IN IMtcCallContext& objContext)
{
    if (ConferenceConfigurationWrapper::IsPaidPreferred())
    {
        MtcSession* pMtcSession = objContext.GetSession();
        if (pMtcSession != IMS_NULL)
        {
            MessageUtil::GetRemoteUri(
                    &pMtcSession->GetISession(), objContext.GetCallInfo().ePeerType, strUri);
        }
    }
    else
    {
        strUri = objContext.GetParticipantInfo().GetRemoteUri();
    }
    ConvertToValidSipUri(strUri, objContext);

    IMS_TRACE_I("GetReferToForInvite [%s]", strUri.GetStr(), 0, 0);
    return strUri;
}

PUBLIC GLOBAL AString& UriFormatter::GetReferToForInvite(
        OUT AString& strUri, IN IMtcCallContext& objContext, IN const ConfUser* pConfUser)
{
    if (pConfUser == IMS_NULL)
    {
        IMS_TRACE_E(0, "conf user null", 0, 0, 0);
        return strUri;
    }

    if ((pConfUser != IMS_NULL) && (pConfUser->aStrTarget.GetLength() <= 0))
    {
        return strUri;
    }

    strUri = pConfUser->aStrTarget;
    ConvertToValidSipUri(strUri, objContext);

    IMS_TRACE_I("GetReferToForInvite [%s]", strUri.GetStr(), 0, 0);
    return strUri;
}

PUBLIC GLOBAL AString& UriFormatter::GetReferToForBye(
        OUT AString& strUri, IN const ConfUser* pConfUser, IN const AString& strInvitedUri)
{
    AString strUserEntity = pConfUser ? pConfUser->aStrUserEntity : AString::ConstNull();
    if (strUserEntity.GetLength() == 0)
    {
        strUri = strInvitedUri;
        return strUri;
    }

    IMS_BOOL bRealAnonymous = strInvitedUri.Contains("anonymous");
    if (bRealAnonymous)
    {
        strUri = strUserEntity;
        return strUri;
    }

    SipAddress objAddr(strUserEntity);
    if (objAddr.IsSchemeTel())
    {
        strUri = strInvitedUri;
        return strUri;
    }

    if (ConferenceConfigurationWrapper::IsReUseReferToUri() == IMS_TRUE)
    {
        strUri = strInvitedUri;
        return strUri;
    }

    IMS_BOOL bInvalidAnonymous = strUserEntity.Contains("anonymous");
    if (bInvalidAnonymous)
    {
        strUri = strInvitedUri;
        return strUri;
    }

    strUri = pConfUser->aStrUserEntity;

    // sip Uri from 'user entity' && no 'anonymous' case only.
    if (strUri.GetLength() > 0 && !strUri.Contains(STR_USER_PHONE))
    {
        SipAddress objSipAddress;
        objSipAddress.Create(strUri);

        if (objSipAddress.GetScheme().EqualsIgnoreCase(Sip::STR_SIP))
        {
            strUri.Append(STR_USER_PHONE);
        }
    }
    return strUri;
}

PRIVATE GLOBAL void UriFormatter::ConvertToValidSipUri(
        IN_OUT AString& strUri, IN IMtcCallContext& objContext)
{
    if (strUri.Contains(ConferenceConst::ANONYMOUS_URI))
    {
        return;
    }

    SipAddress objSipAddress;
    objSipAddress.Create(strUri);
    AString strScheme = objSipAddress.GetScheme();

    // converting TEL to SIP uri.
    if (objSipAddress.IsSchemeTel())
    {
        AString strUserPart = objSipAddress.GetHost();

        if ((strUserPart.GetLength() > 0) && (strUserPart.Contains(TextParser::CHAR_SEMICOLON)))
        {
            AString strLHS;
            AString strRHS;
            strUserPart.SplitF(TextParser::CHAR_SEMICOLON, strLHS, strRHS);
            strUserPart = strLHS;
        }

        strUri = objContext.GetDialingPlan().GetToUri(
                strUserPart, objContext.GetCallInfo(), Scheme::SIP);
    }
    // if objSipAddress has only numbers, create SIP URI via dialing plan
    else if (!objSipAddress.IsSchemeSip() && !objSipAddress.IsSchemeSips())
    {
        if (strUri.GetLength() > 0)
        {
            strUri = objContext.GetDialingPlan().GetToUri(
                    strUri, objContext.GetCallInfo(), Scheme::SIP);
        }
        else
        {
            strUri = AString::ConstEmpty();
        }
    }

    // adding user=phone uri parameter.
    if (strUri.GetLength() > 0 && !strUri.Contains(STR_USER_PHONE))
    {
        strUri.Append(STR_USER_PHONE);
    }
}
