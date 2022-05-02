#ifndef _INTERFACE_MESSAGE_MEDIATOR_H_
#define _INTERFACE_MESSAGE_MEDIATOR_H_

#include "ISipMessage.h"

/**
 * @brief This class is SIP message mediator.
 *
 * This class provides a chance to modify SIP message when it's sent by IMS engine.
 *
 * @see ISIPMessage
 */
class IMessageMediator
{
public:
    /// Category of the message to be sent
    enum
    {
        /// Message for standalone or mid-dialog
        MESSAGE_NORMAL = 0,
        /// Message for re-submitted request only (request for auth. challenge)
        MESSAGE_RESUBMIT = 1,
        /// Message for refresh operation
        MESSAGE_REFRESH = 2,
        /// Message sent automatically by Engine
        /// Some special situations just notify this message class to the application:
        ///    - Implicit NOTIFY request (for incoming REFER request)
        ///    - Implicit NOTIFY response (for incoming NOTIFY request)
        ///    - Media negotiation failure response (400/606, only ISession)
        ///    - Glare condition (491/500, only ISession)
        MESSAGE_AUTOMATIC = 3,
        /// Message sent internally by Engine
        /// It's only for call termination case using BYE request
        MESSAGE_INTERNAL_BYE = 4
    };

public:
    /**
     * @brief Adjusts the SIP message before sending it to the network.
     *
     * The application can add/remove the SIP header & message body part
     * when this method is invoked.
     *
     * NOTE:
     * The application MUST NOT change the SDP body part, and SHOULD be careful to
     * change the SIP mandatory headers.\n
     * If those fields are changed, the Engine does not guarantee the correct behavior.
     *
     * @param piSIPMsg SIP message to be sent to the network
     * @param nMessage Category of this message (MESSAGE_XXX)\n
     *                 #MESSAGE_NORMAL\n
     *                 #MESSAGE_RESUBMIT\n
     *                 #MESSAGE_REFRESH\n
     *                 #MESSAGE_AUTOMATIC\n
     *                 #MESSAGE_INTERNAL_BYE
     * @return If the message is changed, it returns IMS_SUCCESS.\n
     *         Otherwise, it returns IMS_FAILURE or other negative value.
     * @see ISIPMessage
     */
    virtual IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISIPMessage *piSIPMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL) = 0;
};

#endif // _INTERFACE_MESSAGE_MEDIATOR_H_
