#ifndef _INTERFACE_REG_CONTACT_LISTENER_H_
#define _INTERFACE_REG_CONTACT_LISTENER_H_

#include "AString.h"

/**
 * @brief This class provides an interface to monitor the contact binding of IMS registration.
 */
class IRegContactListener
{
public:
    /**
     * @brief Notifies the application that the specified service is added to this contact.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     */
    virtual void RegContact_BindingAdded(
            IN CONST AString& strAppId, IN CONST AString& strServiceId) = 0;

    /**
     * @brief Notifies the application that the specified service is removed from this contact.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     */
    virtual void RegContact_BindingRemoved(
            IN CONST AString& strAppId, IN CONST AString& strServiceId) = 0;

    /**
     * @brief Notifies the application that the contact is in ACTIVE or just transit to ACTIVE
     *        state.
     */
    virtual void RegContact_OnActive() = 0;

    /**
     * @brief Notifies the application that the contact transits to ACTIVE state.
     */
    virtual void RegContact_OnTerminated() = 0;
};

#endif  // _INTERFACE_REG_CONTACT_LISTENER_H_
