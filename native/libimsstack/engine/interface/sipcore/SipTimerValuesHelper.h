#ifndef _SIP_TIMER_VALUES_HELPER_H_
#define _SIP_TIMER_VALUES_HELPER_H_

#include "AString.h"
#include "SipTimerValues.h"

class SIPProfile;

/**
 * @brief This class defines the helper class to set the SIP transaction timer values.
 */
class SIPTimerValuesHelper
{
public:
    /// Types of SIP transaction
    enum
    {
        /// non-INVITE client transaction
        NON_INVITE_CLIENT = 0,
        /// non-INVITE server transaction
        NON_INVITE_SERVER = 1,
        /// INVITE client transaction
        INVITE_CLIENT = 2,
        /// INVITE server transaction
        INVITE_SERVER = 3
    };

private:
    SIPTimerValuesHelper();

public:
    /**
     * @brief Creates SIPTimerValues from the given information.
     *
     * @param nSlotId Current slot id
     * @param pSIPProfile SIP profile for this SIP transaction timer
     * @param nTxnType SIP transaction type\n
     *                 #NON_INVITE_CLIENT\n
     *                 #NON_INVITE_SERVER\n
     *                 #INVITE_CLIENT\n
     *                 #INVITE_SERVER
     * @return The newly created SIPTimerValues.
     */
    static SIPTimerValues GetValues(IN IMS_SINT32 nSlotId,
            IN CONST SIPProfile *pSIPProfile = IMS_NULL,
            IN IMS_SINT32 nTxnType = NON_INVITE_CLIENT);
};

#endif // _SIP_TIMER_VALUES_HELPER_H_
