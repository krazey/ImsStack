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

    /**
     * @brief Sends
     *
     * @param nKey
     */
    virtual void SendPreIncomingCallReceived(IN CallKey nKey) = 0;

    /**
     * @brief Sends
     *
     * @param nKey
     * @param objCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param objParticipantInfo
     */
    virtual void SendIncomingCallReceived(IN CallKey nKey, IN CallInfo& objCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ParticipantInfo& objParticipantInfo) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendStarted(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendStartFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param bAlerted
     */
    virtual void SendProgressing(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_BOOL bAlerted = IMS_FALSE) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendHeld(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendHoldFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendResumed(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendResumeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendHeldBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendResumedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendTerminated(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendIncomingResume(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param eCallTypeToUpdate
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendIncomingUpdate(IN CallType eCallTypeToUpdate, IN CallInfo* pCallInfo,
            IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendUpdated(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendUpdateFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendUpdatedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param eType
     * @param strValue
     * @param IMS_SINT32
     * @param -1
     * @param bValue
     */
    virtual void SendNotifyInfo(IN IMS_UINT32 eType, IN AString strValue = AString::ConstNull(),
            IN IMS_SINT32 nValue = -1, IN IMS_BOOL bValue = IMS_FALSE) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void SendExpanded(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendExpandFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param nReplaceKey
     */
    virtual void SendExpandedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN IMS_SINTP nReplaceKey = 0) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param lstConfUser
     */
    virtual void SendMerged(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN ImsList<ConfUser*>& lstConfUser) = 0;

    /**
     * @brief Sends
     *
     * @param objReason
     */
    virtual void SendMergeFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param bResult
     * @param objReason
     */
    virtual void SendJoined(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param bResult
     * @param objReason
     */
    virtual void SendDropped(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param lstConfUser
     */
    virtual void SendNotifyUsersInfo(IN ImsList<ConfUser*>& lstConfUser) = 0;

    /**
     * @brief Sends
     *
     * @param strDisplayText
     * @param strSubject
     * @param nMaxUserCount
     * @param nUserCount
     * @param strHostEntity
     */
    virtual void SendNotifyConfInfo(IN AString strDisplayText, IN AString strSubject,
            IN IMS_SINT32 nMaxUserCount, IN IMS_UINT32 nUserCount, IN AString strHostEntity) = 0;

    /**
     * @brief Sends
     *
     * @param pCallInfo
     * @param objMediaInfo
     * @param objSuppServices
     * @param nReplaceKey
     * @param IMS_UINTP
     * @param 0
     */
    virtual void SendReplacedBy(IN CallInfo* pCallInfo, IN const MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices, IN IMS_SINTP nReplaceKey = 0,
            IN IMS_UINTP nType = 0) = 0;

    /**
     * @brief Sends
     *
     * @param nResult
     * @param objReason
     */
    virtual void SendEctCompleted(IN IMS_RESULT nResult, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @param bResult
     * @param objReason
     */
    virtual void SendCallPushCompleted(IN IMS_BOOL bResult, IN const CallReasonInfo& objReason) = 0;
};

#endif
