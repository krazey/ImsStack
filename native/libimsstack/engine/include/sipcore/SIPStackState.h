/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090302  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_STACK_STATE_H_
#define _SIP_STACK_STATE_H_

#include "IMSMap.h"
#include "IMSList.h"

class IMutex;
class SipProfile;
class SIPStackTransaction;
class SIPTransactionState;

/*
SIP stact state class

Example

See Also
*/
class SIPStackState
{
private:
    SIPStackState();
    SIPStackState(IN CONST SIPStackState& objRHS);

public:
    ~SIPStackState();

public:
    void CleanUp();
    void StartUp();

    IMS_BOOL AbortTransaction(IN SipTxnKey* pKey, IN SIPTransactionState* pTxnState);
    IMS_BOOL FetchTransaction(
            IN SipTxnKey* pKey, IN IMS_SINT32 nOption, OUT SipTxnKey*& pOutKey, OUT SipTxn*& pTxn);
    IMS_BOOL ReleaseTransaction(
            IN SipTxnKey* pKey, IN IMS_SINT32 nOption, OUT SipTxnKey*& pOutKey, OUT SipTxn*& pTxn);
    void SetTransactionTimerValues(IN IMS_SINT32 nSlotId, IN CONST SipProfile* pSIPProfile);

    static SIPStackState* GetInstance();

private:
    IMS_BOOL AddTransaction(IN SipTxnKey* pKey, IN SipTxn* pTxn);
    SIPStackTransaction* FindTransaction(IN SipTxnKey* pKey);
    SIPStackTransaction* RemoveTransaction(IN SipTxnKey* pKey, IN IMS_SINT32 nOption);
    IMS_UINT32 GetTransactionCount() const;

public:
    enum
    {
        TXN_FETCH = 0,
        TXN_CREATE = 1,
        TXN_REMOVE = 2
    };

private:
    IMutex* piLock;
    IMSMap<IMS_UINT32, IMSList<SIPStackTransaction*>> objTxnAggregate;
};

#endif  // _SIP_STACK_STATE_H_
