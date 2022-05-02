#ifndef _INTERFACE_SIP_IPSEC_STATE_H_
#define _INTERFACE_SIP_IPSEC_STATE_H_

#include "IPAddress.h"

class ISIPIPSecStateListener;

/**
 * @brief This class provides an interface to handle IPSec SA(Security Association) state.
 */
class ISIPIPSecState
{
public:
    /**
     * @brief Clears the existing IPSec security association.
     *
     * After de-registration, the application SHALL invoke this method and clear all the SAs.
     *
     * @param nSAType SA type to be cleared\n
     *                #SA_NEW\n
     *                #SA_OLD
     */
    virtual void ClearIPSecSA(IN IMS_SINT32 nSAType) = 0;

    /**
     * @brief Returns the current state of the specified IPSec security association.
     *
     * @param nSAType SA type to be returned\n
     *                #SA_NEW\n
     *                #SA_OLD
     * @return A current state of the specified IPSec type.\n
     *         #STATE_INACTIVE\n
     *         #STATE_CREATED\n
     *         #STATE_PENDING\n
     *         #STATE_ACTIVE\n
     *         #STATE_TERMINATED\n
     *         #STATE_TERMINATED_PENDING
     */
    virtual IMS_SINT32 GetState(IN IMS_SINT32 nSAType) const = 0;

    /**
     * @brief Checks if the pending transaction exists or not.
     *
     * @param nSAType SA type to be checked\n
     *                #SA_NEW\n
     *                #SA_OLD
     * @return If the pending transaction exists, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL HasPendingTransaction(IN IMS_SINT32 nSAType) const = 0;

    /**
     * @brief Sets the IPSec SA table for the specified security association.
     *
     * It internally makes the SA table for the composition of each direction/transport.\n
     * When setting the new SA, it will internally copy the existing new SA to the old SA.
     *
     * @param nSAType SA type to be set\n
     *                #SA_NEW\n
     *                #SA_OLD
     * @param objIP_U UE's IP address
     * @param nPort_UC UE's client port
     * @param nPort_US UE's server port
     * @param objIP_P P-CSCF's IP address
     * @param nPort_PC P-CSCF's client port
     * @param nPort_PS P-CSCF's server port
     */
    virtual void SetIPSecSA(IN IMS_SINT32 nSAType,
            IN CONST IPAddress &objIP_U, IN IMS_SINT32 nPort_UC, IN IMS_SINT32 nPort_US,
            IN CONST IPAddress &objIP_P, IN IMS_SINT32 nPort_PC, IN IMS_SINT32 nPort_PS) = 0;

    /**
     * @brief Sets the listener to monitor the state of the IPSec security association.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISIPIPSecStateListener *piListener) = 0;

public:
    /// Type of IPSec security association
    enum
    {
        /// New IPSec security association
        SA_NEW = 1,
        /// Old IPSec security association
        SA_OLD = 2
    };

    /// State of IPSec security association
    enum
    {
        /// New/Old SA (default state)
        STATE_INACTIVE = 0,
        /// New SA
        /// - When SA is newly added
        /// - After the initial transaction is failed by transaction timeout
        STATE_CREATED = 1,
        /// New SA
        /// - When the initial transaction is ongoing
        STATE_PENDING = 2,
        /// New/Old SA
        /// - When one transaction is detected on new SA
        /// - When outgoing response is sent on new SA in CREATED state
        /// - When old SA has a pending transaction
        STATE_ACTIVE = 3,
        /// Old SA only
        /// - When old SA has no pending transaction and new SA is transited to active state
        STATE_TERMINATED = 4,
        /// Old SA only
        /// - When old SA is in TERMINATED state and SA lifetime is not expired
        ///  and old SA detects a pending transaction
        STATE_TERMINATED_PENDING = 5
    };
};

#endif // _INTERFACE_SIP_IPSEC_STATE_H_
