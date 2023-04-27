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

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class AString;
struct CallReasonInfo;
struct ConfUser;

/**
 * This class provides the convenience APIs to notify UI of call events through JNI.
 */
class IMtcUiNotifier
{
public:
    virtual ~IMtcUiNotifier() {}

    /**
     * @brief Notifies that there's an incoming call.
     *
     * It doesn't display incoming call UI, but requests to attach the call to provide some
     * information about the call in advance.
     */
    virtual void SendPreIncomingCallReceived() = 0;

    /**
     * @brief Notifies that the incoming call is ready to be displayed to the UI.
     */
    virtual void SendIncomingCallReceived() = 0;

    /**
     * @brief Notifies that the incoming or outgoing call is established and started.
     */
    virtual void SendStarted() = 0;

    /**
     * @brief Notifies that the call setup is failed.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendStartFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call setup is progressing.
     *
     * @param bRemoteAlerted True if the remote is alerting.
     */
    virtual void SendProgressing(IN IMS_BOOL bRemoteAlerted = IMS_FALSE) = 0;

    /**
     * @brief Notifies that the call is held by the user.
     */
    virtual void SendHeld() = 0;

    /**
     * @brief Notifies that the call hold is failed.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendHoldFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call is resumed by the user.
     */
    virtual void SendResumed() = 0;

    /**
     * @brief Notifies that the call resume is failed.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendResumeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call is held by the remote.
     */
    virtual void SendHeldBy() = 0;

    /**
     * @brief Notifies that the call is resumed by the remote.
     */
    virtual void SendResumedBy() = 0;

    /**
     * @brief Notifies that the established call is terminated.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendTerminated(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that a call resume is incoming but need to check UI condition before accept
     *        automatically.
     *
     * Currently it's for Verizon only.
     */
    virtual void SendIncomingResume() = 0;

    /**
     * @brief Notifies that a call update is incoming and need to be accepted or rejected.
     *
     * @param eCallTypeToUpdate The requested call type.
     */
    virtual void SendIncomingUpdate(IN CallType eCallTypeToUpdate) = 0;

    /**
     * @brief Notifies that the incoming or outgoing call update is accepted and successfully
     *        completed.
     */
    virtual void SendUpdated() = 0;

    /**
     * @brief Notifies that the incoming call update is rejected.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendUpdateFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the call update from the remote is accepted, but it's not hold, resume
     *        or call type change.
     *
     * It could be a media codec change.
     */
    virtual void SendUpdatedBy() = 0;

    /**
     * @brief Notifies that there's incoming information.
     *
     * @param eType Type of the information. See INFO_TYPE_* of IUMtcCall.java.
     * @param strValue String value.
     * @param nValue Integer value.
     * @param bValue Boolean value.
     */
    virtual void SendNotifyInfo(IN IMS_UINT32 eType, IN const AString& strValue,
            IN IMS_SINT32 nValue = -1, IN IMS_BOOL bValue = IMS_FALSE) = 0;

    /**
     * @brief
     */
    virtual void SendExpanded() = 0;

    /**
     * @brief
     *
     * @param objReason
     */
    virtual void SendExpandFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief
     *
     * @param nReplaceKey
     */
    virtual void SendExpandedBy(IN IMS_SINTP nReplaceKey = 0) = 0;

    /**
     * @brief Notifies that the call is merged.
     *
     * @param lstConfUser Participants of the conference call.
     */
    virtual void SendMerged(IN const ImsList<ConfUser*>& lstConfUser) = 0;

    /**
     * @brief Notifies that the call is failed to be merged.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendMergeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that a participant is joined or failed to join to the conference call.
     *
     * @param nResult IMS_SUCCESS if successfully joined. Otherwise IMS_FAILURE.
     * @param objReason Reason of the failure. Only applicable if it's failed.
     */
    virtual void SendJoined(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that a participant is dropped or failed to drop from the conference call.
     *
     * @param nResult IMS_SUCCESS if successfully dropped. Otherwise IMS_FAILURE.
     * @param objReason Reason of the failure. Only applicable if it's failed.
     */
    virtual void SendDropped(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies that the conference participants are changed.
     *
     * @param lstConfUser Changed participants of the conference call.
     */
    virtual void SendNotifyUsersInfo(IN const ImsList<ConfUser*>& lstConfUser) = 0;

    /**
     * @brief Notifies that properties of the conference call is changed.
     *
     * @param strDisplayText
     * @param strSubject
     * @param nUserCount
     * @param nMaxUserCount
     * @param strHostEntity
     */
    virtual void SendNotifyConfInfo(IN const AString& strDisplayText, IN const AString& strSubject,
            IN IMS_SINT32 nUserCount, IN IMS_UINT32 nMaxUserCount,
            IN const AString& strHostEntity) = 0;

    /**
     * @brief
     *
     * @param nReplaceKey
     * @param nType
     */
    virtual void SendReplacedBy(IN IMS_SINTP nReplaceKey = 0, IN IMS_UINTP nType = 0) = 0;

    /**
     * @brief Notifies that the call transfer is successful or failed.
     *
     * @param nResult IMS_SUCCESS if it was successful. Otherwise IMS_FAILURE.
     * @param objReason Reason of the failure. Only applicable if it's failed.
     */
    virtual void SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief
     *
     * @param nResult
     * @param objReason
     */
    virtual void SendCallPushCompleted(
            IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;
};

#endif
