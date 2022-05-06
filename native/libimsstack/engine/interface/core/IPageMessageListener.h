#ifndef _INTERFACE_PAGE_MESSAGE_LISTENER_H_
#define _INTERFACE_PAGE_MESSAGE_LISTENER_H_

class IPageMessage;

/**
 * @brief This class provides a listener interface to notify the application of the status
 *        of sent page messages.
 *
 * @see IPageMessage
 */
class IPageMessageListener
{
public:
    /**
     * @brief Notifies the application that the page message was successfully delivered.
     *
     * @param piPageMessage Pointer to IPageMessage
     */
    virtual void PageMessageDelivered(IN IPageMessage* piPageMessage) = 0;

    /**
     * @brief Notifies the application that the page message was not successfully delivered.
     *
     * @param piPageMessage Pointer to IPageMessage
     */
    virtual void PageMessageDeliveryFailed(IN IPageMessage* piPageMessage) = 0;
};

#endif  // _INTERFACE_PAGE_MESSAGE_LISTENER_H_
