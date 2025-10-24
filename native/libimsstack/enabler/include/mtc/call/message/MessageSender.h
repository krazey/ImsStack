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

#ifndef MESSAGE_SENDER_H_
#define MESSAGE_SENDER_H_

#include "call/message/IMessageSender.h"
#include "call/message/MessageFormatter.h"
#include "helper/TransactionTimerUpdateHelper.h"
#include <memory>

class ISession;
enum class CallType;
enum class UpdateType;
struct CallReasonInfo;

class MessageSender final : public IMessageSender
{
public:
    explicit MessageSender(IN IMtcCallContext& objContext, IN ISession& objSession);
    virtual ~MessageSender() override;
    MessageSender(IN const MessageSender&) = delete;
    MessageSender& operator=(IN const MessageSender&) = delete;

public:
    IMS_RESULT Start(IN CallType eCallType) override;
    IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
            IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo) override;
    IMS_RESULT SendPrack() override;
    IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) override;
    IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) override;
    IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) override;
    IMS_RESULT Accept() override;
    IMS_RESULT Reject(IN const CallReasonInfo& objReason) override;
    IMS_RESULT SendAck() override;
    IMS_RESULT Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
            IN IMS_SINT32 eMethod, IN IMS_BOOL bSessionRefresh) override;
    IMS_RESULT AcceptUpdate() override;
    IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) override;
    IMS_RESULT Terminate(IN IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) override;

private:
    void CreateFormatter();

private:
    IMtcCallContext& m_objContext;
    ISession& m_objSession;
    std::unique_ptr<MessageFormatter> m_pFormatter;
    TransactionTimerUpdateHelper m_objTimerUpdateHelper;
};

#endif
