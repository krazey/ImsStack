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
#include "call/ParticipantInfo.h"
#include "MtcDef.h"
#include <binder/Parcel.h>
#include "ImsMap.h"

struct JniCallInfo;
struct CallReasonInfo;
class MediaInfo;
class SuppService;
class ConfUser;

class JniMtcCallThread final : public BaseServiceThread
{
public:
    JniMtcCallThread();
    virtual ~JniMtcCallThread();

    inline void SetSlotId(IN IMS_SINT32 nSlotId) { m_nSlotId = nSlotId; }

    void OnStarted(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnStartFailed(IN const CallReasonInfo& objReason);
    void OnProgressing(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE);
    void OnHeld(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnHoldFailed(IN const CallReasonInfo& objReason);
    void OnResumed(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnResumeFailed(IN const CallReasonInfo& objReason);
    void OnHeldBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnResumedBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnTerminated(IN const CallReasonInfo& objReason);
    void OnIncomingResume(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnIncomingUpdate(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnUpdated(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);
    void OnUpdateFailed(IN const CallReasonInfo& objReason);
    void OnUpdatedBy(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices);

    void OnMerged(IN const JniCallInfo& objCallInfo, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN const IMSList<ConfUser*>& objUsers);
    void OnMergeFailed(IN const CallReasonInfo& objReason);
    void OnConferenceParticipantAdded();
    void OnConferenceParticipantAddFailed(IN const CallReasonInfo& objReason);
    void OnConferenceParticipantRemoved();
    void OnConferenceParticipantRemoveFailed(IN const CallReasonInfo& objReason);
    void OnConferenceInfoChanged(IN const AString& strDisplayText, IN const AString strSubject,
            IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount, IN const AString& strHost);
    void OnConferenceParticipantsInfoChanged(IN const IMSList<ConfUser*>& objUsers);

    void OnEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason);

    void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN const JniCallInfo& objCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo* pParticipantInfo);

    void OnInformationNotificationReceived(IN IMS_UINT32 nType, IN const AString strValue,
            IN IMS_SINT32 nValue, IN IMS_BOOL bValue);

private:
    void SetCallDetails(IN_OUT android::Parcel& objParcel, IN const JniCallInfo& objCallInfo,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices);

private:
    IMS_SINT32 m_nSlotId;
};

#endif
