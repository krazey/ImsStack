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

#ifndef PARTICIPANT_INFO_H_
#define PARTICIPANT_INFO_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "call/message/IMtcMessageHandler.h"

class IMessage;
class IMtcCallContext;

/**
 * This class stores caller and callee info.
 */
class ParticipantInfo final : public IMtcMessageHandler
{
public:
    explicit ParticipantInfo(IN IMtcCallContext& objContext);
    ~ParticipantInfo();
    ParticipantInfo(IN const ParticipantInfo&) = delete;
    ParticipantInfo& operator=(IN const ParticipantInfo&) = delete;

    AString GetLocalNumber() const;
    AString GetLocalUri() const;
    AString GetRemoteNumber() const;
    AString GetRemoteUri() const;
    AString GetRemoteDisplayName() const;
    OipType GetOipType() const;

    void UpdateFromRemoteNumber(IN const AString& strRemoteNumber);

    void HandleRequest(IN RequestType eType, IN const IMessage& objRequest) override;
    void HandleResponse(IN ResponseType eType, IN const IMessage& objResponse) override;

private:
    static const AString URI_SET_BY_IMS_ENGINE;

    AString GetRemoteNumberFromMessage(IN const IMessage& objMessage) const;

    IMtcCallContext& m_objContext;

    AString m_strRemoteNumber;
    AString m_strRemoteUri;
};

#endif
