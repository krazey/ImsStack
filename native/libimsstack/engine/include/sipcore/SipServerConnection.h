/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_SERVER_CONNECTION_H_
#define _SIP_SERVER_CONNECTION_H_

#include "SipConnection.h"

class SIPServerTransactionState;

class SIPServerConnection : public SIPConnection
{
public:
    explicit SIPServerConnection(IN SIPServerTransactionState* pSTState_);
    virtual ~SIPServerConnection();

private:
    SIPServerConnection(IN CONST SIPServerConnection& objRHS);
    SIPServerConnection& operator=(IN CONST SIPServerConnection& objRHS);

public:
    // IConnection interface
    virtual void Close();

    // ISipConnection interface
    virtual IMS_RESULT AddHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual AString GetHeader(IN CONST AString& strName, IN IMS_SINT32 nIndex = 0);
    virtual IMSList<AString> GetHeaders(IN CONST AString& strName);
    virtual const SipMethod& GetMethod() const;
    virtual const AString& GetReasonPhrase() const;
    virtual const AString& GetRequestURI() const;
    virtual IMS_SINT32 GetStatusCode() const;
    virtual IMS_RESULT RemoveHeader(IN CONST AString& strName);
    virtual IMS_RESULT Send();
    virtual IMS_RESULT SetHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual const ByteArray& GetContent() const;
    virtual IMS_RESULT SetContent(IN CONST ByteArray& objContent);
    // IMS extensions
    virtual IMS_SINT32 GetHeaderCount(IN CONST AString& strName) const;
    // MULTI_REG_SIP_PROFILE
    virtual void SetSIPProfile(IN SipProfile* pProfile);

    // ISIPServerTransaction interface
    IMS_RESULT InitResponse(IN IMS_SINT32 nStatusCode);
    IMS_RESULT SetReasonPhrase(IN CONST AString& strReasonPhrase);
    IMS_BOOL IsSameTransaction(IN CONST SIPServerConnection* pOngoingSSC) const;

    // Extension methods
    IMS_RESULT InitRequest();

private:
    void AdjustTimerHFor2XX();
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_REQUEST_RECEIVED,
        STATE_PROVISIONAL_RESPONDED,
        STATE_INITIALIZED,
        // STATE_STREAM_OPEN,
        STATE_COMPLETED,
        STATE_TERMINATED
    };

private:
    IMS_SINT32 nState;

    RCPtr<SIPServerTransactionState> pSTState;
};

#endif  // _SIP_SERVER_CONNECTION_H_
