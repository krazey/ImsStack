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
#ifndef INTERFACE_SIP_MESSAGE_H_
#define INTERFACE_SIP_MESSAGE_H_

#include "ISipMessageBodyPart.h"
#include "ISipObject.h"
#include "SipMethod.h"

/**
 * @brief This class provides an interface to handle SIP message.
 *
 * @see ISipMessageBodyPart
 */
class ISipMessage : public ISipObject
{
public:
    /**
     * @brief Clones the SIP message.
     *
     * @return Pointer to the cloned ISipMessage.
     */
    virtual ISipMessage* Clone() const = 0;

    /**
     * @brief Adds a SIP header to the SIP message.
     *
     * The method always adds the header in the last position of the specified header type.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strValue The header value; null or empty string means a header with no value
     * @param strName The header name, either in full or compact form
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT AddHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the CSeq number.
     *
     * @return A CSeq number.
     */
    virtual IMS_UINT32 GetCSeqNumber() const = 0;

    /**
     * @brief Gets the header field value of the specified header type.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param nIndex The header position in the multiple headers
     * @param strName The header name, either in full or compact form
     * @return The topmost header field value will be returned.\n
     *         It is an empty string if the value was set to be null or empty string.\n
     *         It is null string value, if an error occurs.
     */
    virtual AString GetHeader(IN IMS_SINT32 nType, IN IMS_SINT32 nIndex = 0,
            IN const AString& strName = AString::ConstNull()) const = 0;

