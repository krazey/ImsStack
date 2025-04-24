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
#ifndef SIP_CLIENT_TRANSACTION_STATE_H_
#define SIP_CLIENT_TRANSACTION_STATE_H_

#include "SipForkedTransactionManager.h"
#include "SipTransactionState.h"
#include "SipTransportAddress.h"

class ISipClientTransactionStateListener;

class SipClientTransactionState : public SipTransactionState
{
public:
    explicit SipClientTransactionState(IN IMS_SINT32 nSlotId);
    virtual ~SipClientTransactionState();

    SipClientTransactionState& operator=(IN const SipClientTransactionState&) = delete;

public:
    void Abort() override;
    IMS_SINT32 CheckMessageValidity() override;
    IMS_BOOL FormMessage() override;
    IMS_BOOL InitTxnDetails(IN const SipTransactionState* pTState) override;
    void NotifyTimerExpired() override;
    IMS_BOOL Send(IN SipTimerValues* pTimerValues = IMS_NULL) override;
    IMS_BOOL UpdateTransportDetails() override;

    IMS_BOOL AdjustTransportProtocolAsUdp();
    ::SipMessage* CreateAckRequest(IN ::SipMessage* pRespMessage);
    IMS_BOOL FormMessageForResubmissionRequest();
    inline IMS_BOOL IsTargetUpdated() const { return (m_nRoutingType == TARGET_SR); }
    IMS_BOOL InitCancel(IN const SipClientTransactionState* pInviteTState);
    IMS_BOOL InitRequest(IN const SipMethod& objMethod);
    IMS_BOOL InitRequest(IN const SipMethod& objMethod, IN SipDialogEx* pDialogEx);
    // FORKED_RESPONSE
    IMS_SINT32 RemoveForkedTransaction();
    IMS_BOOL SendWithCredentials(IN SipTimerValues* pTimerValues = IMS_NULL);
    void SetExtensionTokenForViaBranch(IN const AString& strToken);
    // IMPLICIT_ROUTE
    void SetImplicitRouteHeader(IN const AString& strRouteHeader);
    inline void SetListener(IN ISipClientTransactionStateListener* piListener)
    {
        m_piCtsListener = piListener;
    }
    IMS_BOOL UpdateRouteDetails(IN const SipMethod& objMethod);
    IMS_SINT32 HandleResponse(IN ::SipMessage* pSipMsg);
    void ClearAllForkedTransactions();
    IMS_BOOL IsAckSent() const;
    static IMS_SINT32 MatchTransaction(IN ::SipMessage* pSipMsg,
            IN const SipTransportAddress& objFarEnd,
            OUT RcPtr<SipClientTransactionState>& pCtState);

private:
    SipTransactionState* Clone() override;

    IMS_BOOL CorrectRouteHeader(IN_OUT ::SipMessage*& pSipMsg);
    void CheckNSendAck();
    IMS_BOOL HandleForkedResponse(IN const SipMessageInfo& objMsgInfo);
    IMS_BOOL InitAck(IN_OUT ::SipMessage*& pAckSipMsg, IN ::SipMessage* pRespSipMsg);
    IMS_BOOL SetDialogRelatedHeaders(IN const SipMethod& objMethod);
    IMS_BOOL SetMandatoryHeaders(IN const SipMethod& objMethod);
    void SetPaniHeader(IN_OUT ::SipMessage*& pSipMsg);
    IMS_BOOL UpdateTxnDetails(IN const SipMethod& objMethod);

private:
    /// Flag which will be used to determine the destination transport information
    /// to send a request.
    /// If the flag is TARGET_SR,
    ///    the Request-URI will be selected.
    /// Otherwise,
    ///    if the Route entry is present and the flag is TARGET_LR,
    ///    the first Route entry will be selected.
    ///    if the flag is TARGET_NO_ROUTE, the Request-URI will be selected.
    enum
    {
        TARGET_LR = 0,
        TARGET_SR,
        TARGET_NO_ROUTE
    };

    IMS_SINT32 m_nRoutingType;
    SipAddrSpec* m_pImplicitRoute;
    ISipClientTransactionStateListener* m_piCtsListener;
    // FORKED_RESPONSE
    RcPtr<SipForkedTransactionManager> m_pForkedTxnMngr;
    // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
    RcPtr<SipForkedTransactionManager> m_pPersistentForkedTxnMngr;
};

#endif
