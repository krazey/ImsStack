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
#ifndef SIP_SERVER_TRANSACTION_STATE_H_
#define SIP_SERVER_TRANSACTION_STATE_H_

#include "ITimer.h"

#include "SipTransactionState.h"
#include "SipTransportAddress.h"

class SipServerTransactionState : public SipTransactionState, public ITimerListener
{
public:
    SipServerTransactionState(IN IMS_SINT32 nSlotId, IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd);
    virtual ~SipServerTransactionState();

    SipServerTransactionState& operator=(IN const SipServerTransactionState&) = delete;

public:
    IMS_SINT32 CheckMessageValidity() override;
    IMS_BOOL FormMessage() override;
    IMS_BOOL Send(IN SipTimerValues* pTimerValues = IMS_NULL) override;
    IMS_BOOL UpdateTransportDetails() override;

    IMS_BOOL InitResponse(IN IMS_SINT32 nStatusCode);
    IMS_BOOL IsSameTransaction(IN const SipServerTransactionState* pStState) const;
    IMS_SINT32 MatchTransaction(IN ::SipMessage* pSipMsg);
    void RejectRequest(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull());
    inline void SetDefaultContact(IN const AString& strContact)
    {
        m_strDefaultContact = strContact;
    }
    IMS_SINT32 HandleRequest(OUT RcPtr<SipDialogEx>& pOrigDialogEx);

private:
    // ITimerListener interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    IMS_BOOL InitResponse(IN IMS_SINT32 nStatusCode, OUT ::SipMessage*& pOutSipMsg);
    IMS_BOOL UpdateTxnDetails();

    static IMS_BOOL Is100TryingResponseRequired(IN const SipMethod& objMethod);
    static IMS_RESULT SendResponse100Trying(IN SipServerTransactionState* pStState);
    static void StartTimer100Trying(
            IN SipServerTransactionState* pStState, IN IMS_SINT32 nTimerInterval /*milli-seconds*/);
    static void StopTimer100Trying(IN SipServerTransactionState* pStState);

private:
    enum
    {
        STATE_IDLE = 0,
        STATE_PROCEEDING,
        STATE_COMPLETED,
        STATE_CONFIRMED,  // INVITE server txn only
        STATE_TERMINATED
    };

    AString m_strDefaultContact;

    // INVITE transaction
    //   The server transaction MUST generate a 100 (Trying) response unless it knows
    //   that the TU will generate a provisional or final response within 200 ms, in which case
    //   it MAY generate a 100 (Tyring) response.
    // Non-INVITE transaction
    //   RFC 4320
    ITimer* m_piTimer100Trying;
};

#endif
