/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20150210  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "SystemConfig.h"
#include "NatHelper.h"

PRIVATE
NATHelper::NATHelper() :
        ppBindings(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppBindings = new IMSList<IPBinding>*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        ppBindings[i] = new IMSList<IPBinding>();
    }
}

PRIVATE
NATHelper::~NATHelper()
{
    if (ppBindings != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppBindings[i] != IMS_NULL)
            {
                delete ppBindings[i];
            }
        }

        delete[] ppBindings;
    }
}

/*

Remarks

*/
PUBLIC
void NATHelper::Clear(IN IMS_SINT32 nSlotId)
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings != IMS_NULL)
    {
        pBindings->Clear();
    }
}

/*

Remarks

*/
PUBLIC
IPAddress NATHelper::GetPrivateAddress(IN IMS_SINT32 nSlotId, IN const IPAddress& objPublicIP) const
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return IPAddress::NONE;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IPBinding& objIPB = pBindings->GetAt(i);

        if (objPublicIP.Equals(objIPB.GetPublicIP()))
        {
            return objIPB.GetPrivateIP();
        }
    }

    return IPAddress::NONE;
}

/*

Remarks

*/
PUBLIC
IPAddress NATHelper::GetPublicAddress(IN IMS_SINT32 nSlotId, IN const IPAddress& objPrivateIP) const
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return IPAddress::NONE;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IPBinding& objIPB = pBindings->GetAt(i);

        if (objPrivateIP.Equals(objIPB.GetPrivateIP()))
        {
            return objIPB.GetPublicIP();
        }
    }

    return IPAddress::NONE;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL NATHelper::IsBehindNAT(
        IN IMS_SINT32 nSlotId, IN const IPAddress& objPrivateIP /* = IPAddress::NONE */) const
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IPBinding& objIPB = pBindings->GetAt(i);
        const IPAddress& objPublicIP = objIPB.GetPublicIP();

        if (!objPrivateIP.Equals(IPAddress::NONE) && !objPrivateIP.Equals(objIPB.GetPrivateIP()))
        {
            continue;
        }

        if (!objPublicIP.Equals(IPAddress::NONE) && !objPublicIP.Equals(IPAddress::IPv6NONE))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC
void NATHelper::RemovePublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId)
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize();)
    {
        const IPBinding& objIPB = pBindings->GetAt(i);

        if (nId == objIPB.GetId())
        {
            pBindings->RemoveAt(i);
        }
        else
        {
            ++i;
        }
    }
}

/*

Remarks

*/
PUBLIC
void NATHelper::RemovePublicAddress(IN IMS_SINT32 nSlotId, IN const IPAddress& objPrivateIP)
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IPBinding& objIPB = pBindings->GetAt(i);

        if (objPrivateIP.Equals(objIPB.GetPrivateIP()))
        {
            pBindings->RemoveAt(i);
            break;
        }
    }
}

/*

Remarks

*/
PUBLIC
void NATHelper::SetPublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId,
        IN CONST IPAddress& objPrivateIP, IN CONST IPAddress& objPublicIP)
{
    RemoveIPBinding(nSlotId, nId, objPrivateIP);

    if (!objPublicIP.Equals(IPAddress::NONE) && !objPublicIP.Equals(IPAddress::IPv6NONE))
    {
        IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

        if (pBindings != IMS_NULL)
        {
            pBindings->Append(IPBinding(nId, objPrivateIP, objPublicIP));
        }
    }
}

/*

Remarks

*/
PUBLIC GLOBAL NATHelper* NATHelper::GetInstance()
{
    static NATHelper* pNATHelper = IMS_NULL;

    if (pNATHelper == IMS_NULL)
    {
        pNATHelper = new NATHelper();
    }

    return pNATHelper;
}

/*

Remarks

*/
PUBLIC GLOBAL IMS_BOOL NATHelper::IsNATResolverRequired()
{
    // FIXME: SKT only requires, but it's not used in the moment.
    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE
void NATHelper::RemoveIPBinding(
        IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId, IN const IPAddress& objPrivateIP)
{
    IMSList<IPBinding>* pBindings = GetIPBindings(nSlotId);

    if (pBindings == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pBindings->GetSize(); ++i)
    {
        const IPBinding& objIPB = pBindings->GetAt(i);

        if ((nId == objIPB.GetId()) && objPrivateIP.Equals(objIPB.GetPrivateIP()))
        {
            pBindings->RemoveAt(i);
            break;
        }
    }
}

/*

Remarks

*/
PRIVATE
IMSList<NATHelper::IPBinding>* NATHelper::GetIPBindings(IN IMS_SINT32 nSlotId) const
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        nSlotId = IMS_SLOT_0;
    }

    return ppBindings[nSlotId];
}
