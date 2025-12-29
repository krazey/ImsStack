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

#ifndef INTERFACE_MESSAGE_UTILS_H_
#define INTERFACE_MESSAGE_UTILS_H_

#include "AString.h"
#include "Ims3gpp.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "media/IMedia.h"
#include <optional>

class IMessage;
class ISession;
class SipAddress;
struct ConfUser;
struct Ims3gppData;
struct ReasonHeaderValue;

class IMessageUtils
{
public:
    virtual ~IMessageUtils() {}

    /**
     * @brief Gets
     *
     * @param piSession
     * @param eServiceMethod
     * @param nResponseIndex
     * @return
     */
    virtual IMessage* GetPreviousResponse(IN const ISession* piSession,
            IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex = -1) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @param eServiceMethod
     * @param bIsUac
     * @param nResponseIndex
     * @return
     */
    virtual IMessage* GetRemotePreviousMessage(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_BOOL bIsUac, IN IMS_SINT32 nResponseIndex = -1) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @param eServiceMethod
     * @param nResponseIndex
     * @return
     */
    virtual IMS_SINT32 GetResponseStatusCode(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_SINT32 nResponseIndex = -1) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @param ePeerType
     * @return
     */
    virtual ImsList<AString> GetRemoteUris(IN ISession* piSession, IN PeerType ePeerType) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @param ePeerType
     * @return
     */
    virtual AString GetRemoteUri(IN ISession* piSession, IN PeerType ePeerType) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @return
     */
    virtual AString GetSessionId(IN ISession* piSession) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual ImsList<AString> GetHeaders(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetHeader(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetHeaderValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual IMS_SINT32 GetHeaderValueInt(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param strParameterName
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetParameterValue(IN const IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetUserPart(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Extracts the user part from a given URI string.
     *
     * @param strUri The input URI string to parse.
     * @return An AString containing the extracted user part. Returns an empty string
     *         if the user part cannot be found or the URI is invalid.
     */
    virtual AString GetUserPart(IN const AString& strUri) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual ImsList<AString> GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetDisplayName(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual ImsList<AString> GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the value of a specified parameter from the URI of a specified header in a SIP
     *        message.
     *
     * This method searches for the first occurrence of the specified header and extracts the
     * parameter value from its URI.
     *
     * @param piMessage The SIP message to inspect.
     * @param strParameterName The name of the parameter to retrieve the value for.
     * @param eHeaderType The type of the header to search for (e.g., ISipHeader::FROM).
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return The value of the parameter if found. A null string otherwise.
     */
    virtual AString GetParameterValueFromUri(IN IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the value of a specified parameter from a SipAddress object.
     *
     * It first checks for the parameter in the user-info part of the URI (for sip/sips schemes)
     * and then in the main URI parameters.
     *
     * @param objAddress The SipAddress object to extract the parameter from.
     * @param strParameterName The name of the parameter to retrieve.
     * @return The value of the parameter if found. A null string otherwise.
     */
    virtual AString GetParameterValueFromUri(
            IN const SipAddress& objAddress, IN const AString& strParameterName) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param bWithParameters
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual ImsList<AString> GetUris(IN const IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param bWithParameters
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual AString GetUri(IN const IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual IMS_SINT32 GetSosTypeFromServiceUrn(IN const IMessage* piMessage,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param strProtocol
     * @return
     */
    virtual IMS_SINT32 GetCauseFromReasonHeader(
            IN const IMessage* piMessage, IN const AString& strProtocol = AString::ConstNull()) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @param strProtocol
     * @return
     */
    virtual ReasonHeaderValue GetCauseAndTextFromReasonHeader(
            IN const IMessage* piMessage, IN const AString& strProtocol = AString::ConstNull()) = 0;

    /**
     * @brief Gets the highest-priority Reason header value from a SIP message.
     *
     * This function iterates through a prioritized list of protocols and searches
     * the incoming SIP message for the first matching Reason header. The order of
     * protocols in the input list determines their priority. An `AString::ConstNull()`
     * in the list will match a Reason header with any protocol.
     *
     * @param piMessage The SIP message to inspect for Reason headers.
     * @param lstPrioritizedProtocols A list of protocols to search for, in order
     *                                of priority (e.g., SIP, Q.850).
     * @return A ReasonHeaderValue struct containing the cause, text, and protocol
     *         of the first matching Reason header found. If no matching header is
     *         found, a default-constructed ReasonHeaderValue is returned.
     */
    virtual ReasonHeaderValue GetPrioritizedReasonHeader(IN const IMessage* piMessage,
            IN const std::initializer_list<AString>& lstPrioritizedProtocols) = 0;

    /**
     * @brief Gets
     *        This api is deprecated.
     *
     * @param piMessage
     * @param objIms3gpp
     * @return
     */
    virtual Ims3gpp& GetIms3gppFromBody(IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @return
     */
    virtual Ims3gppData GetIms3gppData(IN const IMessage* piMessage) = 0;

    /**
     * @brief Gets
     *
     * @param piMessage
     * @return
     */
    virtual IMS_SINT32 GetStatusCodeInNotify(IN IMessage* piMessage) = 0;

    /**
     * @brief Hass
     *
     * @param piMessage
     * @return
     */
    virtual IMS_BOOL HasSdp(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks
     *
     * @param piMessage
     * @return
     */
    virtual IMS_BOOL IsFocusConf(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks
     *        This api is deprecated.
     *
     * @param piMessage
     * @return
     */
    virtual IMS_BOOL IsInitialRegistrationRequired(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks
     *        This api is deprecated.
     *
     * @param piMessage
     * @return
     */
    virtual IMS_BOOL IsInitialEmergencyRegistrationRequired(IN const IMessage* piMessage) = 0;

    /**
     * @brief Containss
     *
     * @param piMessage
     * @param strValue
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual IMS_BOOL ContainsValue(IN const IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Checks if a header in the message contains a specific value, ignoring case.
     *
     * @param piMessage The SIP message to check.
     * @param strValue The value to search for within the header.
     * @param eHeaderType The type of header to check (e.g., SIP_HEADER_TYPE_CONTACT).
     * @param strHeaderName The name of the header if eHeaderType is SIP_HEADER_TYPE_GENERIC.
     * @return IMS_TRUE if the value is found in the header (case-insensitive), IMS_FALSE otherwise.
     */
    virtual IMS_BOOL ContainsValueIgnoreCase(IN const IMessage* piMessage,
            IN const AString& strValue, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Checks
     *
     * @param piMessage
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual IMS_BOOL IsHeaderPresent(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Containss
     *
     * @param strHeader
     * @param strTag
     * @return
     */
    virtual IMS_BOOL ContainsTag(IN const AString& strHeader, IN const AString& strTag) = 0;

    /**
     * @brief Containss
     *
     * @param piMessage
     * @param strAddress
     * @return
     */
    virtual IMS_BOOL ContainsAddressInPaid(
            IN const IMessage* piMessage, IN const AString& strAddress) = 0;

    /**
     * @brief Gets a P-Asserted-Identity header from the message, handling multiple headers based on
     *        carrier configuration.
     *
     * The policy is configured by KEY_POLICY_FOR_MULTIPLE_P_ASSERTED_IDENTITY_HEADERS_INT.
     *   - PAI_POLICY_PREFER_TOPMOST: Use the first value.
     *   - PAI_POLICY_PREFER_SIP_URI: Use the first value that is a SIP/SIPS URI.
     *
     * @param objMessage The message to get the P-Asserted-Identity header.
     * @return The selected P-Asserted-Identity header string, or a null string if not found.
     */
    virtual AString GetPai(IN const IMessage& objMessage) = 0;

    /**
     * @brief Sets
     *
     * @param piMessage
     * @param strValue
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual IMS_RESULT SetHeader(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Adds
     *
     * @param piMessage
     * @param strValue
     * @param eHeaderType
     * @param strHeaderName
     * @return
     */
    virtual IMS_RESULT AddValueIfNotExists(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Generates
     *
     * @param strHost
     * @return
     */
    virtual AString GenerateContentId(IN const AString& strHost) = 0;

    /**
     * @brief Sets
     *
     * @param piMessage
     * @param lstConfUser
     * @param bWithDialogId
     * @param bMultiPart
     * @return
     */
    virtual IMS_RESULT SetResourceList(IN_OUT IMessage* piMessage,
            IN const ImsList<ConfUser*>& lstConfUser, IN IMS_BOOL bWithDialogId,
            IN IMS_BOOL bMultiPart) = 0;

    /**
     * @brief Checks if the Contact header of the message contains MMTel ICSI feature tag.
     *
     * @param piMessage Message to be checked.
     * @return True if the header contains the feature. False if it doesn't.
     *         {@code std::nullopt} if no Contact header.
     */
    virtual std::optional<IMS_BOOL> IsMmtelFeatureIncluded(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks if the Contact header of the message contains "video" feature tag.
     *
     * Contact headers without the "mmtel" feature tag are considered invalid, therefore,
     * {@code std::nullopt} will be returned.
     *
     * @param piMessage Message to be checked.
     * @return True if the header contains the feature. False if it doesn't.
     *         {@code std::nullopt} if no Contact header.
     */
    virtual std::optional<IMS_BOOL> IsVideoFeatureIncluded(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks if the Contact header of the message contains "text" feature tag.
     *
     * Contact headers without the "mmtel" feature tag are considered invalid, therefore,
     * {@code std::nullopt} will be returned.
     *
     * @param piMessage Message to be checked.
     * @return True if the header contains the feature. False if it doesn't.
     *         {@code std::nullopt} if no Contact header.
     */
    virtual std::optional<IMS_BOOL> IsTextFeatureIncluded(IN const IMessage* piMessage) = 0;

    /**
     * @brief Gets call type from the IMessage.
     *
     * It checks IMedia from the ISession first if the message contains SDP.
     * If the message doesn't contain SDP, the media from the other source of the message
     * (e.g. header fields) could be returned.
     *
     * @param piMessage IMessage to check.
     * @param piSession ISession of the call.
     * @param bCheckRemote Check the remote media instead of the local if true.
     * @return Call type.
     */
    virtual CallType GetCallType(
            IN const IMessage* piMessage, IN ISession* piSession, IN IMS_BOOL bCheckRemote) = 0;

    /**
     * @brief Gets call type from the current IMedia of the ISession.
     *
     * @param piSession ISession of the call.
     * @param bActiveMediaOnly Deleted or rejected media is ignored if true.
     * @param bCheckRemote Check the remote media instead of the local if true.
     * @param bIgnorePort0 Ignore media with port 0 if true.
     * @return Call type.
     */
    virtual CallType GetCallTypeFromSdp(IN ISession* piSession, IN IMS_BOOL bActiveMediaOnly,
            IN IMS_BOOL bCheckRemote, IN IMS_BOOL bIgnorePort0 = IMS_TRUE) = 0;

    /**
     * @brief Gets remote port number for the given media type from the current IMedia of the
     * ISession.
     *
     * @param piSession
     * @param eMediaType
     * @return
     */
    virtual IMS_SINT32 GetRemotePortFromSdp(
            IN ISession* piSession, IN IMS_SINT32 eMediaType = SdpMedia::TYPE_AUDIO) = 0;

    /**
     * @brief Checks
     *
     * @param piSession
     * @param nStatusCode
     * @return
     */
    virtual IMS_BOOL IsResponseExist(IN ISession* piSession, IN IMS_SINT32 nStatusCode) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @param eServiceMethod
     * @return
     */
    virtual IMS_UINT32 GetNumberOfPreviousResponses(
            IN const ISession* piSession, IN IMS_SINT32 eServiceMethod) const = 0;
};

struct ReasonHeaderValue
{
public:
    inline ReasonHeaderValue() :
            strProtocol(AString::ConstNull()),
            nCause(-1),
            strText(AString::ConstNull())
    {
    }
    inline ~ReasonHeaderValue() {}
    inline ReasonHeaderValue(IN const ReasonHeaderValue& objRhs) :
            strProtocol(objRhs.strProtocol),
            nCause(objRhs.nCause),
            strText(objRhs.strText)
    {
    }
    ReasonHeaderValue& operator=(IN const ReasonHeaderValue& objRhs)
    {
        if (this != &objRhs)
        {
            strProtocol = objRhs.strProtocol;
            nCause = objRhs.nCause;
            strText = objRhs.strText;
        }
        return *this;
    }

    AString strProtocol;
    IMS_SINT32 nCause;
    AString strText;
};

struct Ims3gppData
{
public:
    inline Ims3gppData() :
            eType(Ims3gpp::TYPE_UNKNOWN),
            eAlternativeServiceType(Ims3gpp::AlternativeService::TYPE_UNKNOWN),
            eAlternativeServiceAction(Ims3gpp::AlternativeService::ACTION_UNKNOWN)
    {
    }
    inline ~Ims3gppData() {}
    inline Ims3gppData(IN const Ims3gppData& objRhs) :
            eType(objRhs.eType),
            eAlternativeServiceType(objRhs.eAlternativeServiceType),
            eAlternativeServiceAction(objRhs.eAlternativeServiceAction)
    {
    }
    Ims3gppData& operator=(IN const Ims3gppData& objRhs)
    {
        if (this != &objRhs)
        {
            eType = objRhs.eType;
            eAlternativeServiceType = objRhs.eAlternativeServiceType;
            eAlternativeServiceAction = objRhs.eAlternativeServiceAction;
        }
        return *this;
    }

    IMS_SINT32 eType;
    IMS_SINT32 eAlternativeServiceType;
    IMS_SINT32 eAlternativeServiceAction;
};

#endif
