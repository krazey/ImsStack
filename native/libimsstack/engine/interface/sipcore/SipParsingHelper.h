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
#ifndef SIP_PARSING_HELPER_H_
#define SIP_PARSING_HELPER_H_

#include "ByteArray.h"

class ISipHeader;
class ISipMessage;
class ISipMessageBodyPart;

/**
 * @brief This class provides a helper interface to parse SIP components.
 */
class SipParsingHelper
{
private:
    SipParsingHelper();

public:
    /**
     * @brief Ceates SIP header with the specified header name only.
     *
     * @param strName SIP header name
     * @return Pointer to ISipHeader.
     */
    static ISipHeader* CreateHeader(IN const AString& strName);

    /**
     * @brief Ceates SIP header with the specified header name and value.
     *
     * @param strName SIP header name
     * @param strValue SIP header value
     * @return Pointer to ISipHeader.
     */
    static ISipHeader* CreateHeader(IN const AString& strName, IN const AString& strValue);

    /**
     * @brief Ceates SIP header with the specified header type and value.
     *
     * @param nType The defined SIP header type
     * @param strValue SIP header value
     * @return Pointer to ISipHeader.
     */
    static ISipHeader* CreateHeader(IN IMS_SINT32 nType, IN const AString& strValue);

    /**
     * @brief Ceates SIP message with the specified message type.
     *
     * @param nType The defined SIP message type\n
     *              #ISipMessage#TYPE_REQUEST\n
     *              #ISipMessage#TYPE_RESPONSE
     * @return Pointer to ISipMessage.
     */
    static ISipMessage* CreateMessage(IN IMS_SINT32 nType);

    /**
     * @brief Ceates SIP message with the specified raw SIP message string.
     *
     * @param objMessage The raw SIP message
     * @return Pointer to ISipMessage.
     */
    static ISipMessage* CreateMessage(IN const ByteArray& objMessage);

    /**
     * @brief Ceates an empty SIP message body part.
     *
     * @return Pointer to ISipMessageBodyPart.
     */
    static ISipMessageBodyPart* CreateMessageBodyPart();

    /**
     * @brief Ceates the SIP message body parts from the specified SIP message.
     *
     * @param piSipMsg SIP message to be parsed
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL CreateMessageBodyParts(IN_OUT ISipMessage* piSipMsg);

    /**
     * @brief Returns Reason header which the protocol indicates "SIP" from Reason header list.
     *
     * @param objReasonHeaders Reason header list
     * @return Reason header which indicates "SIP" protocol.
     */
    static const AString& GetSipReasonHeader(IN const ImsList<AString>& objReasonHeaders);

    /**
     * @brief Parses Reason header with "cause" / "text" parameters.
     *
     * @param strHeaderValue Header value of Reason header field
     * @param nCause "cause" parameter in Reason header\n
     *               If no exists, nCause is set to (-1).
     * @param strText "text" parameter in Reason header\n
     *                If no exists, strText is set to AString::ConstNull().\n
     *                If parameter exists and the value is empty,
     *                it is set to AString::ConstEmpty().
     * @return If there is no parsing errors, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL ParseReasonHeader(
            IN const AString& strHeaderValue, OUT IMS_SINT32& nCause, OUT AString& strText);

    /**
     * @brief Parses Reason header with "cause" / "text" parameters.
     *
     * @param strHeaderValue Header value of Reason header field
     * @param strProtocol Protocol part in Reason header
     * @param nCause "cause" parameter in Reason header\n
     *               If no exists, nCause is set to (-1).
     * @param strText "text" parameter in Reason header\n
     *                If no exists, strText is set to AString::ConstNull().\n
     *                If parameter exists and the value is empty,
     *                it is set to AString::ConstEmpty().
     * @return If there is no parsing errors, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL ParseReasonHeader(IN const AString& strHeaderValue, OUT AString& strProtocol,
            OUT IMS_SINT32& nCause, OUT AString& strText);
};

#endif
