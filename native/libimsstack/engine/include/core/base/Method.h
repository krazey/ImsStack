/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090729  toastops@                 Created
    </table>

    Description

*/

#ifndef _METHOD_H_
#define _METHOD_H_

// IMS_AUTH_SIP_DIGEST
#include "IMSMap.h"
#include "EngineActivity.h"
#include "Sip.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "ISipClientConnection.h"
#include "SipAddress.h"
// SIP_MESSAGE_MEDIATOR
#include "base/IMessageMediator.h"

class ISIPGenericChallenge; // IMS_AUTH_SIP_DIGEST
class ISIPServerConnection;
class IReferredMessageListener;



class Method
    : public EngineActivity
    , public ISIPClientConnectionListener
    , public ISIPErrorListener
{
public:
    Method();
    virtual ~Method();

private:
    Method(IN CONST Method &objRHS);
    Method& operator=(IN CONST Method &objRHS);

public:
    // IMethod interface
    virtual void Destroy();
    // SIP_MESSAGE_MEDIATOR
    virtual void SetMessageMediator(IN IMessageMediator *piMediator);

    // Overridable method to handle an Exceptions.
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode);
    virtual IMS_BOOL SetReferredMessageListener(IN IReferredMessageListener *piListener);

    IMS_BOOL Equals(IN CONST Method *pMethod) const;
    IMS_BOOL InitMethod(IN CONST AString &strFrom, IN CONST AString &strTo,
            IN CONST SIPAddress &objUserAOR, IN IMS_BOOL bMobileOriginated = IMS_TRUE);
    IMS_BOOL InitMethod(IN CONST Method *pMethod, IN IMS_BOOL bMobileOriginated = IMS_TRUE);
    ISIPDialog* GetDialog() const;
    IMS_BOOL ServerConnection_NotifyRequest(IN ISIPServerConnection *piSSC);

protected:
    // Overridable methods
    virtual IMS_BOOL InitInstance();
    virtual IMS_BOOL NotifySIPRequest(IN ISIPServerConnection *piSSC);
    virtual IMS_BOOL NotifySIPForkedResponse(IN ISIPClientConnection *piSCC,
            IN ISIPClientConnection *piForkedSCC);
    virtual void NotifySIPResponse(IN ISIPClientConnection *piSCC) = 0;
    virtual void NotifySIPError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
            IN CONST AString &strMessage) = 0;

    // MULTI_SUBS
    virtual const AString& GetSubscriberId() const;
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISIPClientConnection *piSCC);

    // SIP_MESSAGE_MEDIATOR
    IMS_RESULT AdjustMessage(IN_OUT ISIPMessage *piSIPMsg,
            IN IMS_SINT32 nMessage = MESSAGE_CLASS_NORMAL);
    void CheckNCreateDialog(IN ISIPConnection *piSC, IN IMS_BOOL bDestroy = IMS_FALSE,
            IN IMS_BOOL bTerminatedDialogRequired = IMS_FALSE);
    void DestroyDialog();
    const SIPAddress* GetUserAOR() const;
    const SIPAddress* GetRemoteUserAOR() const;
    const IMSList<AString>& GetRemoteUserIds() const;
    IMS_BOOL HandleAllSIPResponse(IN ISIPClientConnection *piSCC);
    IMS_BOOL IsMobileOriginated() const;

    // IMS_AUTH_SIP_DIGEST
    void ResetChallengeCount(IN ISIPClientConnection *piSCC);
    IMS_BOOL RespondToChallenge(IN ISIPClientConnection *piSCC);
    IMS_BOOL SetChallengeNCredentials(IN ISIPClientConnection *piSCC);

    void UpdateRemoteUserIds(IN ISIPConnection *piSC);

private:
    // ISIPClientConnectionListener interface
    virtual void ClientConnection_NotifyResponse(IN ISIPClientConnection *piSCC,
            IN ISIPClientConnection *piForkedSCC = IMS_NULL);
    // ISIPErrorListener interface
    virtual void Error_NotifyError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
            IN CONST AString &strMessage);

public:
    class SCCListener
        : public ISIPErrorListener
        , public ISIPClientConnectionListener
    {
    public:
        SCCListener();
        virtual ~SCCListener();

    protected:
        virtual void Error_NotifyError(IN ISIPConnection *piSC, IN IMS_SINT32 nCode,
                IN CONST AString &strMessage);

        virtual void ClientConnection_NotifyResponse(IN ISIPClientConnection *piSCC,
                IN ISIPClientConnection *piForkedSCC = IMS_NULL);
    };

public:
    // SIP_MESSAGE_MEDIATOR
    // Re-define the message category for Method class
    enum
    {
        // Message for standalone or mid-dialog
        MESSAGE_CLASS_NORMAL = IMessageMediator::MESSAGE_NORMAL,
        // Message for re-submitted request only (request for auth. challenge)
        MESSAGE_CLASS_RESUBMIT = IMessageMediator::MESSAGE_RESUBMIT,
        // Message for refresh operation
        MESSAGE_CLASS_REFRESH = IMessageMediator::MESSAGE_REFRESH,
        // Message sent automatically by Engine
        MESSAGE_CLASS_AUTOMATIC = IMessageMediator::MESSAGE_AUTOMATIC,
        // Message sent internally by Engine
        MESSAGE_CLASS_INTERNAL_BYE = IMessageMediator::MESSAGE_INTERNAL_BYE
    };

private:
    // Direction of method
    IMS_BOOL bFlag_MobileOriginated;

    // Logical identity of the initiator of the request; From header field in SIP message
    SIPAddress *pUserAOR;
    // Logical identity of the recipient of the request; To header field in SIP message
    SIPAddress *pRemoteUserAOR;
    // Remote asserted user identities; from P-Asserted-Identity header in SIP message
    IMSList<AString> objRemoteUserIds;

    // Pointer to ISIPDialog object
    ISIPDialog *piDialog;

    // If the authentication is failed for the consecutive three times,
    // then it considers that the method can't be progressing anymore.
    static const IMS_SINT32 MAX_CHALLENGE_COUNT = 2;
    // Authentication challenge which is received from 401/407 response
    // when SIP digest authentication is used
    ISIPGenericChallenge *piAuthChallenge;
    // Authentication challenge counts
    IMSMap<IMS_SINT32, IMS_SINT32> objAuthChallengeMap;

    // It gives a chance to modify the message before sending the SIP message to the network.
    // The application can control the SIP header & message body part using this.
    IMessageMediator *piMessageMediator;
};

#endif // _METHOD_H_
