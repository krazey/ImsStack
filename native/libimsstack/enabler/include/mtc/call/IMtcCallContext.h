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

#ifndef INTERFACE_MTC_CALL_CONTEXT_H_
#define INTERFACE_MTC_CALL_CONTEXT_H_

#include "IMtcContext.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "JniCallInfo.h"

class CurrentLocationDiscoveryController;
class EpsFallbackTrigger;
class IMtcBlockChecker;
class IMtcBlockRule;
class IMtcCall;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class IMtcSession;
class IMtcUiNotifier;
class ISession;
class ISipClientConnection;
class MtcPendingOperationHolder;
class MtcSupplementaryService;
class MtcTimerWrapper;
class ParticipantInfo;
class UdpKeepAliveSender;
class UpdatingInfo;
class UssiController;

/**
 * Provides call-specific context, extending the general {@link IMtcContext}.
 * This interface offers access to objects and states relevant to a single call instance.
 */
class IMtcCallContext : public IMtcContext
{
public:
    virtual ~IMtcCallContext() override {};

    /**
     * @brief Gets the unique key identifying the call.
     *
     * @return The unique call key.
     */
    virtual IMS_UINTP GetCallKey() const = 0;

    /**
     * @brief Checks if the call has been established.
     *
     * @return True if the call has been in the ESTABLISHED state, false otherwise.
     */
    virtual IMS_BOOL IsEstablished() const = 0;

    /**
     * @brief Checks if the call is currently held by the local user.
     *
     * @return True if the call is held by the local user, false otherwise.
     */
    virtual IMS_BOOL IsHeldByMe() const = 0;

    /**
     * @brief Checks if the call is in an unconfirmed remote hold state.
     *
     * When the UE requests a hold, some networks respond with "inactive" media
     * direction. In this scenario, the UE cannot determine if the remote party is also on
     * hold. This flag indicates that ambiguous state.
     *
     * @return True if the call is in an unconfirmed remote hold state, false otherwise.
     */
    virtual IMS_BOOL IsOnUnconfirmedRemoteHold() const = 0;

    /**
     * @brief Checks if the call is a USSI call.
     *
     * @return True if it is a USSI call, false otherwise.
     */
    virtual IMS_BOOL IsUssi() const = 0;

    /**
     * @brief Checks if CSFB is available based on the UE's network status, carrier
     *        configuration and call status.
     *
     * This method evaluates the UE's network conditions, including NR, EPS Attach Type, Roaming,
     * and WiFi status, against the CSFB block conditions specified in the carrier configuration
     * {@link ConfigVoice#KEY_CSFB_BLOCK_CONDITION_INT_ARRAY}.
     *
     * If there's an established call already, it returns false in order to maintain the call.
     *
     * @return True if CSFB is available based on conditions. False otherwise.
     */
    virtual IMS_BOOL IsCsfbAvailable() = 0;

    /**
     * @brief Gets a reference to the call's information structure.
     *
     * @return A reference to the CallInfo object.
     */
    virtual CallInfo& GetCallInfo() = 0;

    /**
     * @brief Gets a reference to the call's participant information.
     *
     * @return A reference to the {@link ParticipantInfo} object.
     */
    virtual ParticipantInfo& GetParticipantInfo() = 0;

    /**
     * @brief Gets the MTC session associated with a given core session object.
     *
     * @param piSession A pointer to the core ISession object.
     * @return A pointer to the corresponding {@link IMtcSession}, or nullptr if not found.
     */
    virtual IMtcSession* GetSession(IN const ISession* piSession) const = 0;

    /**
     * @brief Gets the primary MTC session for the call.
     *
     * @return A pointer to the primary {@link IMtcSession}.
     */
    virtual IMtcSession* GetSession() const = 0;

    /**
     * @brief Gets a list of all MTC sessions associated with this call.
     *
     * There can be one or more sessions by forking if the call is in the early state.
     *
     * @return A constant reference to a list of {@link IMtcSession} pointers.
     */
    virtual const ImsList<IMtcSession*>& GetSessions() const = 0;

    /**
     * @brief Gets the MTC service associated with this call.
     *
     * @return A reference to the {@link IMtcService} object.
     */
    virtual IMtcService& GetService() const = 0;

    /**
     * @brief Gets the UI notifier for this call.
     *
     * @return A reference to the {@link IMtcUiNotifier} object.
     */
    virtual IMtcUiNotifier& GetUiNotifier() = 0;

    /**
     * @brief Gets the media manager for this call.
     *
     * @return A reference to the {@link IMtcMediaManager} object.
     */
    virtual IMtcMediaManager& GetMediaManager() = 0;

    /**
     * @brief Gets the precondition manager for this call.
     *
     * @return A reference to the {@link IMtcPreconditionManager} object.
     */
    virtual IMtcPreconditionManager& GetPreconditionManager() = 0;

    /**
     * @brief Gets the timer wrapper associated with this call.
     *
     * @return A reference to the {@link MtcTimerWrapper} object.
     */
    virtual MtcTimerWrapper& GetTimer() const = 0;

    /**
     * @brief Gets the supplementary service handler for this call.
     *
     * @return A reference to the {@link MtcSupplementaryService} object.
     */
    virtual MtcSupplementaryService& GetSupplementaryService() = 0;

    /**
     * @brief Gets information related to an ongoing call update.
     *
     * @return A reference to the {@link UpdatingInfo} object.
     */
    virtual UpdatingInfo& GetUpdatingInfo() = 0;

