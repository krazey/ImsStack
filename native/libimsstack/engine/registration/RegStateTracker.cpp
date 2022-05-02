/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101207  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "RegContact.h"
#include "SipConfigProxy.h"
#include "RegStateTracker.h"

__IMS_TRACE_TAG_REG__;



PUBLIC
RegStateTracker::RegStateTracker()
    : RCObject()
    , strSubsId(AString::ConstNull())
    , pAuthorizedAOR(IMS_NULL)
    , objIPAddress(IPAddress::NONE)
    , objPublicIPAddress(IPAddress::IPv6NONE)
    , pContactAddressForOutgoingMessage(IMS_NULL)
    , pPreferredContact(IMS_NULL)
    , nTransportExt(0)
    , nPortFlowControl(0)
    , nPortUC(0)
    , nPortUS(0)
    , pSIPProfile(IMS_NULL)
{
}

PUBLIC
RegStateTracker::RegStateTracker(IN const RegStateTracker& objRHS)
    : RCObject(objRHS)
    , strSubsId(objRHS.strSubsId)
    , pAuthorizedAOR(IMS_NULL)
    , objIPAddress(objRHS.objIPAddress)
    , objPublicIPAddress(objRHS.objPublicIPAddress)
    , pContactAddressForOutgoingMessage(IMS_NULL)
    , pPreferredContact(objRHS.pPreferredContact)
    , nTransportExt(objRHS.nTransportExt)
    , nPortFlowControl(objRHS.nPortFlowControl)
    , nPortUC(objRHS.nPortUC)
    , nPortUS(objRHS.nPortUS)
    , pSIPProfile(objRHS.pSIPProfile)
{
    if (objRHS.pAuthorizedAOR != IMS_NULL)
    {
        pAuthorizedAOR = new SIPAddress(*(objRHS.pAuthorizedAOR));
    }

    if (objRHS.pContactAddressForOutgoingMessage != IMS_NULL)
    {
        pContactAddressForOutgoingMessage = new SIPAddress(
                *(objRHS.pContactAddressForOutgoingMessage));
    }
}

PUBLIC VIRTUAL
RegStateTracker::~RegStateTracker()
{
    if (pAuthorizedAOR != IMS_NULL)
    {
        delete pAuthorizedAOR;
    }

    if (pContactAddressForOutgoingMessage != IMS_NULL)
    {
        delete pContactAddressForOutgoingMessage;
    }

    IMS_TRACE_D("Destructor :: RegStateTracker", 0, 0, 0);
}

PUBLIC
const SIPAddress& RegStateTracker::GetAOR() const
{
    //---------------------------------------------------------------------------------------------

    return objAOR;
}

PUBLIC
const AStringArray& RegStateTracker::GetAssociatedURIs() const
{
    //---------------------------------------------------------------------------------------------

    return objAssociatedURIs;
}

PUBLIC
const SIPAddress& RegStateTracker::GetAuthorizedAOR() const
{
    //---------------------------------------------------------------------------------------------

    if (pAuthorizedAOR == IMS_NULL)
        return objAOR;

    return (*pAuthorizedAOR);
}

PUBLIC
const SIPAddress& RegStateTracker::GetContactAddress() const
{
    //---------------------------------------------------------------------------------------------

    if (pPreferredContact != IMS_NULL)
    {
        return pPreferredContact->GetContactAddress();
    }

    return objPreferredContactAddress;
}

PUBLIC
const SIPAddress* RegStateTracker::GetContactAddressForOutgoingMessage() const
{
    //---------------------------------------------------------------------------------------------

    return pContactAddressForOutgoingMessage;
}

PUBLIC
const IPAddress& RegStateTracker::GetIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objIPAddress;
}

PUBLIC
const AStringArray& RegStateTracker::GetPathHeaders() const
{
    //---------------------------------------------------------------------------------------------

    return objPaths;
}

PUBLIC
IMS_SINT32 RegStateTracker::GetPortFlowControl() const
{
    //---------------------------------------------------------------------------------------------

    return nPortFlowControl;
}

PUBLIC
IMS_SINT32 RegStateTracker::GetPortUC() const
{
    //---------------------------------------------------------------------------------------------

    return nPortUC;
}

PUBLIC
IMS_SINT32 RegStateTracker::GetPortUS() const
{
    //---------------------------------------------------------------------------------------------

    return nPortUS;
}

PUBLIC
const RegContact* RegStateTracker::GetPreferredContact() const
{
    //---------------------------------------------------------------------------------------------

    return pPreferredContact;
}

PUBLIC
const IPAddress& RegStateTracker::GetPublicIPAddress() const
{
    //---------------------------------------------------------------------------------------------

    return objPublicIPAddress;
}

PUBLIC
const AStringArray& RegStateTracker::GetSecurityClients() const
{
    //---------------------------------------------------------------------------------------------

    return objSecurityClients;
}

PUBLIC
const AStringArray& RegStateTracker::GetSecurityVerifys() const
{
    //---------------------------------------------------------------------------------------------

    return objSecurityVerifys;
}

PUBLIC
const AStringArray& RegStateTracker::GetServiceRoutes() const
{
    //---------------------------------------------------------------------------------------------

    return objServiceRoutes;
}

PUBLIC
SIPProfile* RegStateTracker::GetSIPProfile() const
{
    return pSIPProfile.Get();
}

PUBLIC
const AString& RegStateTracker::GetSubscriberId() const
{
    //---------------------------------------------------------------------------------------------

    return strSubsId;
}

