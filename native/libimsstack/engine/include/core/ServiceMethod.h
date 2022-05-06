/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090729  toastops@                 Created
    </table>

    Description
*/

#ifndef _SERVICE_METHOD_H_
#define _SERVICE_METHOD_H_

#include "IMSMap.h"
#include "base/Method.h"
#include "Message.h"

class Service;
class PreviousMessage;

class ServiceMethod : public Method
{
public:
    explicit ServiceMethod(IN Service* pService_);
    virtual ~ServiceMethod();

private:
    ServiceMethod(IN CONST ServiceMethod& objRHS);
    ServiceMethod& operator=(IN CONST ServiceMethod& objRHS);

public:
    // IMethod interface
    virtual void Destroy();

    // IServiceMethod interface
    Message* GetNextRequest();
    Message* GetNextResponse();
    Message* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const;
    Message* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const;
    IMSList<Message*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const;
    IMSList<AString> GetRemoteUserId() const;

protected:
    // MULTI_SUBS
    virtual const AString& GetSubscriberId() const;

    void ClearConnection(IN IMS_SINT32 nServiceMethod);
    void CloseConnection(IN IMS_SINT32 nServiceMethod);
    void CopyPreviousMessage(IN IMS_SINT32 nServiceMethod_From, IN IMS_SINT32 nServiceMethod_To);
    ISipClientConnection* CreateCancelConnection(IN ISipClientConnection* piSCC);
    ISipClientConnection* CreateConnection(IN CONST SipMethod& objMethod);
    ISipClientConnection* CreateConnection(IN ISipDialog* piDialog, IN CONST SipMethod& objMethod);
    IMS_BOOL CreateResponse(IN_OUT ISipServerConnection* piSSC, IN IMS_SINT32 nStatusCode,
            IN CONST AString& strPhrase = AString::ConstNull());
    ISipClientConnection* GetClientConnection(IN IMS_SINT32 nServiceMethod) const;
    ISipServerConnection* GetServerConnection(IN IMS_SINT32 nServiceMethod) const;
    Service* GetService() const;
    IMS_BOOL IsPrivacyRequested(IN IMS_BOOL bRequest = IMS_TRUE) const;
    IMS_BOOL IsServiceOpen() const;
    IMS_BOOL RemovePreviousMessage(IN IMS_SINT32 nServiceMethod);
    IMS_BOOL SendNUpdateRequest(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);
    IMS_BOOL SendNUpdateResponse(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);
    IMS_BOOL SendNUpdateRequestEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC,
            IN IMS_SINT32 nMessageClass = MESSAGE_CLASS_NORMAL);
    IMS_BOOL SendNUpdateResponseEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC,
            IN IMS_SINT32 nMessageClass = MESSAGE_CLASS_NORMAL);
    void UpdateConnection(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);
    IMS_BOOL UpdateRequestOnReceived(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);
    IMS_BOOL UpdateRequestOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);
    IMS_BOOL UpdateResponseOnReceived(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);
    IMS_BOOL UpdateResponseOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSC);

private:
    IMS_BOOL AddPreviousResponse(IN IMS_SINT32 nServiceMethod, IN Message* pMessage);
    IMS_BOOL SetPreviousRequest(
            IN IMS_SINT32 nServiceMethod, IN Message* pMessage, IN ISipConnection* piSC);
    ISipConnection* GetConnection(IN IMS_SINT32 nServiceMethod) const;

private:
    // Reference to Service
    Service* pService;

    // Message object to outgoing SIP request message
    Message* pNextRequest;
    // Message object to outgoing SIP response message
    Message* pNextResponse;
    // Storage for the previous SIP message according to the method type
    IMSMap<IMS_SINT32, PreviousMessage*> objPreviousMessages;
};

#endif  //_SERVICE_METHOD_H_
