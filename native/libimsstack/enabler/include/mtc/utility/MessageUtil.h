#ifndef MTC_MESSAGE_UTIL_H_
#define MTC_MESSAGE_UTIL_H_

#include "AString.h"
#include "IMtcService.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

class AStringBuffer;
class ConfUser;
class IMessage;
class IMessageBodyPart;
class Ims3gpp;
class ISession;
class ISipHeader;
class ISipMessage;
class IMtcCall;
class SipAddress;
class SipParameter;

class MessageUtil
{
public:
    // Changed :: GetResponseMsg -> GetPreviousResponse
    static IMessage* GetPreviousResponse(IN const ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_SINT32 nResponseIndex = INVALID_INDEX);
    // Changed :: GetRemotePreviousMsg -> GetRemotePreviousMessage
    static IMessage* GetRemotePreviousMessage(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_BOOL bIsUac, IN IMS_SINT32 nResponseIndex = INVALID_INDEX);
    static IMS_SINT32 GetResponseStatusCode(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_SINT32 nResponseIndex = INVALID_INDEX);
    // Changed :: GetRemoteURIs -> GetRemoteUris
    static IMS_RESULT GetRemoteUris(
            IN ISession* piSession, IN PeerType ePeerType, OUT IMSList<AString>& lstUris);
    // Changed :: GetRemoteURI -> GetRemoteUri
    static IMS_RESULT GetRemoteUri(
            IN ISession* piSession, IN PeerType ePeerType, OUT AString& strUri);
    // Changed :: GetSessionID -> GetSessionId
    static IMS_RESULT GetSessionId(IN ISession* piSession, OUT AString& strSessionId);

