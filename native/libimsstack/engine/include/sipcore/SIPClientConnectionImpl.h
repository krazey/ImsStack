/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CLIENT_CONNECTION_IMPL_H_
#define _SIP_CLIENT_CONNECTION_IMPL_H_

#include "ISipClientConnection.h"
#include "IOnSIPErrorListener.h"
#include "IOnSIPClientConnectionListener.h"

class SIPDialogImpl;
class SIPClientConnection;



class SIPClientConnectionImpl
    : public ISIPClientConnection
    , public IOnSIPErrorListener
    , public IOnSIPClientConnectionListener
{
public:
    explicit SIPClientConnectionImpl(IN SIPClientConnection *pSCC_);
    virtual ~SIPClientConnectionImpl();

private:
    SIPClientConnectionImpl();
    SIPClientConnectionImpl(IN CONST SIPClientConnectionImpl &objRHS);
    SIPClientConnectionImpl& operator=(IN CONST SIPClientConnectionImpl &objRHS);

public:
    IMS_RESULT InitDialogRequest();

private:
    // IConnection interface implementation
    virtual void Close();

    // ISIPConnection interface implementation
    virtual IMS_RESULT AddHeader(IN CONST AString &strName, IN CONST AString &strValue);
    virtual ISIPDialog* GetDialog() const;
    virtual AString GetHeader(IN CONST AString &strName, IN IMS_SINT32 nIndex = 0);
    virtual IMSList<AString> GetHeaders(IN CONST AString &strName);
    virtual const SIPMethod& GetMethod() const;
    virtual const AString& GetReasonPhrase() const;
    virtual const AString& GetRequestURI() const;
    virtual IMS_SINT32 GetStatusCode() const;
    virtual IMS_RESULT RemoveHeader(IN CONST AString &strName);
    virtual IMS_RESULT Send();
    virtual void SetErrorListener(IN ISIPErrorListener *piListener);
    virtual IMS_RESULT SetHeader(IN CONST AString &strName, IN CONST AString &strValue);
    virtual const ByteArray& GetContent() const;
    virtual IMS_RESULT SetContent(IN CONST ByteArray &objContent);
    virtual IMS_SINT32 GetHeaderCount(IN CONST AString &strName) const;
    virtual ISIPMessage* GetMessage() const;
    virtual IMS_SINT32 GetSlotId() const;
    // MULTI_REG_SIP_PROFILE
    virtual void SetSIPProfile(IN SIPProfile *pProfile);
    virtual void SetTransactionTimerValues(IN CONST SIPTimerValues &objTV);

    // ISIPClientConnection interface implementation
    virtual IMS_RESULT InitAck();
    virtual ISIPClientConnection* InitCancel();
    virtual IMS_RESULT InitRequest(IN CONST AString &strMethod, IN ISIPConnectionNotifier *piSCN);
    virtual IMS_RESULT Receive(IN IMS_SLONG nTimeout = 0);
    virtual IMS_RESULT SetCredentials(IN IMSList<Credential> &objCredentials);
    virtual IMS_RESULT SetCredentials(IN CONST Credential &objCredential);
    virtual void SetListener(IN ISIPClientConnectionListener *piListener);
    virtual IMS_RESULT SetRequestURI(IN CONST AString &strURI);
    virtual ISIPGenericChallenge* GetAuthenticationChallenge(IN IMS_SINT32 nIndex = 0) const;
    virtual ISIPAckPackage* GrabAck();
    virtual IMS_RESULT InitResubmissionRequest();
    virtual void RemoveAllChallenges();
    virtual void RemoveAllCredentials();
    virtual IMS_RESULT SetAuthenticationChallenge(IN ISIPGenericChallenge *piChallenge);
    virtual void SetExtensionTokenForViaBranch(IN CONST AString &strToken);
    virtual void SetImplicitRouteHeader(IN CONST AString &strRouteHeader);
    virtual void SetTransportTuple(IN CONST IPAddress &objIPA,
            IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFC = 0xFFFF,
            IN IMS_SINT32 nTransportExt = 0 /* ANY */);

    // IOnSIPErrorListener interface
    virtual void OnError_NotifyError(IN SIPConnection *pSC, IN IMS_SINT32 nCode,
            IN CONST AString &strMessage);

    // IOnSIPClientConnectionListener interface
    virtual void OnClientConnection_NotifyResponse(IN SIPClientConnection *pSCC);
    virtual void OnClientConnection_NotifyForkedResponse(IN SIPClientConnection *pSCC,
            IN SIPClientConnection *pForkedSCC);

private:
    ISIPErrorListener *piErrorListener;
    ISIPClientConnectionListener *piListener;

    SIPDialogImpl *pDialogImpl;
    SIPClientConnection *pSCC;
};

#endif // _SIP_CLIENT_CONNECTION_IMPL_H_
