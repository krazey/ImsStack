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
#ifndef OS_NETWORK_WATCHER_H_
#define OS_NETWORK_WATCHER_H_

#include "INetworkWatcher.h"
#include "ISystemListener.h"
#include "ImsSlot.h"

class OsNetworkWatcher : public ImsSlot, public INetworkWatcher, public ISystemListener
{
public:
    explicit OsNetworkWatcher(IN IMS_SINT32 nSlotId);
    virtual ~OsNetworkWatcher();

    OsNetworkWatcher(IN const OsNetworkWatcher&) = delete;
    OsNetworkWatcher& operator=(IN const OsNetworkWatcher&) = delete;

public:
    // INetworkWatcher
    IMS_UINT32 GetNetworkStatus(IN const AString& strProfile) override;
    NETRADIO_ENTYPE GetNetRadioTechType(IN const AString& strProfile,
            IN IMS_SINT32 nApnType = NetworkPolicy::APN_NONE) override;
    NETRADIO_ENTYPE GetNetRadioTechType() override;
    NETRADIO_ENTYPE GetNetVoiceRadioTechType() override;
    NETSERVICE_ENTYPE GetNetServiceType(IN const AString& strProfile,
            IN IMS_SINT32 nApnType = NetworkPolicy::APN_NONE) override;
    NETSERVICE_ENTYPE GetNetServiceType() override;
    NETSERVICE_ENTYPE GetNetVoiceServiceType() override;
    NETDOMAIN_ENTYPE GetNetDomainType() override;

    /*
     * @see #NETWORK_TYPE_UNKNOWN = 0
     * @see #NETWORK_TYPE_GPRS
     * @see #NETWORK_TYPE_EDGE
     * @see #NETWORK_TYPE_UMTS
     */
    IMS_SINT32 GetNetworkType() override;
    IMS_SINT32 GetCellularServiceState() override;
    IMS_SINT32 GetRoamingState() override;
    IMS_SINT32 GetVoiceRoamingType() override;
    IMS_SINT32 GetDataRoamingType() override;
    AString GetNetworkOperator() const override;
    IMS_BOOL IsImsEmergencyCallSupported() override;
    IMS_BOOL IsImsVoiceCallSupported() override;
    IMS_BOOL IsEmergencyOnly() override;
    IMS_BOOL IsEmergencyAttachSupported() override;
    // 2nd PLMN info for MOCN
    IMS_SINT32 GetMocnPlmnInfo() override;
    IMS_SINT32 GetNetworkRegistrationRejectCause() override;

    // ISystemListener
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

    void UpdateServiceState(IN IMS_SINT32 nState);
    void UpdateRadioTechChanged(IN IMS_SINT32 nState);
    void UpdateVoiceRadioTechChanged(IN IMS_SINT32 nState);

    void UpdateAirplaneMode(IN IMS_SINT32 nState);
    void UpdateDataConnectionStateChanged(IN IMS_SINT32 nState);

    /*
     * @see #STATE_IN_SERVICE
     * @see #STATE_OUT_OF_SERVICE
     * @see #STATE_EMERGENCY_ONLY
     * @see #STATE_POWER_OFF
     */
    IMS_SINT32 GetServiceStateType() const;
    IMS_SINT32 GetVoiceServiceStateType() const;

private:
    static const IMS_CHAR* RadioTechToString(IN IMS_UINT32 nType);

private:
    NETRADIO_ENTYPE m_eNetStatusType;
    NETSERVICE_ENTYPE m_eNetServiceType;
    NETDOMAIN_ENTYPE m_eNetDomainType;
};

#endif
