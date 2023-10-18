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
#ifndef SIP_MESSAGE_HANDLER_H_
#define SIP_MESSAGE_HANDLER_H_

#include "ISipTransportMessageListener.h"

class SipTransactionState;

class SipMessageHandler : public ISipTransportMessageListener
{
private:
    SipMessageHandler();

public:
    ~SipMessageHandler();

    SipMessageHandler(IN const SipMessageHandler&) = delete;
    SipMessageHandler& operator=(IN const SipMessageHandler&) = delete;

public:
    static SipMessageHandler* GetInstance();

private:
    // ISipTransportMessageListener interface
    void TransportMessage_PacketReceived(IN IMS_SINT32 nSlotId, IN const ByteArray& objBuffer,
            IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd) override;

    void NotifyPacketReceived(IN IMS_SINT32 nSlotId, IN const ByteArray& objBuffer,
            IN ::SipMessage* pSipMsg, IN IMS_SINT32 nProcessingResult);
    IMS_SINT32 NotifyRequest(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg,
            IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd);
    IMS_SINT32 NotifyResponse(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg,
            IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd);

    IMS_BOOL CheckIpSecValidityForRequest(IN IMS_SINT32 nSlotId, IN SipTransactionState* pTState,
            IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd);
    IMS_BOOL CheckIpSecValidityForResponse(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg,
            IN const SipTransportAddress& objNearEnd, IN const SipTransportAddress& objFarEnd);
    IMS_BOOL IsIpSecSaMatched(IN IMS_SINT32 nSlotId, IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd);
    IMS_BOOL IsIpSecSaMatchedForUs(
            IN IMS_SINT32 nSlotId, IN const IpAddress& objIp, IN IMS_SINT32 nPort);
    IMS_BOOL IsSecuredMessage(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg);

    IMS_BOOL CheckRegContactValidity(IN IMS_SINT32 nSlotId, IN ::SipMessage* pSipMsg);
};

#endif
