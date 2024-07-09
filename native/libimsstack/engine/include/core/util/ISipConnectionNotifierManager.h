/*
 * Copyright (C) 2024 The Android Open Source Project
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
#ifndef INTERFACE_SIP_CONNECTION_NOTIFIER_MANAGER_H_
#define INTERFACE_SIP_CONNECTION_NOTIFIER_MANAGER_H_

#include "IpAddress.h"

#include "SipAddress.h"

class ISipConnectionNotifier;

class ISipConnectionNotifierManager
{
protected:
    virtual ~ISipConnectionNotifierManager() = default;

public:
    /**
     * @brief Creates an ISipConnectionNotifier object with the specified information.
     *
     * @param strScheme Protocol scheme such as "sip", "sips".
     * @param objIpAddr The IP address to bind to.
     * @param nPortS The server port to bind to.
     * @param nPortC The client port to bind to.
     * @param nPortFlowControl Client port for flow control.
     * @param strParams Additional parameters for this connection notifier such as "transport".
     * @param objUserId User identity to be used for this connection notifier.
     */
    virtual ISipConnectionNotifier* CreateConnectionNotifier(IN const AString& strScheme,
            IN const IpAddress& objIpAddr, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFlowControl, IN const AString& strParams,
            IN const SipAddress& objUserId) = 0;

    /**
     * @brief Returns an ISipConnectionNotifier object matching the specified IP address and
     *        port number.
     *
     * @param objIpAddr The IP address to find.
     * @param nPort The server port to find.
     */
    virtual ISipConnectionNotifier* GetConnectionNotifier(
            IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Releases the specified ISipConnectionNotifier object.
     *
     * @param piScn The ISipConnectionNotifier to release.
     */
    virtual void ReleaseConnectionNotifier(IN ISipConnectionNotifier*& piScn) = 0;

    /**
     * @brief Initializes this object for the specified slot.
     *
     * @param nSlotId Slot id.
     */
    virtual void Init(IN IMS_SINT32 nSlotId) = 0;
};

#endif
