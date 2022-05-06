#ifndef _INTERFACE_METHOD_H_
#define _INTERFACE_METHOD_H_

class IMessageMediator;

/**
 * @brief This class is a base interface for service method of Engine.
 */
class IMethod
{
public:
    /**
     * @brief Destroys IMethod interface.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Sets the SIP message mediator.
     *
     * @param piMessageMediator SIP message mediator
     * @see IMessageMediator
     * @note SIP_MESSAGE_MEDIATOR
     */
    virtual void SetMessageMediator(IN IMessageMediator* piMediator) = 0;
};

#endif  // _INTERFACE_METHOD_H_