    /**
     * @brief Saves the current update information to be used for retrying after a glare condition.
     *
     * This is necessary to prevent the loss of {@link UpdatingInfo} when processing an update from
     * the peer while the {@link MtcCallState::TIMER_RETRY_UPDATE} timer is running due to a glare
     * condition. Therefore, it is not used for a simple glare condition.
     *
     * Note: Calling this function will set the main {@link UpdatingInfo} object to null,
     * as its content is moved for stashing.
     */
    virtual void StashUpdatingInfoInGlare() = 0;

    /**
     * @brief Gets the saved update information for glare condition retry.
     *
     * @return A pointer to the {@link UpdatingInfo} object if saved, nullptr otherwise.
     */
    virtual UpdatingInfo* GetStashedUpdatingInfoInGlare() = 0;

    /**
     * @brief Clears the saved update information for glare condition.
     */
    virtual void ClearStashedUpdatingInfoInGlare() = 0;

    /**
     * @brief Gets the EPS fallback trigger for this call.
     *
     * @return A reference to the {@link EpsFallbackTrigger} object.
     */
    virtual EpsFallbackTrigger& GetEpsFallbackTrigger() = 0;

    /**
     * @brief Gets the controller for publishing the current location.
     *
     * @return A reference to the {@link CurrentLocationDiscoveryController} object.
     */
    virtual CurrentLocationDiscoveryController& GetCurrentLocationDiscoveryController() = 0;

    /**
     * @brief Gets the controller for USSI operations.
     *
     * @return A pointer to the {@link UssiController} object.
     */
    virtual UssiController* GetUssiController() = 0;

    /**
     * @brief Gets the holder for pending operations on this call.
     *
     * @return A reference to the {@link MtcPendingOperationHolder} object.
     */
    virtual MtcPendingOperationHolder& GetPendingOperationHolder() = 0;

    /**
     * @brief Gets the MTC call object itself.
     *
     * @return A reference to the {@link IMtcCall} object.
     */
    virtual IMtcCall& GetCall() = 0;

    /**
     * @brief Gets a list of all other MTC calls.
     *
     * @return A list of pointers to other {@link IMtcCall} objects.
     */
    virtual ImsList<IMtcCall*> GetOtherCalls() = 0;

    /**
     * @brief Sets the held status of the call from the local user's perspective.
     *
     * @param bHeldByMe {@code IMS_TRUE} to indicate the call is held by the local user,
     *                  {@code IMS_FALSE} otherwise.
     */
    virtual void SetHeldByMe(IN IMS_BOOL bHeldByMe) = 0;

    /**
     * @brief Sets the unconfirmed remote hold status for the call.
     *
     * @param bUnconfirmedRemoteHold {@code IMS_TRUE} to indicate the call is in an unconfirmed
     *                               remote hold state, {@code IMS_FALSE} otherwise.
     * @see IsOnUnconfirmedRemoteHold
     */
    virtual void SetUnconfirmedRemoteHold(IN IMS_BOOL bUnconfirmedRemoteHold) = 0;

    /**
     * @brief Sets whether the call had an INVITE transaction timeout.
     *
     * @param bHadTimeout IMS_TRUE if the transaction timeout occurred, IMS_FALSE otherwise.
     */
    virtual void SetHadInviteTransactionTimeout(IN IMS_BOOL bHadTimeout) = 0;

    /**
     * @brief Checks if the call had an INVITE transaction timeout.
     *
     * @return IMS_TRUE if the transaction timeout occurred, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL HadInviteTransactionTimeout() const = 0;

    /**
     * @brief Creates a new MTC session from a core session object.
     *
     * @param piSession A pointer to the core ISession to wrap.
     * @return A pointer to the newly created {@link IMtcSession}.
     */
    virtual IMtcSession* CreateSession(IN ISession* piSession) = 0;

    /**
     * @brief Creates a new MTC session from a new core session object.
     *
     * @return A pointer to the newly created {@link IMtcSession}.
     */
    virtual IMtcSession* CreateSession() = 0;

    /**
     * @brief Creates a block checker for the call.
     *
     * @param lstRules A list of {@link IMtcBlockRule}s to apply.
     * @return A pointer to the newly created {@link IMtcBlockChecker}.
     */
    virtual IMtcBlockChecker* CreateBlockChecker(IN const ImsList<IMtcBlockRule*>& lstRules) = 0;

    /**
     * @brief Creates a JNI-compatible call information object.
     *
     * @return A {@link JniCallInfo} object populated with this call's data.
     */
    virtual JniCallInfo CreateJniCallInfo() = 0;

    /**
     * Creates a keep alive sender with the current configuration and context.
     *
     * @return A new instance of {@link UdpKeepAliveSender}.
     */
    virtual UdpKeepAliveSender* CreateUdpKeepAliveSender() = 0;

    /**
     * @brief Removes the given MtcSession.
     *
     * @param objSession MtcSession to remove.
     */
    virtual void RemoveSession(IN IMtcSession& objSession) = 0;

    /**
     * @brief Removes all the MtcSessions.
     */
    virtual void RemoveAllSessions() = 0;

    /**
     * @brief Deletes the information related to an ongoing call update.
     */
    virtual void DeleteUpdatingInfo() = 0;

    /**
     * @brief Executes a pending operation (e.g., hold, resume) if the current call state allows it.
     */
    virtual void RunPendingOperationIfPossible() = 0;
};

#endif
