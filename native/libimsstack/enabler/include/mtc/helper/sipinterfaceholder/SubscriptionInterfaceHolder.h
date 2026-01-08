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

#ifndef SUBSCRIPTION_INTERFACE_HOLDER_H_
#define SUBSCRIPTION_INTERFACE_HOLDER_H_

#include "ISubscriptionListener.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"

class ISession;
class ICoreService;
class ISubscription;
class IInterfaceHolderListener;

class SubscriptionInterfaceHolder : public ISubscriptionListener, public ITimerListener
{
public:
    explicit SubscriptionInterfaceHolder(IN IInterfaceHolderListener& objListener);
    virtual ~SubscriptionInterfaceHolder() override;
    SubscriptionInterfaceHolder(IN const SubscriptionInterfaceHolder&) = delete;
    SubscriptionInterfaceHolder& operator=(IN const SubscriptionInterfaceHolder&) = delete;

public:
    // ISubscriptionListener interface implementation
    inline void SubscriptionForkedNotify(IN ISubscription*, IN ISubscription*) override {}
    inline void SubscriptionNotify(IN ISubscription*, IN IMessage*) override {}
    inline void SubscriptionStarted(IN ISubscription*) override {}
    inline void SubscriptionStartFailed(IN ISubscription*) override {}
    void SubscriptionTerminated(IN ISubscription* piSubscription) override;

    // ITimerListener interfaces implementation.
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual ISubscription* GetISubscription(IN ISession* piSession, IN const AString& strEvent);
    virtual ISubscription* GetISubscription(IN ICoreService* piCoreService,
            IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent);

    virtual void ReleaseISubscription(
            IN ISubscription* piSubscription, IN IMS_BOOL bTerminated = IMS_FALSE);

    inline virtual IMS_BOOL IsTimerExist(IN ISubscription* piSubscription) const
    {
        return GetTimer(piSubscription) != IMS_NULL;
    }

    inline virtual IMS_UINT32 GetSubscriptionCount() const { return m_objISubscriptions.GetSize(); }

private:
    static IMS_BOOL IsReadyToDestroy(IN const ISubscription* piSubscription);

    void ClearISubscriptions();

    IMS_RESULT StartTimer(IN ISubscription* piSubscription, IN IMS_SINT32 nDuration);
    void StopTimer(IN ITimer* piTimer);

    ITimer* GetTimer(IN const ISubscription* piSubscription) const;

private:
    IInterfaceHolderListener& m_objListener;
    ImsList<ISubscription*> m_objISubscriptions;
    ImsMap<ITimer*, ISubscription*> m_objSubscriptionTerminatedGuardTimers;

    static const IMS_UINT32 TIME_TRANSACTION_TERMINATED_GUARD = 32000;
};

#endif
