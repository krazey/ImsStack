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
#include "SipMethod.h"

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

class IMtcCallContext : public IMtcContext
{
public:
    virtual ~IMtcCallContext(){};

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMS_UINTP GetCallKey() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsHeldByMe() const = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsUssi() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual CallInfo& GetCallInfo() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ParticipantInfo& GetParticipantInfo() = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @return
     */
    virtual IMtcSession* GetSession(IN const ISession* piSession) const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcSession* GetSession() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual const ImsList<IMtcSession*>& GetSessions() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcService& GetService() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcUiNotifier& GetUiNotifier() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcMediaManager& GetMediaManager() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcPreconditionManager& GetPreconditionManager() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual MtcTimerWrapper& GetTimer() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual MtcSupplementaryService& GetSupplementaryService() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual UpdatingInfo& GetUpdatingInfo() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual EpsFallbackTrigger& GetEpsFallbackTrigger() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual CurrentLocationDiscoveryController& GetCurrentLocationDiscoveryController() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual UssiController* GetUssiController() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual MtcPendingOperationHolder& GetPendingOperationHolder() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcCall& GetCall() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual ImsList<IMtcCall*> GetOtherCalls() = 0;
    /**
     * @brief Sets
     *
     * @param bHeldByMe
     */
    virtual void SetHeldByMe(IN IMS_BOOL bHeldByMe) = 0;

    /**
     * @brief Creates
     *
     * @param piSession
     * @return
     */
    virtual IMtcSession* CreateSession(IN ISession* piSession) = 0;

    /**
     * @brief Creates
     *
     * @return
     */
    virtual IMtcSession* CreateSession() = 0;

    /**
     * @brief Creates
     *
     * @param lstRules
     * @return
     */
    virtual IMtcBlockChecker* CreateBlockChecker(IN const ImsList<IMtcBlockRule*>& lstRules) = 0;

    /**
     * @brief Creates
     *
     * @return
     */
    virtual JniCallInfo CreateJniCallInfo() = 0;

    /**
     * @brief Creates
     *
     * @param eMethod
     * @return
     */
    virtual ISipClientConnection* CreateClientConnection(IN SipMethod eMethod) = 0;

    /**
     * Creates UdpKeepAliveSender with the current configuration and context.
     *
     * @return A new instance of UdpKeepAliveSender.
     */
    virtual UdpKeepAliveSender* CreateUdpKeepAliveSender() = 0;

    /**
     * @brief Removes
     *
     * @param piSession
     */
    virtual void RemoveSession(IN const ISession* piSession) = 0;

    /**
     * @brief Removes
     *
     * @param piActiveSession
     */
    virtual void RemoveInactiveSessions(IN const ISession* piActiveSession) = 0;

    /**
     * @brief Deletes
     *
     */
    virtual void DeleteUpdatingInfo() = 0;

    /**
     * @brief
     *
     */
    virtual void RunPendingOperationIfPossible() = 0;
};

#endif
