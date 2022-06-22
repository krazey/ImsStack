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
#include "IMSTypeDef.h"
#include "IMtcCall.h"
#include "IMtcContext.h"
#include "JniCallInfo.h"

class IMtcBlockChecker;
class IMtcBlockRule;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class ISession;
class ISipClientConnection;
class MtcSession;
class MtcSupplementaryService;
class MtcTimerWrapper;
class MtcUiNotifier;
class ParticipantInfo;
class UpdatingInfo;
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
    virtual MtcSession* GetSession(IN const ISession* piSession) const = 0;
    virtual MtcSession* GetSession() const = 0;
    virtual IMtcService& GetService() = 0;
    virtual MtcUiNotifier& GetUiNotifier() = 0;
    virtual IMtcMediaManager& GetMediaManager() = 0;
    virtual IMtcPreconditionManager& GetPreconditionManager() = 0;
    virtual MtcTimerWrapper& GetTimer() = 0;
    virtual MtcSupplementaryService& GetSupplementaryService() = 0;
    virtual UpdatingInfo& GetUpdatingInfo() = 0;
    virtual UssiController* GetUssiController() = 0;
    virtual IMSList<IMtcCall*> GetOtherCalls() = 0;

    virtual void SetHeldByMe(IN IMS_BOOL bHeldByMe) = 0;

    virtual MtcSession* CreateSession(IN ISession* piSession) = 0;
    virtual MtcSession* CreateSession() = 0;
    virtual IMtcBlockChecker* CreateBlockChecker(IN const IMSList<IMtcBlockRule*>& lstRules) = 0;
    virtual JniCallInfo CreateJniCallInfo() = 0;
    virtual ISipClientConnection* CreateClientConnection(IN IMS_SINT32 nMethod) = 0;

    virtual void RemoveSession(IN const ISession* piSession) = 0;
    virtual void RemoveInactiveSessions(IN const ISession* piActiveSession) = 0;
    virtual void DeleteUpdatingInfo() = 0;
};

#endif
