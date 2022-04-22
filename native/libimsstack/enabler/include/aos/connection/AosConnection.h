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
#include "IPAddress.h"

#include "interface/IAosConnection.h"
#include "INetworkConnection.h"

class IAosAppContext;

class AosConnection
    : public IAosConnection
    , public INetConnectionListener
{
public:
    AosConnection(IN IAosAppContext* piAppContext);
    virtual ~AosConnection();

    virtual IMS_BOOL Activate();
    virtual void Deactivate();
    virtual IMS_UINT32 GetState();

    virtual IMS_SINT32 GetConnectionType();
    virtual IMS_SINT32 GetPreferredIpVersion();

    virtual void SetListener(IN IAosConnectionListener* piListener);
    virtual void RemoveListener(IN IAosConnectionListener* piListener);

    virtual IMS_SINT32 GetMtu();
    virtual const IPAddress& GetLocalAddress(IN IMS_SINT32 nIpVersion = 0);
    virtual const AStringArray& GetPcscfAddress(IN IMS_SINT32 nIpVersion = 0);
    virtual IMS_SINT32 GetHostByName(IN const AString& strHostName,
            OUT IMSList<IPAddress>& objIps, IN IMS_SINT32 nIpVersion = 0);
    virtual const AString& GetIfaceName();
    virtual IMS_BOOL IsEpdgEnabled();
    virtual IMS_SINT32 GetIpcanCategory();
    virtual IMS_BOOL SendPingToHostAddress(IN const IPAddress& objHostAddress);

    // Log
    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);

protected:
    void Notify(IN IMS_UINT32 nType = TYPE_STATE_CHANGED);

    IMS_BOOL IsActivationRequested() const;
    IMS_BOOL IsConnected() const;
    IMS_BOOL IsOnDemandControl() const;

    void SetActivationRequested(IN IMS_BOOL bRequest);
    void SetState(IN IMS_UINT32 nState);
    void UpdateIpcanForTrm();

    // INetConnectionListener
    virtual void Connection_Connected(IN INetConnection* piNetConnection);
    virtual void Connection_Disconnected(IN INetConnection* piNetConnection,
            IN IMS_SINT32 nErrorCode);
    virtual void Connection_ConnectionFailed(IN INetConnection* piNetConnection,
            IN IMS_SINT32 nErrorCode);
    virtual void Connection_IpChanged(IN INetConnection* piNetConnection);
    virtual void Connection_IpcanCatChanged(IN INetConnection* piNetConnection);
    virtual void Connection_PcscfChanged(IN INetConnection* piNetConnection);

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

    IMS_SINT32 m_nCnxIpPriority;

    INetConnection* m_piConnection;

    IMS_UINT32 m_nState;

    IMS_BOOL m_bActivationRequested;
    IMS_BOOL m_bOnDemandControl;

    IMSList<IAosConnectionListener*> m_objListeners;

    AString m_strTag;
};

#endif // AOS_CONNECTION_H_
