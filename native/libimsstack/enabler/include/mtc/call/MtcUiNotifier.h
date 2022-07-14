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
#include "IMSTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

class IMtcCallContext;
class ParticipantInfo;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
class MediaInfo;
class SuppService;
struct CallInfo;
struct CallReasonInfo;
struct ConfUser;

// TODO: remove unused parameter
// TODO: get SuppService from the context

class MtcUiNotifier final
{
public:
    explicit MtcUiNotifier(IN IMtcCallContext& objContext);
    ~MtcUiNotifier();
    MtcUiNotifier(IN const MtcUiNotifier&) = delete;
    MtcUiNotifier& operator=(IN const MtcUiNotifier&) = delete;

    // _JNI_MTC_
    inline void SetJniCallThread(IN JniMtcCallThread* pThread) { m_pCallThread = pThread; }
    inline void SetJniServiceThread(IN JniMtcServiceThread* pThread) { m_pServiceThread = pThread; }
    inline void SetJniMediaThread(IN JniMediaSessionThread* pThread) { m_pMediaThread = pThread; }
    inline JniMediaSessionThread* GetJniMediaThread() { return m_pMediaThread; }

    void SendPreIncomingCallReceived(IN CallKey nKey);
    void SendIncomingCallReceived(IN CallKey nKey, IN CallInfo& objCallInfo,
            IN MediaInfo& objMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo& objParticipantInfo);
    void SendStarted(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendStartFailed(IN const CallReasonInfo& objReason);
    void SendProgressing(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE);
    void SendHeld(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendHoldFailed(IN const CallReasonInfo& objReason);
    void SendResumed(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendResumeFailed(IN const CallReasonInfo& objReason);
    void SendHeldBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendResumedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendTerminated(IN const CallReasonInfo& objReason);
    void SendIncomingResume(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendIncomingUpdate(IN CallType eCallTypeToUpdate, IN CallInfo* pCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendUpdated(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendUpdateFailed(IN const CallReasonInfo& objReason);
    void SendUpdatedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendNotifyInfo(IN IMS_UINT32 eType, IN AString strValue = AString::ConstNull(),
            IN IMS_SINT32 nValue = -1, IN IMS_BOOL bValue = IMS_FALSE);
    void SendExpanded(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void SendExpandFailed(IN const CallReasonInfo& objReason);
    void SendExpandedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices, IN IMS_SINTP nReplaceKey = 0);
    void SendMerged(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMSList<ConfUser*>& lstConfUser);
    void SendMergeFailed(IN const CallReasonInfo& objReason);
    void SendJoined(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason);
    void SendDropped(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason);
    void SendNotifyUsersInfo(IN IMSList<ConfUser*>& lstConfUser);
    void SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
            IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount, IN AString strHostEntity);
    void SendReplacedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices, IN IMS_SINTP nReplaceKey = 0,
            IN IMS_UINTP nType = 0);
    void SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason);
    void SendCallPushCompleted(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason);

private:
    IMS_BOOL IsAvailableToSend();

    IMtcCallContext& m_objContext;

    JniMtcCallThread* m_pCallThread;
    JniMtcServiceThread* m_pServiceThread;
    JniMediaSessionThread* m_pMediaThread;
};

#endif
