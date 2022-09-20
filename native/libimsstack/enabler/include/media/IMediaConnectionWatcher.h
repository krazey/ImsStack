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

#ifndef _INTERFACE_IMS_MEDIA_CONNECTION_WATCHER_H_
#define _INTERFACE_IMS_MEDIA_CONNECTION_WATCHER_H_

#include "IpAddress.h"

class INetworkConnection;
class IMediaConnectionWatcherListener;

class IMediaConnectionWatcher
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

public:
    virtual IMS_BOOL GetMediaConnectionType(IN AString& strPDN, IN IMS_SINT32 nSlotId,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId) = 0;
    virtual IMS_BOOL GetMediaConnectionType(IN IPAddress& objIpAddress,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId) = 0;
    virtual IMS_BOOL GetMediaConnectionType(IN INetworkConnection* piNetConnection,
            OUT IMS_SINT32& nMediaConnectionType, OUT IMS_UINT32& nNetworkInterfaceId) = 0;
    virtual IMS_BOOL SetListener(IN IMediaConnectionWatcherListener* piListener, IN AString& strPDN,
            IN IMS_SINT32 nSlotId, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId) = 0;
    virtual IMS_BOOL SetListener(IN IMediaConnectionWatcherListener* piListener,
            IN IPAddress& objIpAddress, OUT IMS_SINT32& nMediaConnectionType,
            OUT IMS_UINT32& nNetworkInterfaceId) = 0;
    virtual IMS_BOOL GetRtpFragmentSize(IN AString& strPDN, IN IMS_SINT32 nSlotId,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nRtpFragmentSize) = 0;
    virtual IMS_BOOL GetRtpFragmentSize(IN IPAddress& objIpAddress,
            OUT INetworkConnection*& piNetConnection, OUT IMS_SINT32& nRtpFragmentSize) = 0;
    virtual IMS_BOOL GetRtpFragmentSize(
            IN INetworkConnection* piNetConnection, OUT IMS_SINT32& nRtpFragmentSize) = 0;
    virtual IMS_BOOL ReleaseListener(IMediaConnectionWatcherListener* piListener) = 0;
};
#endif /* _INTERFACE_IMS_MEDIA_CONNECTION_WATCHER_H_ */
