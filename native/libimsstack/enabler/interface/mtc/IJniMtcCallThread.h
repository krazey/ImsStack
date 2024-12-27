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

#include "IJniEnablerThread.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"

struct CallReasonInfo;
struct ConfUser;
struct JniCallInfo;

class IJniMtcCallThread : public IJniEnablerThread
{
public:
    virtual ~IJniMtcCallThread() {}

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnStarted(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnStartFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call setup is initiating.
     *
     * This method is called after sending an INVITE request, indicating that the call setup process
     * has started but not yet progressed to ringing or connected state.
     *
     * @param objCallInfo The call information.
     * @param objMediaInfo The media information related to the call.
     * @param eRatType The current RAT type.
     */
    virtual void OnInitiating(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN IMS_SINT32 eRatType) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnProgressing(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnHeld(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnHoldFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnResumed(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnResumeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnHeldBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnResumedBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnTerminated(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnIncomingResume(IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnIncomingUpdate(IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnUpdated(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnUpdateFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void OnUpdatedBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies
     *
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param objUsers
     */
    virtual void OnMerged(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnMergeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     */
    virtual void OnConferenceParticipantAdded() = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnConferenceParticipantAddFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     */
    virtual void OnConferenceParticipantRemoved() = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     */
    virtual void OnConferenceParticipantRemoveFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     * @param strDisplayText
     * @param strSubject
     * @param nUserCount
     * @param nMaxUserCount
     * @param strHost
     */
    virtual void OnConferenceInfoChanged(IN const AString& strDisplayText,
            IN const AString strSubject, IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount,
            IN const AString& strHost) = 0;

    /**
     * @brief Notifies
     *
     * @param objUsers
     */
    virtual void OnConferenceParticipantsInfoChanged(IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Notifies
     *
     * @param nResult
     * @param objReason
     */
    virtual void OnEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies there is an incoming call that needs to be processed.
     *
     * @param nCallKey
     * @param objCallInfo The call information.
     * @param objMediaInfo The media information related to the call.
     * @param objSuppServices The supplementary services information related to the call.
     * @param eOipType The OIP type to determine whether show a caller identification or not.
     * @param strRemoteNumber The number of a caller.
     * @param eRatType The current RAT type.
     */
    virtual void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN OipType eOipType,
            IN const AString& strRemoteNumber, IN IMS_SINT32 eRatType) = 0;

    /**
     * @brief Notifies
     *
     * @param nType
     * @param strValue
     * @param nValue
     * @param bValue
     */
    virtual void OnInformationNotificationReceived(IN IMS_UINT32 eType, IN const AString strValue,
            IN IMS_SINT32 nValue, IN IMS_BOOL bValue) = 0;

    /**
     * @brief Notifies the RAT has changed.
     *
     * @param eRatType The changed RAT type.
     */
    virtual void OnRatChanged(IN IMS_SINT32 eRatType) = 0;
};

#endif
