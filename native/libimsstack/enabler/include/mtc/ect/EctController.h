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
#include "call/IMtcCall.h"
#include "ect/IEctReferenceListener.h"
#include <memory>

class IMtcContext;
class IEctControllerListener;
class EctFactory;
class EctReference;

/**
 * @brief Base class for Explicit Call Transfer (ECT) controllers.
 *
 * This class provides the common functionality and interface for different types of call transfers,
 * such as blind and consultative transfers. It handles the {@link IReference} listening
 * and timer events.
 */
class EctController : public IEctReferenceListener, public ITimerListener
{
public:
    /**
     * @brief Constructs a new {@link EctController}.
     *
     * @param objContext The MTC context.
     * @param nCallKey The key of the call to be transferred.
     * @param objListener The listener for ECT controller events.
     * @param objFactory The factory to create ECT related objects.
     */
    explicit EctController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener, IN EctFactory& objFactory);
    virtual ~EctController() override;
    EctController(IN const EctController&) = delete;
    EctController& operator=(IN const EctController&) = delete;

    /** See {@link IEctReferenceListener#OnReferenceStarted}. */
    void OnReferenceStarted() override;

    /** See {@link IEctReferenceListener#OnReferenceStartFailed}. */
    void OnReferenceStartFailed() override;

    /** See {@link IEctReferenceListener#OnReferenceUpdated}. */
    void OnReferenceUpdated(IN IMS_SINT32 nSipFragCode) override;

    /**
     * @brief Called when the internal operation timer expires, indicating a timeout.
     *
     * @param piTimer Pointer to the expired timer.
     */
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    /**
     * @brief Virtual method to initiate a transfer to a specific number.
     *
     * To be implemented by derived classes like {@link BlindTransferController}.
     *
     * @param strNumber The target number for the transfer.
     */
    inline virtual void Transfer(IN const AString& strNumber) { (void)strNumber; }

    /**
     * @brief Virtual method to initiate a transfer.
     *
     * To be implemented by derived classes like {@link ConsultativeTransferController}.
     */
    inline virtual void Transfer() {}

protected:
    /** Timeout for waiting for the transfer operation to complete. */
    static const IMS_UINT32 TIME_WAIT_OPERATION_COMPLETE = 32000;

    IMtcCall* GetTransferee() const;
    virtual void OnSuccess();
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
