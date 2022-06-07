/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef SIP_RT_CONFIG_HELPER_H_
#define SIP_RT_CONFIG_HELPER_H_

#include "IMSMap.h"

#include "ISipRtConfigHelper.h"

class SipRtConfigHelper : public ISipRtConfigHelper
{
public:
    SipRtConfigHelper();
    virtual ~SipRtConfigHelper();

    SipRtConfigHelper(IN const SipRtConfigHelper&) = delete;
    SipRtConfigHelper& operator=(IN const SipRtConfigHelper&) = delete;

public:
    const SipRtConfig::Header* GetHeader(IN const AString& strName) const override;

    inline const SipRtConfig::LogMask* GetLogMask() const { return &m_objLogMask; }
    const SipRtConfig::SocketOption* GetSocketOption(IN IMS_SINT32 nItem) const;
    const SipRtConfig::SocketOption* GetSocketOption(
            IN IMS_SINT32 nItem, IN const IPAddress& objIp, IN IMS_SINT32 nPort = 0) const;
    const SipRtConfig::IpQos* GetIpQos(IN const IPAddress& objIp, IN IMS_SINT32 nPort = 0) const;
    inline const IMSList<SipRtConfig::IpSecSa>& GetIpSecSas() const { return m_objIpSecSas; }
    const SipAddress* GetRegContactUri(IN const AString& strCallId) const;
    inline IMS_BOOL IsFeatureEnabled(IN IMS_SINT32 nFeature) const
    {
        return (m_nFeatures & nFeature) != 0;
    }
    IMS_BOOL IsItemConfigured(IN IMS_SINT32 nItem) const;
    inline IMS_BOOL IsMessageHiddenInLog() const
    {
        return ((m_objLogMask.nValue & SipRtConfig::LogMask::I_MESSAGE_HIDDEN) != 0);
    }
    inline IMS_BOOL IsRoutingInfoHiddenInLog() const
    {
        return ((m_objLogMask.nValue & SipRtConfig::LogMask::I_ROUTING_INFO_HIDDEN) != 0);
    }

private:
    // ISipRtConfigHelper class
    inline void DisableFeature(IN IMS_SINT32 nFeature) override { m_nFeatures &= (~nFeature); }
    inline void EnableFeature(IN IMS_SINT32 nFeature) override { m_nFeatures |= nFeature; }
    inline IMS_SINT32 GetFeatures() const override { return m_nFeatures; }
    void RemoveConfig(IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam) override;
    IMS_RESULT SetConfig(IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam) override;

    IMS_UINT32 GetSocketOptionCount(IN IMS_SINT32 nItem) const;
    void RemoveSocketOption(IN IMS_SINT32 nItem, IN const SipRtConfig::SocketOption* pSockOpt);
    IMS_RESULT SetSocketOption(IN IMS_SINT32 nItem, IN const SipRtConfig::SocketOption* pSockOpt);

private:
    IMSMap<IMS_SINT32, IMS_BOOL> m_objConfigSet;

    // Features
    IMS_SINT32 m_nFeatures;

    // Log mask
    SipRtConfig::LogMask m_objLogMask;

    // Socket options: <Item, List of SocketOption>
    IMSMap<IMS_SINT32, IMSList<SipRtConfig::SocketOption>> m_objSocketOptionMap;

    // IP-level QoS
    IMSList<SipRtConfig::IpQos> m_objIpQoss;

    // SIP header control
    IMSList<SipRtConfig::Header> m_objHeaders;

    // IPSec SA
    IMSList<SipRtConfig::IpSecSa> m_objIpSecSas;

    // RegContactAddress
    IMSList<SipRtConfig::RegContactAddress> m_objRegContacts;
};

#endif
