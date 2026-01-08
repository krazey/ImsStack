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

#ifndef EARLY_UPDATE_ERROR_HANDLER_H_
#define EARLY_UPDATE_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"
#include <unordered_map>

class IMessage;
class IMtcCallContext;

/*
 * It handles error responses when `ISessionListener::SessionUpdateFailed` occurs before the call
 * establishing.
 */
class EarlyUpdateErrorHandler final
{
public:
    explicit EarlyUpdateErrorHandler(IN IMtcCallContext& objContext);
    ~EarlyUpdateErrorHandler();
    EarlyUpdateErrorHandler(const EarlyUpdateErrorHandler&) = delete;
    EarlyUpdateErrorHandler& operator=(const EarlyUpdateErrorHandler&) = delete;

    /**
     * Returns `CallReasonInfo` for the incoming message.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage);

private:
    CallReasonInfo HandleTerminateDialog(IN const IMessage* piMessage) const;
    CallReasonInfo HandleTerminateCall(IN const IMessage* piMessage) const;
    CallReasonInfo HandleGlareCondition(IN const IMessage* piMessage) const;
    CallReasonInfo HandleBlockCallByTimer(IN const IMessage* piMessage) const;
    CallReasonInfo HandleTimeout(IN const IMessage* piMessage) const;
    CallReasonInfo HandleRegistrationRestoration(IN const IMessage* piMessage) const;

    void ControlAos(IN IMS_UINT32 nCommand) const;
    IMS_BOOL RegisterFor503(IN IMS_SINT32 nRetryAfter) const;
    IMS_BOOL IsRegisterWithNextPcscfAndRedialRequiredFor503(IN IMS_SINT32 nRetryAfter) const;
    void SetTimerForImsCallBlocking(IN IMS_SINT32 nRetryAfterInMillis) const;

    typedef CallReasonInfo (EarlyUpdateErrorHandler::*ActionFunc)(IN const IMessage*) const;
    static const std::unordered_map<IMS_SINT32, ActionFunc> objActionFuncMap;

    IMtcCallContext& m_objContext;
    IMS_SINT32 m_eStatusCode;
};

#endif
