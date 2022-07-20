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
#include "call/IMtcCall.h"

class IMessage;
class ISession;
class MessageSender;
class MtcExtensionSet;
struct CallReasonInfo;

class IMtcSession
{
public:
    virtual ~IMtcSession() {}

    virtual IMS_RESULT Start() = 0;
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_BOOL bUserAlert) = 0;
    virtual IMS_RESULT SendPrack() = 0;
    virtual IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) = 0;
    virtual IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) = 0;
    virtual IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) = 0;
    virtual IMS_RESULT SendAck() = 0;
    virtual IMS_RESULT Accept() = 0;
    virtual IMS_RESULT Reject(IN const CallReasonInfo& objReason) = 0;
    virtual IMS_RESULT Update(
            IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod) = 0;
    virtual IMS_RESULT AcceptUpdate() = 0;
    virtual IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) = 0;
    virtual IMS_RESULT Terminate(IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) = 0;

    virtual void HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest) = 0;
    virtual void HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse) = 0;

    virtual void SetCallType(IN CallType eCallType) = 0;
    virtual CallType GetCallType() const = 0;
    virtual ISession& GetISession() = 0;
    virtual MtcExtensionSet& GetExtensionSet() = 0;
    virtual IMS_BOOL IsVideoCapable() const = 0;
    virtual IMS_BOOL IsRttCapable() const = 0;
};

#endif
