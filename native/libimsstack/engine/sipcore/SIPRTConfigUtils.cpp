/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170622  hwangoo.park@             Created
    </table>

    Description

*/
#include "SIPFactoryProxy.h"
#include "SIPRTConfigUtils.h"

PUBLIC GLOBAL SIPRTConfigHelper* SIPRTConfigUtils::GetConfigHelper(IN IMS_SINT32 nSlotId)
{
    return SIPFactoryProxy::GetInstance()->GetRtConfigHelper(nSlotId);
}

PUBLIC GLOBAL IMS_BOOL SIPRTConfigUtils::IsMessageHiddenInLog(IN IMS_SINT32 nSlotId)
{
    SIPRTConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) && pConfigHelper->IsMessageHiddenInLog();
}

PUBLIC GLOBAL IMS_BOOL SIPRTConfigUtils::IsRoutingInfoHiddenInLog(IN IMS_SINT32 nSlotId)
{
    SIPRTConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) && pConfigHelper->IsRoutingInfoHiddenInLog();
}

PUBLIC GLOBAL IMS_BOOL SIPRTConfigUtils::IsFeatureSipTxPacketBlockedEnabled(IN IMS_SINT32 nSlotId)
{
    SIPRTConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsFeatureEnabled(SipRtConfig::FEATURE_SIP_TX_PACKET_BLOCKED);
}

PUBLIC GLOBAL IMS_BOOL SIPRTConfigUtils::IsIPSecSAConfigured(IN IMS_SINT32 nSlotId)
{
    SIPRTConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_IPSEC_SA);
}

PUBLIC GLOBAL IMS_BOOL SIPRTConfigUtils::IsRegContactAddressConfigured(IN IMS_SINT32 nSlotId)
{
    SIPRTConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS);
}

PUBLIC GLOBAL IMS_BOOL SIPRTConfigUtils::IsTcpPortRangeConfigured(IN IMS_SINT32 nSlotId)
{
    SIPRTConfigHelper* pConfigHelper = GetConfigHelper(nSlotId);

    return (pConfigHelper != IMS_NULL) &&
            pConfigHelper->IsItemConfigured(SipRtConfig::CONFIG_I_TCP_PORT_RANGE);
}
