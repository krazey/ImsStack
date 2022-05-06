/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CONNECTION_NOTIFIER_IMPL_H_
#define _SIP_CONNECTION_NOTIFIER_IMPL_H_

#include "ISipConnectionNotifier.h"
#include "IOnSIPServerConnectionListener.h"
#include "IOnSIPConnectionNotifierErrorListener.h"

class ISipServerConnectionListener;
class SIPConnectionNotifier;

class SIPConnectionNotifierImpl :
        public ISipConnectionNotifier,
        public IOnSIPServerConnectionListener,
        public IOnSIPConnectionNotifierErrorListener
{
public:
    explicit SIPConnectionNotifierImpl(IN SIPConnectionNotifier* pSCN_);
    virtual ~SIPConnectionNotifierImpl();

private:
    SIPConnectionNotifierImpl(IN CONST SIPConnectionNotifierImpl& objRHS);
    SIPConnectionNotifierImpl& operator=(IN CONST SIPConnectionNotifierImpl& objRHS);

public:
    SIPConnectionNotifier* GetConnectionNotifier() const;

private:
    // IConnection interface
    virtual void Close();

    // ISipConnectionNotifier interface
    virtual ISipServerConnection* AcceptAndOpen();
    virtual const IPAddress& GetLocalAddress() const;
    virtual IMS_SINT32 GetLocalPort() const;
    virtual void SetListener(IN ISipServerConnectionListener* piListener);
    //// IMS extensions
    virtual ISipServerConnection* AcceptAndOpen(OUT ISipDialog*& piOrigDialog);
    virtual AString GetContactAddress() const;
    // MULTI_REG_SIP_PROFILE
    virtual SipProfile* GetSipProfile() const;
    virtual IMS_SINT32 GetSlotId() const;
    virtual IMS_BOOL IsTransportResourceReserved(IN IMS_SINT32 nType = TRANSPORT_ALL) const;
    virtual IMS_RESULT ReserveTransportResource(IN CONST IPAddress& objIPA, IN IMS_SINT32 nPortS,
            IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl);
    virtual IMS_RESULT RestoreTransportResource(
            IN IMS_SINT32 nType, IN CONST IPAddress& objPeerIP, IN IMS_SINT32 nPeerPort);
    virtual void SetFilter(IN CONST AString& strFilter);
    virtual void SetFromAndContact(IN CONST AString& strFrom, IN CONST AString& strDisplayName,
            IN CONST AString& strUserInfo);
    // MULTI_REG_SIP_PROFILE
    virtual void SetSipProfile(IN SipProfile* pProfile);
    virtual void UpdatePortFlowControl(IN IMS_SINT32 nPort);
    virtual void UpdatePortUc(IN IMS_SINT32 nPort);
    virtual void AddErrorListener(IN ISipConnectionNotifierErrorListener* piListener);
    virtual void RemoveErrorListener(IN ISipConnectionNotifierErrorListener* piListener);

    // IOnSIPServerConnectionListener interface
    virtual void OnServerConnection_NotifyRequest(IN SIPConnectionNotifier* pSCN);
    virtual void OnServerConnection_NotifyForkedRequest(IN SIPConnectionNotifier* pSCN);

    // IOnSIPConnectionNotifierErrorListener interface
    virtual void OnConnectionNotifierError_NotifyError(
            IN SIPConnectionNotifier* pSCN, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

private:
    ISipServerConnectionListener* piListener;
    IMSList<ISipConnectionNotifierErrorListener*> objErrorListeners;

    SIPConnectionNotifier* pSCN;
};

#endif  // _SIP_CONNECTION_NOTIFIER_IMPL_H_
