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

#ifndef REFERENCE_INTERFACE_HOLDER_H_
#define REFERENCE_INTERFACE_HOLDER_H_

#include "IReferenceListener.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ServiceTimer.h"

class ISession;
class ICoreService;
class IReference;
class IInterfaceHolderListener;

class ReferenceInterfaceHolder : public IReferenceListener, public ITimerListener
{
public:
    explicit ReferenceInterfaceHolder(IN IInterfaceHolderListener& objListener);
    virtual ~ReferenceInterfaceHolder() override;
    ReferenceInterfaceHolder(IN const ReferenceInterfaceHolder&) = delete;
    ReferenceInterfaceHolder& operator=(IN const ReferenceInterfaceHolder&) = delete;

public:
    // IReferenceListener interface implementation
    inline void ReferenceDelivered(IN IReference*) override {}
    inline void ReferenceDeliveryFailed(IN IReference*) override {}
    inline void ReferenceNotify(IN IReference*, IN IMessage*) override {}
    void ReferenceTerminated(IN IReference* piReference) override;

    // ITimerListener interfaces implementation.
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual IReference* GetIReference(
            IN ISession* piSession, IN const AString& strReferTo, IN const AString& strMethod);

    virtual void ReleaseIReference(IN IReference* piReference, IN IMS_BOOL bTerminated = IMS_FALSE);

    inline virtual IMS_BOOL IsTimerExist(IN IReference* piReference) const
    {
        return GetTimer(piReference) != IMS_NULL;
    }

    inline virtual IMS_UINT32 GetReferenceCount() const { return m_objIReferences.GetSize(); }

private:
    static IMS_BOOL IsReadyToDestroy(IN const IReference* piReference);

    void ClearIReferences();

    IMS_RESULT StartTimer(IN IReference* piReference, IN IMS_SINT32 nDuration);
    void StopTimer(IN ITimer* piTimer);

    ITimer* GetTimer(IN const IReference* piReference) const;

private:
    IInterfaceHolderListener& m_objListener;
    ImsList<IReference*> m_objIReferences;
    ImsMap<ITimer*, IReference*> m_objReferenceTerminatedGuardTimers;

    static const IMS_UINT32 TIME_TRANSACTION_TERMINATED_GUARD = 32000;
};

#endif
