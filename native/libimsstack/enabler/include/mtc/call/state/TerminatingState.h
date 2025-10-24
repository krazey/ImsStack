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

#ifndef TERMINATING_STATE_H_
#define TERMINATING_STATE_H_

#include "ImsTypeDef.h"
#include "call/state/MtcCallState.h"

class TerminatingState : public MtcCallState
{
public:
    explicit TerminatingState(IN IMtcCallContext& objContext);
    virtual ~TerminatingState() override;
    TerminatingState(IN const TerminatingState&) = delete;
    TerminatingState& operator=(IN const TerminatingState&) = delete;

    void OnEnter() override;
    CallStateName SessionStartFailed(IN ISession* piSession) override;
    CallStateName SessionTerminated(IN ISession* piSession) override;
    CallStateName OnTimerExpired(IN IMS_SINT32 nType) override;
    CallStateName OnRatChanged(IN IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType) override;

private:
    void HandleCallSessionReleased();
    void NotifyCallSessionReleased();

    IMS_BOOL m_bSessionReleasedNotified;
};

#endif
