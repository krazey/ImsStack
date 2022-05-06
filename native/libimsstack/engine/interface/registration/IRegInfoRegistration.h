#ifndef _INTERFACE_REG_INFO_REGISTRATION_H_
#define _INTERFACE_REG_INFO_REGISTRATION_H_

#include "IRegInfoContact.h"

/**
 * @brief This class provides an interface to access the registration element
 *        of the "reginfo" XML document.
 */
class IRegInfoRegistration
{
public:
    /**
     * @brief Returns the AOR of this registration element.
     *
     * @return AOR (Address Of Record) of registration.
     */
    virtual const SipAddress& GetAOR() const = 0;

    /**
     * @brief Returns the contact with the specified contact uri of this registration element.
     *
     * @param objContactUri the contact URI to be retrieved
     * @return Pointer to IRegInfoContact or null.
     */
    virtual IRegInfoContact* GetContact(IN CONST SipAddress& objContactUri) const = 0;

    /**
     * @brief Returns all the contacts of this registration element.
     *
     * @param objContactUri the contact URI to be retrieved
     * @return List of pointer to IRegInfoContact.
     */
    virtual IMSList<IRegInfoContact*> GetContacts() const = 0;

    /**
     * @brief Returns the preferred contact information in this registration element.
     *
     * @return Pointer to IRegInfoContact.
     */
    virtual IRegInfoContact* GetPriorContact() const = 0;

    /**
     * @brief Returns the state of this registration element.
     *
     * @return State of registration element.\n
     *         #STATE_CREATED\n
     *         #STATE_INIT\n
     *         #STATE_ACTIVE\n
     *         #STATE_TERMINATED
     */
    virtual IMS_SINT32 GetState() const = 0;

public:
    /// State value
    enum
    {
        STATE_CREATED = 0,
        STATE_INIT,
        /// When there is at least one contact bound to the AOR(address-of-record)
        STATE_ACTIVE,
        /// When the last contact expires or is removed
        STATE_TERMINATED
    };
};

#endif  // _INTERFACE_REG_INFO_REGISTRATION_H_
