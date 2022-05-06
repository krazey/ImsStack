/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090527  toastops@                 Created
    </table>

    Description
*/

#ifndef _CORE_SERVICE_H_
#define _CORE_SERVICE_H_

#include "Service.h"
#include "ReasonInfo.h"

class IOnCoreServiceListener;
class IOnDirectCoreServiceListener;
class Publication;
class Subscription;

class CoreService : public Service
{
public:
    CoreService(IN CONST AString& strAppId_, IN CONST AString& strServiceId_,
            IN CONST SipAddress* pIMPU_ = IMS_NULL);
    virtual ~CoreService();

private:
    CoreService(IN CONST CoreService& objRHS);
    CoreService& operator=(IN CONST CoreService& objRHS);

public:
    // IConnection interface
    virtual void Close();

    // Service class
    virtual void Abort();
    virtual void HandleSessionInvitationReceived(IN Session* pSession);
    virtual void HandlePageMessageReceived(IN PageMessage* pPageMessage);
    virtual void HandleReferenceReceived(IN Reference* pReference);
    virtual void HandleCapabilityQueryReceived(IN Capabilities* pCapabilities);

    // ICoreService interface
    Capabilities* CreateCapabilities(IN CONST AString& strFrom, IN CONST AString& strTo);
    PageMessage* CreatePageMessage(IN CONST AString& strFrom, IN CONST AString& strTo);
    Publication* CreatePublication(
            IN CONST AString& strFrom, IN CONST AString& strTo, IN CONST AString& strEvent);
    Reference* CreateReference(IN CONST AString& strFrom, IN CONST AString& strTo,
            IN CONST AString& strReferTo, IN CONST AString& strReferMethod);
    Session* CreateSession(IN CONST AString& strFrom, IN CONST AString& strTo);
    SessionEx* CreateSessionEx(IN CONST AString& strFrom, IN CONST AString& strTo);
    Subscription* CreateSubscription(
            IN CONST AString& strFrom, IN CONST AString& strTo, IN CONST AString& strEvent);
    AString GetLocalUserId() const;
    void SetListener(IN IOnCoreServiceListener* piListener);
    ISIPConnectionFactory* CreateSIPConnectionFactory();
    void SetDirectListener(IN IOnDirectCoreServiceListener* piListener);

private:
    virtual void Exception_NotifyError(IN IMS_SINT32 nErrorCode);
    virtual IMS_BOOL ServerConnection_NotifyRequest(IN ISipServerConnection* piSSC);

    IMS_SINT32 CheckAndHandleDirectSIPRequest(IN ISipServerConnection* piSSC);

private:
    // Refer to ICoreService class
    enum
    {
        // The transaction will be handled by the owner of direct listener.
        // The owner has a responsibility of the resource release to SIPServerConnection.
        RESULT_DIRECT_TXN_HANDLED = 0,
        // The transaction is not handled by the owner of direct listener.
        // The invoker should release the SIP server connection calling close() method.
        RESULT_DIRECT_TXN_NOT_HANDLED = 1,
        // The transaction is handled by the owner of direct listener.
        // But, it also should be handled by the default listener.
        // It's usage is only for SIP message modification after receiving the message.
        RESULT_DIRECT_TXN_BYPASS = 2
    };

    ReasonInfo objReasonInfo;
    IOnCoreServiceListener* piCoreServiceListener;
    IOnDirectCoreServiceListener* piDirectCoreServiceListener;
};

#endif  // _CORE_SERVICE_H_
