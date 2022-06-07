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
#ifndef SIP_KEEP_ALIVE_HELPER_H_
#define SIP_KEEP_ALIVE_HELPER_H_

#include "ImsSlot.h"
#include "ISipKeepAliveHelper.h"
#include "ISipKeepAliveListener.h"
#include "SipSocketAddress.h"

class SipKeepAliveHelper : public ImsSlot, public ISipKeepAliveHelper, public ISipKeepAliveListener
{
public:
    explicit SipKeepAliveHelper(IN IMS_SINT32 nSlotId);
    inline virtual ~SipKeepAliveHelper() {}

    SipKeepAliveHelper(IN const SipKeepAliveHelper&) = delete;
    SipKeepAliveHelper& operator=(IN const SipKeepAliveHelper&) = delete;

private:
    // ISipObject class
    void Destroy() override;

    // ISipKeepAliveHelper class
    IMS_RESULT SendPacket(IN const ByteArray& objPacket) override;
    void SetListener(IN ISipKeepAliveHelperListener* piListener) override;
    void SetTransportTupleD(IN const IPAddress& objIp, IN IMS_SINT32 nPort) override;
    void SetTransportTupleS(IN const IPAddress& objIp, IN IMS_SINT32 nPort,
            IN IMS_SINT32 nProtocol = Sip::TRANSPORT_UDP) override;

    // ISipKeepAliveListener class
    void KeepAlive_PongReceived() override;

private:
    SipSocketAddress m_objNearEnd;
    SipSocketAddress m_objFarEnd;
    ISipKeepAliveHelperListener* m_piListener;
};

#endif
