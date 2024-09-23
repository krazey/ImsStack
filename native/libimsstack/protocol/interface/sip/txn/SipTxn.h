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
#ifndef __SIP_TXN_H__
#define __SIP_TXN_H__

#include "ISipUserData.h"
#include "SipRefBase.h"
#include "SipTimerContext.h"
#include "msg/SipMessage.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnKey.h"
#include "txn/SipTxnTimerValues.h"

extern SIP_BOOL Sip_Cbk_FetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);
extern SIP_BOOL Sip_Cbk_ReleaseTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);

class SipTxn : public SipRefBase
{
public:
    enum
    {
        /* set by Stack User for sending request or response */
        SEND = 0,
        /* set by transport layer on receiving side of request or response */
        RECV,
        INVALID_DIR
    };

    /* Type of Transaction is defined */
    enum
    {
        INV_CLI_TXN = 0,
        INV_SER_TXN,
        NON_INV_CLI_TXN,
        NON_INV_SER_TXN,
        INVALID_TXN
    };

    enum
    {
        /* Request is received and no txn exist (Invite, Non-Invite, ACK) */
        STATUS_NEW_REQ_RECVD = 0,
        /* On recv of failure/success responses(INV & Non-INV) and
        transaction exists in valid state
        On recv of failure ACK,
        Re-transmitted 2xx for Invite.
        Pls Note: On Receive of INVITE Failure response, stack send the failure ACK to network*/
        STATUS_VALID_MESSAGE,
        /* Re-transmitted request is received in state where no action is required
        Re-transmitted INVITE request or Retransmitted Failure ACK request received in
        confirmed state Re-transmitted non-INVITE request received in Trying state*/
        STATUS_IGNORE_REQ,
        /* Response received in state where no action is required
        INVITE 1xx received in completed state
        Non-INVITE response is received in completed state */
        STATUS_IGNORE_RESP,
        /* No Txn exists when Non-INVITE responses or failure INVITE response is received */
        STATUS_STRAY_RESP,
        /* Re-transmitted req/resp are received in valid state
        For re-transmitted request, stack re-transmit the last response to network
        In INVITE Client txn, retransmitted failure resp can be received for which
        failure ACK to be sent
        */
        STATUS_RETRANSMISSION,
        /* Error occurs when stack try to send message to the network */
        STATUS_ERROR_ON_SEND,
        /* When txn handler fails to handle receive message --> failure conditions */
        STATUS_INVALID_MESSAGE,
        /*For Handling Stray 2xx*/
        STATUS_2XX_STRAY_RESP,
        STATUS_STRAY_PRACK,
        STATUS_INVALID

    };

    enum
    {
        TIMER_T1 = 0,
        TIMER_T2,
        TIMER_T4,
        TIMER_A,
        TIMER_B,
        TIMER_C,
        TIMER_D,
        TIMER_E,
        TIMER_F,
        TIMER_G,
        TIMER_H,
        TIMER_I,
        TIMER_J,
        TIMER_K,
        TIMER_OTHER,
        TIMER_TYPE_INVALID
    };

    enum
    {
        INV_CLI_SEND_INV_REQ_EVT = 0,
        INV_CLI_TIMERA_B_TIME_OUT_EVT,
        INV_CLI_TIMERD_TIME_OUT_EVT,
        INV_CLI_RECV_1XX_RESP_EVT,
        INV_CLI_RECV_2XX_RESP_EVT,
        INV_CLI_RECV_3XX_6XX_RESP_EVT,
        INV_CLI_TRANSP_ERROR_EVT,
        INV_CLI_INVALID_EVT
    };

    /* States for INVITE Client FSM */
    enum
    {
        INV_CLI_IDLE_ST = 0,
        INV_CLI_CALLING_ST,
        INV_CLI_PROCEEDING_ST,
        INV_CLI_COMPLETED_ST,
        INV_CLI_TERMINATED_ST,
        INV_CLI_INVALID_ST
    };

    enum
    {
        INV_SER_RECV_INV_REQ_EVT = 0,
        INV_SER_SEND_NON_100_PROV_RESP_EVT,
        INV_SER_SEND_3XX_6XX_FAILURE_RESP_EVT,
        INV_SER_SEND_2XX_SUCCESS_RESP_EVT,
        INV_SER_TRANSP_ERROR_EVT,
        INV_SER_RECV_ACK_REQ_EVT,
        INV_SER_TIMER_G_H_TIME_OUT_EVT,
        INV_SER_TIMER_I_TIME_OUT_EVT,
        INV_SER_INVALID_EVT
    };

    /* States for INVITE Server FSM */
    enum
    {
        INV_SER_IDLE_ST = 0,
        INV_SER_PROCEEDING_ST,
        INV_SER_COMPLETED_ST,
        INV_SER_CONFIRMED_ST,
        INV_SER_TERMINATED_ST,
        INV_SER_INVALID_ST
    };

    enum
    {
        NON_INV_CLI_SEND_NON_INV_REQ_EVT = 0,
        NON_INV_CLI_TIMER_E_F_TIME_OUT_EVT,
        NON_INV_CLI_RECV_1XX_RESP_EVT,
        NON_INV_CLI_RECV_2XX_6XX_RESP_EVT,
        NON_INV_CLI_TRANSP_ERROR_EVT,
        NON_INV_CLI_TIMER_K_TIME_OUT_EVT,
        NON_INV_CLI_INVALID_EVT
    };

    /* States for non-INVITE Client FSM */
    enum
    {
        NON_INV_CLI_IDLE_ST = 0,
        NON_INV_CLI_TRYING_ST,
        NON_INV_CLI_PROCEEDING_ST,
        NON_INV_CLI_COMPLETED_ST,
        NON_INV_CLI_TERMINATED_ST,
        NON_INV_CLI_INVALID_ST
    };

