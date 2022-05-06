#ifndef _INTERFACE_REFERENCE_LISTENER_H_
#define _INTERFACE_REFERENCE_LISTENER_H_

class IReference;
class IMessage;

/**
 * @brief This class provides a listener interface to notify an application
 *        about events regarding an IReference.
 *
 * @see IReference
 */
class IReferenceListener
{
public:
    /**
     * @brief Notifies the application that the reference was successfully delivered.
     *
     * @param piReference Pointer to IReference
     */
    virtual void ReferenceDelivered(IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application that the reference was not successfully delivered.
     *
     * @param piReference Pointer to IReference
     */
    virtual void ReferenceDeliveryFailed(IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application with status reports regarding the Reference.
     *
     * @param piReference Pointer to IReference
     * @param piNotify Pointer to IMessage (including NOTIFY request)
     */
    virtual void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) = 0;

    /**
     * @brief Notifies the application that a reference has been terminated.
     *
     * @param piReference Pointer to IReference
     */
    virtual void ReferenceTerminated(IN IReference* piReference) = 0;
};

#endif  // _INTERFACE_REFERENCE_LISTENER_H_