    /**
     * @brief Gets the number of the header field value of the specified header type.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strName The header name, either in full or compact form
     * @return The number of header field will be returned.
     */
    virtual IMS_SINT32 GetHeaderCount(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const = 0;

    /**
     * @brief Gets the header field value(s) of the specified header type.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strName The header name, either in full or compact form
     * @return It returns the list of header field values (topmost first).\n
     *         If an error occurs, it returns the empty list.
     */
    virtual IMSList<AString> GetHeaders(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const = 0;

    /**
     * @brief Gets the SIP method.
     *
     * @return The reference to SipMethod.
     */
    virtual const SipMethod& GetMethod() const = 0;

    /**
     * @brief Gets the reason phrase of the SIP message.
     *
     * @return SIP reason phrase.\n
     *         It returns null string if the reason phrase is not available.\n
     *         It returns an empty string if the reason phrase was set with null or empty string.
     */
    virtual const AString& GetReasonPhrase() const = 0;

    /**
     * @brief Gets a Request-URI.
     *
     * @return The Request-URI of SIP message.\n
     *         It returns null string if the Request-URI is not available.
     */
    virtual const AString& GetRequestUri() const = 0;

    /**
     * @brief Gets SIP response status code.
     *
     * @return SIP status code, 1xx, 2xx, 3xx, 4xx, 5xx, 6xx.\n
     *         If the status code is not available, the method returns 0.
     */
    virtual IMS_SINT32 GetStatusCode() const = 0;

    /**
     * @brief Gets the type of SIP message.
     *
     * @return Type of SIP message.\n
     *         #TYPE_REQUEST\n
     *         #TYPE_RESPONSE
     */
    virtual IMS_SINT32 GetType() const = 0;

    /**
     * @brief Adds a SIP header to the SIP message.
     *
     * The method always adds the header in the first position of the specified header type.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strValue The header value; null or empty string means a header with no value
     * @param strName The header name, either in full or compact form
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT PrependHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) = 0;

    /**
     * @brief Removes the header from the SIP message.
     *
     * If the multiple header field values exist, the topmost is removed.\n
     * If the specified header is not found, this method does nothing.
     *
     * @param nType The defined SIP header type
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strName The header name, either in full or compact form
     */
    virtual void RemoveHeader(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) = 0;

    /**
     * @brief Sets a header value in the SIP message.
     *
     * If the header does not exist, it will be added to the message.
     * Otherwise, the existing header is overwritten.\n
     * If the multiple header field values exist, the topmost is overwritten.
     *
     * @param nType The defined SIP header type\n
     *              If this value is ISipHeader::UNKNOWN, strName MUST be
     *              a valid header name.
     * @param strValue The header value; null or empty string means a header with no value
     * @param strName The header name, either in full or compact form
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetHeader(IN IMS_SINT32 nType, IN const AString& strValue,
            IN const AString& strName = AString::ConstNull()) = 0;

    /**
     * @brief Creates a new SIP message body part and adds it to the SIP message.
     *
     * This method is only used to set non-SDP content types.
     *
     * @return Pointer to new ISipMessageBodyPart.
     */
    virtual ISipMessageBodyPart* CreateBodyPart() = 0;

    /**
     * @brief Creates a new SIP message body part and adds it to the SIP message.
     *
     * This method is only used to set SDP content type.
     *
     * @return Pointer to new ISipMessageBodyPart.
     */
    virtual ISipMessageBodyPart* CreateSdpBodyPart() = 0;

    /**
     * @brief Returns all message body parts that are added to the SIP message.
     *
     * This method returns all message body parts excepting SDP message body.
     *
     * @return List of pointer to ISipMessageBodyPart.
     */
    virtual IMSList<ISipMessageBodyPart*> GetBodyParts() const = 0;

    /**
     * @brief Returns a SDP message body if present.
     *
     * @return Pointer to ISipMessageBodyPart.\n
     *         It returns null pointer if SDP message body does not exist.
     */
    virtual ISipMessageBodyPart* GetSdpBodyPart() const = 0;

    /**
     * @brief Returns the list of SDP message body if present.
     *
     * @return List of pointer to ISipMessageBodyPart.\n
     *         The list will be empty if SDP message body does not exist.
     */
    virtual IMSList<ISipMessageBodyPart*> GetSdpBodyParts() const = 0;

    /**
     * @brief Updates the SIP headers & message body parts with the specified SIP message.
     *
     * @param piSipMsg Pointer to the SIP message
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT CopyHeadersAndBodyParts(IN const ISipMessage* piSipMsg) = 0;

    /**
     * @brief Checks if the specified header field is present.
     *
     * @param nType Type of SIP header
     * @param strName SIP header name; pass this argument if the type is unknown
     * @return If the specified header field exists, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsHeaderPresent(
            IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull()) const = 0;

    /**
     * @brief Checks if the message is RPR (Reliable Provisional Response).
     *
     * @return If this is a reliable provisional response message, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsMessageRpr() const = 0;

    /**
     * @brief Checks if the specified option tag is required in the remote endpoint.
     *
     * @param strOption SIP option tag to be checked
     * @return If this message requires an option tag, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsOptionRequired(IN const AString& strOption) const = 0;

    /**
     * @brief Checks if the specified option tag is supported in the remote endpoint.
     *
     * @param strOption SIP option tag to be checked
     * @return If this message has an option tag, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsOptionSupported(IN const AString& strOption) const = 0;

    /**
     * @brief Removes all the message body parts if present.
     */
    virtual void RemoveBodyParts() = 0;

    /**
     * @brief Returns the raw SIP message according to the specified level.
     *
     * @param nOptions Options for SIP message forming (start-line/header-part/body-part)
     * @return Raw SIP message as string.
     */
    virtual ByteArray ToByteArray(IN IMS_SINT32 nOptions = OPT_ALL) const = 0;

public:
    /// Types of SIP message
    enum
    {
        /// SIP request message
        TYPE_REQUEST = 0,
        /// SIP response message
        TYPE_RESPONSE,
        TYPE_ANY
    };

    /// Options for SIP message encoding as string
    enum
    {
        /// When encoding SIP start-line only
        OPT_START_LINE = 0x01,
        /// When encoding SIP header parts only
        OPT_HEADER_PART = 0x02,
        /// When encoding SIP message body parts only
        OPT_BODY_PART = 0x04,
        /// When encoding full SIP message
        OPT_ALL = (OPT_START_LINE | OPT_HEADER_PART | OPT_BODY_PART)
    };
};

#endif
