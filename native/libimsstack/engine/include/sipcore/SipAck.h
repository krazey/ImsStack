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
#ifndef SIP_ACK_H_
#define SIP_ACK_H_

#include "ITimer.h"

#include "SipClientTransactionState.h"

class SipAck : public ITimerListener
{
public:
    SipAck(IN SipClientTransactionState* pCtState, IN IMS_SINT32 nAliveInterval);
    virtual ~SipAck();

    SipAck(IN const SipAck&) = delete;
    SipAck& operator=(IN const SipAck&) = delete;

public:
    IMS_BOOL IsSameTransaction(IN ::SipTxnKey* pTxnKey) const;
    inline IMS_BOOL IsStrayAck() const { return m_piTimer == IMS_NULL; }
    void RetransmitMessage();

private:
    // ITimerListener class
    void Timer_TimerExpired(IN ITimer* piTimer) override;

private:
    RCPtr<SipClientTransactionState> m_pCtState;
    ITimer* m_piTimer;
};

#endif
