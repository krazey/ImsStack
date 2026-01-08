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

#ifndef GROUP_CALL_CONTROLLER_H_
#define GROUP_CALL_CONTROLLER_H_

#include "conferencecall/ConferenceController.h"

class IMtcCallContext;
class IConferenceReference;
class SuppService;
struct CallStartOperationParams;
struct CallInfo;
struct ConfUser;
struct MediaInfo;
template <class T>
class ImsList;

class GroupCallController final : public ConferenceController
{
public:
    explicit GroupCallController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory);
    virtual ~GroupCallController() override;
    GroupCallController(IN const GroupCallController&) = delete;
    GroupCallController& operator=(IN const GroupCallController&) = delete;

public:
    void OnReferenceStartFailed(IN IConferenceReference* piConfRef) override;

protected:
    void ProcessGroupCall(IN ImsList<ConfUser*>& objUsers, IN CallInfo& objCallInfo,
            IN MediaInfo& objMediaInfo, IN ImsList<SuppService*>& objSuppServices) override;
    void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation* pOperation) override;
    void Recover() override;

private:
    void RecoverOnCreating();
    void RecoverOnReferring();
};

#endif
