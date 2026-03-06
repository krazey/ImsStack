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
#include "ImsTypeDef.h"

class SuppService;
enum class OipType;
struct CallReasonInfo;
struct ConfUser;
struct JniCallInfo;
struct MediaInfo;
template <class T>
class ImsList;

/**
 * @brief Interface for notifying the JNI layer about MTC call events.
 *
 * This interface defines the callback methods used by the native MTC stack to inform
 * the upper layers (Java/JNI) about changes in call state, such as incoming calls,
 * call establishment, termination, hold/resume, and conference events.
 */
class IJniMtcCallThread : public IJniEnablerThread
{
public:
    /**
     * @brief Notifies that the call has been successfully started (established).
     *
     * This callback is triggered when the call enters the active state (200 OK/ACK exchanged).
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnStarted(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call start request failed.
     *
     * This callback is triggered when the outgoing call attempt fails due to an error response
     * (e.g., 4xx, 5xx, 6xx) or a timeout.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnStartFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call setup is initiating.
     *
     * This method is called after sending an INVITE request, indicating that the call setup process
     * has started but not yet progressed to ringing or connected state.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     */
    virtual void OnInitiating(
            IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Notifies that the call is progressing (e.g. ringing).
     *
     * This callback is triggered when a provisional response (e.g., 180 Ringing, 183 Session
     * Progress), 200-UPDATE, or UPDATE are received from the remote party.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnProgressing(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call has been held locally.
     *
     * This callback is triggered when a local hold request has been successfully processed.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnHeld(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call hold request failed.
     *
     * This callback is triggered when a local hold request fails.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnHoldFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call has been resumed locally.
     *
     * This callback is triggered when a local resume request has been successfully processed.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnResumed(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call resume request failed.
     *
     * This callback is triggered when a local resume request fails.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnResumeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call has been held by the remote party.
     *
     * This callback is triggered when the remote party puts the call on hold (e.g., receives a
     * re-INVITE with sendonly/inactive SDP).
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnHeldBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call has been resumed by the remote party.
     *
     * This callback is triggered when the call has been successfully resumed by the remote party
     * (operation completed).
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnResumedBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call has been terminated.
     *
     * This callback is triggered when the call session ends, either by a BYE request or due to
     * an error during establishment.
     *
     * @param objReason The {@code CallReasonInfo} describing the termination reason.
     */
    virtual void OnTerminated(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies of an incoming resume request.
     *
     * This callback is triggered when a re-INVITE request for resuming the call is received from
     * the remote party, before the operation is fully processed.
     *
     * While standard resume requests are often handled automatically within the native layer,
     * certain carrier specifications or specific call types (e.g., Video Resume, adjustments
     * of media direction for voice/video calls) require the upper layer to inspect and
     * potentially modify media parameters (such as audio/video direction) based on local
     * status (e.g., camera availability) before explicitly accepting the request. As seen in
     * {@code ImsCallSessionImpl#onCallUpdateResumeReceived}, this allows for carrier-specific
     * verification and media session synchronization.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnIncomingResume(IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies of an incoming update request (e.g. video upgrade).
     *
     * This callback is triggered when a re-INVITE or UPDATE request is received, typically for
     * modifying media properties (e.g., upgrading to video).
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnIncomingUpdate(IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call has been updated (e.g. media type changed).
     *
     * This callback is triggered when a local update request (e.g., media change) completes
     * successfully.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnUpdated(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that the call update request failed.
     *
     * This callback is triggered when a local update request fails.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnUpdateFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call has been updated by the remote party.
     *
     * This callback is triggered when a remote update request is successfully applied.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     */
    virtual void OnUpdatedBy(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies that calls have been successfully merged into a conference.
     *
     * This callback is triggered when a conference merge operation completes successfully.
     *
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     * @param objUsers The list of {@code ConfUser} participating in the conference.
     */
    virtual void OnMerged(IN const JniCallInfo& objCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsList<SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Notifies that the call merge request failed.
     *
     * This callback is triggered when a conference merge operation fails.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnMergeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that a participant has been added to the conference.
     */
    virtual void OnConferenceParticipantAdded() = 0;

    /**
     * @brief Notifies that adding a participant to the conference failed.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnConferenceParticipantAddFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that a participant has been removed from the conference.
     */
    virtual void OnConferenceParticipantRemoved() = 0;

    /**
     * @brief Notifies that removing a participant from the conference failed.
     *
     * @param objReason The {@code CallReasonInfo} describing the failure reason.
     */
    virtual void OnConferenceParticipantRemoveFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that conference information has changed.
     *
     * @param strDisplayText The display text for the conference.
     * @param strSubject The subject of the conference.
     * @param nUserCount The current number of users in the conference.
     * @param nMaxUserCount The maximum number of users allowed in the conference.
     * @param strHost The host of the conference.
     */
    virtual void OnConferenceInfoChanged(IN const AString& strDisplayText,
            IN const AString strSubject, IN IMS_UINT32 nUserCount, IN IMS_UINT32 nMaxUserCount,
            IN const AString& strHost) = 0;

    /**
     * @brief Notifies that the list of conference participants has changed.
     *
     * @param objUsers The updated list of {@code ConfUser}.
     */
    virtual void OnConferenceParticipantsInfoChanged(IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Notifies the result of an Explicit Call Transfer (ECT).
     *
     * This callback is triggered when an ECT operation completes.
     *
     * @param nResult The result of the ECT operation.
     * @param objReason The {@code CallReasonInfo} describing the result details.
     */
    virtual void OnEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that an incoming call is ready to be alerted to the user.
     *
     * This callback is triggered when an incoming call is ready to be alerted to the user.
     *
     * @param nCallKey The unique call key.
     * @param objCallInfo The {@code JniCallInfo} containing call information.
     * @param objMediaInfo The {@code MediaInfo} related to the call.
     * @param objSuppServices The list of {@code SuppService} related to the call.
     * @param eOipType The {@code OipType} to determine whether to show caller identification.
     * @param strRemoteNumber The remote party's number.
     */
    virtual void OnIncomingCallReceived(IN IMS_UINTP nCallKey, IN const JniCallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices,
            IN OipType eOipType, IN const AString& strRemoteNumber) = 0;

    /**
     * @brief Notifies of a USSI (Unstructured Supplementary Service Data over IMS) event.
     *
     * This callback is triggered when a USSI event occurs, such as receiving a USSD message
     * or an error.
     *
     * @param eType The type of notification (e.g., {@code UssiEventNotifier::INFO_TYPE_USSI}).
     * @param strValue The string value associated with the notification (e.g., USSD string).
     * @param nValue The integer value associated with the notification (e.g.,
     *               {@code UssiModeType}).
     * @param bValue Boolean value associated with the notification.
     */
    virtual void OnInformationNotificationReceived(IN IMS_UINT32 eType, IN const AString strValue,
            IN IMS_SINT32 nValue, IN IMS_BOOL bValue) = 0;

    /**
     * @brief Notifies that the call information has changed.
     *
     * This callback is triggered when specific call details are updated, such as RAT (Radio Access
     * Technology) change or Cross SIM Call status change.
     *
     * @param objCallInfo The updated {@code JniCallInfo}.
     */
    virtual void OnCallInfoChanged(IN const JniCallInfo& objCallInfo) = 0;
};

#endif
