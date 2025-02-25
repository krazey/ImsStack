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

#ifndef START_ERROR_HANDLER_H_
#define START_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"
#include <unordered_map>

class IMessage;
class IMtcCallContext;
class ISession;

/*
 * It handles error responses when `ISessionListener::SessionStartFailed` occurs.
 */
class StartErrorHandler final
{
public:
    explicit StartErrorHandler(IN IMtcCallContext& objContext, IN ISession& objSession);
    ~StartErrorHandler();
    StartErrorHandler(const StartErrorHandler&) = delete;
    StartErrorHandler& operator=(const StartErrorHandler&) = delete;

    /**
     * Returns `CallReasonInfo` for the incoming message. Possibly contains some extra logic,
     * but retrying is not handled here.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage) const;

    static CallReasonInfo GetDefaultCallReasonInfo(
            IN IMtcCallContext& objContext, IN const IMessage& objMessage);
    static IMS_SINT32 GetDefaultReasonCode(
            IN IMtcCallContext& objContext, IN IMS_SINT32 nStatusCode);
    static IMS_SINT32 GetDefaultExtraCode(
            IN IMtcCallContext& objContext, IN const IMessage& objMessage);

private:
    CallReasonInfo HandleTransactionTimeout() const;
    CallReasonInfo HandleCsfb(IN const IMessage& objMessage) const;
    CallReasonInfo HandleSilentReinvite(IN const IMessage& objMessage) const;
    CallReasonInfo HandleSilentReinviteBySdpContent(IN const IMessage& objMessage) const;
    CallReasonInfo HandleSilentReinviteByRetryAfter(IN const IMessage& objMessage) const;
    CallReasonInfo HandleRegistrationRestorationOnIms3gppByPolicy(
            IN const IMessage& objMessage) const;
    CallReasonInfo HandleRedirectionByContact(IN const IMessage& objMessage) const;
    CallReasonInfo HandleNonUeDetectableEmergencyCall(IN const IMessage& objMessage) const;
    CallReasonInfo HandleForbiddenByPolicy(IN const IMessage& objMessage) const;
    CallReasonInfo HandleTerminateByReasonPhrase(IN const IMessage& objMessage) const;
    CallReasonInfo HandleUssiCsfb(IN const IMessage& objMessage) const;
    CallReasonInfo HandleBlockCallByTimer(IN const IMessage& objMessage) const;
    CallReasonInfo HandleTriggerEpsfb(IN const IMessage& objMessage) const;
    CallReasonInfo HandleTerminateByResponseSource(IN const IMessage& objMessage) const;
    CallReasonInfo HandleTerminateByReasonHeaderText(IN const IMessage& objMessage) const;

    CallReasonInfo RegisterAfterMayPerformCsfb() const;

    static IMS_BOOL IsTransactionTimeout(IN const IMessage* piMessage);
    IMS_BOOL IsIpcanResourceUnavailable(IN const IMessage& objMessage) const;
    IMS_BOOL IsAlternativeEmergencyService(IN const IMessage& objMessage) const;
    IMS_BOOL IsInitialRegistrationRequired(IN const IMessage& objMessage) const;
    IMS_BOOL IsRoaming() const;
    IMS_BOOL IsCsfbActionRequiredStatusCode(IN IMS_SINT32 nStatusCode) const;

    void ControlAos(IN IMS_UINT32 nCommand) const;
    IMS_BOOL RegisterFor503(IN IMS_SINT32 nRetryAfter) const;
    IMS_BOOL IsRegisterWithNextPcscfAndRedialRequiredFor503(IN IMS_SINT32 nRetryAfter) const;
    AString GetPathHeader() const;
    AString GetServiceRouteHeader() const;
    AString GetSupported() const;
    void SetTimerForImsCallBlocking(IN IMS_SINT32 nRetryAfterInMillis) const;

    typedef CallReasonInfo (StartErrorHandler::*ActionFunc)(IN const IMessage&) const;
    static const std::unordered_map<IMS_SINT32, ActionFunc> objActionFuncMap;

    IMtcCallContext& m_objContext;
    ISession& m_objSession;
};

#endif
