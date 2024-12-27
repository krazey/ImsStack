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
     * @brief Notifies that an incoming call is automatically rejected for call logs.
     *
     * @param objReason The reason causing this event.
     */
    virtual void SendIncomingCallRejected(IN const CallReasonInfo& objReason) = 0;

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
     * @brief Notifies that the call setup is initiating.
     *
     * This method is called after sending an INVITE request, indicating that the call setup process
     * has started.
     */
    virtual void SendInitiating() = 0;

    /**
     * @brief Notifies that the call setup is progressing.
     */
    virtual void SendProgressing() = 0;

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

    /**
     * @brief Notifies the RAT has changed.
     *
     * @param eRatType The changed RAT type.
     */
    virtual void SendRatChanged(IN IMS_SINT32 eRatType) = 0;
};

#endif
