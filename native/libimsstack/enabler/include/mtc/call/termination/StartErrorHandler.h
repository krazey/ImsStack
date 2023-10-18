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

private:
    CallReasonInfo HandleTransactionTimeout() const;
    CallReasonInfo HandleResponse(IN const IMessage& objMessage) const;

    CallReasonInfo Handle3xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle380Response(IN const IMessage& objMessage) const;
    CallReasonInfo HandleRedirection(IN const IMessage& objMessage) const;

    CallReasonInfo Handle4xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle403Response(IN const IMessage& objMessage) const;
    CallReasonInfo Handle404Response() const;
    static CallReasonInfo Handle407Response();
    CallReasonInfo Handle488Response(IN const IMessage& objMessage) const;

    CallReasonInfo Handle5xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle500Response(IN const IMessage& objMessage) const;
    CallReasonInfo Handle503Response(IN const IMessage& objMessage) const;
    CallReasonInfo Handle504Response(IN const IMessage& objMessage) const;

    CallReasonInfo Handle6xxResponse(IN const IMessage& objMessage) const;

    CallReasonInfo HandleRedialByNetworkContext() const;

    CallReasonInfo HandleRedialEmergencyWithNextPcscf() const;

    IMS_SINT32 GetDefaultExtraCode(IN const IMessage& objMessage) const;
    static IMS_BOOL IsTransactionTimeout(IN const IMessage* piMessage);
    IMS_BOOL IsRetry1xRequiredForNormalCall(IN const IMessage& objMessage) const;
    static IMS_BOOL IsConditionCheckRequiredBeforeRetry1x(IN const IMessage& objMessage);
    IMS_BOOL IsNonUeDetectableEmergencyCall(IN const IMessage& objMessage) const;
    IMS_BOOL IsIpcanResourceUnavailable(IN const IMessage& objMessage) const;
    IMS_BOOL IsAlternativeEmergencyService(IN const IMessage& objMessage) const;
    IMS_BOOL IsInitialRegistrationRequired(IN const IMessage& objMessage) const;
    IMS_BOOL IsByMaxCallLimit(IN const IMessage& objMessage) const;
    IMS_BOOL IsRedialEmergencyWithNextPcscfRequired(IN const IMessage* piMessage) const;
    IMS_BOOL IsRoaming() const;
    IMS_BOOL IsEpsOnlyAttach() const;

    void ControlAos(IMS_UINT32 nCommand) const;
    AString GetPathHeader() const;
    AString GetServiceRouteHeader() const;
    AString GetSupported() const;

    IMtcCallContext& m_objContext;
    ISession& m_objSession;
};

#endif
