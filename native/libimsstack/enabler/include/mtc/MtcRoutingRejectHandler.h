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

#ifndef MTC_ROUTING_REJECT_HANDLER_H_
#define MTC_ROUTING_REJECT_HANDLER_H_

#include "ISipRoutingRejectListener.h"
#include "ImsTypeDef.h"
#include "SipStatusCode.h"

class IMtcContext;
class INetworkWatcher;
class ISipMessage;

/*
 * This class receives notification from the IMS engine when a routing reject occurs.
 * And determines the status code for rejecting.
 */
class MtcRoutingRejectHandler final : public ISipRoutingRejectListener
{
public:
    explicit MtcRoutingRejectHandler(
            IN IMtcContext& objContext, IN INetworkWatcher& objNetworkWatcher);
    ~MtcRoutingRejectHandler();
    MtcRoutingRejectHandler(const MtcRoutingRejectHandler&) = delete;
    MtcRoutingRejectHandler& operator=(const MtcRoutingRejectHandler&) = delete;

    IMS_BOOL RoutingReject_NotifyRequest(
            IN ISipMessage* pSipMessage, IN_OUT SipStatusCode& objStatusCode) override;
    IMS_BOOL RoutingReject_NotifyRequest(IN ISipServerConnection* pSipServerConnection,
            IN_OUT SipStatusCode& objStatusCode) override;

private:
    SipStatusCode GetRoutingRejectCodeForInvite(IN const SipStatusCode& objDefaultStatusCode) const;

    IMtcContext& m_objContext;
    INetworkWatcher& m_objNetworkWatcher;
};

#endif
