/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100506  hwangoo.park@             Created
    </table>

    Description
*/

#ifndef _PAGE_MESSAGE_H_
#define _PAGE_MESSAGE_H_

#include "ServiceMethod.h"

class IOnPageMessageListener;

class PageMessage : public ServiceMethod
{
public:
    explicit PageMessage(IN Service* pService);
    virtual ~PageMessage();

private:
    PageMessage(IN CONST PageMessage& objRHS);
    PageMessage& operator=(IN CONST PageMessage& objRHS);

public:
    // Method class
    virtual void Destroy();

    // IPageMessage interface
    const ByteArray& GetContent() const;
    AString GetContentType() const;
    IMS_SINT32 GetState() const;
    IMS_RESULT Send(IN CONST ByteArray& objContent, IN CONST AString& strContentType);
    void SetListener(IN IOnPageMessageListener* piListener);

    //// IMS extensions
    IMS_RESULT Accept(IN IMS_SINT32 nStatusCode = 200);
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0);

protected:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Method class
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piSCC);

    // Handlie the incoming request / outgoing response message
    virtual IMS_BOOL NotifySIPRequest(IN ISipServerConnection* piSSC);

    // Handle to the outgoing request / incoming response message
    virtual void NotifySIPResponse(IN ISipClientConnection* piSCC);
    virtual void NotifySIPError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

private:
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to the state in IPageMessage
    enum
    {
        STATE_UNSENT = 1,
        STATE_SENT = 2,
        STATE_RECEIVED = 3
    };

protected:
    enum
    {
        AMSG_PAGE_MESSAGE_RECEIVED = AMSG_USER,
        AMSG_PAGE_MESSAGE_DELIVERED,
        AMSG_PAGE_MESSAGE_DELIVERY_FAILED,
        AMSG_PAGE_MESSAGE_MAX
    };

private:
    IMS_SINT32 nState;
    IOnPageMessageListener* piListener;
};

#endif  // _PAGE_MESSAGE_H_
