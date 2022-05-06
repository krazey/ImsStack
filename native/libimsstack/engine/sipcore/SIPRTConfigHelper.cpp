/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110528  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "SipDebug.h"
#include "SIPPortManager.h"
#include "SIPRTConfigHelper.h"

__IMS_TRACE_TAG_SIP__;

PUBLIC
SIPRTConfigHelper::SIPRTConfigHelper() :
        objConfigSet(IMSMap<IMS_SINT32, IMS_BOOL>()),
        nFeatures(SipRtConfig::FEATURE_NONE),
        objIPQoSs(IMSList<SipRtConfig::IpQos>()),
        objHeaders(IMSList<SipRtConfig::Header>()),
        objIPSecSAs(IMSList<SipRtConfig::IpSecSa>())
{
    for (IMS_SINT32 i = 0; i < SipRtConfig::CONFIG_I_MAX; ++i)
    {
        objConfigSet.Add(i, IMS_FALSE);
    }
}

PUBLIC VIRTUAL SIPRTConfigHelper::~SIPRTConfigHelper() {}

/*

Remarks

*/
PUBLIC VIRTUAL const SipRtConfig::Header* SIPRTConfigHelper::GetHeader(
        IN CONST AString& strName) const
{
    //---------------------------------------------------------------------------------------------

    if (objHeaders.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const SipRtConfig::Header& objHeader = objHeaders.GetAt(i);

        if (strName.EqualsIgnoreCase(objHeader.strName))
        {
            return &objHeader;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
const SipRtConfig::LogMask* SIPRTConfigHelper::GetLogMask() const
{
    //---------------------------------------------------------------------------------------------

    return &objLogMask;
}

/*

Remarks

*/
PUBLIC
const SipRtConfig::SocketOption* SIPRTConfigHelper::GetSocketOption(IN IMS_SINT32 nItem) const
{
    IMS_SLONG nIndex = objSOMap.GetIndexOfKey(nItem);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    const IMSList<SipRtConfig::SocketOption>& objSOs = objSOMap.GetValueAt(nIndex);

    if (objSOs.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objSOs.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSO = objSOs.GetAt(i);

        if (objSO.IsGlobalOption())
        {
            return &objSO;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
const SipRtConfig::SocketOption* SIPRTConfigHelper::GetSocketOption(
        IN IMS_SINT32 nItem, IN CONST IPAddress& objIP, IN IMS_SINT32 nPort /* = 0*/) const
{
    //---------------------------------------------------------------------------------------------

    if ((nPort == 0) && (objIP.Equals(IPAddress::NONE) || objIP.Equals(IPAddress::IPv6NONE)))
    {
        return GetSocketOption(nItem);
    }

    IMS_SLONG nIndex = objSOMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    const IMSList<SipRtConfig::SocketOption>& objSOs = objSOMap.GetValueAt(nIndex);

    if (objSOs.IsEmpty())
    {
        return IMS_NULL;
    }

    // Condition: IP & Port
    for (IMS_UINT32 i = 0; i < objSOs.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSO = objSOs.GetAt(i);

        if ((objSO.nPort == nPort) && objIP.Equals(objSO.objIpAddr))
        {
            return &objSO;
        }
    }

    // Condition: IP
    for (IMS_UINT32 i = 0; i < objSOs.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSO = objSOs.GetAt(i);

        if ((objSO.nPort == 0) && objIP.Equals(objSO.objIpAddr))
        {
            return &objSO;
        }
    }

    // Condition: Port
    for (IMS_UINT32 i = 0; i < objSOs.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSO = objSOs.GetAt(i);

        if ((objSO.objIpAddr.Equals(IPAddress::NONE) ||
                    objSO.objIpAddr.Equals(IPAddress::IPv6NONE)) &&
                (objSO.nPort == nPort))
        {
            return &objSO;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
const SipRtConfig::IpQos* SIPRTConfigHelper::GetIpQos(
        IN CONST IPAddress& objIP, IN IMS_SINT32 nPort /* = 0 */) const
{
    //---------------------------------------------------------------------------------------------

    if (objIPQoSs.IsEmpty())
    {
        IMS_TRACE_D("No IP-level QoS supports", 0, 0, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objIPQoSs.GetSize(); ++i)
    {
        const SipRtConfig::IpQos& objIPQoS = objIPQoSs.GetAt(i);

        if (objIP.Equals(objIPQoS.objIpAddr))
        {
            if (objIPQoS.nPort == 0)
            {
                IMS_TRACE_D("IP-level QoS supports - IP (%s)", SipDebug::GetIp(objIP), 0, 0);
                return &objIPQoS;
            }
            else
            {
                if (nPort == objIPQoS.nPort)
                {
                    IMS_TRACE_D("IP-level QoS supports - IP (%s), Port (%d)",
                            SipDebug::GetIp(objIP), nPort, 0);
                    return &objIPQoS;
                }
            }
        }
    }

    IMS_TRACE_D("No IP-level QoS supports", 0, 0, 0);

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
const IMSList<SipRtConfig::IpSecSa>& SIPRTConfigHelper::GetIpSecSas() const
{
    //---------------------------------------------------------------------------------------------

    return objIPSecSAs;
}

/*

Remarks

*/
PUBLIC
const SipAddress* SIPRTConfigHelper::GetRegContactUri(IN CONST AString& strCallId) const
{
    //---------------------------------------------------------------------------------------------

    if (objRegContacts.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objRegContacts.GetSize(); ++i)
    {
        const SipRtConfig::RegContactAddress& objRegContactA = objRegContacts.GetAt(i);

        if (strCallId.Equals(objRegContactA.GetCallId()))
        {
            const SipAddress& objUri = objRegContactA.GetUri();
            return &objUri;
        }
    }

    return IMS_NULL;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPRTConfigHelper::IsFeatureEnabled(IN IMS_SINT32 nFeature) const
{
    //---------------------------------------------------------------------------------------------

    return (nFeatures & nFeature) != 0;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPRTConfigHelper::IsItemConfigured(IN IMS_SINT32 nItem) const
{
    IMS_SLONG nIndex = objConfigSet.GetIndexOfKey(nItem);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return IMS_FALSE;
    }

    return objConfigSet.GetValueAt(nIndex);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPRTConfigHelper::IsMessageHiddenInLog() const
{
    //---------------------------------------------------------------------------------------------

    return ((objLogMask.nValue & SipRtConfig::LogMask::I_MESSAGE_HIDDEN) != 0);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPRTConfigHelper::IsRoutingInfoHiddenInLog() const
{
    //---------------------------------------------------------------------------------------------

    return ((objLogMask.nValue & SipRtConfig::LogMask::I_ROUTING_INFO_HIDDEN) != 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPRTConfigHelper::DisableFeature(IN IMS_SINT32 nFeature)
{
    //---------------------------------------------------------------------------------------------

    nFeatures &= (~nFeature);
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPRTConfigHelper::EnableFeature(IN IMS_SINT32 nFeature)
{
    //---------------------------------------------------------------------------------------------

    nFeatures |= nFeature;
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_SINT32 SIPRTConfigHelper::GetFeatures() const
{
    //---------------------------------------------------------------------------------------------

    return nFeatures;
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPRTConfigHelper::RemoveConfig(
        IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam)
{
    //---------------------------------------------------------------------------------------------

    switch (nItem)
    {
        case SipRtConfig::CONFIG_I_LOG_MASK:
            objLogMask.nValue = SipRtConfig::LogMask::I_NONE;
            break;

        case SipRtConfig::CONFIG_I_REUSEADDR:       // FALL-THROUGH
        case SipRtConfig::CONFIG_I_LINGER:          // FALL-THROUGH
        case SipRtConfig::CONFIG_I_SHUTDOWN:        // FALL-THROUGH
        case SipRtConfig::CONFIG_I_KEEPALIVE:       // FALL-THROUGH
        case SipRtConfig::CONFIG_I_TCP_KEEP_COUNT:  // FALL-THROUGH
        case SipRtConfig::CONFIG_I_TCP_KEEP_IDLE:   // FALL-THROUGH
        case SipRtConfig::CONFIG_I_TCP_KEEP_INTERVAL:
            RemoveSocketOption(nItem, DYNAMIC_CAST(SipRtConfig::SocketOption*, pParam));

            if (GetSocketOptionCount(nItem) != 0)
            {
                IMS_TRACE_D("SipRtConfig :: SocketOption(%d) is removed; sizeOfMap=%d", nItem,
                        objSOMap.GetSize(), 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_IP_QOS:
            if (objIPQoSs.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                objIPQoSs.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < objIPQoSs.GetSize(); ++i)
            {
                const SipRtConfig::IpQos& objIPQoS = objIPQoSs.GetAt(i);

                if (objIPQoS.Equals(*pParam))
                {
                    objIPQoSs.RemoveAt(i);
                    break;
                }
            }

            if (!objIPQoSs.IsEmpty())
            {
                IMS_TRACE_D("SipRtConfig :: IpQos is removed; size=%d", objIPQoSs.GetSize(), 0, 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_SIP_HEADER:
            if (objHeaders.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                objHeaders.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
            {
                const SipRtConfig::Header& objHeader = objHeaders.GetAt(i);

                if (objHeader.Equals(*pParam))
                {
                    objHeaders.RemoveAt(i);
                    break;
                }
            }

            if (!objHeaders.IsEmpty())
            {
                IMS_TRACE_D(
                        "SipRtConfig :: Header is removed; size=%d", objHeaders.GetSize(), 0, 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_IPSEC_SA:
            if (objIPSecSAs.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                objIPSecSAs.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < objIPSecSAs.GetSize(); ++i)
            {
                const SipRtConfig::IpSecSa& objIPSecSA = objIPSecSAs.GetAt(i);

                if (objIPSecSA.Equals(*pParam))
                {
                    objIPSecSAs.RemoveAt(i);
                    break;
                }
            }

            if (!objIPSecSAs.IsEmpty())
            {
                IMS_TRACE_D(
                        "SipRtConfig :: IpSecSa is removed; size=%d", objIPSecSAs.GetSize(), 0, 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_TCP_PORT_RANGE:
        {
            SIPPortManager::GetInstance()->Clear();
        }
        break;

        case SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS:
            if (objRegContacts.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                objRegContacts.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < objRegContacts.GetSize(); ++i)
            {
                const SipRtConfig::RegContactAddress& objRegContactA = objRegContacts.GetAt(i);

                if (objRegContactA.Equals(*pParam))
                {
                    objRegContacts.RemoveAt(i);
                    break;
                }
            }

            if (!objRegContacts.IsEmpty())
            {
                IMS_TRACE_D("SipRtConfig :: RegContactAddress is removed; size=%d",
                        objRegContacts.GetSize(), 0, 0);
                return;
            }
            break;

        default:
            IMS_TRACE_E(0, "Unsupported configuration item (%d)", nItem, 0, 0);
            return;
    }

    IMS_SLONG nIndex = objConfigSet.GetIndexOfKey(nItem);

    if (nIndex >= 0)
    {
        objConfigSet.SetValueAt(nIndex, IMS_FALSE);
    }

    IMS_TRACE_D("SIP run-time configuration (%d) is removed", nItem, 0, 0);
}

/*

Remarks

*/
PRIVATE VIRTUAL IMS_RESULT SIPRTConfigHelper::SetConfig(
        IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam)
{
    //---------------------------------------------------------------------------------------------

    if (pParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
        return IMS_FAILURE;
    }

    switch (nItem)
    {
        case SipRtConfig::CONFIG_I_LOG_MASK:
        {
            SipRtConfig::LogMask* pLogMask = DYNAMIC_CAST(SipRtConfig::LogMask*, pParam);

            if (pLogMask == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            objLogMask.nValue = pLogMask->nValue;
        }
        break;

        case SipRtConfig::CONFIG_I_REUSEADDR:       // FALL-THROUGH
        case SipRtConfig::CONFIG_I_LINGER:          // FALL-THROUGH
        case SipRtConfig::CONFIG_I_SHUTDOWN:        // FALL-THROUGH
        case SipRtConfig::CONFIG_I_KEEPALIVE:       // FALL-THROUGH
        case SipRtConfig::CONFIG_I_TCP_KEEP_COUNT:  // FALL-THROUGH
        case SipRtConfig::CONFIG_I_TCP_KEEP_IDLE:   // FALL-THROUGH
        case SipRtConfig::CONFIG_I_TCP_KEEP_INTERVAL:
            if (SetSocketOption(nItem, DYNAMIC_CAST(SipRtConfig::SocketOption*, pParam)) !=
                    IMS_SUCCESS)
            {
                return IMS_FAILURE;
            }

            if (GetSocketOptionCount(nItem) > 1)
            {
                IMS_TRACE_D("SipRtConfig :: SocketOption(%d) is set; sizeOfMap=%d", nItem,
                        objSOMap.GetSize(), 0);
                return IMS_SUCCESS;
            }
            break;

        case SipRtConfig::CONFIG_I_IP_QOS:
        {
            SipRtConfig::IpQos* pIPQoS = DYNAMIC_CAST(SipRtConfig::IpQos*, pParam);

            if (pIPQoS == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < objIPQoSs.GetSize(); ++i)
            {
                SipRtConfig::IpQos& objIPQoS = objIPQoSs.GetAt(i);

                if (objIPQoS.Equals(*pIPQoS))
                {
                    // Only updates the value
                    objIPQoS.nValue = pIPQoS->nValue;
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!objIPQoSs.Append(*pIPQoS))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (objIPQoSs.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig :: IpQos is set; size=%d", objIPQoSs.GetSize(), 0, 0);
                return IMS_SUCCESS;
            }
        }
        break;

        case SipRtConfig::CONFIG_I_SIP_HEADER:
        {
            SipRtConfig::Header* pHeader = DYNAMIC_CAST(SipRtConfig::Header*, pParam);

            if (pHeader == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
            {
                SipRtConfig::Header& objHeader = objHeaders.GetAt(i);

                if (objHeader.Equals(*pHeader))
                {
                    // Only updates the value
                    objHeader.strParameter = pHeader->strParameter;
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!objHeaders.Append(*pHeader))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (objHeaders.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig :: Header is set; size=%d", objHeaders.GetSize(), 0, 0);
                return IMS_SUCCESS;
            }
        }
        break;

        case SipRtConfig::CONFIG_I_IPSEC_SA:
        {
            SipRtConfig::IpSecSa* pIPSecSA = DYNAMIC_CAST(SipRtConfig::IpSecSa*, pParam);

            if (pIPSecSA == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < objIPSecSAs.GetSize(); ++i)
            {
                SipRtConfig::IpSecSa& objIPSecSA = objIPSecSAs.GetAt(i);

                if (objIPSecSA.Equals(*pIPSecSA))
                {
                    // Already exists.
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!objIPSecSAs.Append(*pIPSecSA))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (objIPSecSAs.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig :: IpSecSa is set; size=%d", objIPSecSAs.GetSize(), 0, 0);
                return IMS_SUCCESS;
            }
        }
        break;

        case SipRtConfig::CONFIG_I_TCP_PORT_RANGE:
        {
            SipRtConfig::TcpPortRange* pPortRange =
                    DYNAMIC_CAST(SipRtConfig::TcpPortRange*, pParam);

            if (pPortRange == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (pPortRange->nPortStart <= 0)
            {
                RemoveConfig(nItem, pPortRange);
                return IMS_SUCCESS;
            }

            SIPPortManager::GetInstance()->SetPortC(pPortRange->nPortStart, pPortRange->nPortEnd);
        }
        break;

        case SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS:
        {
            SipRtConfig::RegContactAddress* pRegContactA =
                    DYNAMIC_CAST(SipRtConfig::RegContactAddress*, pParam);

            if (pRegContactA == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < objRegContacts.GetSize(); ++i)
            {
                SipRtConfig::RegContactAddress& objRegContactA = objRegContacts.GetAt(i);

                if (objRegContactA.Equals(*pRegContactA))
                {
                    // Already exists. Overwrite the existing URI.
                    objRegContactA.objUri = pRegContactA->objUri;
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!objRegContacts.Append(*pRegContactA))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (objRegContacts.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig :: RegContactAddress is set; size=%d",
                        objRegContacts.GetSize(), 0, 0);
                return IMS_SUCCESS;
            }
        }
        break;

        default:
            IMS_TRACE_E(0, "Unsupported configuration item (%d)", nItem, 0, 0);
            return IMS_FAILURE;
    }

    IMS_SLONG nIndex = objConfigSet.GetIndexOfKey(nItem);

    if (nIndex >= 0)
    {
        objConfigSet.SetValueAt(nIndex, IMS_TRUE);
    }

    IMS_TRACE_D("SIP run-time configuration (%d) is set", nItem, 0, 0);

    return IMS_SUCCESS;
}

/*

Remarks

*/
PRIVATE
IMS_UINT32 SIPRTConfigHelper::GetSocketOptionCount(IN IMS_SINT32 nItem) const
{
    IMS_SLONG nIndex = objSOMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return 0;
    }

    const IMSList<SipRtConfig::SocketOption>& objSOs = objSOMap.GetValueAt(nIndex);

    return objSOs.GetSize();
}

/*

Remarks

*/
PRIVATE
void SIPRTConfigHelper::RemoveSocketOption(
        IN IMS_SINT32 nItem, IN CONST SipRtConfig::SocketOption* pSO)
{
    if (pSO == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
        return;
    }

    IMS_SLONG nIndex = objSOMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return;
    }

    IMSList<SipRtConfig::SocketOption>& objSOs = objSOMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSOs.GetSize(); ++i)
    {
        SipRtConfig::SocketOption& objSO = objSOs.GetAt(i);

        if (pSO->Equals(objSO))
        {
            objSOs.RemoveAt(i);
            break;
        }
    }

    if (objSOs.IsEmpty())
    {
        objSOMap.RemoveAt(nIndex);
    }
}

/*

Remarks

*/
PRIVATE
IMS_RESULT SIPRTConfigHelper::SetSocketOption(
        IN IMS_SINT32 nItem, IN CONST SipRtConfig::SocketOption* pSO)
{
    if (pSO == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SLONG nIndex = objSOMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        IMSList<SipRtConfig::SocketOption> objSOs;
        objSOs.Append(*pSO);

        objSOMap.Add(nItem, objSOs);

        return IMS_SUCCESS;
    }

    IMSList<SipRtConfig::SocketOption>& objSOs = objSOMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSOs.GetSize(); ++i)
    {
        SipRtConfig::SocketOption& objSO = objSOs.GetAt(i);

        if (pSO->Equals(objSO))
        {
            objSO.nValue = pSO->nValue;
            return IMS_SUCCESS;
        }
    }

    objSOs.Append(*pSO);

    return IMS_SUCCESS;
}
