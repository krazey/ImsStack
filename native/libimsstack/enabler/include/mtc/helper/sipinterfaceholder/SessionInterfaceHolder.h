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

#include "ISessionListener.h"
#include "ImsList.h"
#include "ServiceTimer.h"
#include "call/IMtcCall.h"
#include <unordered_map>

class ISession;
class ICoreService;
class IInterfaceHolderListener;

class SessionInterfaceHolder : public ISessionListener, public ITimerListener
{
public:
    explicit SessionInterfaceHolder();
    virtual ~SessionInterfaceHolder();
    SessionInterfaceHolder(IN const SessionInterfaceHolder&) = delete;
    SessionInterfaceHolder& operator=(IN const SessionInterfaceHolder&) = delete;

public:
    // ISessionListener interface implementation
    inline void SessionAlerting(IN ISession*) override {}
    inline void SessionReferenceReceived(IN ISession*, IN IReference*) override {}
    void SessionStarted(IN ISession* piSession) override;
    void SessionStartFailed(IN ISession* piSession) override;
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

    virtual void AddListener(IN IInterfaceHolderListener* piListener);
    virtual void RemoveListener(IN IInterfaceHolderListener* piListener);

    virtual ISession* GetISession(IN CallKey nKey, IN ICoreService* piCoreService,
            IN const AString& strFrom, IN const AString& strTo);
    virtual void AddISession(IN CallKey nKey, IN ISession* piSession);
    virtual void ReleaseISession(IN ISession* piSession);
    virtual void ReleaseISession(IN ISession* piSession, IN IMS_BOOL bEnforceDestroy,
            IN IMS_BOOL bSessionTerminatedOrStartFailed);

    inline virtual IMS_BOOL IsTimerExist(IN ISession* piSession) const
    {
        return GetTimer(piSession) != IMS_NULL;
    }

    inline virtual IMS_UINT32 GetSessionCount() const { return m_objSessionRecords.size(); }

private:
    static IMS_BOOL IsReadyToDestroy(
            IN ISession* piSession, IN IMS_BOOL bSessionTerminatedOrStartFailed);

    void StartTimer(IN ISession* piSession);
    void StopTimer(IN ITimer* piTimer);
    ITimer* GetTimer(IN const ISession* piSession) const;

    class SessionRecord
    {
    public:
        explicit SessionRecord(IN ISession* piSession)
        {
            this->piSession = piSession;
            piTimer = IMS_NULL;
        }

        SessionRecord(IN const SessionRecord&) = delete;
        SessionRecord& operator=(IN const SessionRecord&) = delete;

        ISession* piSession;
        ITimer* piTimer;
    };

    ImsList<IInterfaceHolderListener*> m_objListeners;
    std::unordered_multimap<CallKey, SessionRecord*> m_objSessionRecords;

    static const IMS_UINT32 TIME_TRANSACTION_TERMINATED_GUARD = 128000;
};

#endif
