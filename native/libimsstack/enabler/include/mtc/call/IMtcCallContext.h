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

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "IMtcCall.h"
#include "IMtcContext.h"
#include "JniCallInfo.h"
#include "sipcore/SipMethod.h"

class EpsFallbackTrigger;
class IMtcBlockChecker;
class IMtcBlockRule;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class ISession;
class ISipClientConnection;
class IMtcSession;
class MtcSupplementaryService;
class MtcTimerWrapper;
class IMtcUiNotifier;
class ParticipantInfo;
class UpdatingInfo;
class UdpKeepAliveSender;
class UssiController;

class IMtcCallContext : public IMtcContext
{
public:
    virtual ~IMtcCallContext(){};

    virtual IMS_UINTP GetCallKey() const = 0;
    virtual IMS_BOOL IsHeldByMe() const = 0;
    virtual IMS_BOOL IsUssi() const = 0;

    virtual CallInfo& GetCallInfo() = 0;
    virtual ParticipantInfo& GetParticipantInfo() = 0;
    virtual IMtcSession* GetSession(IN const ISession* piSession) const = 0;
    virtual IMtcSession* GetSession() const = 0;
    virtual IMtcService& GetService() = 0;
    virtual IMtcUiNotifier& GetUiNotifier() = 0;
    virtual IMtcMediaManager& GetMediaManager() = 0;
    virtual IMtcPreconditionManager& GetPreconditionManager() = 0;
    virtual MtcTimerWrapper& GetTimer() = 0;
    virtual MtcSupplementaryService& GetSupplementaryService() = 0;
    virtual UpdatingInfo& GetUpdatingInfo() = 0;
    virtual EpsFallbackTrigger& GetEpsFallbackTrigger() = 0;
    virtual UdpKeepAliveSender& GetUdpKeepAliveSender() = 0;
    virtual UssiController* GetUssiController() = 0;
    virtual ImsList<IMtcCall*> GetOtherCalls() = 0;

    virtual void SetHeldByMe(IN IMS_BOOL bHeldByMe) = 0;

    virtual IMtcSession* CreateSession(IN ISession* piSession) = 0;
    virtual IMtcSession* CreateSession() = 0;
    virtual IMtcBlockChecker* CreateBlockChecker(IN const ImsList<IMtcBlockRule*>& lstRules) = 0;
    virtual JniCallInfo CreateJniCallInfo() = 0;
    virtual ISipClientConnection* CreateClientConnection(IN SipMethod eMethod) = 0;

    virtual void RemoveSession(IN const ISession* piSession) = 0;
    virtual void RemoveInactiveSessions(IN const ISession* piActiveSession) = 0;
    virtual void DeleteUpdatingInfo() = 0;
};

#endif
