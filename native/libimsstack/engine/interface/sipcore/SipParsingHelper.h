#ifndef _SIP_PARSING_HELPER_H_
#define _SIP_PARSING_HELPER_H_

#include "ByteArray.h"

class ISIPHeader;
class ISIPMessage;
class ISIPMessageBodyPart;

/**
 * @brief This class provides a helper interface to parse SIP components.
 */
class SIPParsingHelper
{
private:
    SIPParsingHelper();

public:
    /**
     * @brief Ceates SIP header with the specified header name only.
     *
     * @param strName SIP header name
     * @return Pointer to ISIPHeader.
     */
    static ISIPHeader* CreateHeader(IN CONST AString &strName);

    /**
     * @brief Ceates SIP header with the specified header name and value.
     *
     * @param strName SIP header name
     * @param strValue SIP header value
     * @return Pointer to ISIPHeader.
     */
    static ISIPHeader* CreateHeader(IN CONST AString &strName, IN CONST AString &strValue);

    /**
     * @brief Ceates SIP header with the specified header type and value.
     *
     * @param nType The defined SIP header type
     * @param strValue SIP header value
     * @return Pointer to ISIPHeader.
     */
    static ISIPHeader* CreateHeader(IN IMS_SINT32 nType, IN CONST AString &strValue);

    /**
     * @brief Ceates SIP message with the specified message type.
     *
     * @param nType The defined SIP message type\n
     *              #ISIPMessage#TYPE_REQUEST\n
     *              #ISIPMessage#TYPE_RESPONSE
     * @return Pointer to ISIPMessage.
     */
    static ISIPMessage* CreateMessage(IN IMS_SINT32 nType);

    /**
     * @brief Ceates SIP message with the specified raw SIP message string.
     *
     * @param objMessage The raw SIP message
     * @return Pointer to ISIPMessage.
     */
    static ISIPMessage* CreateMessage(IN CONST ByteArray &objMessage);

    /**
     * @brief Ceates an empty SIP message body part.
     *
     * @return Pointer to ISIPMessageBodyPart.
     */
    static ISIPMessageBodyPart* CreateMessageBodyPart();

    /**
     * @brief Ceates the SIP message body parts from the specified SIP message.
     *
     * @param piSIPMsg SIP message to be parsed
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL CreateMessageBodyParts(IN_OUT ISIPMessage *piSIPMsg);

    /**
     * @brief Returns Reason header which the protocol indicates "SIP" from Reason header list.
     *
     * @param objReasonHeaders Reason header list
     * @return Reason header which indicates "SIP" protocol.
     */
    static const AString& GetSIPReasonHeader(IN CONST IMSList<AString> &objReasonHeaders);

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
    static IMS_BOOL ParseReasonHeader(IN CONST AString &strHeaderValue,
            OUT IMS_SINT32 &nCause, OUT AString &strText);

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
    static IMS_BOOL ParseReasonHeader(IN CONST AString &strHeaderValue,
            OUT AString &strProtocol, OUT IMS_SINT32 &nCause, OUT AString &strText);
};

#endif // _SIP_PARSING_HELPER_H_
