/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100603  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_CONNECTION_NOTIFIER_MANAGER_H_
#define _SIP_CONNECTION_NOTIFIER_MANAGER_H_

#include "IPAddress.h"
#include "SipAddress.h"

class ISIPConnectionNotifier;
class SIPConnectionNotifierManagerPrivate;



class SIPConnectionNotifierManager
{
private:
    SIPConnectionNotifierManager();
    ~SIPConnectionNotifierManager();

    SIPConnectionNotifierManager(IN const SIPConnectionNotifierManager& objRHS);
    SIPConnectionNotifierManager& operator=(IN const SIPConnectionNotifierManager& objRHS);

public:
    ISIPConnectionNotifier* CreateConnectionNotifier(
            IN CONST AString &strScheme, IN CONST IPAddress &objIPA, IN IMS_SINT32 nPortS,
            IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl,
            IN CONST AString &strParams, IN CONST SIPAddress &objUserId);
    ISIPConnectionNotifier* GetConnectionNotifier(IN CONST IPAddress &objIP,
            IN IMS_SINT32 nPort);
    void ReleaseConnectionNotifier(IN ISIPConnectionNotifier *&piSCN);

    static SIPConnectionNotifierManager* GetInstance();
    static void Init(IN IMS_SINT32 nSlotId);

private:
    SIPConnectionNotifierManagerPrivate *pSCNMngrP;
};

#endif // _SIP_CONNECTION_NOTIFIER_MANAGER_H_
