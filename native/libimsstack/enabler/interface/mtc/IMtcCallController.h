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

#ifndef INTERFACE_MTC_CALL_CONTROLLER_H_
#define INTERFACE_MTC_CALL_CONTROLLER_H_

#include "IMtcService.h"
#include "INativeEnabler.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "call/IMtcCall.h"

class AString;
class IMtcCallContext;
class ISession;
class ISilentRedialHelper;
class SuppService;
enum class CallType;
enum class KeyType;
enum class SuppType;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;
union Key;

class IMtcCallController : public INativeEnabler
{
public:
    virtual ~IMtcCallController() {}

    /**
     * Creates a new outgoing call.
     *
     * @param eServiceType Service type of the new call.
     * @return The key of the new call.
     */
    virtual CallKey Open(IN ServiceType eServiceType, IN CallInfo& objCallInfo) = 0;

    /**
     * Sets an interface to interact with the Java layer.
     *
     * @param nCallKey Key of the call to be manipulated.
     */
    virtual void Attach(IN CallKey nCallKey) = 0;

    /**
     * Detaches the call associated with the specified call key.
     *
     * @param nCallKey Key of the call to be manipulated.
     */
    virtual void Detach(IN CallKey nCallKey) = 0;

    /**
     * Creates a call to handle the incoming call.
     *
     * @param pService Service of the incoming call.
     * @param piSession Session of the incoming call.
     */
    virtual void HandleIncoming(IN IMtcService* pService, IN ISession* piSession) = 0;

    /**
     * Sets the call information to start the outgoing call.
     *
     * @param nCallKey Key of the call to be manipulated.
     * @param eCallType Type of the call.
     * @param strTarget Remote target.
     * @param objMediaInfo Media of the call.
     * @param lstSuppServices Supplementary services.
     */
    virtual void Start(IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * Notifies the call that the user is alerted by the incoming call.
     *
     * @param nCallKey Key of the call to be manipulated.
     */
    virtual void HandleUserAlert(IN CallKey nCallKey) = 0;

    // Accepts an incoming call.
    // - nIMSKey: Key of the call to be manipulated.

    /**
     * @brief Accepts
     *
     * @param nCallKey
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void Accept(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Rejects an incoming call.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - objMediaInfo: `MediaInfo`

    /**
     * @brief Rejects
     *
     * @param nCallKey
     * @param objReason
     */
    virtual void Reject(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Holds a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - objMediaInfo: `MediaInfo`

    /**
     * @brief Holds
     *
     * @param nCallKey
     * @param objMediaInfo
     */
    virtual void Hold(IN CallKey nCallKey, IN MediaInfo& objMediaInfo) = 0;

    // Resumes a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - objMediaInfo: `MediaInfo`

    /**
     * @brief Resumes
     *
     * @param nCallKey
     * @param objMediaInfo
     */
    virtual void Resume(IN CallKey nCallKey, IN MediaInfo& objMediaInfo) = 0;

    // Accepts the resume request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - objMediaInfo: `MediaInfo`

    /**
     * @brief Accepts
     *
     * @param nCallKey
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void AcceptResume(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Rejects the resume request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Rejected reason.

    /**
     * @brief Rejects
     *
     * @param nCallKey
     * @param objReason
     */
    virtual void RejectResume(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Terminates a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Terminate reason.

    /**
     * @brief Terminates
     *
     * @param nCallKey
     * @param objReason
     */
    virtual void Terminate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Modifies media parameters of a call.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - objMediaInfo: `MediaInfo`

    /**
     * @brief Updates
     *
     * @param nCallKey
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void Update(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Cancels the ongoing modification request.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Canceled reason.

    /**
     * @brief Cancels
     *
     * @param nCallKey
     * @param objReason
     */
    virtual void CancelUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Accepts the modification request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - eSessionType: See MtcDef::SESSIONTYPE_*
    // - objMediaInfo: `MediaInfo`

    /**
     * @brief Accepts
     *
     * @param nCallKey
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void AcceptUpdate(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Rejects the modification request from the remote.
    // - nIMSKey: Key of the call to be manipulated.
    // - objReason: Rejected reason.

    /**
     * @brief Rejects
     *
     * @param nCallKey
     * @param objReason
     */
    virtual void RejectUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    // Sends USSD. Nothing happens if the specified call isn't a USSI session.
    // - nIMSKey: Key of the call to be manipulated.
    // - aStrUSSD: USSD string.

    /**
     * @brief Sends
     *
     * @param nCallKey
     * @param strUssd
     */
    virtual void SendUssd(IN CallKey nCallKey, IN const AString& strUssd) = 0;

    // Handles conference call related IMS messages.
    /*
    void StartGroupCall(IN CallKey nCallKey, IN IMS_UINT32 nCmd, IN ImsList<ConfUser*>& objUsers,
            IN CallInfo& objCallInfo, IN MediaInfo& objMediaInfo,
            IN ImsMap<SuppType, SuppService*>& objSuppServices);
    */

    /**
     * @brief Merges
     *
     * @param nCallKey
     * @param objUsers
     */
    virtual void MergeToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Adds
     *
     * @param nCallKey
     * @param objUsers
     */
    virtual void AddToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Removes
     *
     * @param nCallKey
     * @param objUsers
     */
    virtual void RemoveFromConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) = 0;

    // TODO: Consider ECT, SRVCC

    /**
     * @brief Transfers
     *
     * @param nCallKey
     * @param strTarget
     */
    virtual void Transfer(IN CallKey nCallKey, IN const AString& strTarget) = 0;

    /**
     * @brief Gets
     *
     * @param objContext
     * @param objReason
     * @return
     */
    virtual ISilentRedialHelper& GetRedialHelper(
            IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Releases
     *
     */
    virtual void ReleaseRedialHelper() = 0;
};

#endif
