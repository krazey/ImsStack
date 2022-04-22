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

#ifndef _INTERFACE_IMS_MEDIA_CONNECTION_WATCHER_LISTENER_H_
#define _INTERFACE_IMS_MEDIA_CONNECTION_WATCHER_LISTENER_H_

#include "AString.h"

class INetworkConnection;

class IMediaConnectionWatcherListener
{
public:
    virtual void NotifyMediaConnection(IN INetworkConnection *piNetConnection,
            IN IMS_SINT32 nMediaConnectionType, IN IMS_UINT32 nNetworkInterfaceId) = 0;
    virtual void NotifyIPChanged(IMS_BOOL bIsIPv6) = 0;
    virtual void NotifyWifiEarlyRouteSetup(IN IMS_UINT32 nNetworkInferfaceID) = 0;
};
#endif  /* _INTERFACE_IMS_MEDIA_CONNECTION_WATCHER_LISTENER_H_ */
