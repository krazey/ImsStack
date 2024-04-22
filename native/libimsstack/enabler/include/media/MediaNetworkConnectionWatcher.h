/**
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

#ifndef MEDIA_NETWORK_CONNECTION_WATCHER_H_
#define MEDIA_NETWORK_CONNECTION_WATCHER_H_

#include "INetworkConnection.h"

class IMediaNetworkConnectionListener;

class MediaNetworkConnectionWatcher : public INetworkConnectionListener
{
public:
    enum
    {
        /**
         * Unknown access network
         */
        UNKNOWN = 0,
        /**
         * GSM EDGE Radio Access Network
         */
        GERAN,
        /**
         * Universal Terrestrial Radio Access Network
         */
        UTRAN,
        /**
         * Evolved Universal Terrestrial Radio Access Network
         */
        EUTRAN,
        /**
         * CDMA 2000 network
         */
        CDMA2000,
        /**
         * Interworking Wireless LAN
         */
        IWLAN,
        /**
         * Next-Generation Radio Access Network (NGRAN).
         * Note NGRAN is only for standalone mode. Non-standalone mode uses AccessNetwork EUTRAN.
         */
        NGRAN,
    };

    explicit MediaNetworkConnectionWatcher(IN const IpAddress& objIpAddress);
    virtual ~MediaNetworkConnectionWatcher();
    virtual void SetListener(IN IMediaNetworkConnectionListener* piListener);
    /* INetworkConnectionListener Interface Impl */
    virtual void NetworkConnection_OnConnected(IN INetworkConnection* pNetConnection) override;
    virtual void NetworkConnection_OnDisconnected(
            IN INetworkConnection* pNetConnection, IN IMS_SINT32 nErrorCode) override;
    virtual void NetworkConnection_OnConnectionFailed(
            IN INetworkConnection* pNetConnection, IN IMS_SINT32 nErrorCode) override;
    virtual void NetworkConnection_OnIpChanged(IN INetworkConnection* pNetConnection) override;
    virtual void NetworkConnection_OnIpcanChanged(IN INetworkConnection* pNetConnection) override;
    virtual void NetworkConnection_OnPcscfChanged(IN INetworkConnection* pNetConnection) override;

    /** Get the network connection type */
    virtual IMS_SINT32 GetNetworkType() const { return m_nMediaConnectionType; }

    /** Get the mtu size */
    virtual IMS_SINT32 GetMtu() const { return m_nMtu; }

private:
    static IMS_SINT32 ConvertNetworkType(IN INetworkConnection* pNetConnection);
    static const IMS_CHAR* PrintNetworkType(IN IMS_SINT32 nMediaConnectionType);
    void UpdateParameters(IN INetworkConnection* pNetConnection);

    IMediaNetworkConnectionListener* m_piListener;
    INetworkConnection* m_pNetConnection;
    IMS_SINT32 m_nMediaConnectionType;
    IMS_SINT32 m_nMtu;
};

#endif
