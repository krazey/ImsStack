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
#include "SIPConnectionNotifier.h"
#include "ISipServerConnectionListener.h"
#include "ISipConnectionNotifierErrorListener.h"
#include "SIPConnectionNotifierImpl.h"

__IMS_TRACE_TAG_SIP__;



PUBLIC
SIPConnectionNotifierImpl::SIPConnectionNotifierImpl(IN SIPConnectionNotifier *pSCN_)
    : piListener(IMS_NULL)
    , pSCN(pSCN_)
{
    pSCN->SetListener(this);
    pSCN->SetErrorListener(this);
}

PUBLIC VIRTUAL
SIPConnectionNotifierImpl::~SIPConnectionNotifierImpl()
{
    //---------------------------------------------------------------------------------------------

    if (pSCN != IMS_NULL)
    {
        pSCN->SetListener(IMS_NULL);
        pSCN->SetErrorListener(IMS_NULL);
        pSCN->Close();
    }
}

/*

Remarks

*/
PUBLIC
SIPConnectionNotifier* SIPConnectionNotifierImpl::GetConnectionNotifier() const
{
    //---------------------------------------------------------------------------------------------

    return pSCN;
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::Close()
{
    //---------------------------------------------------------------------------------------------

    objErrorListeners.Clear();

    if (pSCN != IMS_NULL)
    {
        pSCN->SetListener(IMS_NULL);
        pSCN->SetErrorListener(IMS_NULL);
        pSCN->Close();
        pSCN = IMS_NULL;
    }

    delete this;
}

/*

Remarks

*/
PRIVATE VIRTUAL
ISIPServerConnection* SIPConnectionNotifierImpl::AcceptAndOpen()
{
    //---------------------------------------------------------------------------------------------

    return pSCN->AcceptAndOpen();
}

/*

Remarks

*/
PRIVATE VIRTUAL
const IPAddress& SIPConnectionNotifierImpl::GetLocalAddress() const
{
    //---------------------------------------------------------------------------------------------

    return pSCN->GetLocalAddress();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_SINT32 SIPConnectionNotifierImpl::GetLocalPort() const
{
    //---------------------------------------------------------------------------------------------

    return pSCN->GetLocalPort();
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::SetListener(IN ISIPServerConnectionListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    this->piListener = piListener;
}

/*

Remarks

*/
PRIVATE VIRTUAL
ISIPServerConnection* SIPConnectionNotifierImpl::AcceptAndOpen(OUT ISIPDialog *&piOrigDialog)
{
    //---------------------------------------------------------------------------------------------

    return pSCN->AcceptAndOpen(piOrigDialog);
}

/*

Remarks

*/
PRIVATE VIRTUAL
AString SIPConnectionNotifierImpl::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    return pSCN->GetContactAddress();
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL
SIPProfile* SIPConnectionNotifierImpl::GetSIPProfile() const
{
    //---------------------------------------------------------------------------------------------

    return pSCN->GetSIPProfile();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_SINT32 SIPConnectionNotifierImpl::GetSlotId() const
{
    //---------------------------------------------------------------------------------------------

    return pSCN->GetSlotId();
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_BOOL SIPConnectionNotifierImpl::IsTransportResourceReserved(
        IN IMS_SINT32 nType/* = TRANSPORT_ALL*/) const
{
    //---------------------------------------------------------------------------------------------

    return pSCN->IsTransportResourceReserved(nType);
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_RESULT SIPConnectionNotifierImpl::ReserveTransportResource(IN CONST IPAddress &objIPA,
        IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl)
{
    //---------------------------------------------------------------------------------------------

    return pSCN->ReserveTransportResource(objIPA, nPortS, nPortC, nPortFlowControl);
}

/*

Remarks

*/
PRIVATE VIRTUAL
IMS_RESULT SIPConnectionNotifierImpl::RestoreTransportResource(IN IMS_SINT32 nType,
        IN CONST IPAddress &objPeerIP, IN IMS_SINT32 nPeerPort)
{
    //---------------------------------------------------------------------------------------------

    return pSCN->RestoreTransportResource(nType, objPeerIP, nPeerPort);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::SetFilter(IN CONST AString &strFilter)
{
    //---------------------------------------------------------------------------------------------

    pSCN->SetFilter(strFilter);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::SetFromAndContact(IN CONST AString &strFrom,
        IN CONST AString &strDisplayName, IN CONST AString &strUserInfo)
{
    //---------------------------------------------------------------------------------------------

    pSCN->SetFromAndContact(strFrom, strDisplayName, strUserInfo);
}

/*

Remarks
 MULTI_REG_SIP_PROFILE
*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::SetSIPProfile(IN SIPProfile *pProfile)
{
    //---------------------------------------------------------------------------------------------

    pSCN->SetSIPProfile(pProfile);
}

/*

Remarks
 RFC5626_FLOW_CONTROL
*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::UpdatePortFlowControl(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    pSCN->UpdatePortFlowControl(nPort);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::UpdatePortUC(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    pSCN->UpdatePortUC(nPort);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::AddErrorListener(IN ISIPConnectionNotifierErrorListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objErrorListeners.GetSize(); ++i)
    {
        ISIPConnectionNotifierErrorListener *piErrorListener = objErrorListeners.GetAt(i);

        if (piErrorListener == piListener)
        {
            IMS_TRACE_D("SCNImpl :: ErrorListener(%p) is already added", piListener, 0, 0);
            return;
        }
    }

    objErrorListeners.Append(piListener);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::RemoveErrorListener(IN ISIPConnectionNotifierErrorListener *piListener)
{
    //---------------------------------------------------------------------------------------------

    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objErrorListeners.GetSize(); ++i)
    {
        ISIPConnectionNotifierErrorListener *piErrorListener = objErrorListeners.GetAt(i);

        if (piErrorListener == piListener)
        {
            objErrorListeners.RemoveAt(i);
            break;
        }
    }

    if (objErrorListeners.IsEmpty())
    {
        IMS_TRACE_D("SCNImpl :: No error listeners", 0, 0, 0);
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::OnServerConnection_NotifyRequest(IN SIPConnectionNotifier *pSCN)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSCN != pSCN)
    {
        IMS_TRACE_E(0, "SCN MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ServerConnection_NotifyRequest(this);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::OnServerConnection_NotifyForkedRequest(
        IN SIPConnectionNotifier *pSCN)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSCN != pSCN)
    {
        IMS_TRACE_E(0, "SCN MISMATCHED", 0, 0, 0);
        return;
    }

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    piListener->ServerConnection_NotifyRequest(this, IMS_TRUE);
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierImpl::OnConnectionNotifierError_NotifyError(
        IN SIPConnectionNotifier *pSCN, IN IMS_SINT32 nCode, IN CONST AString &strMessage)
{
    //---------------------------------------------------------------------------------------------

    if (this->pSCN != pSCN)
    {
        IMS_TRACE_E(0, "SCN MISMATCHED", 0, 0, 0);
        return;
    }

    if (objErrorListeners.IsEmpty())
    {
        IMS_TRACE_E(0, "NO LISTENER", 0, 0, 0);
        return;
    }

    IMSList<ISIPConnectionNotifierErrorListener*> objTempListeners = objErrorListeners;

    for (IMS_UINT32 i = 0; i < objTempListeners.GetSize(); ++i)
    {
        ISIPConnectionNotifierErrorListener *piErrorListener = objTempListeners.GetAt(i);

        if (piErrorListener != IMS_NULL)
        {
            piErrorListener->ConnectionNotifierError_NotifyError(this, nCode, strMessage);
        }
    }
}
