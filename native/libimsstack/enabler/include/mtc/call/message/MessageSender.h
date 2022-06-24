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

#ifndef MTC_MESSAGE_SENDER_H_
#define MTC_MESSAGE_SENDER_H_

#include "call/message/MessageFormatter.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include "SipMethod.h"
#include "MtcDef.h"
#include <memory>

class IMtcSessionContext;
class ISession;
struct CallReasonInfo;

class MessageSender
{
public:
    MessageSender(IN IMtcSessionContext& objContext);
    ~MessageSender();
    MessageSender(IN const MessageSender&) = delete;
    MessageSender& operator=(IN const MessageSender&) = delete;

public:
    IMS_RESULT Start();
    IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
            IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo);
    IMS_RESULT SendPrack();
    IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode);
    IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType);
    IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode);
    IMS_RESULT Accept();
    IMS_RESULT Reject(IN const CallReasonInfo& objReason);
    IMS_RESULT SendAck();
    IMS_RESULT Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
            IN IMS_SINT32 eMethod = SipMethod::INVITE, IN IMS_BOOL bSessionRefresh = IMS_FALSE);
    IMS_RESULT AcceptUpdate();
    IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason);
    IMS_RESULT Terminate(IN IMS_BOOL bUseBye, IN const CallReasonInfo& objReason);

private:
    void CreateFormatter();

private:
    IMtcSessionContext& m_objContext;
    ISession& m_objSession;
    std::unique_ptr<MessageFormatter> m_pFormatter;
    TransactionTimerUpdateHelper m_objTimerUpdateHelper;
};

#endif
