/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140318  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_ACK_PACKAGE_H_
#define _SIP_ACK_PACKAGE_H_

#include "IMSList.h"
#include "SIPStackHeaders.h"
#include "ISipAckPackage.h"

class SIPClientTransactionState;
class SIPAck;
class SIPAckPackagePrivate;

class SIPAckPackage : public ISipAckPackage
{
private:
    SIPAckPackage(IN CONST AString& strCallId_);

public:
    virtual ~SIPAckPackage();

public:
    void AddAck(IN SIPClientTransactionState* pCTState, IN IMS_SINT32 nAliveInterval);
    IMS_BOOL IsSamePackage(IN CONST AString& strCallId) const;
    IMS_BOOL NotifyStray2xx(IN SipTxnKey* pstTxnKey);

private:
    // ISipObject class
    virtual void Destroy();

    // ISipAckPackage class
    virtual void RemoveStrayAcks();

public:
    static SIPAckPackage* CreateAckPackage(IN CONST AString& strCallId);
    static IMS_BOOL HandleStray2xx(IN SipMessage* pstMessage);
    static void Init();

private:
    AString strCallId;
    IMSList<SIPAck*> objAcks;

    static SIPAckPackagePrivate* pAckPackageP;
};

#endif  // _SIP_ACK_PACKAGE_H_
