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
#include "SdpMedia.h"
#include "call/IMtcCall.h"
#include "media/IMedia.h"
#include <optional>

class IMessage;
class ISession;
class SipAddress;
struct ConfUser;
struct Ims3gppData;
struct ReasonHeaderValue;

/**
 * An interface for a collection of utility functions to parse and manipulate SIP messages.
 * This class provides a simplified API to extract information from SIP headers, bodies, and session
 * context without requiring direct interaction with the lower-level SIP stack implementation.
 */
class IMessageUtils
{
public:
    virtual ~IMessageUtils() {}

    /**
     * @brief Gets a previous response message for a specific method within a session.
     *
     * @param piSession The session to search within.
     * @param eServiceMethod The SIP method whose response is needed (e.g., SESSION_START).
     * @param nResponseIndex The index of the response to retrieve. -1 for the latest response.
     * @return A pointer to the {@link IMessage} object representing the response,
               or null if not found.
     */
    virtual IMessage* GetPreviousResponse(IN const ISession* piSession,
            IN IMS_SINT32 eServiceMethod, IN IMS_SINT32 nResponseIndex = -1) = 0;

    /**
     * @brief Gets a previous message (request or response) from the remote party within a session.
     *
     * @deprecated Not used
     * @param piSession The session to search within.
     * @param eServiceMethod The SIP method associated with the message (e.g., SESSION_START).
     * @param bIsUac True if the local side is the UAC, which means this will fetch the remote
                     response.
                    False if the local side is the UAS, which will fetch the remote request.
     * @param nResponseIndex The index of the message to retrieve. -1 for the latest one.
     * @return A pointer to the remote {@link IMessage} object, or null if not found.
     */
    virtual IMessage* GetRemotePreviousMessage(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_BOOL bIsUac, IN IMS_SINT32 nResponseIndex = -1) = 0;

    /**
     * @brief Gets the status code from a previous response message within a session.
     *
     * @param piSession The session to search within.
     * @param eServiceMethod The SIP method whose response status code is needed
     *                       (e.g., SESSION_START).
     * @param nResponseIndex The index of the response. -1 for the latest response.
     * @return The SIP status code, or -1 if the response is not found.
     */
    virtual IMS_SINT32 GetResponseStatusCode(IN ISession* piSession, IN IMS_SINT32 eServiceMethod,
            IN IMS_SINT32 nResponseIndex = -1) = 0;

    /**
     * @brief Gets all remote URIs from a session.
     *
     * @param piSession The session to inspect.
     * @param ePeerType The peer type of the local side.
     * @return A list of strings, where each string is a remote URI.
     */
    virtual ImsList<AString> GetRemoteUris(IN ISession* piSession, IN PeerType ePeerType) = 0;

    /**
     * @brief Gets the primary remote URI from a session.
     *
     * @param piSession The session to inspect.
     * @param ePeerType The peer type of the local side.
     * @return A string containing the remote URI, or a null string if not found.
     */
    virtual AString GetRemoteUri(IN ISession* piSession, IN PeerType ePeerType) = 0;

    /**
     * @brief Gets the session ID from a session.
     *
     * @param piSession The session to get the ID from.
     * @return A string containing the session ID.
     */
    virtual AString GetSessionId(IN ISession* piSession) = 0;

