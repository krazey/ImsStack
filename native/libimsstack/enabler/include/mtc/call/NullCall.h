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

#ifndef NULL_CALL_H_
#define NULL_CALL_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class AString;
class ISession;
class SuppService;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;
template <class T>
class ImsList;

/**
 * This class represents the call that doesn't exist. It has no states and basically does nothing.
 */
class NullCall final : public IMtcCall
{
public:
    NullCall() {}
    virtual ~NullCall() override {}
    NullCall(IN const NullCall&) = delete;
    NullCall& operator=(IN const NullCall&) = delete;

    inline void Attach() override {}

    inline void Start(
            IN CallType, IN const AString&, IN MediaInfo&, IN const ImsList<SuppService*>&) override
    {
    }

    inline void StartConference(IN CallType, IN const AString&, IN MediaInfo&,
            IN const ImsList<SuppService*>&, IN const ImsList<ConfUser*>&) override
    {
    }
    inline void StartConference(
            IN CallType, IN const AString&, IN const ImsList<ConfUser*>&) override
    {
    }
    inline void HandleIncoming(IN ISession*) override {}
    inline void HandleUserAlert() override {}
    inline void Accept(IN CallType, IN MediaInfo&) override {}
    inline void Reject(IN const CallReasonInfo&) override {}
    inline void Hold(IN MediaInfo&) override {}
    inline void Resume(IN MediaInfo&) override {}
    inline void AcceptResume(IN CallType, IN MediaInfo&) override {}
    inline void RejectResume(IN const CallReasonInfo&) override {}
    inline void Update(IN CallType, IN MediaInfo&) override {}
    inline void AcceptUpdate(IN CallType, IN MediaInfo&) override {}
    inline void RejectUpdate(IN const CallReasonInfo&) override {}
    inline void CancelUpdate(IN const CallReasonInfo&) override {}
    inline void Terminate(IN const CallReasonInfo&) override {}
    inline void SendUssd(IN const AString&) override {}
    inline const AString& GetLogTag() const override
    {
        static const AString sLogTag("Null_Call");
        return sLogTag;
    }

    inline CallKey GetKey() const override { return CALL_KEY_INVALID; }
    inline CallType GetCallType() const override { return CallType::UNKNOWN; }
    inline State GetState() const override { return State::IDLE; }
    inline IMtcCallContext& GetCallContext() override
    {
        return *(reinterpret_cast<IMtcCallContext*>(this));
    }
};

#endif
