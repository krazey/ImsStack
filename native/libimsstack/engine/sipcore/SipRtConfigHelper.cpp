/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipDebug.h"
#include "SipPortManager.h"
#include "SipRtConfigHelper.h"

__IMS_TRACE_TAG_SIP_CORE__;

PUBLIC
SipRtConfigHelper::SipRtConfigHelper() :
        m_objConfigSet(ImsMap<IMS_SINT32, IMS_BOOL>()),
        m_nFeatures(SipRtConfig::FEATURE_NONE),
        m_objIpQoss(ImsList<SipRtConfig::IpQos>()),
        m_objHeaders(ImsList<SipRtConfig::Header>()),
        m_objIpSecSas(ImsList<SipRtConfig::IpSecSa>())
{
    for (IMS_SINT32 i = 0; i < SipRtConfig::CONFIG_I_MAX; ++i)
    {
        m_objConfigSet.Add(i, IMS_FALSE);
    }
}

PUBLIC VIRTUAL SipRtConfigHelper::~SipRtConfigHelper() {}

PUBLIC VIRTUAL const SipRtConfig::Header* SipRtConfigHelper::GetHeader(
        IN const AString& strName) const
{
    if (m_objHeaders.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
    {
        const SipRtConfig::Header& objHeader = m_objHeaders.GetAt(i);

        if (strName.EqualsIgnoreCase(objHeader.strName))
        {
            return &objHeader;
        }
    }

    return IMS_NULL;
}

PUBLIC
const SipRtConfig::SocketOption* SipRtConfigHelper::GetSocketOption(IN IMS_SINT32 nItem) const
{
    IMS_SLONG nIndex = m_objSocketOptionMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    const ImsList<SipRtConfig::SocketOption>& objSocketOptions =
            m_objSocketOptionMap.GetValueAt(nIndex);

    if (objSocketOptions.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < objSocketOptions.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSocketOption = objSocketOptions.GetAt(i);

        if (objSocketOption.IsGlobalOption())
        {
            return &objSocketOption;
        }
    }

    return IMS_NULL;
}

PUBLIC
const SipRtConfig::SocketOption* SipRtConfigHelper::GetSocketOption(
        IN IMS_SINT32 nItem, IN const IpAddress& objIp, IN IMS_SINT32 nPort /*= 0*/) const
{
    if ((nPort == 0) && (objIp.Equals(IpAddress::NONE) || objIp.Equals(IpAddress::IPv6NONE)))
    {
        return GetSocketOption(nItem);
    }

    IMS_SLONG nIndex = m_objSocketOptionMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    const ImsList<SipRtConfig::SocketOption>& objSocketOptions =
            m_objSocketOptionMap.GetValueAt(nIndex);

    if (objSocketOptions.IsEmpty())
    {
        return IMS_NULL;
    }

    // Condition: IP & Port
    for (IMS_UINT32 i = 0; i < objSocketOptions.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSocketOption = objSocketOptions.GetAt(i);

        if ((objSocketOption.nPort == nPort) && objIp.Equals(objSocketOption.objIpAddr))
        {
            return &objSocketOption;
        }
    }

    // Condition: IP
    for (IMS_UINT32 i = 0; i < objSocketOptions.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSocketOption = objSocketOptions.GetAt(i);

        if ((objSocketOption.nPort == 0) && objIp.Equals(objSocketOption.objIpAddr))
        {
            return &objSocketOption;
        }
    }

    // Condition: Port
    for (IMS_UINT32 i = 0; i < objSocketOptions.GetSize(); ++i)
    {
        const SipRtConfig::SocketOption& objSocketOption = objSocketOptions.GetAt(i);

        if ((objSocketOption.objIpAddr.Equals(IpAddress::NONE) ||
                    objSocketOption.objIpAddr.Equals(IpAddress::IPv6NONE)) &&
                (objSocketOption.nPort == nPort))
        {
            return &objSocketOption;
        }
    }

    return IMS_NULL;
}

PUBLIC
const SipRtConfig::IpQos* SipRtConfigHelper::GetIpQos(
        IN const IpAddress& objIp, IN IMS_SINT32 nPort /*= 0*/) const
{
    if (m_objIpQoss.IsEmpty())
    {
        IMS_TRACE_D("No IP-level QoS supports", 0, 0, 0);
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objIpQoss.GetSize(); ++i)
    {
        const SipRtConfig::IpQos& objIpQos = m_objIpQoss.GetAt(i);

        if (objIp.Equals(objIpQos.objIpAddr))
        {
            if (objIpQos.nPort == 0)
            {
                IMS_TRACE_D("IP-level QoS: %s", SipDebug::GetIp(objIp), 0, 0);
                return &objIpQos;
            }
            else
            {
                if (nPort == objIpQos.nPort)
                {
                    IMS_TRACE_D("IP-level QoS: %s|%d", SipDebug::GetIp(objIp), nPort, 0);
                    return &objIpQos;
                }
            }
        }
    }

    IMS_TRACE_D("No IP-level QoS supports", 0, 0, 0);

    return IMS_NULL;
}

PUBLIC
const SipAddress* SipRtConfigHelper::GetRegContactUri(IN const AString& strCallId) const
{
    if (m_objRegContacts.IsEmpty())
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objRegContacts.GetSize(); ++i)
    {
        const SipRtConfig::RegContactAddress& objRegContact = m_objRegContacts.GetAt(i);

        if (strCallId.Equals(objRegContact.GetCallId()))
        {
            const SipAddress& objUri = objRegContact.GetUri();
            return &objUri;
        }
    }

    return IMS_NULL;
}

