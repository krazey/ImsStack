#ifndef _INTERFACE_CORE_SERVICE_LISTENER_H_
#define _INTERFACE_CORE_SERVICE_LISTENER_H_

class ICoreService;
class IReasonInfo;
class IPageMessage;
class IReference;
class ISession;
class IMessage;
class ICapabilities;

/**
 * @brief This class provides a listener interface to receive notifications on remotely
 *        initiated core service methods.
 *
 * @see ICoreService
 */
class ICoreServiceListener
{
public:
    /**
     * @brief Notifies the application when a page message is received from a remote endpoint.
     *
     * @param piService Pointer to the concerned IService
     * @param piMessage Pointer to the received IPageMessage
     */
    virtual void CoreService_PageMessageReceived(
            IN ICoreService* piService, IN IPageMessage* piMessage) = 0;

    /**
     * @brief Notifies the application when a reference request is received from a remote endpoint.
     *
     * Only references that are created outside of a session are notified in this method.
     *
     * @param piService Pointer to the concerned IService
     * @param piReference Pointer to the received IReference
     */
    virtual void CoreService_ReferenceReceived(
            IN ICoreService* piService, IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application when a ICoreService is closed.
     *
     * @param piService Pointer to the concerned IService
     * @param piReasonInfo Pointer to IReasonInfo
     */
    virtual void CoreService_ServiceClosed(
            IN ICoreService* piService, IN IReasonInfo* piReasonInfo) = 0;

    /**
     * @brief Notifies the application when a session invitation is received
     *        from a remote endpoint.
     *
     * @param piService Pointer to the concerned IService
     * @param piSession Pointer to the received session invitation
     */
    virtual void CoreService_SessionInvitationReceived(
            IN ICoreService* piService, IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application when an unsolicited notify is received.
     *
     * @param piService Pointer to the concerned IService
     * @param piNotify Pointer to the received NOTIFY message
     */
    virtual void CoreService_UnsolicitedNotifyReceived(
            IN ICoreService* piService, IN IMessage* piNotify) = 0;

    //// IMS extensions
    /**
     * @brief Notifies the application when a capability query is received from a remote endpoint.
     *
     * @param piService Pointer to the concerned IService
     * @param piCapabilities Pointer to the received capability query
     */
    virtual void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities) = 0;
};

#endif  // _INTERFACE_CORE_SERVICE_LISTENER_H_