PUBLIC
IMS_SINT32 RegStateTracker::GetTransportExt() const
{
    //---------------------------------------------------------------------------------------------

    return nTransportExt;
}

PUBLIC
IMS_BOOL RegStateTracker::IsWithinTrustDomain(IN IMS_SINT32 nSlotId) const
{
    //---------------------------------------------------------------------------------------------

    return SIPConfigProxy::IsTrustDomainConfigured(nSlotId, GetSIPProfile());
}

PRIVATE
void RegStateTracker::SetAOR(IN CONST SIPAddress &objAOR)
{
    //---------------------------------------------------------------------------------------------

    this->objAOR = objAOR;
}

PRIVATE
void RegStateTracker::SetAssociatedURIs(IN CONST AStringArray &objAssociatedURIs)
{
    //---------------------------------------------------------------------------------------------

    this->objAssociatedURIs = objAssociatedURIs;

    // The topmost user identity is an authorized & registered explicitly by the network
    if (this->objAssociatedURIs.IsEmpty())
    {
        if (pAuthorizedAOR != IMS_NULL)
        {
            delete pAuthorizedAOR;
            pAuthorizedAOR = IMS_NULL;
        }
    }
    else
    {
        const AString &strIMPU = this->objAssociatedURIs.GetFirstElement();

        if (pAuthorizedAOR != IMS_NULL)
        {
            delete pAuthorizedAOR;
            pAuthorizedAOR = IMS_NULL;
        }

        pAuthorizedAOR = new SIPAddress();

        if (pAuthorizedAOR != IMS_NULL)
        {
            if (!pAuthorizedAOR->Create(strIMPU))
            {
                IMS_TRACE_E(0, "Creating an authorized AOR failed", 0, 0, 0);
            }
        }
        else
        {
            IMS_TRACE_E(0, "Creating an authorized AOR failed", 0, 0, 0);
        }
    }
}

PRIVATE
void RegStateTracker::SetPathHeaders(IN CONST AStringArray &objPaths)
{
    //---------------------------------------------------------------------------------------------

    this->objPaths = objPaths;
}

PRIVATE
void RegStateTracker::SetPortFlowControl(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    nPortFlowControl = nPort;
}

PRIVATE
void RegStateTracker::SetPortUC(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    nPortUC = nPort;
}

PRIVATE
void RegStateTracker::SetPortUS(IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    nPortUS = nPort;
}

PRIVATE
void RegStateTracker::SetPreferredContact(IN RegContact *pContact)
{
    //---------------------------------------------------------------------------------------------

    this->pPreferredContact = pContact;

    if (this->pPreferredContact != IMS_NULL)
    {
        objIPAddress = this->pPreferredContact->GetIPAddress();
        objPreferredContactAddress = this->pPreferredContact->GetContactAddress();
    }
}

PRIVATE
void RegStateTracker::SetPublicIPAddress(IN CONST IPAddress &objIP)
{
    //---------------------------------------------------------------------------------------------

    objPublicIPAddress = objIP;
}

PRIVATE
void RegStateTracker::SetSecurityClients(IN CONST IMSList<SIPSecurityHeader> &objClients)
{
    //---------------------------------------------------------------------------------------------

    objSecurityClients.RemoveAllElements();

    for (IMS_UINT32 i = 0; i < objClients.GetSize(); ++i)
    {
        const SIPSecurityHeader &objHeader = objClients.GetAt(i);

        objSecurityClients.AddElement(objHeader.ToString());
    }
}

PRIVATE
void RegStateTracker::SetSecurityVerifys(IN CONST IMSList<SIPSecurityHeader> &objVerifys)
{
    //---------------------------------------------------------------------------------------------

    objSecurityVerifys.RemoveAllElements();

    for (IMS_UINT32 i = 0; i < objVerifys.GetSize(); ++i)
    {
        const SIPSecurityHeader &objHeader = objVerifys.GetAt(i);

        objSecurityVerifys.AddElement(objHeader.ToString());
    }
}

PRIVATE
void RegStateTracker::SetServiceRoutes(IN CONST AStringArray &objServiceRoutes)
{
    //---------------------------------------------------------------------------------------------

    this->objServiceRoutes = objServiceRoutes;
}

PRIVATE
void RegStateTracker::SetSIPProfile(IN SIPProfile *pProfile)
{
    //---------------------------------------------------------------------------------------------

    this->pSIPProfile = pProfile;
}

PRIVATE
void RegStateTracker::SetSubscriberId(IN CONST AString &strSubsId)
{
    //---------------------------------------------------------------------------------------------

    this->strSubsId = strSubsId;
}

PRIVATE
void RegStateTracker::SetTransportExt(IN IMS_SINT32 nTransportExt)
{
    //---------------------------------------------------------------------------------------------

    this->nTransportExt = nTransportExt;
}

PRIVATE
void RegStateTracker::SetUserInfoForContactHeader(IN CONST AString &strUserInfo)
{
    //---------------------------------------------------------------------------------------------

    if (pContactAddressForOutgoingMessage != IMS_NULL)
    {
        delete pContactAddressForOutgoingMessage;
        pContactAddressForOutgoingMessage = IMS_NULL;
    }

    if (strUserInfo.IsNULL())
    {
        return;
    }

    pContactAddressForOutgoingMessage = new SIPAddress(GetContactAddress());

    if (pContactAddressForOutgoingMessage != IMS_NULL)
    {
        pContactAddressForOutgoingMessage->SetUser(strUserInfo);
    }
}
