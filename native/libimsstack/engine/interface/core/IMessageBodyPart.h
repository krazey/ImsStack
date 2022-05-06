#ifndef _INTERFACE_MESSAGE_BODY_PART_H_
#define _INTERFACE_MESSAGE_BODY_PART_H_

#include "ByteArray.h"

/**
 * @brief This class provides an interface to access and control SIP message body parts.
 *
 * A MessageBodyPart can contain different kinds of content,
 * for example text, an image or an audio clip.
 *
 * @see IMessage, ISipMessageBodyPart
 */
class IMessageBodyPart
{
public:
    /**
     * @brief Gets the content of this IMessageBodyPart.
     *
     * @return The content of message body part.
     */
    virtual const ByteArray& GetContent() const = 0;

    /**
     * @brief Returns the value of a header in this IMessageBodyPart.
     *
     * Only headers with the prefix "Content-" are allowed to be retrieved with this method.
     *
     * @param strName The header name, in full form
     * @return A string containing the header value or null if the header doesn't exist.
     */
    virtual AString GetHeader(IN CONST AString& strName) const = 0;

    /**
     * @brief Sets the content to this IMessageBodyPart.
     *
     * @param objContent The content of message body part
     * @return If the content is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetContent(IN CONST ByteArray& objContent) = 0;

    /**
     * @brief Sets a header to this IMessageBodyPart.
     *
     * If the header name already exists, the value will be replaced.
     * Only header with the prefix "Content-" are allowed to set with this method.
     *
     * @param strName The header name, in full form
     * @param strValue The header value
     * @return If the header is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetHeader(IN CONST AString& strName, IN CONST AString& strValue) = 0;
};

#endif  // _INTERFACE_MESSAGE_BODY_PART_H_
