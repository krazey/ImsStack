/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_SERVER_CONNECTION_IMPL_H_
#define _SIP_SERVER_CONNECTION_IMPL_H_

#include "ISipServerConnection.h"
#include "IOnSIPErrorListener.h"

class SIPDialogImpl;
class SIPServerConnection;

class SIPServerConnectionImpl : public ISipServerConnection, public IOnSIPErrorListener
{
public:
    explicit SIPServerConnectionImpl(IN SIPServerConnection* pSSC_);
    virtual ~SIPServerConnectionImpl();

private:
    SIPServerConnectionImpl();
    SIPServerConnectionImpl(IN CONST SIPServerConnectionImpl& objRHS);
    SIPServerConnectionImpl& operator=(IN CONST SIPServerConnectionImpl& objRHS);

private:
    // IConnection interface implementation
    virtual void Close();

    // ISipConnection interface implementation
    virtual IMS_RESULT AddHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual ISipDialog* GetDialog() const;
    virtual AString GetHeader(IN CONST AString& strName, IN IMS_SINT32 nIndex = 0);
    virtual IMSList<AString> GetHeaders(IN CONST AString& strName);
    virtual const SipMethod& GetMethod() const;
    virtual const AString& GetReasonPhrase() const;
    virtual const AString& GetRequestUri() const;
    virtual IMS_SINT32 GetStatusCode() const;
    virtual IMS_RESULT RemoveHeader(IN CONST AString& strName);
    virtual IMS_RESULT Send();
    virtual void SetErrorListener(IN ISipErrorListener* piListener);
    virtual IMS_RESULT SetHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual const ByteArray& GetContent() const;
    virtual IMS_RESULT SetContent(IN CONST ByteArray& objContent);
    virtual IMS_SINT32 GetHeaderCount(IN CONST AString& strName) const;
    virtual ISipMessage* GetMessage() const;
    virtual IMS_SINT32 GetSlotId() const;
    // MULTI_REG_SIP_PROFILE
    virtual void SetSipProfile(IN SipProfile* pProfile);
    virtual void SetTransactionTimerValues(IN CONST SipTimerValues& objTV);

    // ISIPServerTransaction interface
    virtual IMS_RESULT InitResponse(IN IMS_SINT32 nStatusCode);
    virtual IMS_RESULT SetReasonPhrase(IN CONST AString& strReasonPhrase);
    virtual IMS_BOOL IsSameTransaction(IN CONST ISipServerConnection* piOngoingSSC) const;

    // IOnSIPErrorListener interface
    virtual void OnError_NotifyError(
            IN SIPConnection* pSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

private:
    ISipErrorListener* piErrorListener;

    SIPDialogImpl* pDialogImpl;
    SIPServerConnection* pSSC;
};

#endif  // _SIP_SERVER_CONNECTION_IMPL_H_
