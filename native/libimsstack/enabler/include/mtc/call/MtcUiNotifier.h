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

#ifndef MTC_UI_NOTIFIER_H_
#define MTC_UI_NOTIFIER_H_

#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcUiNotifier.h"

class IMtcCallContext;
class ParticipantInfo;
class IJniMtcCallThread;
class SuppService;
struct CallInfo;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;

// TODO: remove unused parameter
// TODO: get SuppService from the context
// TODO: make all apis to return a result and let MtcCall logic handle the result.

class MtcUiNotifier final : public IMtcUiNotifier
{
public:
    explicit MtcUiNotifier(IN IMtcCallContext& objContext);
    ~MtcUiNotifier();
    MtcUiNotifier(IN const MtcUiNotifier&) = delete;
    MtcUiNotifier& operator=(IN const MtcUiNotifier&) = delete;

    void SendPreIncomingCallReceived(IN CallKey nKey) override;
    void SendIncomingCallReceived(IN CallKey nKey, IN CallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo& objParticipantInfo) override;
    void SendStarted(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendStartFailed(IN const CallReasonInfo& objReason) override;
    void SendProgressing(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE) override;
    void SendHeld(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendHoldFailed(IN const CallReasonInfo& objReason) override;
    void SendResumed(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendResumeFailed(IN const CallReasonInfo& objReason) override;
    void SendHeldBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendResumedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendTerminated(IN const CallReasonInfo& objReason) override;
    void SendIncomingResume(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendIncomingUpdate(IN CallType eCallTypeToUpdate, IN CallInfo* pCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendUpdated(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendUpdateFailed(IN const CallReasonInfo& objReason) override;
    void SendUpdatedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendNotifyInfo(IN IMS_UINT32 eType, IN const AString& strValue, IN IMS_SINT32 nValue = -1,
            IN IMS_BOOL bValue = IMS_FALSE) override;
    void SendExpanded(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void SendExpandFailed(IN const CallReasonInfo& objReason) override;
    void SendExpandedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_SINTP nReplaceKey = 0) override;
    void SendMerged(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ImsList<ConfUser*>& lstConfUser) override;
    void SendMergeFailed(IN const CallReasonInfo& objReason) override;
    void SendJoined(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) override;
    void SendDropped(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) override;
    void SendNotifyUsersInfo(IN ImsList<ConfUser*>& lstConfUser) override;
    void SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
            IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount,
            IN AString strHostEntity) override;
    void SendReplacedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN IMS_SINTP nKey = 0,
            IN IMS_UINTP nType = 0) override;
    void SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) override;
    void SendCallPushCompleted(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) override;

private:
    IJniMtcCallThread* GetCallThread();
    IMtcCallContext& m_objContext;
};

#endif
