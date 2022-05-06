/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_CONNECTION_H_
#define _SIP_CONNECTION_H_

#include "Connection.h"
#include "ISIPTransactionStateListener.h"
#include "ISIPTransportErrorListener.h"
#include "SIPMessage.h"
#include "SIPDialog.h"
#include "SipTimerValues.h"

class SipProfile;
class IOnSIPErrorListener;

class SIPConnection :
        public Connection,
        public ISIPTransactionStateListener,
        public ISIPTransportErrorListener
{
protected:
    SIPConnection();
    virtual ~SIPConnection();

private:
    SIPConnection(IN CONST SIPConnection& objRHS);
    SIPConnection& operator=(IN CONST SIPConnection& objRHS);

public:
    // IConnection interface
    virtual void Close();

    // ISipConnection interface
    virtual IMS_RESULT AddHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual SIPDialog* GetDialog() const;
    virtual AString GetHeader(IN CONST AString& strName, IN IMS_SINT32 nIndex = 0);
    virtual IMSList<AString> GetHeaders(IN CONST AString& strName);
    virtual const SipMethod& GetMethod() const;
    virtual const AString& GetReasonPhrase() const;
    virtual const AString& GetRequestURI() const;
    virtual IMS_SINT32 GetStatusCode() const;
    virtual IMS_RESULT RemoveHeader(IN CONST AString& strName);
    virtual IMS_RESULT Send();
    virtual void SetErrorListener(IN IOnSIPErrorListener* piListener);
    virtual IMS_RESULT SetHeader(IN CONST AString& strName, IN CONST AString& strValue);
    virtual const ByteArray& GetContent() const;
    virtual IMS_RESULT SetContent(IN CONST ByteArray& objContent);
    // IMS extensions
    virtual IMS_SINT32 GetHeaderCount(IN CONST AString& strName) const;
    inline virtual ISipMessage* GetMessage() const { return pMessage; }
    // MULTI_REG_SIP_PROFILE
    virtual void SetSIPProfile(IN SipProfile* pProfile);
    void SetTransactionTimerValues(IN CONST SipTimerValues& objTV);

protected:
    // ISIPTransactionStateListener interface
    virtual void TransactionState_TimerExpired();

    // ISIPTransportErrorListener interface
    virtual void TransportError_NotifyError(IN IMS_SINT32 nCode, IN CONST AString& strMessage);

    // SIP_TRANSPORT_ERROR_REPORT_ON_TXN
    virtual IMS_BOOL IsTransportErrorReportRequired(
            IN IMS_SINT32 nCode, IN CONST AString& strMessage) const;

    SipTimerValues* GetTransactionTimerValues() const;
    void InitMessage(
            IN SIPMessage* pMessage_ = IMS_NULL, IN IMS_SINT32 nType_ = SIPMessage::TYPE_REQUEST);
    void NotifyError(IN IMS_SINT32 nCode, IN CONST AString& strMessage);

private:
    static IMS_BOOL IsCommaSeparatedListHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName);
    static IMS_BOOL IsInaccessibleHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName);
    static IMS_BOOL IsReadOnlyHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName);

protected:
    IOnSIPErrorListener* piErrorListener;

    SIPDialog* pDialog;
    SIPMessage* pMessage;

    // SIP transaction timer values
    SipTimerValues* pTV;
};

#endif  // _SIP_CONNECTION_H_
