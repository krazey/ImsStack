#ifndef _INTERFACE_REG_INFO_H_
#define _INTERFACE_REG_INFO_H_

#include "IRegInfoRegistration.h"

/**
 * @brief This class provides an interface to access the "reginfo" XML document.
 */
class IRegInfo
{
public:
    /**
     * @brief Returns the registration element information of reginfo.
     *
     * @param strAOR the public user identity to be retrieved
     * @return Pointer to IRegInfoRegistration or null
     */
    virtual IRegInfoRegistration* GetRegistration(IN CONST AString& strAOR) const = 0;

    /**
     * @brief Returns the registration element information of reginfo.
     *
     * @param strAOR the public user identity to be retrieved
     * @return Pointer to IRegInfoRegistration or null
     */
    virtual IRegInfoRegistration* GetRegistration(IN CONST SipAddress& objAOR) const = 0;

    /**
     * @brief Returns all the registration elements of reginfo.
     *
     * @return List of pointer to IRegInfoRegistration
     */
    virtual IMSList<IRegInfoRegistration*> GetRegistrations() const = 0;
};

#endif  // _INTERFACE_REG_INFO_H_
