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

#ifndef UPDATE_ERROR_HANDLER_H_
#define UPDATE_ERROR_HANDLER_H_

#include "CallReasonInfo.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IMessage;
class IMtcCallContext;
class SipMethod;

/*
 * It handles error responses when `ISessionListener::SessionUpdateFailed` occurs after the call
 * establishing.
 */
class UpdateErrorHandler final
{
public:
    explicit UpdateErrorHandler(IN IMtcCallContext& objContext);
    ~UpdateErrorHandler();
    UpdateErrorHandler(const UpdateErrorHandler&) = delete;
    UpdateErrorHandler& operator=(const UpdateErrorHandler&) = delete;

    /**
     * Returns `CallReasonInfo` for the incoming message.
     *
     * @param piMessage Received error response. Could be null if no response has came.
     * @return See `CallReasonInfo.h` for the possible values.
     */
    CallReasonInfo Handle(IN const IMessage* piMessage) const;
    static IMS_UINT32 GetGlareTimeMillisecond(IN PeerType ePeerType);

private:
    CallReasonInfo HandleResponse(IN const IMessage& objMessage) const;
    static CallReasonInfo Handle3xxResponse(IN const IMessage& objMessage);
    CallReasonInfo Handle4xxResponse(IN const IMessage& objMessage) const;
    CallReasonInfo Handle5xxResponse(IN const IMessage& objMessage) const;
    static CallReasonInfo Handle6xxResponse(IN const IMessage& objMessage);
    CallReasonInfo Handle503Response(IN const IMessage& objMessage) const;
    void RegisterFor503(IN IMS_SINT32 nRetryAfter) const;
    IMS_BOOL IsRegisterWithNextPcscfRequiredFor503(
            IN IMS_SINT32 nRetryAfter, IN const SipMethod& objMethod) const;

    IMtcCallContext& m_objContext;
};

#endif
