/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_MANAGER_H_
#define _SIP_MANAGER_H_

#include "SIPDialogState.h"
#include "SIPConnectionNotifier.h"

class SIPManager
{
private:
    SIPManager();
    SIPManager(IN CONST SIPManager& objRHS);

public:
    ~SIPManager();

public:
    IMS_BOOL AttachDialogState(IN SIPDialogState* pDState);
    void DetachDialogState(IN SIPDialogState* pDState);
    RCPtr<SIPDialogState> LookupDialogState(IN SIPDialogState* pDState, IN SipMessage* pstMessage,
            IN IMS_BOOL bCheckForked = IMS_FALSE, OUT IMS_BOOL* pbIsForked = IMS_NULL);

    IMS_BOOL AttachConnectionNotifier(IN SIPConnectionNotifier* pSCN);
    void DetachConnectionNotifier(IN SIPConnectionNotifier* pSCN);
    SIPConnectionNotifier* LookupConnectionNotifier(IN CONST SIPTransportAddress& objTA,
            IN CONST AString& strFilter = AString::ConstNull());

    static SIPManager* GetInstance();

private:
    IMS_BOOL StartUp();
    void CleanUp();

private:
    friend class StaticSIP;

    enum
    {
        STATE_INACTIVE,
        STATE_ACTIVE,
        STATE_PENDING
    };

    IMS_SINT32 nState;
    IMSList<SIPDialogState*> objDialogStates;
    IMSList<SIPConnectionNotifier*> objSCNs;

    // IMSList<SIPSLSubscription*> objSubscriptions;
    // IMSList<SIPConnection*> objTransactions;
    // IMSList<SIPDialogImpl*> objDialogs;
    // IMSList<SIPRefresher*> objRefreshers;
};

#endif  // _SIP_MANAGER_H_
