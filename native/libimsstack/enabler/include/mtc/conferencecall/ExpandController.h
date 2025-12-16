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

#ifndef EXPAND_CONTROLLER_H_
#define EXPAND_CONTROLLER_H_

#include "conferencecall/ConferenceController.h"

class IMtcCallContext;
class IConferenceReference;
class SuppService;
struct CallInfo;
struct CallStartOperationParams;
struct ConfUser;
template <class T>
class ImsList;

class ExpandController : public ConferenceController
{
public:
    explicit ExpandController(IN CallKey nConfCallKey, IMtcContext& objContext,
            IN CallConnectionIdManager& objConnectionIdManager, IN ConferenceFactory& objFactory);
    virtual ~ExpandController() override;
    ExpandController(IN const ExpandController&) = delete;
    ExpandController& operator=(IN const ExpandController&) = delete;

public:
    // void Updated(IN IMS_UINTP nParam) override;
    void OnReferenceStarted(IN IConferenceReference* piConfRef) override;
    void OnReferenceStartFailed(IN IConferenceReference* piConfRef) override;
    void OnReferenceUpdated(IN IConferenceReference* piConfRef, IN IMS_SINT32 nSipFragCode,
            IN ReferSubscriptionState eState) override;

protected:
    void ProcessExpand(IN ImsList<ConfUser*>& objUsers) override;
    void StartConferenceCall(IN ConferenceOperationQueue::ConferenceOperation* pOperation) override;
    IMS_BOOL IsStartFinalSipfragWaitTimer() const override;
    void Recover() override;

    void OnCallUpdated(IN IMS_UINT32 nType, IN IMS_UINTP nCallKey) override;
    void UpdateUserStatusByReferResult(IN ConfUser* pUser, IN IConferenceReference* piConfRef,
            IN IMS_SINT32 nStatusCode = SipStatusCode::SC_200) override;

    void NotifyCmdResult() override;

private:
    void StopMedia1to1Session();
    static void Resume1to1Session();
    void ProcessJoinAfterExpand();
    void RecoverOnCreating();
    void RecoverOnReferring();
};

#endif