    /* Events for non-INVITE Server FSM */
    enum
    {
        NON_INV_SER_RECV_NON_INV_REQ_EVT = 0,
        NON_INV_SER_SEND_1XX_RESP_EVT,
        NON_INV_SER_SEND_2XX_6XX_RESP_EVT,
        NON_INV_SER_TRANSP_ERROR_EVT,
        NON_INV_SER_TIMER_J_TIME_OUT_EVT,
        NON_INV_SER_INVALID_EVT
    };

    /* States for non-INVITE Server FSM */
    enum
    {
        NON_INV_SER_IDLE_ST = 0,
        NON_INV_SER_TRYING_ST,
        NON_INV_SER_PROCEEDING_ST,
        NON_INV_SER_COMPLETED_ST,
        NON_INV_SER_TERMINATED_ST,
        NON_INV_SER_INVALID_ST
    };

    static constexpr SIP_INT32 OPT_FETCH = 0;
    static constexpr SIP_INT32 OPT_CREATE = 1;
    static constexpr SIP_INT32 OPT_REMOVE = 2;

private:
    /* Txn Type */
    SIP_INT32 m_eTxnType;

    /* Key which is stored in Txn Hash Table */
    SipTxnKey* m_pTxnKey;

    SipMessage* m_pSipMsg;

    /* SIP Transport */
    SipTransportInfo* m_pTranspInfo;

    /* User Specific Data or User Key Info */
    ISipUserData* m_pUserData;

    /* FSM Current State */
    SIP_UINT16 m_nTxnState;

    /* Counter for Number of Retransmissions */
    SIP_UINT16 m_nRetransmissionCount;

    /* Transaction Related Timers */
    /* Type of Timer Started */
    SIP_INT32 m_eTimerType;
    /* Timer ID */
    SIP_VOID* m_pvTimerId;
    /* Maximum Timer Duration */
    SIP_UINT32 m_nMaxDuration;
    /* Total Duration of Timer Expired : Elapsed Time */
    SIP_UINT32 m_nDurationExpired;
    /* Current Value of Timer Duration */
    SIP_UINT32 m_nCurrentDuration;

    SipTxnTimerValues objTxnTimerValues;

public:
    SipTxn();

    SipTxn(SIP_INT32 eTxnType, SipTxnKey* pTxnKey, SipMessage* pSipMsg,
            SipTimerContext* pSipTxnTimerContext, SIP_UINT16* pnError);

    SIP_BOOL InvokeFsm(SIP_UINT16 nEvent, SIP_VOID* pvData, SIP_UINT16* pnError);
    SIP_BOOL AbortTxn();

    /* Abstract API to start Transaction Timer */
    SIP_BOOL StartTxnTimer(SIP_UINT32 eTimerType, SIP_UINT32 nDuration, SIP_UINT16* pnError);

    SIP_BOOL StopTxnTimer();

    SIP_BOOL PrepareACK(
            IN SipMessage* pSipRespMsg, IN SIP_BOOL bSetMsgBody, OUT SipMessage** ppSipAckMsg);

    SIP_VOID RemoveFromTxnPool();

    SIP_BOOL UpdateTranspInfo(SipTransportInfo* pTranspInfo);

    inline SIP_INT32 GetTxnType() const { return m_eTxnType; }
    inline SipTxnKey* GetTxnKey() { return m_pTxnKey; }
    SipTransportInfo* GetTranspInfo();
    inline SIP_UINT16 GetTxnState() const { return m_nTxnState; }

    inline SIP_UINT16 GetRetransmissionCount() const { return m_nRetransmissionCount; }

    inline SIP_UINT32 GetDurationExpired() const { return m_nDurationExpired; }

    inline SIP_UINT32 GetMaxDuration() const { return m_nMaxDuration; }

    inline SIP_UINT32 GetCurrentDuration() const { return m_nCurrentDuration; }

    SIP_VOID* GetTimerId();
    ISipUserData* GetUserData();
    SIP_INT32 GetMsgSentProto();
    inline const SipTxnTimerValues& GetSipTxnTimers() const { return objTxnTimerValues; }
    /* Fill Transaction Properties */
    inline SIP_VOID SetTxnState(SIP_UINT16 nTxnState) { m_nTxnState = nTxnState; }
    inline SIP_VOID SetMaxDuration(SIP_UINT32 nMaxDuration) { m_nMaxDuration = nMaxDuration; }
    inline SIP_VOID SetCurrentDuration(SIP_UINT32 nCurDuration)
    {
        m_nCurrentDuration = nCurDuration;
    }
    inline SIP_VOID SetTimerId(SIP_VOID* pvTimerId) { m_pvTimerId = pvTimerId; }
    SIP_VOID SetUserData(ISipUserData* pUserData);
    /* Increment Txn Count by one*/
    inline SIP_VOID IncreaseTxnCount()
    {
        m_nRetransmissionCount = m_nRetransmissionCount + SIP_ONE;
    }

    inline SIP_VOID IncreaseDurationExpired(SIP_UINT32 nDuration)
    {
        m_nDurationExpired = m_nDurationExpired + nDuration;
    }

    SIP_BOOL IsTxnTerminated();

    SIP_VOID InitRetransmissionInfo();

    SIP_VOID SetResponseCode(SIP_UINT16 nRespCode);

private:
    virtual ~SipTxn();
};

/*Timer Callback API*/
SIP_VOID CbkTxnTimeout(SIP_VOID* pvobjTimeoutData, const SIP_VOID* pvTimerId);

SIP_VOID SipTxn_RemoveFromTxnPool(SipTxnKey* pTxnKey);

#endif  //__SIP_TXN_H__
