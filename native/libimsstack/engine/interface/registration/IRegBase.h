#ifndef _INTERFACE_REG_BASE_H_
#define _INTERFACE_REG_BASE_H_

#include "ISipMessage.h"

class IMessageMediator;

/**
 * @brief This class provides a base interface for IMS registration.
 */
class IRegBase
{
public:
    /**
     * @brief Returns the next outgoing SIP request message.
     *
     * If the next message doesn't exist, this method will create the message and return it.
     *
     * @return Pointer to the next SIP request message.
     * @see ISipMessage
     */
    virtual ISipMessage* GetNextRequest() = 0;

    /**
     * @brief Returns the previous SIP request message.
     *
     * If any operation does not execute, the NULL message will be returned.
     *
     * @return Pointer to the previous SIP request message.
     * @see ISipMessage
     */
    virtual ISipMessage* GetPreviousRequest() const = 0;

    /**
     * @brief Returns the previous SIP response message.
     *
     * If any SIP response did not receive, the NULL message will be returned.
     *
     * @return Pointer to the previous SIP response message.
     * @see ISipMessage
     */
    virtual ISipMessage* GetPreviousResponse() const = 0;

    /**
     * @brief Sets the SIP message mediator.
     *
     * @param piMessageMediator SIP message mediator
     * @see IMessageMediator
     * @note SIP_MESSAGE_MEDIATOR
     */
    virtual void SetSipMessageMediator(IN IMessageMediator* piMediator) = 0;
};

#endif  // _INTERFACE_REG_BASE_H_
