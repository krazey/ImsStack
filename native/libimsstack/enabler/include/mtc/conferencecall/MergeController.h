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

#ifndef MERGE_CONTROLLER_H_
#define MERGE_CONTROLLER_H_

#include "ImsList.h"
#include "conferencecall/ConferenceController.h"

class IMtcCallContext;
class SuppService;
struct CallInfo;
struct CallStartOperationParams;
struct ConfUser;

class MergeController final : public ConferenceController
{
public:
    explicit MergeController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory);
    virtual ~MergeController();
    MergeController(IN const MergeController&) = delete;
    MergeController& operator=(IN const MergeController&) = delete;

protected:
    void ProcessMerge(IN ImsList<ConfUser*>& objUsers) override;
    void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation* pOperation) override;
    IMS_BOOL IsStartFinalSipfragWaitTimer() const override;
    void Recover() override;
    void OnIndividualCallTerminated(IN IMS_UINTP nCallKey) override;

private:
    void ProcessMergeWithoutRefer(IN ImsList<ConfUser*>& objUsers);
    void UpdateUserStateByCallTerminated(IN IMS_UINTP nCallKey);
    void RecoverOnCreating();
    void RecoverOnReferring();
    IMS_BOOL RecoverOnConferenceCallFailed();
    void ClearIndividualCallOnMergeFailed();
    void UpdateStartCallType(IN const ImsList<ConfUser*> objUsers);
    CallType m_eStartCallType;
};

#endif
