/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_RT_CONFIG_HELPER_H_
#define _SIP_RT_CONFIG_HELPER_H_

#include "IMSMap.h"
#include "ISipRtConfigHelper.h"



class SIPRTConfigHelper
    : public ISIPRTConfigHelper
{
public:
    SIPRTConfigHelper();
    virtual ~SIPRTConfigHelper();

public:
    virtual const SIPRTConfig::Header* GetHeader(IN CONST AString &strName) const;

    const SIPRTConfig::LogMask* GetLogMask() const;
    const SIPRTConfig::SocketOption* GetSocketOption(IN IMS_SINT32 nItem) const;
    const SIPRTConfig::SocketOption* GetSocketOption(IN IMS_SINT32 nItem,
            IN CONST IPAddress &objIP, IN IMS_SINT32 nPort = 0) const;
    const SIPRTConfig::IPQoS* GetIPQoS(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort = 0) const;
    const IMSList<SIPRTConfig::IPSecSA>& GetIPSecSAs() const;
    const SIPAddress* GetRegContactUri(IN CONST AString &strCallId) const;
    IMS_BOOL IsFeatureEnabled(IN IMS_SINT32 nFeature) const;
    IMS_BOOL IsItemConfigured(IN IMS_SINT32 nItem) const;
    IMS_BOOL IsMessageHiddenInLog() const;
    IMS_BOOL IsRoutingInfoHiddenInLog() const;

private:
    // ISIPRTConfigHelper class
    virtual void DisableFeature(IN IMS_SINT32 nFeature);
    virtual void EnableFeature(IN IMS_SINT32 nFeature);
    virtual IMS_SINT32 GetFeatures() const;
    virtual void RemoveConfig(IN IMS_SINT32 nItem, IN SIPRTConfig::Base *pParam);
    virtual IMS_RESULT SetConfig(IN IMS_SINT32 nItem, IN SIPRTConfig::Base *pParam);

    IMS_UINT32 GetSocketOptionCount(IN IMS_SINT32 nItem) const;
    void RemoveSocketOption(IN IMS_SINT32 nItem,
            IN CONST SIPRTConfig::SocketOption *pSO);
    IMS_RESULT SetSocketOption(IN IMS_SINT32 nItem,
            IN CONST SIPRTConfig::SocketOption *pSO);

private:
    IMSMap<IMS_SINT32, IMS_BOOL> objConfigSet;

    // Features
    IMS_SINT32 nFeatures;

    // Log mask
    SIPRTConfig::LogMask objLogMask;

    // Socket options: <Item, List of SO>
    IMSMap<IMS_SINT32, IMSList<SIPRTConfig::SocketOption> > objSOMap;

    // IP-level QoS
    IMSList<SIPRTConfig::IPQoS> objIPQoSs;

    // SIP header control
    IMSList<SIPRTConfig::Header> objHeaders;

    // IPSec SA
    IMSList<SIPRTConfig::IPSecSA> objIPSecSAs;

    // RegContactAddress
    IMSList<SIPRTConfig::RegContactAddress> objRegContacts;
};

#endif // _SIP_RT_CONFIG_HELPER_H_
