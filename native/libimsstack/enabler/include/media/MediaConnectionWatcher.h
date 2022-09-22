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

#ifndef _IMS_MEDIA_CONNECTION_WATCHER_H_
#define _IMS_MEDIA_CONNECTION_WATCHER_H_

#include "ImsList.h"
#include "INetworkConnection.h"
#include "IMediaConnectionWatcher.h"
#include "IEventListener.h"

class IMediaConnectionWatcherListener;

class MediaConnectionWatcher : public IMediaConnectionWatcher, public IEventListener
{
private:
    class NetConnectionWatcher  // Internal-Class
            : public INetworkConnectionListener
    {
    public:
        NetConnectionWatcher();
        virtual ~NetConnectionWatcher();
        IMS_BOOL AddListener(IN IMediaConnectionWatcherListener* piListener);
        IMS_BOOL RemoveListener(IN IMediaConnectionWatcherListener* piListener);
        IMS_SINT32 GetMediaConnectionType();
        IMS_UINT32 GetNetworkInterfaceID();
        void SetMediaConnectionType(IN IMS_SINT32 nMediaConnectionType);
        IMS_BOOL SetINetConnection(IN INetworkConnection* piNetConnection);
        IMS_UINT32 GetListenerLength();
        IMS_SINT32 GetMtuSize();

    public:
        /* INetworkConnectionListener Interface Impl */
        virtual void NetworkConnection_OnConnected(IN INetworkConnection* piNetConnection);
        virtual void NetworkConnection_OnDisconnected(
                IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode);
        virtual void NetworkConnection_OnConnectionFailed(
                IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode);
        virtual void NetworkConnection_OnIpChanged(IN INetworkConnection* piNetConnection);
        virtual void NetworkConnection_OnIpcanChanged(IN INetworkConnection* piNetConnection);
        virtual void NetworkConnection_OnPcscfChanged(IN INetworkConnection* piNetConnection);

    private:
        IMS_BOOL hasListener(IN IMediaConnectionWatcherListener* piListener);
        IMS_BOOL hasListener(
                IN IMediaConnectionWatcherListener* piListener, OUT IMS_UINT32& nIndex);
        void setMtuSize(IN IMS_SINT32 nMtuSize);

    public:
        IMSList<IMediaConnectionWatcherListener*> m_objListeners;
        INetworkConnection* m_piNetConnection;

    private:
        IMS_SINT32 m_nMediaConnectionType;
        IMS_SINT32 m_nMtuSize;
    };

private:
    MediaConnectionWatcher();
    virtual ~MediaConnectionWatcher();
    MediaConnectionWatcher::NetConnectionWatcher* findNetConnectionWatcher(
            IN INetworkConnection* piNetConnection);
    void clearMediaConnectionWatcher();

public:
    static IMS_SINT32 convertNetworkType(IN INetworkConnection* piNetConnection);
    static const IMS_CHAR* printNetworkType(IN IMS_SINT32 nMediaConnectionType);
    static IMS_SINT32 CalculateRtpFragmentSize(IN INetworkConnection *piNetConnection/*,
            IN IMS_SINT32 nSlotId = 0*/);
    static IMS_SINT32 CalculateRtpFragmentSize(IN NetConnectionWatcher *pNetConnectionWatcher/*,
            IN IMS_SINT32 nSlotId = 0*/);

    /* IMediaConnectionWatcher Interface Impl*/
    static MediaConnectionWatcher* GetMediaConnectionWatcher(IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    virtual IMS_BOOL GetMediaConnectionType(IN AString& strPDN, IN IMS_SINT32 nSlotID,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId);
    virtual IMS_BOOL GetMediaConnectionType(IN IPAddress& objIpAddress,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId);
    virtual IMS_BOOL GetMediaConnectionType(IN INetworkConnection* piNetConnection,
            OUT IMS_SINT32& nMediaConnectionType, OUT IMS_UINT32& nNetworkInterfaceId);
    virtual IMS_BOOL SetListener(IN IMediaConnectionWatcherListener* piListener, IN AString& strPDN,
            IN IMS_SINT32 nSlotID, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId);
    virtual IMS_BOOL SetListener(IN IMediaConnectionWatcherListener* piListener,
            IN IPAddress& objIpAddress, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId);
    virtual IMS_BOOL ReleaseListener(IMediaConnectionWatcherListener* piListener);

    virtual IMS_BOOL GetRtpFragmentSize(IN AString& strPDN, IN IMS_SINT32 nSlotID,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nRtpFragmentSize);
    virtual IMS_BOOL GetRtpFragmentSize(IN IPAddress& objIpAddress,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nRtpFragmentSize);
    virtual IMS_BOOL GetRtpFragmentSize(
            IN INetworkConnection* piNetConnection, OUT IMS_SINT32& nRtpFragmentSize);

    /* IEventListener Interfcace Impl */
    virtual void Event_NotifyEvent(
            IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam);

private:
    IMSList<NetConnectionWatcher*> objWatchers;
    // IMS_BOOL   m_bVoWIFISupport;
};
#endif /* _IMS_MEDIA_CONNECTION_WATCHER_H_ */
