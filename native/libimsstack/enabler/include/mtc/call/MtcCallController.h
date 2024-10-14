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

#ifndef MTC_CALL_CONTROLLER_H_
#define MTC_CALL_CONTROLLER_H_

#include "IMtcCallController.h"
#include "IMtcService.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "call/IMtcCall.h"

class IMtcCallManager;
class IMtcContext;
class ISession;
class ISilentRedialHelper;
class SilentRedialHelper;
enum class KeyType;
struct ConfUser;
union Key;

/**
 * Provides operations to manipulate calls. Each operation could be failed or not handled if the
 * current status is not applicable or some other reasons.
 */
class MtcCallController final : public IMtcCallController
{
public:
    explicit MtcCallController(IN IMtcContext& objContext);
    virtual ~MtcCallController();
    MtcCallController(IN const MtcCallController&) = delete;
    MtcCallController& operator=(IN const MtcCallController&) = delete;

    inline void NotifyJniEnablerSet() override {}

    CallKey Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo) override;
    void Attach(IN CallKey nCallKey) override;
    void Detach(IN CallKey nCallKey) override;
    void HandleIncoming(IN IMtcService* pService, IN ISession* piSession) override;
    void Start(IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void HandleUserAlert(IN CallKey nCallKey) override;
    void Accept(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void Reject(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void Hold(IN CallKey nCallKey, IN MediaInfo& objMediaInfo) override;
    void Resume(IN CallKey nCallKey, IN MediaInfo& objMediaInfo) override;
    void AcceptResume(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void RejectResume(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void Terminate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void Update(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void CancelUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void AcceptUpdate(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) override;
    void RejectUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void SendUssd(IN CallKey nCallKey, IN const AString& strUssd) override;

    /*
    void StartGroupCall(IN CallKey nCallKey, IN IMS_UINT32 nCmd, IN ImsList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN ImsMap<SuppType, SuppService*>& objSuppServices);
    */

    void MergeToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) override;
    void AddToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) override;
    void RemoveFromConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) override;

    // TODO: Consider ECT, SRVCC
    void Transfer(IN CallKey nCallKey, IN const AString& strTarget) override;

    ISilentRedialHelper& GetRedialHelper(
            IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason) override;
    void ReleaseRedialHelper() override;

private:
    IMtcContext& m_objContext;
    IMtcCallManager& m_objCallManager;
    SilentRedialHelper* m_pRedialHelper;
};

#endif
