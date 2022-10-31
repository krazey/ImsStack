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

#ifndef INTERFACE_MTC_UI_NOTIFIER_H_
#define INTERFACE_MTC_UI_NOTIFIER_H_

#include "AString.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

class ParticipantInfo;
class MediaInfo;
class SuppService;
struct CallInfo;
struct CallReasonInfo;
struct ConfUser;

class IMtcUiNotifier
{
public:
    virtual ~IMtcUiNotifier() {}

    virtual void SendPreIncomingCallReceived(IN CallKey nKey) = 0;
    virtual void SendIncomingCallReceived(IN CallKey nKey, IN CallInfo& objCallInfo,
            IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo& objParticipantInfo) = 0;
    virtual void SendStarted(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendStartFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void SendProgressing(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE) = 0;
    virtual void SendHeld(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendHoldFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void SendResumed(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendResumeFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void SendHeldBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendResumedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendTerminated(IN const CallReasonInfo& objReason) = 0;
    virtual void SendIncomingResume(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendIncomingUpdate(IN CallType eCallTypeToUpdate, IN CallInfo* pCallInfo,
            IN MediaInfo* pMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendUpdated(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendUpdateFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void SendUpdatedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendNotifyInfo(IN IMS_UINT32 eType, IN AString strValue = AString::ConstNull(),
            IN IMS_SINT32 nValue = -1, IN IMS_BOOL bValue = IMS_FALSE) = 0;
    virtual void SendExpanded(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void SendExpandFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void SendExpandedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_SINTP nReplaceKey = 0) = 0;
    virtual void SendMerged(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ImsList<ConfUser*>& lstConfUser) = 0;
    virtual void SendMergeFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void SendJoined(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) = 0;
    virtual void SendDropped(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) = 0;
    virtual void SendNotifyUsersInfo(IN ImsList<ConfUser*>& lstConfUser) = 0;
    virtual void SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
            IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount, IN AString strHostEntity) = 0;
    virtual void SendReplacedBy(IN CallInfo* pCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN IMS_SINTP nReplaceKey = 0,
            IN IMS_UINTP nType = 0) = 0;
    virtual void SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;
    virtual void SendCallPushCompleted(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) = 0;
};

#endif