PUBLIC
IMS_BOOL SipRtConfigHelper::IsItemConfigured(IN IMS_SINT32 nItem) const
{
    IMS_SLONG nIndex = m_objConfigSet.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return IMS_FALSE;
    }

    return m_objConfigSet.GetValueAt(nIndex);
}

PRIVATE VIRTUAL void SipRtConfigHelper::RemoveConfig(
        IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam)
{
    switch (nItem)
    {
        case SipRtConfig::CONFIG_I_LOG_MASK:
            m_objLogMask.nValue = SipRtConfig::LogMask::I_NONE;
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
                IMS_TRACE_D("SipRtConfig: SocketOption(%d) is removed; sizeOfMap=%d", nItem,
                        m_objSocketOptionMap.GetSize(), 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_IP_QOS:
            if (m_objIpQoss.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                m_objIpQoss.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < m_objIpQoss.GetSize(); ++i)
            {
                const SipRtConfig::IpQos& objIpQos = m_objIpQoss.GetAt(i);

                if (objIpQos.Equals(*pParam))
                {
                    m_objIpQoss.RemoveAt(i);
                    break;
                }
            }

            if (!m_objIpQoss.IsEmpty())
            {
                IMS_TRACE_D("SipRtConfig: IpQos is removed; size=%d", m_objIpQoss.GetSize(), 0, 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_SIP_HEADER:
            if (m_objHeaders.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                m_objHeaders.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
            {
                const SipRtConfig::Header& objHeader = m_objHeaders.GetAt(i);

                if (objHeader.Equals(*pParam))
                {
                    m_objHeaders.RemoveAt(i);
                    break;
                }
            }

            if (!m_objHeaders.IsEmpty())
            {
                IMS_TRACE_D(
                        "SipRtConfig: Header is removed; size=%d", m_objHeaders.GetSize(), 0, 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_IPSEC_SA:
            if (m_objIpSecSas.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                m_objIpSecSas.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < m_objIpSecSas.GetSize(); ++i)
            {
                const SipRtConfig::IpSecSa& objIpSecSa = m_objIpSecSas.GetAt(i);

                if (objIpSecSa.Equals(*pParam))
                {
                    m_objIpSecSas.RemoveAt(i);
                    break;
                }
            }

            if (!m_objIpSecSas.IsEmpty())
            {
                IMS_TRACE_D(
                        "SipRtConfig: IpSecSa is removed; size=%d", m_objIpSecSas.GetSize(), 0, 0);
                return;
            }
            break;

        case SipRtConfig::CONFIG_I_TCP_PORT_RANGE:
        {
            SipPortManager::GetInstance()->Clear();
        }
        break;

        case SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS:
            if (m_objRegContacts.IsEmpty())
            {
                break;
            }

            // Remove all the configuration values
            if (pParam == IMS_NULL)
            {
                m_objRegContacts.Clear();
                break;
            }

            for (IMS_UINT32 i = 0; i < m_objRegContacts.GetSize(); ++i)
            {
                const SipRtConfig::RegContactAddress& objRegContact = m_objRegContacts.GetAt(i);

                if (objRegContact.Equals(*pParam))
                {
                    m_objRegContacts.RemoveAt(i);
                    break;
                }
            }

            if (!m_objRegContacts.IsEmpty())
            {
                IMS_TRACE_D("SipRtConfig: RegContactAddress is removed; size=%d",
                        m_objRegContacts.GetSize(), 0, 0);
                return;
            }
            break;

        default:
            IMS_TRACE_E(0, "Unsupported configuration item (%d)", nItem, 0, 0);
            return;
    }

    IMS_SLONG nIndex = m_objConfigSet.GetIndexOfKey(nItem);

    if (nIndex >= 0)
    {
        m_objConfigSet.SetValueAt(nIndex, IMS_FALSE);
    }

    IMS_TRACE_D("SIP run-time configuration (%d) is removed", nItem, 0, 0);
}

PRIVATE VIRTUAL IMS_RESULT SipRtConfigHelper::SetConfig(
        IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam)
{
    if (pParam == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
        return IMS_FAILURE;
    }

    switch (nItem)
    {
        case SipRtConfig::CONFIG_I_LOG_MASK:
        {
            const SipRtConfig::LogMask* pLogMask = DYNAMIC_CAST(SipRtConfig::LogMask*, pParam);

            if (pLogMask == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            m_objLogMask.nValue = pLogMask->nValue;
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
                IMS_TRACE_D("SipRtConfig: SocketOption(%d) is set; sizeOfMap=%d", nItem,
                        m_objSocketOptionMap.GetSize(), 0);
                return IMS_SUCCESS;
            }
            break;

        case SipRtConfig::CONFIG_I_IP_QOS:
        {
            SipRtConfig::IpQos* pIpQos = DYNAMIC_CAST(SipRtConfig::IpQos*, pParam);

            if (pIpQos == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < m_objIpQoss.GetSize(); ++i)
            {
                SipRtConfig::IpQos& objIpQos = m_objIpQoss.GetAt(i);

                if (objIpQos.Equals(*pIpQos))
                {
                    // Only updates the value
                    objIpQos.nValue = pIpQos->nValue;
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!m_objIpQoss.Append(*pIpQos))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (m_objIpQoss.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig: IpQos is set; size=%d", m_objIpQoss.GetSize(), 0, 0);
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

            for (IMS_UINT32 i = 0; i < m_objHeaders.GetSize(); ++i)
            {
                SipRtConfig::Header& objHeader = m_objHeaders.GetAt(i);

                if (objHeader.Equals(*pHeader))
                {
                    // Only updates the value
                    objHeader.strParameter = pHeader->strParameter;
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!m_objHeaders.Append(*pHeader))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (m_objHeaders.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig: Header is set; size=%d", m_objHeaders.GetSize(), 0, 0);
                return IMS_SUCCESS;
            }
        }
        break;

        case SipRtConfig::CONFIG_I_IPSEC_SA:
        {
            const SipRtConfig::IpSecSa* pIpSecSa = DYNAMIC_CAST(SipRtConfig::IpSecSa*, pParam);

            if (pIpSecSa == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < m_objIpSecSas.GetSize(); ++i)
            {
                const SipRtConfig::IpSecSa& objIpSecSa = m_objIpSecSas.GetAt(i);

                if (objIpSecSa.Equals(*pIpSecSa))
                {
                    // Already exists.
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!m_objIpSecSas.Append(*pIpSecSa))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (m_objIpSecSas.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig: IpSecSa is set; size=%d", m_objIpSecSas.GetSize(), 0, 0);
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

            SipPortManager::GetInstance()->SetPortC(pPortRange->nPortStart, pPortRange->nPortEnd);
        }
        break;

        case SipRtConfig::CONFIG_I_REG_CONTACT_ADDRESS:
        {
            SipRtConfig::RegContactAddress* pRegContact =
                    DYNAMIC_CAST(SipRtConfig::RegContactAddress*, pParam);

            if (pRegContact == IMS_NULL)
            {
                IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
                return IMS_FAILURE;
            }

            for (IMS_UINT32 i = 0; i < m_objRegContacts.GetSize(); ++i)
            {
                SipRtConfig::RegContactAddress& objRegContact = m_objRegContacts.GetAt(i);

                if (objRegContact.Equals(*pRegContact))
                {
                    // Already exists. Overwrite the existing URI.
                    objRegContact.objUri = pRegContact->objUri;
                    return IMS_SUCCESS;
                }
            }

            // New configured item
            if (!m_objRegContacts.Append(*pRegContact))
            {
                IMS_TRACE_E(0, "Setting the configuration (%d) value failed", nItem, 0, 0);
                return IMS_FAILURE;
            }

            if (m_objRegContacts.GetSize() > 1)
            {
                IMS_TRACE_D("SipRtConfig: RegContactAddress is set; size=%d",
                        m_objRegContacts.GetSize(), 0, 0);
                return IMS_SUCCESS;
            }
        }
        break;

        default:
            IMS_TRACE_E(0, "Unsupported configuration item (%d)", nItem, 0, 0);
            return IMS_FAILURE;
    }

    IMS_SLONG nIndex = m_objConfigSet.GetIndexOfKey(nItem);

    if (nIndex >= 0)
    {
        m_objConfigSet.SetValueAt(nIndex, IMS_TRUE);
    }

    IMS_TRACE_D("SIP run-time configuration (%d) is set", nItem, 0, 0);

    return IMS_SUCCESS;
}

PRIVATE
IMS_UINT32 SipRtConfigHelper::GetSocketOptionCount(IN IMS_SINT32 nItem) const
{
    IMS_SLONG nIndex = m_objSocketOptionMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return 0;
    }

    const ImsList<SipRtConfig::SocketOption>& objSocketOptions =
            m_objSocketOptionMap.GetValueAt(nIndex);

    return objSocketOptions.GetSize();
}

PRIVATE
void SipRtConfigHelper::RemoveSocketOption(
        IN IMS_SINT32 nItem, IN const SipRtConfig::SocketOption* pSockOpt)
{
    if (pSockOpt == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
        return;
    }

    IMS_SLONG nIndex = m_objSocketOptionMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        return;
    }

    ImsList<SipRtConfig::SocketOption>& objSocketOptions = m_objSocketOptionMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSocketOptions.GetSize(); ++i)
    {
        SipRtConfig::SocketOption& objSocketOption = objSocketOptions.GetAt(i);

        if (pSockOpt->Equals(objSocketOption))
        {
            objSocketOptions.RemoveAt(i);
            break;
        }
    }

    if (objSocketOptions.IsEmpty())
    {
        m_objSocketOptionMap.RemoveAt(nIndex);
    }
}

PRIVATE
IMS_RESULT SipRtConfigHelper::SetSocketOption(
        IN IMS_SINT32 nItem, IN const SipRtConfig::SocketOption* pSockOpt)
{
    if (pSockOpt == IMS_NULL)
    {
        IMS_TRACE_E(0, "Invalid parameter for configuration item (%d)", nItem, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SLONG nIndex = m_objSocketOptionMap.GetIndexOfKey(nItem);

    if (nIndex < 0)
    {
        ImsList<SipRtConfig::SocketOption> objSocketOptions;
        objSocketOptions.Append(*pSockOpt);

        m_objSocketOptionMap.Add(nItem, objSocketOptions);

        return IMS_SUCCESS;
    }

    ImsList<SipRtConfig::SocketOption>& objSocketOptions = m_objSocketOptionMap.GetValueAt(nIndex);

    for (IMS_UINT32 i = 0; i < objSocketOptions.GetSize(); ++i)
    {
        SipRtConfig::SocketOption& objSocketOption = objSocketOptions.GetAt(i);

        if (pSockOpt->Equals(objSocketOption))
        {
            objSocketOption.nValue = pSockOpt->nValue;
            return IMS_SUCCESS;
        }
    }

    objSocketOptions.Append(*pSockOpt);

    return IMS_SUCCESS;
}
