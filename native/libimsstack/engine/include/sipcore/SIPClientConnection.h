/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CLIENT_CONNECTION_H_
#define _SIP_CLIENT_CONNECTION_H_

#include "Credential.h"
#include "SIPConnection.h"
#include "ISIPClientTransactionStateListener.h"
#include "ISIPClientTransmissionListener.h"
#include "SIPClientTransmissionProxy.h"
#include "SIPClientTransactionState.h"

class IOnSIPClientConnectionListener;
class ISIPGenericChallenge;
class ISIPAckPackage;
class SIPConnectionNotifier;
class SIPAuHelper;



class SIPClientConnection
    : public SIPConnection
    , public ISIPClientTransactionStateListener
    , public ISIPClientTransmissionListener
{
public:
    SIPClientConnection();
    explicit SIPClientConnection(IN CONST AString &strTargetURI_);
    explicit SIPClientConnection(IN SIPClientTransactionState *pCTState_);
    virtual ~SIPClientConnection();

private:
    SIPClientConnection(IN CONST SIPClientConnection &objRHS);
    SIPClientConnection& operator=(IN CONST SIPClientConnection &objRHS);

public:
    // IConnection interface
    virtual void Close();

    // ISIPConnection interface
    virtual IMS_RESULT AddHeader(IN CONST AString &strName, IN CONST AString &strValue);
    virtual AString GetHeader(IN CONST AString &strName, IN IMS_SINT32 nIndex = 0);
    virtual IMSList<AString> GetHeaders(IN CONST AString &strName);
    virtual const SIPMethod& GetMethod() const;
    virtual const AString& GetReasonPhrase() const;
    virtual const AString& GetRequestURI() const;
    virtual IMS_SINT32 GetStatusCode() const;
    virtual IMS_RESULT RemoveHeader(IN CONST AString &strName);
    virtual IMS_RESULT Send();
    virtual IMS_RESULT SetHeader(IN CONST AString &strName, IN CONST AString &strValue);
    virtual const ByteArray& GetContent() const;
    virtual IMS_RESULT SetContent(IN CONST ByteArray &objContent);
    // IMS extensions
    virtual IMS_SINT32 GetHeaderCount(IN CONST AString &strName) const;
    // MULTI_REG_SIP_PROFILE
    virtual void SetSIPProfile(IN SIPProfile *pProfile);

    // ISIPClientConnection interface
    IMS_RESULT InitAck();
    SIPClientConnection* InitCancel();
    IMS_RESULT InitRequest(IN CONST AString &strMethod, IN SIPConnectionNotifier *pSCN);
    IMS_RESULT Receive(IN IMS_SLONG nTimeout = 0);
    IMS_RESULT SetCredentials(IN IMSList<Credential> &objCredentials);
    IMS_RESULT SetCredentials(IN CONST Credential &objCredential);
    void SetListener(IN IOnSIPClientConnectionListener *piListener);
    IMS_RESULT SetRequestURI(IN CONST AString &strURI);
    ISIPGenericChallenge* GetAuthenticationChallenge(IN IMS_SINT32 nIndex = 0) const;
    ISIPAckPackage* GrabAck();
    IMS_RESULT InitResubmissionRequest();
    void RemoveAllChallenges();
    void RemoveAllCredentials();
    IMS_RESULT SetAuthenticationChallenge(IN ISIPGenericChallenge *piChallenge);
    void SetExtensionTokenForViaBranch(IN CONST AString &strToken);
    void SetImplicitRouteHeader(IN CONST AString &strRouteHeader);
    void SetTransportTuple(IN CONST IPAddress &objIPA,
            IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFC = 0xFFFF,
            IN IMS_SINT32 nTransportExt = 0 /* ANY */);

    // Extension methods
    IMS_RESULT InitDialogRequest(IN CONST SIPMethod &objMethod,  IN SIPDialogEx *pDialogEx);
    IMS_RESULT SendWithCredentials();

private:
    // ISIPClientTransactionStateListener interface
    virtual void ClientTransactionState_ForkedResponseReceived(
            IN SIPClientTransactionState *pCTState);
    virtual void ClientTransactionState_ResponseReceived(IN SipMessage *pstMessage);

    // SIP_TRANSPORT_ERROR_REPORT_ON_TXN
    virtual IMS_BOOL IsTransportErrorReportRequired(IN IMS_SINT32 nCode,
            IN CONST AString &strMessage) const;

    // ISIPClientTransmissionListener class
    virtual void ClientTransmission_NotifyError(
            IN IMS_SINT32 nCode, IN CONST AString &strMessage);
    virtual void ClientTransmission_TransmissionCompleted();

    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_INITIALIZED,
        //STATE_STREAM_OPEN,
        STATE_PROCEEDING,
        STATE_COMPLETED,
        STATE_UNAUTHORIZED,
        STATE_TERMINATED
    };

private:
    static const AString ANONYMOUS_URI;

    IMS_SINT32 nState;
    IMS_BOOL bACKSent;
    IMS_BOOL bResubmissionRequestInitialized;

    // Request-URI should be equal to the INVITE request
    //    : CANCEL & ACK to non-2xx response
    AString strTargetURI;
    RCPtr<SIPClientTransactionState> pCTState;

    // Require & Proxy-Require headers to the INVITE request
    //IMSList<SIPHeader*> *pInviteRequires;    // TODO:: move to Session class (J281)

    IMSList<SIPMessage*> objResponseMessages;
    SIPAuHelper *pAuHelper;

    IOnSIPClientConnectionListener *piListener;

    // UDP_FALLBACK
    SIPClientTransmissionProxy *pTransmissionProxy;
};

#endif // _SIP_CLIENT_CONNECTION_H_
