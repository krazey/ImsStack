/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170622  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_RT_CONFIG_UTILS_H_
#define _SIP_RT_CONFIG_UTILS_H_

#include "SipRtConfigHelper.h"

class SIPRTConfigUtils
{
private:
    SIPRTConfigUtils();

public:
    static SIPRTConfigHelper* GetConfigHelper(IN IMS_SINT32 nSlotId);

    static IMS_BOOL IsMessageHiddenInLog(IN IMS_SINT32 nSlotId);
    static IMS_BOOL IsRoutingInfoHiddenInLog(IN IMS_SINT32 nSlotId);

    /** Features */
    static IMS_BOOL IsFeatureSipTxPacketBlockedEnabled(IN IMS_SINT32 nSlotId);

    /** Configurations */
    static IMS_BOOL IsIPSecSAConfigured(IN IMS_SINT32 nSlotId);
    static IMS_BOOL IsRegContactAddressConfigured(IN IMS_SINT32 nSlotId);
    static IMS_BOOL IsTcpPortRangeConfigured(IN IMS_SINT32 nSlotId);
};

#endif  // _SIP_RT_CONFIG_UTILS_H_
