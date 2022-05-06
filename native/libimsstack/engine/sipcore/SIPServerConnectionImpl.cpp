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
#include "SIPPrivate.h"
#include "ISipErrorListener.h"
#include "SIPDialogImpl.h"
#include "SIPServerConnection.h"
#include "SIPServerConnectionImpl.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPServerConnectionImpl::SIPServerConnectionImpl(IN SIPServerConnection* pSSC_) :
        piErrorListener(IMS_NULL),
        pDialogImpl(IMS_NULL),
        pSSC(pSSC_)
{
    pSSC->SetErrorListener(this);

    SIPDialog* pDialog = pSSC->GetDialog();

    if (pDialog != IMS_NULL)
    {
        pDialogImpl = new SIPDialogImpl(new SIPDialog(*pDialog));

        if (pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        }
    }
}

PUBLIC VIRTUAL SIPServerConnectionImpl::~SIPServerConnectionImpl()
{
    //---------------------------------------------------------------------------------------------

    if (pDialogImpl != IMS_NULL)
    {
        pDialogImpl->Destroy();
    }

    if (pSSC != IMS_NULL)
    {
        pSSC->SetErrorListener(IMS_NULL);
        pSSC->Close();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPServerConnectionImpl::Close()
{
    //---------------------------------------------------------------------------------------------

    if (pDialogImpl != IMS_NULL)
    {
        pDialogImpl->Destroy();
        pDialogImpl = IMS_NULL;
    }

    pSSC->SetErrorListener(IMS_NULL);
    pSSC->Close();
    pSSC = IMS_NULL;

    delete this;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::AddHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->AddHeader(strName, strValue);
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipDialog* SIPServerConnectionImpl::GetDialog() const
{
    //---------------------------------------------------------------------------------------------

    return pDialogImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL AString SIPServerConnectionImpl::GetHeader(
        IN CONST AString& strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetHeader(strName, nIndex);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMSList<AString> SIPServerConnectionImpl::GetHeaders(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetHeaders(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipMethod& SIPServerConnectionImpl::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetMethod();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& SIPServerConnectionImpl::GetReasonPhrase() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetReasonPhrase();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& SIPServerConnectionImpl::GetRequestUri() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetRequestURI();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPServerConnectionImpl::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetStatusCode();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::RemoveHeader(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->RemoveHeader(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::Send()
{
    //---------------------------------------------------------------------------------------------

    return pSSC->Send();
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPServerConnectionImpl::SetErrorListener(IN ISipErrorListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    piErrorListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::SetHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->SetHeader(strName, strValue);
}

/*

Remarks

*/
PRIVATE VIRTUAL const ByteArray& SIPServerConnectionImpl::GetContent() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetContent();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::SetContent(IN CONST ByteArray& objContent)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->SetContent(objContent);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPServerConnectionImpl::GetHeaderCount(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetHeaderCount(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipMessage* SIPServerConnectionImpl::GetMessage() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetMessage();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPServerConnectionImpl::GetSlotId() const
{
    //---------------------------------------------------------------------------------------------

    return pSSC->GetSlotId();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL void SIPServerConnectionImpl::SetSipProfile(IN SipProfile* pProfile)
{
    //---------------------------------------------------------------------------------------------

    pSSC->SetSIPProfile(pProfile);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPServerConnectionImpl::SetTransactionTimerValues(
        IN CONST SipTimerValues& objTV)
{
    //---------------------------------------------------------------------------------------------

    pSSC->SetTransactionTimerValues(objTV);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::InitResponse(IN IMS_SINT32 nStatusCode)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->InitResponse(nStatusCode);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPServerConnectionImpl::SetReasonPhrase(
        IN CONST AString& strReasonPhrase)
{
    //---------------------------------------------------------------------------------------------

    return pSSC->SetReasonPhrase(strReasonPhrase);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_BOOL SIPServerConnectionImpl::IsSameTransaction(
        IN CONST ISipServerConnection* piOngoingSSC) const
{
    const SIPServerConnectionImpl* pSSCImpl =
            DYNAMIC_CAST(const SIPServerConnectionImpl*, piOngoingSSC);

    //---------------------------------------------------------------------------------------------

    if (pSSCImpl == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FALSE;
    }

    if (pSSCImpl->pSSC == IMS_NULL)
    {
        // Ignore the CANCEL request
        // because the ongoing transaction is already completed or terminated.
        return IMS_FALSE;
    }

    return pSSC->IsSameTransaction(pSSCImpl->pSSC);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPServerConnectionImpl::OnError_NotifyError(
        IN SIPConnection* pSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pSSC != pSC)
    {
        IMS_TRACE_E(0, "SSC MISMATCHED", 0, 0, 0);
        return;
    }

    if (piErrorListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piErrorListener->Error_NotifyError(this, nCode, strMessage);
}
