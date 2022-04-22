#ifndef INTERFACE_MTC_MESSAGE_HANDLER_H_
#define INTERFACE_MTC_MESSAGE_HANDLER_H_

#include "IMSTypeDef.h"

class IMessage;

/*
 * This class formats requests and responses and get information from messages.
 */
class IMtcMessageHandler
{
public:
    virtual ~IMtcMessageHandler() {}

    /**
     * Format given request message before the request is sent to the remote.
     * Request method should be decided already.
     */
    virtual void FormatRequest(IN IMS_UINT32 nMethod, IN_OUT IMessage& objRequest) = 0;

    /**
     * Format given response message before the response is sent to the remote.
     * Response code should be decided already.
     */
    virtual void FormatResponse(IN IMS_UINT32 nMethod, IN_OUT IMessage& objResponse) = 0;

    /**
     * Get information and update its state if needed from given request from the remote.
     */
    virtual void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) = 0;

    /**
     * Get information and update its state if needed from given response from the remote.
     */
    virtual void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) = 0;
};

#endif
