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
#ifndef INTERFACE_JNI_MTC_CALL_THREAD_H_
#define INTERFACE_JNI_MTC_CALL_THREAD_H_

#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "ImsMap.h"
#include "ImsList.h"
#include "conferencecall/ConferenceDef.h"
#include "IJniEnablerThread.h"

class SuppService;
class ParticipantInfo;
struct CallReasonInfo;
struct JniCallInfo;

class IJniMtcCallThread : public IJniEnablerThread
{
public:
    virtual ~IJniMtcCallThread() {}

    virtual void OnStarted(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnStartFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnProgressing(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE) = 0;
    virtual void OnHeld(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnHoldFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnResumed(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnResumeFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnHeldBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnResumedBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnTerminated(IN const CallReasonInfo& objReason) = 0;
    virtual void OnIncomingResume(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnIncomingUpdate(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnUpdated(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;
    virtual void OnUpdateFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnUpdatedBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    virtual void OnMerged(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) = 0;
    virtual void OnMergeFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnConferenceParticipantAdded() = 0;
    virtual void OnConferenceParticipantAddFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnConferenceParticipantRemoved() = 0;
    virtual void OnConferenceParticipantRemoveFailed(IN const CallReasonInfo& objReason) = 0;
    virtual void OnConferenceInfoChanged(IN const AString& strDisplayText,
            IN const AString strSubject, IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount,
            IN const AString& strHost) = 0;
    virtual void OnConferenceParticipantsInfoChanged(IN const ImsList<ConfUser*>& objUsers) = 0;

    virtual void OnEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    virtual void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN const JniCallInfo& objCallInfo,
            IN MediaInfo* pMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo* pParticipantInfo) = 0;

    virtual void OnInformationNotificationReceived(IN IMS_UINT32 nType, IN const AString strValue,
            IN IMS_SINT32 nValue, IN IMS_BOOL bValue) = 0;
};

#endif
