#ifndef _INTERFACE_REG_USER_IDENTITY_NOTIFIER_H_
#define _INTERFACE_REG_USER_IDENTITY_NOTIFIER_H_

#include "AStringArray.h"

/**
 * @brief This class provides an interface to adjust public user identities
 *        which are provisioned from the IMS network.
 */
class IRegUserIdentityNotifier
{
public:
    /**
     * @brief Notifies the application when the network provisioned user identities are received
     *        (200 OK to REGISTER).
     *
     * Then, the application can reorder the default public user identities according to
     * the operator's policy.
     *
     * @param objUserIds Network provisioned user identities(from P-Associated-URI w/ same order)
     * @param objReorderedUserIds Reordered default public user identities
     * @return If default ordered user identities are changed, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL RegUserIdentity_ReorderUserIdentities(
            IN CONST AStringArray& objUserIds, OUT AStringArray& objReorderedUserIds) = 0;
};

#endif  // _INTERFACE_REG_USER_IDENTITY_NOTIFIER_H_
