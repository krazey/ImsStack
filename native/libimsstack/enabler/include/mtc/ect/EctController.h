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

#ifndef ECT_CONTROLLER_H_
#define ECT_CONTROLLER_H_

#include "CallReasonInfo.h"
#include "ITimer.h"
#include "ImsTypeDef.h"
#include "call/IMtcCallManager.h"
#include "ect/IEctReferenceListener.h"
#include <memory>

class IMtcContext;
class IMtcCall;
class IEctControllerListener;
class EctFactory;
class EctReference;

class EctController : public IEctReferenceListener, public ITimerListener
{
public:
    explicit EctController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory);
    virtual ~EctController();
    EctController(IN const EctController&) = delete;
    EctController& operator=(IN const EctController&) = delete;

    // IEctReferenceListener implementation
    void OnReferenceStarted() override;
    void OnReferenceStartFailed() override;
    void OnReferenceUpdated(IN IMS_SINT32 nSipFragCode) override;

    // ITimerListener implementation
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    inline virtual void Transfer(IN const AString& strNumber) { (void)strNumber; }
    inline virtual void Transfer() {}

protected:
    static const IMS_UINT32 TIME_WAIT_OPERATION_COMPLETE = 3000;

    IMtcCall* GetTransferee() const;
    virtual void OnCompleted();  // TODO: OnSucceeded?
    virtual void OnFailed();

    void NotifyResult(IN IMS_RESULT nResult, IN IMS_SINT32 nReason = CODE_NONE) const;
    void CreateReference();
    void TerminateTransfereeCall() const;

    void StartTimer();
    void StopTimer();

    IMtcContext& m_objContext;
    CallKey m_nTransfereeKey;
    IEctControllerListener& m_objListener;
    EctFactory& m_objFactory;
    std::unique_ptr<EctReference> m_pReference;
    ITimer* m_piTimer;
};

#endif