    /**
     * @brief Gets all occurrences of a specified header from a message.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A list of strings containing header field values.
     */
    virtual ImsList<AString> GetHeaders(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the first occurrence of a specified header from a message.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string of header field value, or a null string if not found.
     */
    virtual AString GetHeader(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the value of the first occurrence of a specified header without parameters.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string of header field value without parameters, or a null string if not found.
     */
    virtual AString GetHeaderValue(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the int value of the first occurrence of a specified header without parameters.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return The integer representation of the header value without parameters,
     *         or ISipHeader#INVALID_INT on failure.
     */
    virtual IMS_SINT32 GetHeaderValueInt(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the value of a parameter from a specified header.
     *
     * @param piMessage The message to inspect.
     * @param strParameterName The name of the parameter to retrieve.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string containing the parameter's value, or a null string if not found.
     */
    virtual AString GetParameterValue(IN const IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the user part of the URI from a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string containing the user part (e.g., "user" from "sip:user@host").
     */
    virtual AString GetUserPart(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Extracts the user part from a given URI string.
     *
     * @param strUri The input URI string to parse.
     * @return A string containing the extracted user part. Returns an empty string
     *         if the user part cannot be found or the URI is invalid.
     */
    virtual AString GetUserPart(IN const AString& strUri) = 0;

    /**
     * @brief Gets the user IDs from all occurrences of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A list of strings, each containing a user ID.
     */
    virtual ImsList<AString> GetUserIds(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the user ID from the first occurrence of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string containing the user ID, or a null string if not found.
     */
    virtual AString GetUserId(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the display name of the URI from the first occurrence of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string containing the display name, or a null string if not found.
     */
    virtual AString GetDisplayName(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the host parts of the URI from all occurrences of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A list of strings, each containing a host part (e.g., "host" from "sip:user@host").
     */
    virtual ImsList<AString> GetHosts(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the host part of the URI from the first occurrence of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string containing the host part (e.g., "host" from "sip:user@host").
     */
    virtual AString GetHost(IN IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets a parameter value from the URI within a specified header.
     *
     * This method searches for the first occurrence of the specified header and extracts the
     * parameter value from its URI.
     *
     * @param piMessage The message to inspect.
     * @param strParameterName The name of the parameter to retrieve.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return The value of the parameter, or a null string if not found.
     */
    virtual AString GetParameterValueFromUri(IN IMessage* piMessage,
            IN const AString& strParameterName, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets a parameter value from a SipAddress object.
     *
     * It first checks for the parameter in the user-info part of the URI (for sip/sips schemes)
     * and then in the main URI parameters.
     *
     * @param objAddress The SipAddress object to extract the parameter from.
     * @param strParameterName The name of the parameter to retrieve.
     * @return The value of the parameter, or a null string if not found.
     */
    virtual AString GetParameterValueFromUri(
            IN const SipAddress& objAddress, IN const AString& strParameterName) = 0;

    /**
     * @brief Gets the URIs from all occurrences of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param bWithParameters If true, include URI parameters in the result.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A list of strings, each containing a URI.
     */
    virtual ImsList<AString> GetUris(IN const IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the URI from the first occurrence of a specified header.
     *
     * @param piMessage The message to inspect.
     * @param bWithParameters If true, include URI parameters in the result.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A string containing the URI, or a null string if not found.
     */
    virtual AString GetUri(IN const IMessage* piMessage, IN IMS_BOOL bWithParameters,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the emergency service type from a URN in a specified header.
     *
     * @param piMessage The message to inspect.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return The emergency service type (e.g., EXTRA_CODE_EMERGENCYSERVICE_GENERIC).
     */
    virtual IMS_SINT32 GetSosTypeFromServiceUrn(IN const IMessage* piMessage,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the cause code from a Reason header.
     *
     * @param piMessage The message to inspect.
     * @param strProtocol The protocol to look for (e.g., "SIP", "Q.850").
     *                    If null, any protocol matches.
     * @return The cause code, or -1 if not found.
     */
    virtual IMS_SINT32 GetCauseFromReasonHeader(
            IN const IMessage* piMessage, IN const AString& strProtocol = AString::ConstNull()) = 0;

    /**
     * @brief Gets the cause code and text from a Reason header.
     *
     * @param piMessage The message to inspect.
     * @param strProtocol The protocol to look for (e.g., "SIP", "Q.850").
     *                    If null, any protocol matches.
     * @return A ReasonHeaderValue struct containing the parsed information.
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
     * @brief Parses a 3GPP XML body from a message.
     *
     * @deprecated Not used
     * @param piMessage The message containing the XML body.
     * @param objIms3gpp An output parameter to be filled with the parsed 3GPP data.
     * @return A reference to the populated objIms3gpp.
     */
    virtual Ims3gpp& GetIms3gppFromBody(IN const IMessage* piMessage, OUT Ims3gpp& objIms3gpp) = 0;

    /**
     * @brief Parses a 3GPP XML body from a message.
     *
     * @param piMessage The message containing the XML body.
     * @return An Ims3gppData struct containing the parsed information.
     */
    virtual Ims3gppData GetIms3gppData(IN const IMessage* piMessage) = 0;

    /**
     * @brief Gets the status code from the sipfrag body of a SIP NOTIFY message.
     *
     * @param piMessage The NOTIFY message to inspect.
     * @return The status code from the message body, or SipStatusCode::SC_INVALID if not found.
     */
    virtual IMS_SINT32 GetStatusCodeInNotify(IN IMessage* piMessage) = 0;

    /**
     * @brief Checks if the message has a non-empty SDP body.
     *
     * @param piMessage The message to check.
     * @return True if an SDP body is present, false otherwise.
     */
    virtual IMS_BOOL HasSdp(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks if the message indicates a conference focus.
     *
     * This is typically done by checking for an "isfocus" parameter in the Contact header.
     *
     * @param piMessage The message to check.
     * @return True if the message indicates a conference focus, false otherwise.
     */
    virtual IMS_BOOL IsFocusConf(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks if an initial registration is required based on the 3GPP XML body from a
     *        message.
     *
     * @deprecated Not used
     * @param piMessage The message to inspect.
     * @return True if re-registration is required, false otherwise.
     */
    virtual IMS_BOOL IsInitialRegistrationRequired(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks if an initial emergency registration is required based on the 3GPP XML body
     *        from a message.
     *
     * @deprecated Not used
     * @param piMessage The message to inspect.
     * @return True if emergency registration is required, false otherwise.
     */
    virtual IMS_BOOL IsInitialEmergencyRegistrationRequired(IN const IMessage* piMessage) = 0;

    /**
     * @brief Checks if a header in the message contains a specific value (case-sensitive).
     *
     * @param piMessage The message to check.
     * @param strValue The value to search for within the header.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return True if the value is found, false otherwise.
     */
    virtual IMS_BOOL ContainsValue(IN const IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Checks if a header in the message contains a specific value, ignoring case.
     *
     * @param piMessage The SIP message to check.
     * @param strValue The value to search for within the header.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return True if the value is found, false otherwise.
     */
    virtual IMS_BOOL ContainsValueIgnoreCase(IN const IMessage* piMessage,
            IN const AString& strValue, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Checks if a specific header is present in the message.
     *
     * @param piMessage The message to check.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return True if the header is present, false otherwise.
     */
    virtual IMS_BOOL IsHeaderPresent(IN const IMessage* piMessage, IN IMS_SINT32 eHeaderType,
            IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Checks if a header string contains a specific tag.
     *
     * @param strHeader The full header string to search within.
     * @param strTag The tag to search for.
     * @return True if the tag is found, false otherwise.
     */
    virtual IMS_BOOL ContainsTag(IN const AString& strHeader, IN const AString& strTag) = 0;

    /**
     * @brief Checks if the P-Associated-Identity header contains a specific address.
     *
     * @param piMessage The message to inspect.
     * @param strAddress The address to look for.
     * @return True if the address is found, false otherwise.
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
     * @brief Sets or replaces a header in a message.
     *
     * @param piMessage The message to modify.
     * @param strValue The full value of the header to set.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A result code indicating success or failure.
     */
    virtual IMS_RESULT SetHeader(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Adds a header to a message, but only if a header with the same value doesn't already
     *        exist.
     *
     * @param piMessage The message to modify.
     * @param strValue The full value of the header to add.
     * @param eHeaderType The type of the header. See ISipHeader.
     * @param strHeaderName The name of the header if eHeaderType is UNKNOWN.
     * @return A result code indicating success or failure.
     */
    virtual IMS_RESULT AddValueIfNotExists(IN IMessage* piMessage, IN const AString& strValue,
            IN IMS_SINT32 eHeaderType, IN const AString& strHeaderName = AString::ConstNull()) = 0;

    /**
     * @brief Generates a unique Content-ID value.
     *
     * @param strHost The host string to include in the Content-ID.
     * @return A string containing the generated Content-ID.
     */
    virtual AString GenerateContentId(IN const AString& strHost) = 0;

    /**
     * @brief Constructs and sets a resource-list body for a conference-related message.
     *
     * @param piMessage The message to modify.
     * @param lstConfUser A list of conference users to include in the resource list.
     * @param bWithDialogId If true, include dialog identifiers in the list.
     * @param bMultiPart If true, format the body as a multipart MIME message.
     * @return A result code indicating success or failure.
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
     * @brief Gets call type of the session from the message.
     *
     * It checks IMedia from the ISession first if the message contains SDP.
     * If the message doesn't contain SDP, the media from the other source of the message
     * (e.g. header fields) could be returned - not applied currently.
     *
     * @param piMessage Message to be checked.
     * @param piSession The session to inspect.
     * @param bCheckRemote Check the remote SDP instead of the local if true.
     * @return Call type.
     */
    virtual CallType GetCallType(
            IN const IMessage* piMessage, IN ISession* piSession, IN IMS_BOOL bCheckRemote) = 0;

    /**
     * @brief Gets call type from the current IMedia of the ISession.
     *
     * @param piSession The session to inspect.
     * @param bActiveMediaOnly Deleted or rejected media is ignored if true.
     * @param bCheckRemote Check the remote media instead of the local if true.
     * @param bIgnorePort0 Ignore media with port 0 if true.
     * @return Call type.
     */
    virtual CallType GetCallTypeFromSdp(IN ISession* piSession, IN IMS_BOOL bActiveMediaOnly,
            IN IMS_BOOL bCheckRemote, IN IMS_BOOL bIgnorePort0 = IMS_TRUE) = 0;

    /**
     * @brief Gets remote port number for the given media type from the current IMedia of the
     *        ISession.
     *
     * @param piSession The session to inspect.
     * @param eMediaType The type of the media to inspect.
     * @return Port number. -1 if the media is not found.
     */
    virtual IMS_SINT32 GetRemotePortFromSdp(
            IN ISession* piSession, IN IMS_SINT32 eMediaType = SdpMedia::TYPE_AUDIO) = 0;

    /**
     * @brief Checks if a response with a specific status code has been received in the session.
     *
     * @param piSession The session to inspect.
     * @param nStatusCode The SIP status code to look for.
     * @return True if such a response exists, false otherwise.
     */
    virtual IMS_BOOL IsResponseExist(IN ISession* piSession, IN IMS_SINT32 nStatusCode) = 0;

    /**
     * @brief Gets the number of previous responses received for a specific method.
     *
     * @param piSession The session to inspect.
     * @param eServiceMethod The SIP method (e.g., SESSION_START).
     * @return The count of received responses for that method.
     */
    virtual IMS_UINT32 GetNumberOfPreviousResponses(
            IN const ISession* piSession, IN IMS_SINT32 eServiceMethod) const = 0;
};

/** A struct to hold the parsed values from a SIP Reason header. */
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

/** A struct to hold data parsed from a 3GPP IMS XML body. */
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
