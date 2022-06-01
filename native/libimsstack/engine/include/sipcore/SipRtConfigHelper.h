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

class SIPRTConfigHelper : public ISipRtConfigHelper
{
public:
    SIPRTConfigHelper();
    virtual ~SIPRTConfigHelper();

public:
    virtual const SipRtConfig::Header* GetHeader(IN CONST AString& strName) const;

    const SipRtConfig::LogMask* GetLogMask() const;
    const SipRtConfig::SocketOption* GetSocketOption(IN IMS_SINT32 nItem) const;
    const SipRtConfig::SocketOption* GetSocketOption(
            IN IMS_SINT32 nItem, IN CONST IPAddress& objIP, IN IMS_SINT32 nPort = 0) const;
    const SipRtConfig::IpQos* GetIpQos(IN CONST IPAddress& objIP, IN IMS_SINT32 nPort = 0) const;
    const IMSList<SipRtConfig::IpSecSa>& GetIpSecSas() const;
    const SipAddress* GetRegContactUri(IN CONST AString& strCallId) const;
    IMS_BOOL IsFeatureEnabled(IN IMS_SINT32 nFeature) const;
    IMS_BOOL IsItemConfigured(IN IMS_SINT32 nItem) const;
    IMS_BOOL IsMessageHiddenInLog() const;
    IMS_BOOL IsRoutingInfoHiddenInLog() const;

private:
    // ISipRtConfigHelper class
    virtual void DisableFeature(IN IMS_SINT32 nFeature);
    virtual void EnableFeature(IN IMS_SINT32 nFeature);
    virtual IMS_SINT32 GetFeatures() const;
    virtual void RemoveConfig(IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam);
    virtual IMS_RESULT SetConfig(IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam);

    IMS_UINT32 GetSocketOptionCount(IN IMS_SINT32 nItem) const;
    void RemoveSocketOption(IN IMS_SINT32 nItem, IN CONST SipRtConfig::SocketOption* pSO);
    IMS_RESULT SetSocketOption(IN IMS_SINT32 nItem, IN CONST SipRtConfig::SocketOption* pSO);

private:
    IMSMap<IMS_SINT32, IMS_BOOL> objConfigSet;

    // Features
    IMS_SINT32 nFeatures;

    // Log mask
    SipRtConfig::LogMask objLogMask;

    // Socket options: <Item, List of SO>
    IMSMap<IMS_SINT32, IMSList<SipRtConfig::SocketOption>> objSOMap;

    // IP-level QoS
    IMSList<SipRtConfig::IpQos> objIPQoSs;

    // SIP header control
    IMSList<SipRtConfig::Header> objHeaders;

    // IPSec SA
    IMSList<SipRtConfig::IpSecSa> objIPSecSAs;

    // RegContactAddress
    IMSList<SipRtConfig::RegContactAddress> objRegContacts;
};

#endif  // _SIP_RT_CONFIG_HELPER_H_