    static IMS_RESULT GetHeaders(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT IMSList<AString>& lstHeaders,
            IN const AString& strHeaderName = AString::ConstNull());
    static IMS_RESULT GetHeader(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strHeader, IN const AString& strHeaderName = AString::ConstNull());
    static IMS_RESULT GetHeaderValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strValue, IN const AString& strHeaderName = AString::ConstNull());
    static IMS_SINT32 GetHeaderValueInt(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetHeaderParameterValue -> GetParameterValue
    static IMS_RESULT GetParameterValue(IN const IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
            IN const AString& strHeaderName = AString::ConstNull());

    // Changed :: GetUserPartsFromXXXX -> GetUserParts
    static IMSList<AString> GetUserParts(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetUserPartFromXXXX -> GetUserPart
    static IMS_RESULT GetUserPart(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strUserPart, IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetUserIDsFromHdr -> GetUserIds(piMessage, ISipHeader::FROM)
    static IMS_RESULT GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT IMSList<AString>& lstUserIds,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetUserIDFromHdr -> GetUserId(piMessage, ISipHeader::FROM)
    static IMS_RESULT GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strUserId, IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetUserDisplaynamesFromXXX -> GetDisplayNames
    static IMS_RESULT GetDisplayNames(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT IMSList<AString>& lstDisplayNames,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetUserDisplaynameFromXXX -> GetDisplayName
    static IMS_RESULT GetDisplayName(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strDisplayName, IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetHosts -> GetHosts(piMessage, ISipHeader::CONTACT_NORMAL)
    static IMS_RESULT GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT IMSList<AString>& lstHosts, IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetHost -> GetHost(piMessage, ISipHeader::CONTACT_NORMAL)
    static IMS_RESULT GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT AString& strHost, IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetURIParameters -> GetParameterValueFromUri
    static IMS_RESULT GetParameterValueFromUri(IN IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType, OUT AString& strValue,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetContactURIs -> GetUris(piMessage, bWithParameters, ISipHeader::CONTACT_NORMAL)
    // Changed :: GetRemoteURIsFromHdr -> GetUris(piMessage, IMS_FALSE, ISipHeader::FROM)
    static IMS_RESULT GetUris(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, OUT IMSList<AString>& lstUris,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetContactURI -> GetUri(piMessage, bWithParameters, ISipHeader::CONTACT_NORMAL)
    // Changed :: GetRemoteURIFromHdr -> GetUri(piMessage, IMS_FALSE, ISipHeader::FROM)
    static IMS_RESULT GetUri(IN IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, OUT AString& strUri,
            IN const AString& strHeaderName = AString::ConstNull());

    static IMS_SINT32 GetFeatures(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: GetSOSTypeFromServiceURN -> GetSosTypeFromServiceUrn
    static IMS_SINT32 GetSosTypeFromServiceUrn(IN const IMessage* piMessage,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull());

    // Changed :: GetCauseFromReason -> GetCauseFromReasonHeader
    static IMS_SINT32 GetCauseFromReasonHeader(
            IN const IMessage* piMessage, IN const AString& strProtocol = AString::ConstNull());
    static IMS_RESULT GetCauseAndTextFromReasonHeader(IN const IMessage* piMessage,
            OUT IMS_SINT32& nCause, OUT AString& strText,
            IN const AString& strProtocol = AString::ConstNull());
    static IMS_SINT32 GetSupportedFeatures(IN IMessage* piMessage);
    static IMS_SINT32 GetRequireFeatures(IN IMessage* piMessage);
    // Removed :: GetIDFromEvent -> GetParameterValue(piMessage, STR_ID, ISipHeader::EVENT, strId)
    // Removed :: GetSubscriptionState ->
    //            GetHeaderValue(piMessage, ISipHeader::SUBSCRIPTION_STATE, strSubscriptionState)
    // Changed :: GetIms3gppFromMsgBody -> GetIms3gppFromBody
    static IMS_RESULT GetIms3gppFromBody(IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp);
    static IMS_SINT32 GetStatusCodeInNotify(IN IMessage* piMessage);

    // Removed :: IsQoSAttr
    // Changed :: HasSDP -> HasSdp
    static IMS_BOOL HasSdp(IN const IMessage* piMessage);
    static IMS_BOOL IsFocusConf(IN const IMessage* piMessage);
    // Removed :: IsRestoration -> IsInitialRegistrationRequired
    static IMS_BOOL IsInitialRegistrationRequired(IN const IMessage* piMessage);
    static IMS_BOOL ContainsValue(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull());
    static IMS_BOOL HasValue(IN const IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull());
    static IMS_BOOL IsHeaderPresent(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull());
    static IMS_BOOL ContainsTag(IN const AString& strHeader, IN const AString& strTag);
    static IMS_BOOL ContainsAddressInPaid(
            IN const IMessage* piMessage, IN const AString& strAddress);

    static IMS_RESULT SetHeader(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull());
    // Changed :: AddHeader -> AddValueIfNotExists
    static IMS_RESULT AddValueIfNotExists(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull());

    // Changed :: GenerateCID -> GenerateContentId
    static IMS_RESULT GenerateContentId(IN const AString& strHost, OUT AString& strContentId,
            IN IMS_BOOL bAngleQuote = IMS_FALSE);
    // Changed :: SetResourceListByConf -> SetResourceListByConfUser
    static IMS_RESULT SetResourceListByConfUser(IN_OUT IMessage* piMessage,
            IN const AString& strContentId, IN IMSList<ConfUser*>& lstConfUser,
            IN IMS_BOOL bMultiPart, IN IMS_BOOL bCopyControl = IMS_TRUE);
    // Changed :: SetResourceListByURI -> SetResourceListByEntryUri
    static IMS_RESULT SetResourceListByEntryUri(IN_OUT IMessage* piMessage,
            IN const AString& strContentId, IN IMSList<AString>& lstEntryUri,
            IN IMS_BOOL bMultiPart, IN IMS_BOOL bCopyControl = IMS_TRUE);
    static IMS_BOOL IsVideoFeatureIncluded(IN const IMessage* piMessage);
    static IMS_BOOL IsTextFeatureIncluded(IN const IMessage* piMessage);

private:
    static ISipMessage* GetSipMessage(IN const IMessage* piMessage);
    static IMS_RESULT GetAddresses(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            OUT IMSList<SipAddress>& lstAddresses,
            IN const AString& strHeaderName = AString::ConstNull());
    static void GetParameterValueFromUnknownHeaderBody(
            IN const AString& strBody, IN const AString& strParameterName, OUT AString& strValue);
    static IMS_RESULT GetUrnValue(IN const IMessage* piMessage, IN const AString& strId,
            IN IMS_SINT32 eHeaderType, OUT AString& strValue,
            IN const AString& strHeaderName = AString::ConstNull());
    static IMS_RESULT SetResourceList(IN_OUT IMessage* piMessage, IN const AString& strContentId,
            IN IMS_BOOL bMultiPart, IN AStringBuffer& objXml);

public:
    // legacy API
    static ServiceType CheckServiceType(IN IMessage* piMessage, IN IMtcCall* pSession = IMS_NULL,
            IN ISession* piSession = IMS_NULL);
    static CallType GetCallType(
            IN const IMessage* piMessage, IN ISession* piSession, IN IMS_BOOL bPeerView);
    static CallType GetCallTypeFromSdp(IN ISession* piSession, IN IMS_BOOL bNego,
            IN IMS_BOOL bPeerView);  // TODO: change name of bPeerView
    static CallType GetCallTypeFromAcceptContact(
            IN IMessage* piMessage, IN ISession* piSession = IMS_NULL);
    static IMS_SINT32 CheckRttUpdateRequest(
            IN IMessage* piMessage, IN ISession* piSession = IMS_NULL);
    static IMS_BOOL IsSessionRefresh(IN ISession* piSession);
    static IMS_BOOL IsTextSession(IN ISession* piSession);
    static IMS_BOOL IsResponseExist(IN ISession* piSession, IN IMS_SINT32 nStatusCode);

public:
    static const IMS_CHAR STR_199[];
    static const IMS_CHAR STR_ALERT_URN_CALL_WAITING[];
    static const IMS_CHAR STR_ANONYMOUS[];
    static const IMS_CHAR STR_UNAVAILABLE[];
    static const IMS_CHAR STR_CONTENT_DISPOSITION_RECIPIENT_LIST[];
    static const IMS_CHAR STR_CONTENT_ID[];
    static const IMS_CHAR STR_CONTENT_TYPE_3GPP_IMS_XML[];
    static const IMS_CHAR STR_CONTENT_TYPE_APPLICATION[];
    static const IMS_CHAR STR_CONTENT_TYPE_RESOURCE_LISTS_XML[];
    static const IMS_CHAR STR_CONTENT_TYPE_SIP_FRAG[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_SDP[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML[];
    static const IMS_CHAR STR_HISTINFO[];
    static const IMS_CHAR STR_ICSI[];
    static const IMS_CHAR STR_ID[];
    static const IMS_CHAR STR_NONE[];
    static const IMS_CHAR STR_PARAMETER_IS_FOCUS[];
    static const IMS_CHAR STR_PRECONDITION[];
    static const IMS_CHAR STR_REASON_FAILURE_TO_TRANSITION[];
    static const IMS_CHAR STR_REASON_HANDOVER_CANCELLED[];
    static const IMS_CHAR STR_RELEASE_CAUSE_1[];
    static const IMS_CHAR STR_RELEASE_CAUSE_2[];
    static const IMS_CHAR STR_RELEASE_CAUSE_3[];
    static const IMS_CHAR STR_RELEASE_CAUSE_4[];
    static const IMS_CHAR STR_RELEASE_CAUSE_5[];
    static const IMS_CHAR STR_RELEASE_CAUSE_6[];
    static const IMS_CHAR STR_REPLACES[];
    static const IMS_CHAR STR_SERVICE[];
    static const IMS_CHAR STR_SOS_AMBULANCE[];
    static const IMS_CHAR STR_SOS_ANIMAL_CONTROL[];
    static const IMS_CHAR STR_SOS_COUNTRY_SPECIFIC[];
    static const IMS_CHAR STR_SOS_FIRE[];
    static const IMS_CHAR STR_SOS_GAS[];
    static const IMS_CHAR STR_SOS_MARINE[];
    static const IMS_CHAR STR_SOS_MOUNTAIN[];
    static const IMS_CHAR STR_SOS_PHYSICIAN[];
    static const IMS_CHAR STR_SOS_POISON[];
    static const IMS_CHAR STR_SOS_POLICE[];
    static const IMS_CHAR STR_SOS[];
    static const IMS_CHAR STR_SRVCC_FEATURE_A[];
    static const IMS_CHAR STR_SRVCC_FEATURE_B[];
    static const IMS_CHAR STR_SRVCC_FEATURE_M[];
    static const IMS_CHAR STR_SUPPORTED[];
    static const IMS_CHAR STR_TIMER[];
    static const IMS_CHAR STR_URN[];
    static const IMS_CHAR STR_VIDEO[];
    static const IMS_CHAR STR_TEXT[];
    static const IMS_CHAR STR_P_TTA_VOLTE_INFO[];
    static const IMS_CHAR STR_AVCHANGE[];

private:
    static const IMS_SINT32 INVALID_INDEX;
};

#endif
