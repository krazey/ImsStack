/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SipPrivate.h"
#include "SipStackHeaders.h"
#include "SipHeaderName.h"
#include "IOnSipErrorListener.h"
#include "SipFactoryProxy.h"
// SIP_MESSAGE_TRACKER
#include "SipMessageTracker.h"
// SIP_IPSEC_STATE
#include "SipIpSecState.h"
#include "SipConnection.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPConnection::SIPConnection() :
        Connection(),
        piErrorListener(IMS_NULL),
        pDialog(IMS_NULL),
        pMessage(IMS_NULL),
        pTV(IMS_NULL)
{
}

PUBLIC VIRTUAL SIPConnection::~SIPConnection()
{
    //---------------------------------------------------------------------------------------------

    if (pDialog != IMS_NULL)
        delete pDialog;

    if (pMessage != IMS_NULL)
        delete pMessage;

    if (pTV != IMS_NULL)
        delete pTV;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPConnection::Close()
{
    //---------------------------------------------------------------------------------------------

    Connection::Close();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPConnection::AddHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    AString strHValue;

    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nHType = SIPStack::GetHeaderTypeFromName(strName);

    if (SIPConnection::IsReadOnlyHeader(nHType, strName) ||
            SIPConnection::IsInaccessibleHeader(nHType, strName))
    {
        SIPPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // Check dialog or subscription specific read only headers : From / To / Route

    strHValue = strValue.Trim();

    if (strHValue.IsNULL() || strHValue.IsEmpty())
    {
        if (pMessage->PrependHeader(nHType, AString::ConstEmpty(), strName) != IMS_SUCCESS)
            return IMS_FAILURE;
    }
    else
    {
        // TODO:: IMS_SINT32 nHCount = pMessage->GetHeaderCount(nHType, strHNameFF);
        /*
         *    " abc , cde, fgh " --> abc/cde/fgh
         */
        AStringArray objTokens;

        if (SIPConnection::IsCommaSeparatedListHeader(nHType, strName))
        {
            objTokens = strHValue.Split(TextParser::CHAR_COMMA);
        }
        else
        {
            objTokens.AddElement(strHValue);
        }

        for (IMS_SINT32 i = objTokens.GetCount() - 1; i >= 0; --i)
        {
            pMessage->PrependHeader(nHType, objTokens.GetElementAt(i), strName);

            // TODO:: If failed, remove the appended headers from the message (using nHCount)
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL SIPDialog* SIPConnection::GetDialog() const
{
    //---------------------------------------------------------------------------------------------

    return pDialog;

    /* TODO:: verify below logic

    if (pDialogImpl == IMS_NULL)
        return IMS_NULL;

    IMS_SINT32 nState = pDialogImpl->GetState();

    if ((nState == ISipDialog::STATE_EARLY)
            || (nState == ISipDialog::STATE_CONFIRMED))
    {
        return pDialogImpl;
    }

    return IMS_NULL;
    */
}

/*

Remarks

*/
PUBLIC VIRTUAL AString SIPConnection::GetHeader(
        IN CONST AString& strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return AString::ConstNull();
    }

    IMS_SINT32 nHType = SIPStack::GetHeaderTypeFromName(strName);

    if (SIPConnection::IsInaccessibleHeader(nHType, strName))
    {
        SIPPrivate::SetLastError(SipError::INVALID_OPERATION);
        return AString::ConstNull();
    }

    return pMessage->GetHeader(nHType, nIndex, strName);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMSList<AString> SIPConnection::GetHeaders(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMSList<AString>();
    }

    IMS_SINT32 nHType = SIPStack::GetHeaderTypeFromName(strName);

    if (SIPConnection::IsInaccessibleHeader(nHType, strName))
    {
        SIPPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMSList<AString>();
    }

    return pMessage->GetHeaders(nHType, strName);
}

/*

Remarks

*/
PUBLIC VIRTUAL const SipMethod& SIPConnection::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    return pMessage->GetMethod();
}

/*

Remarks

*/
PUBLIC VIRTUAL const AString& SIPConnection::GetReasonPhrase() const
{
    //---------------------------------------------------------------------------------------------

    return pMessage->GetReasonPhrase();
}

/*

Remarks

*/
PUBLIC VIRTUAL const AString& SIPConnection::GetRequestURI() const
{
    //---------------------------------------------------------------------------------------------

    return pMessage->GetRequestUri();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 SIPConnection::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    return pMessage->GetStatusCode();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPConnection::RemoveHeader(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nHType = SIPStack::GetHeaderTypeFromName(strName);

    if (SIPConnection::IsReadOnlyHeader(nHType, strName) ||
            SIPConnection::IsInaccessibleHeader(nHType, strName))
    {
        SIPPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    pMessage->RemoveHeader(nHType, strName);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPConnection::Send()
{
    //---------------------------------------------------------------------------------------------

    return IMS_FAILURE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void SIPConnection::SetErrorListener(IN IOnSIPErrorListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    piErrorListener = piListener;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPConnection::SetHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    AString strHValue;

    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    IMS_SINT32 nHType = SIPStack::GetHeaderTypeFromName(strName);

    if (SIPConnection::IsReadOnlyHeader(nHType, strName) ||
            SIPConnection::IsInaccessibleHeader(nHType, strName))
    {
        SIPPrivate::SetLastError(SipError::INVALID_OPERATION);
        return IMS_FAILURE;
    }

    // Check dialog or subscription specific read only headers : From / To / Route

    strHValue = strValue.Trim();

    if (strHValue.IsNULL() || strHValue.IsEmpty())
    {
        if (pMessage->SetHeader(nHType, AString::ConstEmpty(), strName) != IMS_SUCCESS)
            return IMS_FAILURE;
    }
    else
    {
        // TODO:: IMS_SINT32 nHCount = pMessage->GetHeaderCount(nHType, strHNameFF);
        /*
         *    " abc , cde, fgh " --> abc/cde/fgh
         */
        AStringArray objTokens;

        if (SIPConnection::IsCommaSeparatedListHeader(nHType, strName))
        {
            objTokens = strHValue.Split(TextParser::CHAR_COMMA);
        }
        else
        {
            objTokens.AddElement(strHValue);
        }

        pMessage->RemoveHeader(nHType, strName);

        // Overwritable header
        if (pMessage->PrependHeader(nHType, objTokens.GetElementAt(objTokens.GetCount() - 1),
                    strName) != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }

        for (IMS_SINT32 i = objTokens.GetCount() - 2; i >= 0; --i)
        {
            pMessage->PrependHeader(nHType, objTokens.GetElementAt(i), strName);
            // TODO:: If failed, remove the appended headers from the message (using nHCount)
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL const ByteArray& SIPConnection::GetContent() const
{
    SIPMessageBodyPart* pBodyPart = pMessage->GetBodyPart();

    //---------------------------------------------------------------------------------------------

    if (pBodyPart == IMS_NULL)
        return ByteArray::ConstNull();

    return pBodyPart->GetContent();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_RESULT SIPConnection::SetContent(IN CONST ByteArray& objContent)
{
    SIPMessageBodyPart* pBodyPart = pMessage->GetBodyPart();

    //---------------------------------------------------------------------------------------------

    // If the message body already exists, then throw exception ???

    if (pBodyPart == IMS_NULL)
    {
        pBodyPart = DYNAMIC_CAST(SIPMessageBodyPart*, pMessage->CreateBodyPart());

        if (pBodyPart == IMS_NULL)
        {
            SIPPrivate::SetLastError(SipError::NO_MEMORY);
            return IMS_FAILURE;
        }
    }

    pBodyPart->SetContent(objContent);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 SIPConnection::GetHeaderCount(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    if (strName.GetLength() == 0)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return 0;
    }

    IMS_SINT32 nHType = SIPStack::GetHeaderTypeFromName(strName);

    if (SIPConnection::IsInaccessibleHeader(nHType, strName))
    {
        SIPPrivate::SetLastError(SipError::INVALID_OPERATION);
        return 0;
    }

    return pMessage->GetHeaderCount(nHType, strName);
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PUBLIC VIRTUAL void SIPConnection::SetSIPProfile(IN SipProfile* /*pProfile*/)
{
    // no-op
}

/*

Remarks

*/
PUBLIC
void SIPConnection::SetTransactionTimerValues(IN CONST SipTimerValues& objTV)
{
    //---------------------------------------------------------------------------------------------

    if (pTV != IMS_NULL)
    {
        delete pTV;
    }

    pTV = new SipTimerValues(objTV);

    if (pTV == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SIP timer values failed", 0, 0, 0);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPConnection::TransactionState_TimerExpired()
{
    //---------------------------------------------------------------------------------------------

    // SIP_MESSAGE_TRACKER
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SIPMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());
        const SipMethod& objMethod = SIPConnection::GetMethod();

        if (pDialog != IMS_NULL)
        {
            pMessageTracker->NotifyMessageSent(objMethod, GetStatusCode(), pDialog->GetCallId(),
                    SipError::TRANSACTION_TIMER_EXPIRED);
        }
        else
        {
            ISipMessage* piSIPMsg = GetMessage();

            pMessageTracker->NotifyMessageSent(objMethod, GetStatusCode(),
                    (piSIPMsg != IMS_NULL) ? piSIPMsg->GetHeader(ISipHeader::CALL_ID)
                                           : AString::ConstNull(),
                    SipError::TRANSACTION_TIMER_EXPIRED);
        }
    }

    // SIP_IPSEC_STATE
    if (pFactoryProxy->IsIPSecStateEnabled(GetSlotId()) && (pMessage != IMS_NULL))
    {
        SIPIPSecState* pIPSecState = pFactoryProxy->GetIPSecState(GetSlotId());
        pIPSecState->NotifyMessageSentFailed(pMessage->GetMessage());
    }

    NotifyError(SipError::TRANSACTION_TIMER_EXPIRED, AString("Transaction Timer Expired"));
}

/*

Remarks

*/
PROTECTED VIRTUAL void SIPConnection::TransportError_NotifyError(
        IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    // SIP_TRANSPORT_ERROR_REPORT_ON_TXN
    if (!IsTransportErrorReportRequired(nCode, strMessage))
    {
        // Ignore the transport error...
        IMS_TRACE_D("Transport error (%d, %s) will be ignored...", nCode, strMessage.GetStr(), 0);
        return;
    }

    // SIP_MESSAGE_TRACKER
    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsMessageTrackerEnabled(GetSlotId()))
    {
        SIPMessageTracker* pMessageTracker = pFactoryProxy->GetMessageTracker(GetSlotId());

        if (pDialog != IMS_NULL)
        {
            pMessageTracker->NotifyMessageSent(
                    SIPConnection::GetMethod(), GetStatusCode(), pDialog->GetCallId(), nCode);
        }
        else
        {
            ISipMessage* piSIPMsg = GetMessage();

            pMessageTracker->NotifyMessageSent(SIPConnection::GetMethod(), GetStatusCode(),
                    (piSIPMsg != IMS_NULL) ? piSIPMsg->GetHeader(ISipHeader::CALL_ID)
                                           : AString::ConstNull(),
                    nCode);
        }
    }

    // SIP_IPSEC_STATE
    if (pFactoryProxy->IsIPSecStateEnabled(GetSlotId()) && (pMessage != IMS_NULL))
    {
        SIPIPSecState* pIPSecState = pFactoryProxy->GetIPSecState(GetSlotId());
        pIPSecState->NotifyMessageSentFailed(pMessage->GetMessage());
    }

    if (piErrorListener != IMS_NULL)
    {
        piErrorListener->OnError_NotifyError(this, nCode, strMessage);
    }
}

/*

Remarks
 SIP_TRANSPORT_ERROR_REPORT_ON_TXN
*/
PROTECTED VIRTUAL IMS_BOOL SIPConnection::IsTransportErrorReportRequired(
        IN IMS_SINT32 nCode, IN CONST AString& strMessage) const
{
    //---------------------------------------------------------------------------------------------

    if (nCode == SipError::TRANSPORT_ERROR)
    {
        AString strTECode;

        strTECode.Sprintf("%d", SipError::TRANSPORT_E_CODE_104);

        // SipError::TRANSPORT_E_CODE_104
        if (strMessage.StartsWith(strTECode))
        {
            // Ignore "Socket is closed by peer"(104) event notification
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED
SipTimerValues* SIPConnection::GetTransactionTimerValues() const
{
    //---------------------------------------------------------------------------------------------

    return pTV;
}

// IMS extensions
/*

Remarks

*/
PROTECTED
void SIPConnection::InitMessage(IN SIPMessage* pMessage_ /* = IMS_NULL */,
        IN IMS_SINT32 nType_ /* = SIPMessage::TYPE_REQUEST */)
{
    //---------------------------------------------------------------------------------------------

    if (pMessage != IMS_NULL)
    {
        delete pMessage;
        pMessage = IMS_NULL;
    }

    if (pMessage_ != IMS_NULL)
        pMessage = pMessage_;
    else
        pMessage = new SIPMessage(nType_);
}

/*

Remarks

*/
PROTECTED
void SIPConnection::NotifyError(IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    if (piErrorListener == IMS_NULL)
        return;

    piErrorListener->OnError_NotifyError(this, nCode, strMessage);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnection::IsCommaSeparatedListHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName)
{
    //---------------------------------------------------------------------------------------------

    // TODO:: configuration options
    (void)strHName;

    switch (nHType)
    {
        case ISipHeader::AUTHORIZATION:
        case ISipHeader::PROXY_AUTHORIZATION:
        case ISipHeader::WWW_AUTHENTICATE:
        case ISipHeader::PROXY_AUTHENTICATE:
        case ISipHeader::DATE:
            return IMS_FALSE;

        default:
            return IMS_TRUE;
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnection::IsInaccessibleHeader(IN IMS_SINT32 nHType, IN CONST AString& strHName)
{
    //---------------------------------------------------------------------------------------------

    switch (nHType)
    {
        case ISipHeader::AUTHORIZATION:
        case ISipHeader::CALL_ID:
        case ISipHeader::CSEQ:
        case ISipHeader::MIN_EXPIRES:
        case ISipHeader::MAX_FORWARDS:
        case ISipHeader::SERVICE_ROUTE:
        case ISipHeader::PROXY_AUTHENTICATE:
        case ISipHeader::PROXY_AUTHORIZATION:
        case ISipHeader::RECORD_ROUTE:
        case ISipHeader::SECURITY_SERVER:
        case ISipHeader::SECURITY_VERIFY:
        case ISipHeader::VIA:
        case ISipHeader::WWW_AUTHENTICATE:
            return IMS_TRUE;

        case ISipHeader::UNKNOWN:
            if ((strHName[0] == 'A') || (strHName[0] == 'a'))
            {
                if (strHName.EqualsIgnoreCase(SipHeaderName::AUTHENTICATION_INFO))
                    return IMS_TRUE;
            }
            return IMS_FALSE;

        default:
            return IMS_FALSE;
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnection::IsReadOnlyHeader(IN IMS_SINT32 nHType, IN CONST AString& /* strHName */)
{
    //---------------------------------------------------------------------------------------------

    switch (nHType)
    {
        case ISipHeader::AUTHORIZATION:
        case ISipHeader::MIN_EXPIRES:
        case ISipHeader::MAX_FORWARDS:
        case ISipHeader::SERVICE_ROUTE:
        case ISipHeader::PROXY_AUTHORIZATION:
        case ISipHeader::RECORD_ROUTE:
        case ISipHeader::SECURITY_SERVER:
        case ISipHeader::SECURITY_VERIFY:
        case ISipHeader::VIA:
            return IMS_TRUE;

        default:
            return IMS_FALSE;
    }
}
