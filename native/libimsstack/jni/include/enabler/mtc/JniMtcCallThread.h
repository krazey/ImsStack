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

#ifndef JNI_MTC_CALL_THREAD_H_
#define JNI_MTC_CALL_THREAD_H_

#include "BaseService.h"
#include "BaseServiceThread.h"
#include "IJniMtcCallThread.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "MtcDef.h"
#include <binder/Parcel.h>

class SuppService;
struct CallReasonInfo;
struct ConfUser;
struct JniCallInfo;
struct MediaInfo;

class JniMtcCallThread final : public BaseServiceThread, public IJniMtcCallThread
{
public:
    JniMtcCallThread();
    virtual ~JniMtcCallThread();

    void OnStarted(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnStartFailed(IN const CallReasonInfo& objReason) override;
    void OnInitiating(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo) override;
    void OnProgressing(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnHeld(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnHoldFailed(IN const CallReasonInfo& objReason) override;
    void OnResumed(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnResumeFailed(IN const CallReasonInfo& objReason) override;
    void OnHeldBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnResumedBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnTerminated(IN const CallReasonInfo& objReason) override;
    void OnIncomingResume(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnIncomingUpdate(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnUpdated(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;
    void OnUpdateFailed(IN const CallReasonInfo& objReason) override;
    void OnUpdatedBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) override;

    void OnMerged(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) override;
    void OnMergeFailed(IN const CallReasonInfo& objReason) override;
    void OnConferenceParticipantAdded() override;
    void OnConferenceParticipantAddFailed(IN const CallReasonInfo& objReason) override;
    void OnConferenceParticipantRemoved() override;
    void OnConferenceParticipantRemoveFailed(IN const CallReasonInfo& objReason) override;
    void OnConferenceInfoChanged(IN const AString& strDisplayText, IN const AString strSubject,
            IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount,
            IN const AString& strHost) override;
    void OnConferenceParticipantsInfoChanged(IN const ImsList<ConfUser*>& objUsers) override;

    void OnEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) override;

    void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN OipType eOipType,
            IN const AString& strRemoteNumber) override;

    void OnInformationNotificationReceived(IN IMS_UINT32 eType, IN const AString strValue,
            IN IMS_SINT32 nValue, IN IMS_BOOL bValue) override;

private:
    static void SetCallDetails(IN_OUT android::Parcel& objParcel, IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices);
};

#endif
