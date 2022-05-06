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
#include "ServiceTrace.h"
#include "SIPPrivate.h"
#include "ISipErrorListener.h"
#include "ISipClientConnectionListener.h"
#include "SIPConnectionNotifierImpl.h"
#include "SIPDialogImpl.h"
#include "SIPClientConnection.h"
#include "SIPClientConnectionImpl.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPClientConnectionImpl::SIPClientConnectionImpl(IN SIPClientConnection* pSCC_) :
        piErrorListener(IMS_NULL),
        piListener(IMS_NULL),
        pDialogImpl(IMS_NULL),
        pSCC(pSCC_)
{
    pSCC->SetErrorListener(this);
    pSCC->SetListener(this);
}

PUBLIC VIRTUAL SIPClientConnectionImpl::~SIPClientConnectionImpl()
{
    //---------------------------------------------------------------------------------------------

    if (pDialogImpl != IMS_NULL)
    {
        pDialogImpl->Destroy();
    }

    if (pSCC != IMS_NULL)
    {
        pSCC->SetErrorListener(IMS_NULL);
        pSCC->SetListener(IMS_NULL);
        pSCC->Close();
    }
}

/*

Remarks

*/
PUBLIC
IMS_RESULT SIPClientConnectionImpl::InitDialogRequest()
{
    //---------------------------------------------------------------------------------------------

    if (pSCC == IMS_NULL)
    {
        IMS_TRACE_E(0, "SCC is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (pDialogImpl == IMS_NULL)
    {
        SIPDialog* pDialog = pSCC->GetDialog();

        if (pDialog == IMS_NULL)
        {
            IMS_TRACE_E(0, "Dialog is null", 0, 0, 0);
            return IMS_FAILURE;
        }

        pDialogImpl = new SIPDialogImpl(new SIPDialog(*pDialog));

        if (pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Creating SIPDialogImpl failed", 0, 0, 0);
            return IMS_FAILURE;
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::Close()
{
    //---------------------------------------------------------------------------------------------

    if (pDialogImpl != IMS_NULL)
    {
        pDialogImpl->Destroy();
        pDialogImpl = IMS_NULL;
    }

    pSCC->SetErrorListener(IMS_NULL);
    pSCC->SetListener(IMS_NULL);
    pSCC->Close();
    pSCC = IMS_NULL;

    delete this;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::AddHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->AddHeader(strName, strValue);
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipDialog* SIPClientConnectionImpl::GetDialog() const
{
    //---------------------------------------------------------------------------------------------

    return pDialogImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL AString SIPClientConnectionImpl::GetHeader(
        IN CONST AString& strName, IN IMS_SINT32 nIndex /* = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetHeader(strName, nIndex);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMSList<AString> SIPClientConnectionImpl::GetHeaders(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetHeaders(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL const SipMethod& SIPClientConnectionImpl::GetMethod() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetMethod();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& SIPClientConnectionImpl::GetReasonPhrase() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetReasonPhrase();
}

/*

Remarks

*/
PRIVATE VIRTUAL const AString& SIPClientConnectionImpl::GetRequestUri() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetRequestURI();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPClientConnectionImpl::GetStatusCode() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetStatusCode();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::RemoveHeader(IN CONST AString& strName)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->RemoveHeader(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::Send()
{
    IMS_RESULT nResult = pSCC->Send();

    // in-dialog & dialogUsage used
    if (nResult == IMS_SUCCESS)
    {
        SIPDialog* pDialog = (pDialogImpl != IMS_NULL) ? pDialogImpl->GetDialog() : IMS_NULL;
        SIPDialog* pSccDialog = pSCC->GetDialog();

        if ((pSccDialog != IMS_NULL) && (pDialog != IMS_NULL) &&
                (pDialog->GetState() == SIPDialog::STATE_CONFIRMED))
        {
            const SipMethod& objMethod = GetMethod();

            if (objMethod.Equals(SipMethod::SUBSCRIBE) || objMethod.Equals(SipMethod::REFER))
            {
                *pDialog = *pSccDialog;
            }
        }
    }

    return nResult;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetErrorListener(IN ISipErrorListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    piErrorListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::SetHeader(
        IN CONST AString& strName, IN CONST AString& strValue)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->SetHeader(strName, strValue);
}

/*

Remarks

*/
PRIVATE VIRTUAL const ByteArray& SIPClientConnectionImpl::GetContent() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetContent();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::SetContent(IN CONST ByteArray& objContent)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->SetContent(objContent);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPClientConnectionImpl::GetHeaderCount(IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetHeaderCount(strName);
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipMessage* SIPClientConnectionImpl::GetMessage() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetMessage();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPClientConnectionImpl::GetSlotId() const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetSlotId();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetSipProfile(IN SipProfile* pProfile)
{
    //---------------------------------------------------------------------------------------------

    pSCC->SetSIPProfile(pProfile);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetTransactionTimerValues(
        IN CONST SipTimerValues& objTV)
{
    //---------------------------------------------------------------------------------------------

    pSCC->SetTransactionTimerValues(objTV);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::InitAck()
{
    //---------------------------------------------------------------------------------------------

    return pSCC->InitAck();
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipClientConnection* SIPClientConnectionImpl::InitCancel()
{
    //---------------------------------------------------------------------------------------------

    // 3 To-Tag removal needs to be handled by the user because the re-INVITE may be cancelled
    // 3 Session implementation has the responsibility of the to-tag removal.

    SIPClientConnection* pCANCEL = pSCC->InitCancel();

    if (pCANCEL == IMS_NULL)
    {
        return IMS_NULL;
    }

    SIPClientConnectionImpl* pCANCELImpl = new SIPClientConnectionImpl(pCANCEL);

    if (pCANCELImpl == IMS_NULL)
    {
        SIPPrivate::SetLastError(SipError::CONNECTION_NOT_FOUND);
        return IMS_NULL;
    }

    return pCANCELImpl;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::InitRequest(
        IN CONST AString& strMethod, IN ISipConnectionNotifier* piSCN)
{
    SIPConnectionNotifierImpl* pSCNImpl = DYNAMIC_CAST(SIPConnectionNotifierImpl*, piSCN);
    SIPConnectionNotifier* pSCN = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pSCNImpl != IMS_NULL)
    {
        pSCN = pSCNImpl->GetConnectionNotifier();
    }

    if (pSCC->InitRequest(strMethod, pSCN) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    SIPDialog* pDialog = pSCC->GetDialog();

    if (pDialog != IMS_NULL)
    {
        pDialogImpl = new SIPDialogImpl(new SIPDialog(*pDialog));

        if (pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        }
    }

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::Receive(IN IMS_SLONG /* nTimeout = 0 */)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->Receive();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::SetCredentials(
        IN IMSList<Credential>& objCredentials)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->SetCredentials(objCredentials);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::SetCredentials(
        IN CONST Credential& objCredential)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->SetCredentials(objCredential);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetListener(
        IN ISipClientConnectionListener* piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::SetRequestUri(IN CONST AString& strURI)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->SetRequestURI(strURI);
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipGenericChallenge* SIPClientConnectionImpl::GetAuthenticationChallenge(
        IN IMS_SINT32 nIndex /* = 0 */) const
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GetAuthenticationChallenge(nIndex);
}

/*

Remarks

*/
PRIVATE VIRTUAL ISipAckPackage* SIPClientConnectionImpl::GrabAck()
{
    //---------------------------------------------------------------------------------------------

    return pSCC->GrabAck();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::InitResubmissionRequest()
{
    //---------------------------------------------------------------------------------------------

    return pSCC->InitResubmissionRequest();
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::RemoveAllChallenges()
{
    //---------------------------------------------------------------------------------------------

    pSCC->RemoveAllChallenges();
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::RemoveAllCredentials()
{
    //---------------------------------------------------------------------------------------------

    pSCC->RemoveAllCredentials();
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPClientConnectionImpl::SetAuthenticationChallenge(
        IN ISipGenericChallenge* piChallenge)
{
    //---------------------------------------------------------------------------------------------

    return pSCC->SetAuthenticationChallenge(piChallenge);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetExtensionTokenForViaBranch(
        IN CONST AString& strToken)
{
    //---------------------------------------------------------------------------------------------

    pSCC->SetExtensionTokenForViaBranch(strToken);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetImplicitRouteHeader(
        IN CONST AString& strRouteHeader)
{
    //---------------------------------------------------------------------------------------------

    pSCC->SetImplicitRouteHeader(strRouteHeader);
}

/*

Remarks
 RFC5626_FLOW_CONTROL, MULTI_REG_TRANSPORT
*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::SetTransportTuple(IN CONST IPAddress& objIPA,
        IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFC /* = 0xFFFF */,
        IN IMS_SINT32 nTransportExt /* = 0 (ANY) */)
{
    //---------------------------------------------------------------------------------------------

    pSCC->SetTransportTuple(objIPA, nPortS, nPortC, nPortFC, nTransportExt);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::OnError_NotifyError(
        IN SIPConnection* pSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage)
{
    //---------------------------------------------------------------------------------------------

    if (pSCC != pSC)
    {
        IMS_TRACE_E(0, "SCC MISMATCHED", 0, 0, 0);
        return;
    }

    if (piErrorListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piErrorListener->Error_NotifyError(this, nCode, strMessage);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::OnClientConnection_NotifyResponse(
        IN SIPClientConnection* pSCC)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSCC != pSCC)
    {
        IMS_TRACE_E(0, "SCC MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ClientConnection_NotifyResponse(this);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPClientConnectionImpl::OnClientConnection_NotifyForkedResponse(
        IN SIPClientConnection* pSCC, IN SIPClientConnection* pForkedSCC)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSCC != pSCC)
    {
        IMS_TRACE_E(0, "SCC MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        pForkedSCC->Close();

        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    SIPClientConnectionImpl* pSCCImpl = new SIPClientConnectionImpl(pForkedSCC);

    if (pSCCImpl == IMS_NULL)
    {
        pForkedSCC->Close();
        return;
    }

    SIPDialog* pDialog = pForkedSCC->GetDialog();

    if (pDialog != IMS_NULL)
    {
        pSCCImpl->pDialogImpl = new SIPDialogImpl(new SIPDialog(*pDialog));

        if (pSCCImpl->pDialogImpl == IMS_NULL)
        {
            IMS_TRACE_E(0, "Allocating DialogImpl failed", 0, 0, 0);
        }
    }

    piListener->ClientConnection_NotifyResponse(this, pSCCImpl);
}
