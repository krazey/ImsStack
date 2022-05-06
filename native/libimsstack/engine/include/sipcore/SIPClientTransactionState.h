/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CLIENT_TRANSACTION_STATE_H_
#define _SIP_CLIENT_TRANSACTION_STATE_H_

#include "SIPTransactionState.h"
#include "SIPForkedTransactionManager.h"
#include "SIPTransportAddress.h"

class ISIPClientTransactionStateListener;

class SIPClientTransactionState : public SIPTransactionState
{
public:
    explicit SIPClientTransactionState(IN IMS_SINT32 nSlotId);
    virtual ~SIPClientTransactionState();

private:
    SIPClientTransactionState& operator=(IN CONST SIPClientTransactionState& objRHS);

public:
    virtual void Abort();
    virtual IMS_SINT32 CheckMessageValidity();
    virtual IMS_BOOL FormMessage();
    virtual IMS_BOOL FormMessageForResubmissionRequest();
    virtual IMS_BOOL InitTxnDetails(IN CONST SIPTransactionState* pTState);
    virtual IMS_BOOL Send(IN SipTimerValues* pTV = IMS_NULL);
    virtual IMS_BOOL UpdateTransportDetails();

    IMS_BOOL AdjustTransportProtocolAsUDP();
    SipMessage* CreateAckRequest(IN SipMessage* pstRespMessage);
    inline IMS_BOOL IsTargetUpdated() const { return (nRoutingType == TARGET_SR); }
    IMS_BOOL InitCancel(IN CONST SIPClientTransactionState* pInviteTState);
    IMS_BOOL InitRequest(IN CONST SipMethod& objMethod);
    IMS_BOOL InitRequest(IN CONST SipMethod& objMethod, IN SIPDialogEx* pDialogEx);
    // FORKED_RESPONSE
    IMS_SINT32 RemoveForkedTransaction();
    IMS_BOOL SendWithCredentials(IN SipTimerValues* pTV = IMS_NULL);
    void SetExtensionTokenForViaBranch(IN CONST AString& strToken);
    // IMPLICIT_ROUTE
    void SetImplicitRouteHeader(IN CONST AString& strRouteHeader);
    inline void SetListener(IN ISIPClientTransactionStateListener* piListener)
    {
        this->piListener = piListener;
    }
    IMS_BOOL UpdateRouteDetails(IN CONST SipMethod& objMethod);
    IMS_SINT32 HandleResponse(IN SipMessage* pstMessage);
    static IMS_SINT32 MatchTransaction(IN SipMessage* pstMessage,
            IN CONST SIPTransportAddress& objFarEnd,
            OUT RCPtr<SIPClientTransactionState>& pCTState);

private:
    virtual SIPTransactionState* Clone();

    IMS_BOOL CorrectRouteHeader(IN_OUT SipMessage*& pstMessage);
    void CheckNSendAck();
    IMS_BOOL HandleForkedResponse(IN CONST SIPMessageInfo& objMInfo);
    IMS_BOOL InitAck(IN_OUT SipMessage*& pstAckMessage, IN SipMessage* pstRespMessage);
    IMS_BOOL SetDialogRelatedHeaders(IN CONST SipMethod& objMethod);
    IMS_BOOL SetMandatoryHeaders(IN CONST SipMethod& objMethod);
    void SetPANIHeader(IN CONST SipMethod& objMethod, IN_OUT SipMessage*& pstMessage);
    IMS_BOOL UpdateTxnDetails(IN CONST SipMethod& objMethod);

private:
    // Flag which will be used to determine the destination transport information
    // to send a request.
    // If the flag is TARGET_SR,
    //    the Request-URI will be selected.
    // Otherwise,
    //    if the Route entry is present and the flag is TARGET_LR,
    //    the first Route entry will be selected.
    //    if the flag is TARGET_NO_ROUTE, the Request-URI will be selected.
    enum
    {
        TARGET_LR = 0,
        TARGET_SR,
        TARGET_NO_ROUTE
    };

    IMS_SINT32 nRoutingType;
    // IMPLICIT_ROUTE
    SipAddrSpec* pstImplicitRoute;
    ISIPClientTransactionStateListener* piListener;

    // FORKED_RESPONSE
    RCPtr<SIPForkedTransactionManager> pForkedTxnMngr;
    // FORKED_RESPONSE_TO_SUPPORT_EARLY_DIALOG_TERMINATION
    RCPtr<SIPForkedTransactionManager> pPersistentForkedTxnMngr;
};

#endif  // _SIP_CLIENT_TRANSACTION_STATE_H_
