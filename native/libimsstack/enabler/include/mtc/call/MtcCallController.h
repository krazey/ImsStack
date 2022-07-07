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

#include "ImsTypeDef.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "call/IMtcCall.h"
#include "IMtcCallController.h"
#include "IMtcService.h"

class IMtcCallManager;
class IMtcContext;
class ISession;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
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
    MtcCallController(IN IMtcContext& objContext);
    virtual ~MtcCallController();
    MtcCallController(IN const MtcCallController&) = delete;
    MtcCallController& operator=(IN const MtcCallController&) = delete;

    void TerminateCalls(
            IN KeyType eKeyType, IN Key nKey, IN const CallReasonInfo& objReason) override;
    void RemoveCalls(IN KeyType eKeyType, IN Key nKey) override;
    CallKey Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo) override;
    void Attach(IN CallKey nCallKey, IN JniMtcCallThread* pJniMtcCallThread,
            IN JniMediaSessionThread* pJniMediaThread) override;
    void Detach(IN CallKey nCallKey) override;
    void HandleIncoming(IN IMtcService* pService, IN ISession* piSession,
            IN JniMtcServiceThread* pServiceThread) override;
    void Start(IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IDialogEvent* pDialog) override;
    void HandleUserAlert(IN CallKey nCallKey) override;
    void Accept(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void Reject(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void Hold(IN CallKey nCallKey, IN MediaInfo* pMediaInfo) override;
    void Resume(IN CallKey nCallKey, IN MediaInfo* pMediaInfo) override;
    void AcceptResume(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void RejectResume(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void Terminate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void Update(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void CancelUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void AcceptUpdate(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void RejectUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) override;
    void SendUssd(IN CallKey nCallKey, IN const AString& strUssd) override;

    /*
    void StartGroupCall(IN CallKey nCallKey, IN IMS_UINT32 nCmd, IN IMSList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN IMSMap<SuppType, SuppService*>& objSuppServices);
    */

    void MergeToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers) override;
    void AddToConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers) override;
    void RemoveFromConference(IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers) override;

    // TODO: Consider ECT, SRVCC
    void Transfer(IN CallKey nCallKey, IN const AString& strTarget) override;

    void HandleIpcanChanged() override;

private:
    IMtcContext& m_objContext;
    IMtcCallManager& m_objCallManager;
};

#endif
