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
#ifndef AOS_CONNECTION_H_
#define AOS_CONNECTION_H_

#include "AStringArray.h"
#include "IpAddress.h"

#include "interface/IAosConnection.h"
#include "INetworkConnection.h"

class IAosAppContext;

class AosConnection : public IAosConnection, public INetworkConnectionListener
{
public:
    explicit AosConnection(IN IAosAppContext* piAppContext);
    virtual ~AosConnection();

    IMS_BOOL Activate() override;
    void Deactivate() override;
    IMS_BOOL IsActivationRequested() override;
    IMS_UINT32 GetState() override;

    IMS_SINT32 GetConnectionType() override;

    void SetListener(IN IAosConnectionListener* piListener) override;
    void RemoveListener(IN IAosConnectionListener* piListener) override;

    IMS_SINT32 GetMtu() override;
    const IpAddress& GetLocalAddress(IN IMS_SINT32 nIpVersion = 0) override;
    const AStringArray& GetPcscfAddress(IN IMS_SINT32 nIpVersion = 0) override;
    IMS_SINT32 GetHostByName(IN const AString& strHostName, OUT ImsList<IpAddress>& objIps,
            IN IMS_SINT32 nIpVersion = 0) override;
    const AString& GetIfaceName() override;
    IMS_BOOL IsEpdgEnabled() override;
    IMS_SINT32 GetIpcanCategory() override;
    IMS_BOOL IsLimitedServicePcoValue() override;
    IMS_SINT32 GetCarrierSignalPcoValue() override;
    void SetCarrierSignalPcoValue(IN IMS_SINT32 nValue) override;

    // Log
    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);

protected:
    void Notify(IN IMS_UINT32 nType = TYPE_STATE_CHANGED);

    IMS_BOOL IsConnected() const;
    IMS_BOOL IsOnDemandControl() const;

    void SetActivationRequested(IN IMS_BOOL bRequest);
    void SetState(IN IMS_UINT32 nState);
    void UpdateIpcanForTrm();

    // INetworkConnectionListener
    void NetworkConnection_OnConnected(IN INetworkConnection* piNetConnection) override;
    void NetworkConnection_OnDisconnected(
            IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode) override;
    void NetworkConnection_OnConnectionFailed(
            IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode) override;
    void NetworkConnection_OnIpChanged(IN INetworkConnection* piNetConnection) override;
    void NetworkConnection_OnIpcanChanged(IN INetworkConnection* piNetConnection) override;
    void NetworkConnection_OnPcscfChanged(IN INetworkConnection* piNetConnection) override;

    enum
    {
        TYPE_STATE_CHANGED = 0,
        TYPE_IP_CHANGED,
        TYPE_IPCAN_CHANGED,
        TYPE_PCSCF_CHANGED,
        TYPE_CONNECTION_FAILED
    };

protected:
    IAosAppContext* m_piContext;

    IMS_SINT32 m_nSlotId;

    // NetworkPolicy::APN_IMS
    IMS_SINT32 m_nCnxType;

    INetworkConnection* m_piConnection;

    IMS_UINT32 m_nState;

    IMS_BOOL m_bActivationRequested;

    ImsList<IAosConnectionListener*> m_objListeners;

    AString m_strTag;

    IMS_SINT32 m_nPcoValue;
    static const IMS_SINT32 PCO_INVALID_VALUE = -1;
    static const IMS_SINT32 PCO_LIMITED_SERVICE_VALUE = 5;

protected:
    friend class AosConnectionTest;
};

#endif  // AOS_CONNECTION_H_
