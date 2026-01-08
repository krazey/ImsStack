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

#ifndef INTERFACE_MTC_SESSION_H_
#define INTERFACE_MTC_SESSION_H_

#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/message/IMtcMessageHandler.h"

class IMessage;
class ISession;
class MessageSender;
class MtcExtensionSet;
struct CallReasonInfo;

class IMtcSession : public IMtcMessageHandler
{
public:
    /**
     * @brief Starts
     *
     * @return
     */
    virtual IMS_RESULT Start() = 0;

    /**
     * @brief Sends
     *
     * @param bUserAlert
     * @return
     */
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_BOOL bUserAlert, IN IMS_BOOL bReliable) = 0;

    /**
     * @brief Sends
     *
     * @return
     */
    virtual IMS_RESULT SendPrack(IN IMS_BOOL bSdpOfferRequired) = 0;

    /**
     * @brief Responds
     *
     * @param eStatusCode
     * @return
     */
    virtual IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Sends
     *
     * @param eUpdateType
     * @return
     */
    virtual IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) = 0;

    /**
     * @brief Responds
     *
     * @param eStatusCode
     * @return
     */
    virtual IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Sends
     *
     * @return
     */
    virtual IMS_RESULT SendAck() = 0;

    /**
     * @brief Accepts
     *
     * @return
     */
    virtual IMS_RESULT Accept() = 0;

    /**
     * @brief Rejects
     *
     * @param objReason
     * @return
     */
    virtual IMS_RESULT Reject(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Updates
     *
     * @param eUpdateType
     * @param bIncludeAlertInfo
     * @param eMethod
     * @return
     */
    virtual IMS_RESULT Update(
            IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod) = 0;

    /**
     * @brief Accepts
     *
     * @return
     */
    virtual IMS_RESULT AcceptUpdate() = 0;

    /**
     * @brief Cancels
     *
     * @param objReason
     * @return
     */
    virtual IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates
     *
     * @param CallReasonInfo&
     * @return
     */
    virtual IMS_RESULT Terminate(IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Marks the session as terminated or start-failed.
     *
     * This method is called to update the session's state when it is no longer active,
     * specifically in the following cases:
     * - ISessionListener#SessionTerminated() is invoked.
     * - ISessionListener#SessionStartFailed() is invoked.
     */
    virtual void SetSessionTerminatedOrStartFailed() = 0;

    /**
     * @brief Sets the current CallType with the input param.
     *
     * @param eCallType The CallType to set.
     */
    virtual void SetCallType(IN CallType eCallType) = 0;

    /**
     * @brief Sets the call type after adjusting it based on the session's capabilities.
     *
     * @param eCallType The CallType to set. It's downgraded if the session doesn't support the
     *                  required features.
     */
    virtual void SetCapableCallType(IN CallType eCallType) = 0;

    /**
     * @brief Gets the current CallType.
     *
     * @return The current CallType.
     *         If it's after handling an incoming SIP message, it will be from the SIP message.
     *         If #SetCallType is invoked previously and no handling of incoming SIP message after
     *         that, it will be same as the input param of #SetCallType.
     */
    virtual CallType GetCallType() const = 0;

    /**
     * @brief Gets the previous CallType.
     *
     * @return The previous CallType.
     */
    virtual CallType GetPreviousCallType() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ISession& GetISession() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual MtcExtensionSet& GetExtensionSet() = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsVideoCapable() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsRttCapable() const = 0;

    /**
     * @brief Checks if there's a pending PRACK transaction.
     *
     * @return True when it hasn't sent a PRACK for 183 or received a PRACK_RESPONSE for the PRACK.
     */
    virtual IMS_BOOL IsPrackPending() const = 0;

    /**
     * @brief Gets the UpdateType of Early UPDATE previously sent and not succeeded yet.
     *
     * @return The UpdateType. If no previous Early UPDATE sent and not succeeded yet exists,
     *         UpdateType::NONE.
     */
    virtual UpdateType GetOngoingUpdateType() const = 0;
};

#endif
