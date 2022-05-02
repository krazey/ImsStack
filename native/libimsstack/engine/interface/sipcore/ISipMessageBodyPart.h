#ifndef _INTERFACE_SIP_MESSAGE_BODY_PART_H_
#define _INTERFACE_SIP_MESSAGE_BODY_PART_H_

#include "ByteArray.h"
#include "ISipObject.h"

/**
 * @brief This class provides an interface to handle SIP message body part.
 *
 * @see ISIPMessage
 */
class ISIPMessageBodyPart
    : public ISIPObject
{
public:
    /**
     * @brief Clones the SIP message body part.
     *
     * @return Pointer to the cloned ISIPMessageBodyPart.
     */
    virtual ISIPMessageBodyPart* Clone() const = 0;

    /**
     * @brief Copies the SIP message body part from the specified one.
     *
     * @param piBodyPart SIP message body part to be copied
     */
    virtual void CopyFrom(IN CONST ISIPMessageBodyPart *piBodyPart) = 0;

    /**
     * @brief Gets the header field value of the specified header type.
     *
     * @param nType The defined SIP content header type\n
     *              #CONTENT_TYPE\n
     *              #CONTENT_DISPOSITION\n
     *              #CONTENT_TRANSFER_ENCODING\n
     *              #CONTENT_ID\n
     *              #CONTENT_DESCRIPTION\n
     *              #CONTENT_UNKNOWN
     * @param strName The header name, either in full or compact form\n
     *               It's mandatory if nType is CONTENT_UNKNOWN
     * @return The topmost header field value will be returned.\n
     *         It is an empty string if the value was set to be null or empty string.\n
     *         It is null string value, if an error occurs.
     */
    virtual AString GetHeader(IN IMS_SINT32 nType,
            IN CONST AString &strName = AString::ConstNull()) const = 0;

    /**
     * @brief Sets a header value in the SIP message body part.
     *
     * @param nType The defined SIP content header type\n
     *              #CONTENT_TYPE\n
     *              #CONTENT_DISPOSITION\n
     *              #CONTENT_TRANSFER_ENCODING\n
     *              #CONTENT_ID\n
     *              #CONTENT_DESCRIPTION\n
     *              #CONTENT_UNKNOWN
     * @param strValue The header value; null or empty string means a header with no value
     * @param strName The header name, either in full or compact form\n
     *                It's mandatory if nType is CONTENT_UNKNOWN
     */
    virtual void SetHeader(IN IMS_SINT32 nType, IN CONST AString &strValue,
            IN CONST AString &strName = AString::ConstNull()) = 0;

    /**
     * @brief Gets the SIP message body part as a byte array.
     *
     * @return The content as byte array.
     */
    virtual const ByteArray& GetContent() const = 0;

    /**
     * @brief Sets the content to the SIP message body part.
     *
     * @param objContent a content of SIP message body part.
     */
    virtual void SetContent(IN CONST ByteArray &objContent) = 0;

public:
    /// SIP content related headers
    enum
    {
        CONTENT_TYPE = 0,
        CONTENT_DISPOSITION,
        CONTENT_TRANSFER_ENCODING,
        CONTENT_ID,
        CONTENT_DESCRIPTION,
        CONTENT_UNKNOWN
    };
};

#endif // _INTERFACE_SIP_MESSAGE_BODY_PART_H_
