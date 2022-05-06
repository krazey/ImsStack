#ifndef _INTERFACE_PAGE_MESSAGE_H_
#define _INTERFACE_PAGE_MESSAGE_H_

#include "IServiceMethod.h"
#include "ByteArray.h"

class IPageMessageListener;

/**
 * @brief This class provides an interface for simple instant messages or exchange of
 *        small amounts of content outside of a session.
 *
 * @see IServiceMethod, IPageMessageListener
 */
class IPageMessage : public IServiceMethod
{
public:
    /**
     * @brief Returns the content from this IPageMessage.
     *
     * This method will return the content
     * from the firest body part if there are more than one.
     *
     * @return Byte array containing the content.
     */
    virtual const ByteArray& GetContent() const = 0;

    /**
     * @brief Returns the content MIME type of this IPageMessage.
     *
     * This method will return
     * the content MIME type from the first body part if there are more than one.
     *
     * @return Content MIME type.
     */
    virtual AString GetContentType() const = 0;

    /**
     * @brief Returns the current state of the state machine of the IPageMessage.
     *
     * @return The current state of this IPageMessage.\n
     *         #STATE_UNSENT\n
     *         #STATE_SENT\n
     *         #STATE_RECEIVED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Sends the IPageMessage.
     *
     * This method can be invoked Send(ByteArray::ConstNull(), AString::ConstNull())
     * if the application uses the IMessage interface to fill the content.\n
     * In the case that the application uses both the IMessage interface
     * and Send(objContent, strContentType), the content will be added as the last body part.
     *
     * @param objContent Byte array containing the content to be sent
     * @param strContentType Content MIME type
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Send(IN CONST ByteArray& objContent, IN CONST AString& strContentType) = 0;

    /**
     * @brief Sets a listener for this IPageMessage, replacing any previous IPageMessageListener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN IPageMessageListener* piListener) = 0;

    //// IMS extensions

    /**
     * @brief Sends a successful final response to an incoming page message from a remote endpoint.
     *
     * @param nStatusCode SIP status code
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Accept(IN IMS_SINT32 nStatusCode = 200) = 0;

    /**
     * @brief Sends a failure final response to an incoming page message from a remote endpoint.
     *
     * @param nStatusCode SIP status code
     * @param nRetryAfter Value for Retry-After header field (seconds unit)
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0) = 0;

public:
    /// States of IPageMessage
    enum
    {
        STATE_UNSENT = 1,
        STATE_SENT = 2,
        STATE_RECEIVED = 3
    };
};

#endif  // _INTERFACE_PAGE_MESSAGE_H_
