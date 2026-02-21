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
#include "call/IMtcCall.h"
#include <functional>

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
template <class T>
class ImsList;
union Key;

/**
 * @brief This class defines the primary interface for controlling and managing {@code MtcCall}.
 *
 * It acts as a facade for all call-related operations, delegating actions to the appropriate call
 * objects and managers.
 */
class IMtcCallController : public INativeEnabler
{
public:
    /**
     * @brief Creates a new outgoing call.
     *
     * @param eServiceType Service type of the new call.
     * @param objCallInfo Call information.
     * @param strLogTag Log tag for the call.
     * @return The key of the new call.
     */
    virtual CallKey Open(
            IN ServiceType eServiceType, IN CallInfo& objCallInfo, IN const AString& strLogTag) = 0;

    /**
     * @brief Sets an interface to interact with the Java layer.
     *
     * This is typically called for an incoming call after the framework is ready to proceed with
     * handling it.
     *
     * @param nCallKey Key of the call to be attached.
     */
    virtual void Attach(IN CallKey nCallKey) = 0;

    /**
     * @brief Detaches the call associated with the specified call key.
     *
     * This is called when the call is terminated and its resources can be released.
     *
     * @param nCallKey Key of the call to be detached.
     */
    virtual void Detach(IN CallKey nCallKey) = 0;

    /**
     * @brief Creates a call to handle the incoming call.
     *
     * @param pService Service of the incoming call.
     * @param piSession Session of the incoming call.
     */
    virtual void HandleIncoming(IN IMtcService* pService, IN ISession* piSession) = 0;

    /**
     * @brief Sets the call information to start the outgoing call.
     *
     * @param nCallKey Key of the call to be started.
     * @param eCallType Type of the call.
     * @param strTarget Remote target.
     * @param objMediaInfo Media of the call.
     * @param lstSuppServices Supplementary services.
     */
    virtual void Start(IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Notifies the call that the user is alerted by the incoming call.
     *
     * @param nCallKey Key of the call being alerted.
     */
    virtual void HandleUserAlert(IN CallKey nCallKey) = 0;

    /**
     * @brief Accepts an incoming call.
     *
     * @param nCallKey Key of the call to be accepted.
     * @param eCallType The desired call type for the accepted call.
     * @param objMediaInfo The media information for the accepted call.
     */
    virtual void Accept(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects an incoming call.
     *
     * @param nCallKey Key of the call to be rejected.
     * @param objReason The reason for rejecting the call.
     */
    virtual void Reject(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Holds a call.
     *
     * @param nCallKey Key of the call to be held.
     * @param objMediaInfo The media information to be used for the hold operation.
     */
    virtual void Hold(IN CallKey nCallKey, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Resumes a call.
     *
     * @param nCallKey Key of the call to be resumed.
     * @param objMediaInfo The media information to be used for the resume operation.
     */
    virtual void Resume(IN CallKey nCallKey, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Accepts the resume request from the remote.
     *
     * @param nCallKey Key of the call for which resume is being accepted.
     * @param eCallType The call type after resuming.
     * @param objMediaInfo The media information for the resumed call.
     */
    virtual void AcceptResume(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects the resume request from the remote.
     *
     * @param nCallKey Key of the call for which resume is being rejected.
     * @param objReason The reason for rejecting the resume request.
     */
    virtual void RejectResume(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates a call.
     *
     * @param nCallKey Key of the call to be terminated.
     * @param objReason The reason for terminating the call.
     */
    virtual void Terminate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Modifies media parameters of a call.
     *
     * @param nCallKey Key of the call to be updated.
     * @param eCallType The new call type.
     * @param objMediaInfo The new media information.
     */
    virtual void Update(IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Cancels the ongoing modification request.
     *
     * @param nCallKey Key of the call for which the update is being canceled.
     * @param objReason The reason for canceling the update.
     */
    virtual void CancelUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Accepts the modification request from the remote.
     *
     * @param nCallKey Key of the call for which the update is being accepted.
     * @param eCallType The call type after the update.
     * @param objMediaInfo The media information for the updated call.
     */
    virtual void AcceptUpdate(
            IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects the modification request from the remote.
     *
     * @param nCallKey Key of the call for which the update is being rejected.
     * @param objReason The reason for rejecting the update.
     */
    virtual void RejectUpdate(IN CallKey nCallKey, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends a USSD.
     *
     * @param nCallKey Key of the call to send USSD over.
     * @param strUssd The USSD string to send.
     */
    virtual void SendUssd(IN CallKey nCallKey, IN const AString& strUssd) = 0;

    /**
     * @brief Merges one or more calls into a conference.
     *
     * @param nCallKey Key of the call that will host the conference.
     * @param objUsers List of users (and their calls) to be merged into the conference.
     */
    virtual void MergeToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Adds participants to an existing conference call.
     *
     * @param nCallKey Key of the conference call.
     * @param objUsers List of users to add to the conference.
     */
    virtual void AddToConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Removes participants from an existing conference call.
     *
     * @param nCallKey Key of the conference call.
     * @param objUsers List of users to remove from the conference.
     */
    virtual void RemoveFromConference(IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Transfers an ongoing call to a new target.
     *
     * @param nCallKey Key of the call to be transferred.
     * @param strTarget The URI of the transfer target.
     */
    virtual void Transfer(IN CallKey nCallKey, IN const AString& strTarget) = 0;

    /**
     * @brief Handles a BYE transaction for a call.
     *
     * This is used to perform an action (like registration recovery) after a BYE transaction
     * completes or times out.
     *
     * @param nCallKey Key of the call being terminated.
     * @param objOperation The operation to be executed when the BYE transaction is completed.
     */
    virtual void HandleByeTransaction(
            IN CallKey nCallKey, IN std::function<void(ISession&)> objOperation) = 0;

    /**
     * @brief Gets or creates a helper for handling silent redial scenarios.
     *
     * If a helper for the same redial type already exists, it is returned. Otherwise, a new one
     * is created.
     *
     * @param objContext The context of the call that needs redialing.
     * @param objReason The reason that triggered the redial.
     * @return A reference to the silent redial helper.
     */
    virtual ISilentRedialHelper& GetRedialHelper(
            IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Gets the currently active SilentRedialHelper instance.
     *
     * This function returns the pointer to the helper object that is currently managing a redial
     * sequence, without creating a new one.
     *
     * @return Pointer to ISilentRedialHelper if one exists, otherwise nullptr.
     */
    virtual const ISilentRedialHelper* GetActiveRedialHelper() const = 0;

    /**
     * @brief Releases the current silent redial helper instance.
     *
     * This should be called when the redial sequence is completed or no longer needed.
     */
    virtual void ReleaseRedialHelper() = 0;
};

#endif
