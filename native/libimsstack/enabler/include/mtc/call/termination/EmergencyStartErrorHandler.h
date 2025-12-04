/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef EMERGENCY_START_ERROR_HANDLER_H_
#define EMERGENCY_START_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
#include "IMessage.h"
#include "IPhoneInfoCall.h"
#include "ImsTypeDef.h"
#include "ServicePhoneInfo.h"
#include "SipStatusCode.h"
#include <optional>
#include <unordered_map>

class IMtcCallContext;
class ISession;
class MtcConfigurationProxy;

/*
 * It handles error responses for emergency routing emergency call when
 * `ISessionListener::SessionStartFailed` occurs
 */
class EmergencyStartErrorHandler final
{
public:
    explicit EmergencyStartErrorHandler(IN IMtcCallContext& objContext, IN ISession& objSession);
    ~EmergencyStartErrorHandler();
    EmergencyStartErrorHandler(const EmergencyStartErrorHandler&) = delete;
    EmergencyStartErrorHandler& operator=(const EmergencyStartErrorHandler&) = delete;

    /**
     * Returns `CallReasonInfo` for that allows AP DS module to perform domain reselection.
     *
     * @param objProxy MtcConfigurationProxy for reading configuration.
     * @param objReason This is the currently configured CallReasonInfo.
     * @return `CallReasonInfo.h` that will be overridden to perform domain reselection.
     */
    static std::optional<CallReasonInfo> MaybeGetOverriddenCallReasonInfo(
            IN const MtcConfigurationProxy& objProxy, IN const CallReasonInfo& objReason);

    /**
     * Returns `CallReasonInfo` for the incoming message.
     * Generally, it returns CODE_LOCAL_CALL_CS_RETRY_REQUIRED to allow the AP DS module to perform
     * domain reselection.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage) const;

private:
    static IMS_SINT32 GetDefaultReasonCode(
            IN const MtcConfigurationProxy& objProxy, IN IMS_SINT32 nStatusCode);
    static IMS_SINT32 GetStatusCode(IN const IMessage* piMessage)
    {
        return piMessage ? piMessage->GetStatusCode() : SipStatusCode::SC_INVALID;
    }

    CallReasonInfo HandleSilentReinviteWithNextPcscfIfEpdn(IN const IMessage* piMessage) const;
    CallReasonInfo HandleSilentReinviteByRetryAfter(IN const IMessage* piMessage) const;
    CallReasonInfo HandleSilentReinviteAsVoipByRttRejection(IN const IMessage* piMessage) const;
    CallReasonInfo HandleSilentReinviteWithAnonymousByNetworkRejection(
            IN const IMessage* piMessage) const;
    CallReasonInfo HandleSilentReinviteCrossSimByTempFailure(IN const IMessage* piMessage) const;
    CallReasonInfo HandleSilentReinviteCrossSimByPermFailure(IN const IMessage* piMessage) const;
    CallReasonInfo HandleTerminate(IN const IMessage* piMessage) const;
    CallReasonInfo HandleSilentReinviteToAlternatePcscfOnce(IN const IMessage* piMessage) const;

    IMS_SINT32 GetExtraCode(IN IMS_SINT32 nCode, IN const IMessage* piMessage) const;
    inline IMS_BOOL IsCrossSimRedialAvailable() const
    {
        return PhoneInfoService::GetPhoneInfoService()
                ->GetCallInfo(m_objContext.GetSlotId())
                ->IsCrossSimRedialingAvailable();
    }
    void ControlAos(IN IMS_UINT32 nCommand) const;

    typedef CallReasonInfo (EmergencyStartErrorHandler::*ActionFunc)(IN const IMessage*) const;
    static const std::unordered_map<IMS_SINT32, ActionFunc> objActionFuncMap;

    IMtcCallContext& m_objContext;
    ISession& m_objSession;
};

#endif
