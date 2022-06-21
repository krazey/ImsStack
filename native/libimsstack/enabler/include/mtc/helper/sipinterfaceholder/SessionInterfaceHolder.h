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

#ifndef SESSION_INTERFACE_HOLDER_H_
#define SESSION_INTERFACE_HOLDER_H_

#include "ServiceTimer.h"
#include "IMSList.h"
#include "IMSMap.h"
#include "ISessionListener.h"

class ISession;
class ICoreService;
class IInterfaceHolderListener;

class SessionInterfaceHolder final : public ISessionListener, public ITimerListener
{
public:
    explicit SessionInterfaceHolder(IN IInterfaceHolderListener& objListener);
    ~SessionInterfaceHolder();
    SessionInterfaceHolder(IN const SessionInterfaceHolder&) = delete;
    SessionInterfaceHolder& operator=(IN const SessionInterfaceHolder&) = delete;

public:
    // ISessionListener interface implementation
    inline void SessionAlerting(IN ISession*) override {}
    inline void SessionReferenceReceived(IN ISession*, IN IReference*) override {}
    inline void SessionStarted(IN ISession*) override {}
    inline void SessionStartFailed(IN ISession*) override {}
    void SessionTerminated(IN ISession* piSession) override;
    inline void SessionUpdated(IN ISession*) override {}
    inline void SessionUpdateFailed(IN ISession*) override {}
    inline void SessionUpdateReceived(IN ISession*) override {}
    inline void SessionCancelDelivered(IN ISession*) override {}
    inline void SessionCancelDeliveryFailed(IN ISession*) override {}
    inline void SessionEarlyMediaUpdated(IN ISession*) override {}
    inline void SessionEarlyMediaUpdateFailed(IN ISession*) override {}
    inline void SessionEarlyMediaUpdateReceived(IN ISession*) override {}
    inline void SessionForkedResponseReceived(IN ISession*, IN ISession*) override {}
    inline void SessionPrackDelivered(IN ISession*) override {}
    inline void SessionPrackDeliveryFailed(IN ISession*) override {}
    inline void SessionPrackReceived(IN ISession*) override {}
    inline void SessionProvisionalResponseReceived(IN ISession*, IN IMS_UINT32) override {}
    inline void SessionRprDeliveryFailed(IN ISession*) override {}
    inline void SessionRprReceived(IN ISession*, IN IMS_UINT32) override {}
    inline void SessionTransactionReceived(IN ISession*, IN ISipServerConnection*) override {}

    // ITimerListener interface implementation.
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    ISession* GetISession(
            IN ICoreService* pICoreService, IN const AString& strFrom, IN const AString& strTo);
    void AddISession(IN ISession* piSession);
    void ReleaseISession(IN ISession* piSession, IN IMS_BOOL bTerminated = IMS_FALSE);

private:
    IMS_BOOL IsReadyToDestroy(IN ISession* piSession);

    void ClearISessions();

    IMS_RESULT StartTimer(IN ISession* piSession, IN IMS_SINT32 nDuration);
    void StopTimer(IN ITimer* piTimer);
    ITimer* GetTimer(IN ISession* piSession);

private:
    IInterfaceHolderListener& m_objListener;
    IMSList<ISession*> m_objISessions;
    IMSMap<ITimer*, ISession*> m_objTerminatedGuardTimers;

    static const IMS_UINT32 TIME_TRANSACTION_TERMINATED_GUARD = 128000;
};

#endif
