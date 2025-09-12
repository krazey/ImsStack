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
#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "IMessage.h"
#include "ISipMessage.h"
#include "MessageBodyPart.h"

class AppConfig;

class Message : public IMessage
{
public:
    Message(IN AppConfig* pAppConfig, IN IMS_SINT32 nState);
    ~Message() override;

    Message(IN const Message&) = delete;
    Message& operator=(IN const Message&) = delete;

public:
    inline ISipMessage* GetMessage() const override { return m_piSipMsg; }

    ISipMessageBodyPart* CreateBodyPartEx();
    void UpdateSentMessage(IN ISipMessage* piSipMsg);
    static Message* CreateMessage(IN Message* pMessage);
    static Message* CreateUnsentMessage(IN AppConfig* pAppConfig, IN IMS_BOOL bRequest);
    static Message* CreateReceivedMessage(IN AppConfig* pAppConfig, IN ISipMessage* piSipMsg);
    static const IMS_CHAR* GetMessageType(IN IMS_SINT32 nServiceMethod);

private:
    // IMessage interface
    IMessageBodyPart* CreateBodyPart() override;
    ImsList<IMessageBodyPart*> GetBodyParts() const override;
    IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strValue) override;
    ImsList<AString> GetHeaders(IN const AString& strName) const override;
    inline const SipMethod& GetMethod() const override { return m_piSipMsg->GetMethod(); }
    inline const AString& GetReasonPhrase() const override { return m_piSipMsg->GetReasonPhrase(); }
    inline IMS_SINT32 GetState() const override { return m_nState; }
    inline IMS_SINT32 GetStatusCode() const override { return m_piSipMsg->GetStatusCode(); }

    IMS_BOOL CreateBodyParts();
    static IMS_BOOL IsInaccessibleHeader(IN IMS_SINT32 nHType, IN const AString& strHName);
    static IMS_BOOL IsReadOnlyHeader(IN IMS_SINT32 nHType, IN const AString& strHName);

private:
    AppConfig* m_pAppConfig;
    IMS_SINT32 m_nState;
    ISipMessage* m_piSipMsg;
    ImsList<MessageBodyPart*> m_objBodyParts;
};

#endif
