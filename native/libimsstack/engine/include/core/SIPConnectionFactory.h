/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140203  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_CONNECTION_FACTORY_H_
#define _SIP_CONNECTION_FACTORY_H_

#include "util/IDialogMethod.h"
#include "util/ICancellableMethod.h"
#include "ISIPConnectionFactory.h"
#include "EngineActivity.h"

class Service;

class SIPConnectionFactory :
        public EngineActivity,
        public IDialogMethod,
        public ICancellableMethod,
        public ISIPConnectionFactory
{
public:
    SIPConnectionFactory(IN Service* pService_);
    SIPConnectionFactory(IN Service* pService_, IN ISipServerConnection* piSSC);
    virtual ~SIPConnectionFactory();

public:
    // EngineActivity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // IDialogMethod class
    virtual IMS_BOOL Dialog_Compare(IN ISipServerConnection* piSSC) const;
    virtual IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSSC);

    // ICancellableMethod class
    virtual IMS_BOOL Cancellable_Compare(IN ISipServerConnection* piSSC_CANCEL) const;
    virtual IMS_BOOL Cancellable_NotifyRequest(IN ISipServerConnection* piSSC_CANCEL);

    // IMethod class
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator* piMediator);

    // ISIPConnectionFactory class
    virtual ISipClientConnection* CreateClientConnection(
            IN CONST SipMethod& objMethod, IN CONST SipAddress* pFrom, IN CONST SipAddress* pTo);
    virtual ISipClientConnection* CreateClientConnection(
            IN ISipDialog* piDialog, IN CONST SipMethod& objMethod);
    virtual IMS_BOOL CreateResponse(IN_OUT ISipServerConnection* piSSC, IN IMS_SINT32 nStatusCode,
            IN CONST AString& strPhrase = AString::ConstNull());
    virtual ISipServerConnection* GetNewServerConnection();
    virtual void SetDialog(IN ISipDialog* piDialog);
    virtual void SetListener(IN ISIPConnectionFactoryListener* piListener);
    virtual void SetSSCForCANCEL(IN ISipServerConnection* piSSC);

private:
    enum
    {
        AMSG_SSC_FOR_MID_DIALOG_RECEIVED = AMSG_USER
    };

    Service* pService;
    ISipDialog* piDialog;
    ISIPConnectionFactoryListener* piListener;
    // It is only maintained for a new incoming request
    ISipServerConnection* piInitialSSC;
    ISipServerConnection* piInviteSSC;
};

#endif  // _SIP_CONNECTION_FACTORY_H_
