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
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"
#include "network/OsIpSecPolicy.h"
#include "network/OsIpSecSa.h"
#include "network/OsIpSecSp.h"
#include "network/OsNetworkIpSec.h"
#include "system-intf/System.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsNetworkIpSec::OsNetworkIpSec() :
        m_nNextAvailableId(1),
        m_bSdbFlushCapability(IMS_FALSE),
        m_objPolicy(IMSList<OsIpSecPolicy*>()),
        m_objSaParams(IMSMap<IMS_UINTP, IpSecSaParameter>())
{
}

PUBLIC VIRTUAL OsNetworkIpSec::~OsNetworkIpSec() {}

PUBLIC VIRTUAL IIpSecPolicy* OsNetworkIpSec::CreatePolicy()
{
    OsIpSecPolicy* pPolicy = new OsIpSecPolicy(GetAvailableId());

    IMS_TRACE_D("CreatePolicy :: Policy(%p:%d)", pPolicy, pPolicy->GetId(), 0);

    m_objPolicy.Append(pPolicy);

    return pPolicy;
}

PUBLIC VIRTUAL void OsNetworkIpSec::DestroyPolicy(IN IIpSecPolicy* piPolicy)
{
    OsIpSecPolicy* pPolicy = DYNAMIC_CAST(OsIpSecPolicy*, piPolicy);

    IMS_TRACE_D("DestroyPolicy :: Policy(%p:%d)", piPolicy, pPolicy->GetId(), 0);

    for (IMS_UINT32 i = 0; i < m_objPolicy.GetSize(); i++)
    {
        OsIpSecPolicy* pTmpPolicy = m_objPolicy.GetAt(i);

        if (pTmpPolicy == pPolicy)
        {
            IMS_TRACE_I("DestroyPolicy :: Policy(%p) removed", pPolicy, 0, 0);
            delete pTmpPolicy;
            m_objPolicy.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void OsNetworkIpSec::DestroyAllPolicies()
{
    IMS_TRACE_D("DestroyAllPolicies :: Policy-size(%d) -- starts", m_objPolicy.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objPolicy.GetSize(); i++)
    {
        OsIpSecPolicy* pPolicy = m_objPolicy.GetAt(i);

        const IMSList<OsIpSecSp*>& objSPs = pPolicy->GetSPs();
        const IMSList<OsIpSecSa*>& objSAs = pPolicy->GetSAs();

        IMS_TRACE_D("DestroyAllPolicies :: Policy(%p) , SP-size(%d), SA-size(%d)", pPolicy,
                objSPs.GetSize(), objSAs.GetSize());

        IMS_SLONG nIndex = m_objSaParams.GetIndexOfKey(reinterpret_cast<IMS_UINTP>(pPolicy));

        if (nIndex >= 0)
        {
            const IpSecSaParameter& objSaParam = m_objSaParams.GetValueAt(nIndex);

            System::GetInstance()->RemoveIpSecSaParameter(objSaParam.GetIpSecId(), GetSlotId());

            m_objSaParams.RemoveAt(nIndex);
        }

        delete pPolicy;
        pPolicy = IMS_NULL;
    }

    m_objPolicy.Clear();

    IMS_TRACE_D("DestroyAllPolicies -- ends", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkIpSec::AddPolicy(IN IIpSecPolicy* piPolicy)
{
    IMS_TRACE_D("AddPolicy(SP+SA) -- starts", 0, 0, 0);

    OsIpSecPolicy* pPolicy = DYNAMIC_CAST(OsIpSecPolicy*, piPolicy);
    const IMSList<OsIpSecSp*>& objSPs = pPolicy->GetSPs();
    const IMSList<OsIpSecSa*>& objSAs = pPolicy->GetSAs();

    IMS_TRACE_I("AddPolicy :: SP-size=%d, SA-size=%d", objSPs.GetSize(), objSAs.GetSize(), 0);

    OsIpSecSa* pIpSecSa;
    IMS_UINT32 i = 0;

    for (i = 0; i < objSAs.GetSize(); i++)
    {
        pIpSecSa = objSAs.GetAt(i);
        pIpSecSa->DisplayInfo();
    }

    // We don't need to create for all the SAs
    // because the information for SA is equal except for the direction of local & remote address.
    pIpSecSa = objSAs.GetAt(0);

    IpSecSaParameter objSaParam = pIpSecSa->CreateSaParameter(pPolicy->GetId());

    for (i = 0; i < objSPs.GetSize(); i++)
    {
        OsIpSecSp* pIpSecSp = objSPs.GetAt(i);

        pIpSecSp->DisplayInfo();

        IpSecSaParameter::Policy objPolicy = pIpSecSp->CreateSaPolicy();

        objSaParam.AddPolicy(objPolicy);
    }

    IMS_TRACE_D("SaParameter: %s", objSaParam.ToString().GetStr(), 0, 0);

    if (System::GetInstance()->AddIpSecSaParameter(objSaParam, GetSlotId()) > 0)
    {
        m_objSaParams.Add(reinterpret_cast<IMS_UINTP>(pPolicy), objSaParam);
    }

    IMS_TRACE_D("AddPolicy(SP+SA) -- ends", 0, 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsNetworkIpSec::DeletePolicy(IN IIpSecPolicy* piPolicy)
{
    IMS_TRACE_D("DeletePolicy(SP+SA) -- starts", 0, 0, 0);

    OsIpSecPolicy* pPolicy = DYNAMIC_CAST(OsIpSecPolicy*, piPolicy);
    IMS_SLONG nIndex = m_objSaParams.GetIndexOfKey(reinterpret_cast<IMS_UINTP>(pPolicy));

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "Finding Policy is failed.", 0, 0, 0);
        return;
    }

    const IpSecSaParameter& objSaParam = m_objSaParams.GetValueAt(nIndex);

    System::GetInstance()->RemoveIpSecSaParameter(objSaParam.GetIpSecId(), GetSlotId());

    m_objSaParams.RemoveAt(nIndex);

    IMS_TRACE_D("DeletePolicy(SP+SA) -- ends", 0, 0, 0);
}

PUBLIC VIRTUAL void OsNetworkIpSec::FlushPolicies()
{
    IMS_TRACE_I("FlushPolicies :: Delete & Flush SDB - capability=%s, ignored...",
            _TRACE_B_(m_bSdbFlushCapability), 0, 0);
}

PUBLIC VIRTUAL void OsNetworkIpSec::DumpPolicy(IN IIpSecPolicy* piPolicy)
{
    (void)piPolicy;
}

PUBLIC VIRTUAL IIpSecPolicy* OsNetworkIpSec::GetPolicy(IN IMS_SINT32 nId) const
{
    for (IMS_UINT32 i = 0; i < m_objPolicy.GetSize(); i++)
    {
        OsIpSecPolicy* pPolicy = m_objPolicy.GetAt(i);

        if (pPolicy->GetId() == nId)
        {
            return pPolicy;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkIpSec::ApplyIpSecTransform(IN ISocket* piSocket,
        IN const SocketAddress& objLocal, IN const SocketAddress* pRemote /*= IMS_NULL*/)
{
    if (piSocket == IMS_NULL)
    {
        IMS_TRACE_E(0, "Socket object is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // pRemote (IMS_NULL): UDP socket, TCP server socket
    // pRemote (non-IMS_NULL): TCP client socket
    IMS_TRACE_I("ApplyIpSecTransform - sa-size=%d, socket=%d", m_objSaParams.GetSize(),
            piSocket->GetSocketId(), 0);

    IMS_SINT32 nTransportProtocol = IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_UDP;

    if (piSocket->GetSocketType() == ISocket::SOCKET_ENTYPE::TYPE_STREAM)
    {
        nTransportProtocol = IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_TCP;
    }

    for (IMS_UINT32 i = 0; i < m_objSaParams.GetSize(); i++)
    {
        const IpSecSaParameter& objSaParam = m_objSaParams.GetValueAt(i);
        const IMSList<IpSecSaParameter::Policy>& objPolicys = objSaParam.GetPolicys();

        for (IMS_UINT32 j = 0; j < objPolicys.GetSize(); j++)
        {
            IMS_BOOL bApplyIpSec = IMS_FALSE;
            IpSecSaParameter::Policy& objPolicy =
                    const_cast<IpSecSaParameter::Policy&>(objPolicys.GetAt(j));

            if ((objPolicy.GetSocketId() == IpSecSaParameter::Policy::SOCKET_NOT_SET) &&
                    (objPolicy.GetTransportProtocol() == nTransportProtocol))
            {
                if (pRemote == IMS_NULL)
                {
                    if (objLocal.Equals(objPolicy.GetLocalAddress()) ||
                            objLocal.Equals(objPolicy.GetRemoteAddress()))
                    {
                        bApplyIpSec = IMS_TRUE;
                    }
                }
                else if (objLocal.Equals(objPolicy.GetLocalAddress()) &&
                        pRemote->Equals(objPolicy.GetRemoteAddress()))
                {
                    bApplyIpSec = IMS_TRUE;
                }
            }

            if (bApplyIpSec)
            {
                if (System::GetInstance()->ApplyIpSecSa(objSaParam.GetIpSecId(), objPolicy.GetSpi(),
                            piSocket->GetSocketId(), GetSlotId()) > 0)
                {
                    objPolicy.SetSocketId(piSocket->GetSocketId());
                }
                else
                {
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsNetworkIpSec::ApplyIpSecTransform(
        IN ISocket* piSocket, IN ISocket* piServerSocket)
{
    if (piSocket == IMS_NULL || piServerSocket == IMS_NULL)
    {
        IMS_TRACE_E(0, "Socket object is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("ApplyIpSecTransform - sa-size=%d, socket=%d, serverSocket=%d",
            m_objSaParams.GetSize(), piSocket->GetSocketId(), piServerSocket->GetSocketId());

    for (IMS_UINT32 i = 0; i < m_objSaParams.GetSize(); i++)
    {
        const IpSecSaParameter& objSaParam = m_objSaParams.GetValueAt(i);
        const IMSList<IpSecSaParameter::Policy>& objPolicys = objSaParam.GetPolicys();

        for (IMS_UINT32 j = 0; j < objPolicys.GetSize(); j++)
        {
            IpSecSaParameter::Policy& objPolicy =
                    const_cast<IpSecSaParameter::Policy&>(objPolicys.GetAt(j));

            if (objPolicy.GetSocketId() == piServerSocket->GetSocketId() &&
                    !objPolicy.HasAcceptedSocketId(piSocket->GetSocketId()))
            {
                if (System::GetInstance()->ApplyIpSecSa(objSaParam.GetIpSecId(), objPolicy.GetSpi(),
                            piSocket->GetSocketId(), GetSlotId()) > 0)
                {
                    objPolicy.AddAcceptedSocketId(piSocket->GetSocketId());
                }
                else
                {
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsNetworkIpSec::RemoveIpSecTransforms(IN IMS_SINT32 nSocketId)
{
    IMS_TRACE_I(
            "RemoveIpSecTransforms - sa-size=%d, socket=%d", m_objSaParams.GetSize(), nSocketId, 0);

    for (IMS_UINT32 i = 0; i < m_objSaParams.GetSize(); i++)
    {
        const IpSecSaParameter& objSaParam = m_objSaParams.GetValueAt(i);
        const IMSList<IpSecSaParameter::Policy>& objPolicys = objSaParam.GetPolicys();

        for (IMS_UINT32 j = 0; j < objPolicys.GetSize(); j++)
        {
            IpSecSaParameter::Policy& objPolicy =
                    const_cast<IpSecSaParameter::Policy&>(objPolicys.GetAt(j));

            if (objPolicy.GetSocketId() == nSocketId)
            {
                System::GetInstance()->RemoveIpSecSa(
                        objSaParam.GetIpSecId(), objPolicy.GetSpi(), nSocketId, GetSlotId());

                objPolicy.SetSocketId(IpSecSaParameter::Policy::SOCKET_NOT_SET);
            }
            else if (objPolicy.HasAcceptedSocketId(nSocketId))
            {
                System::GetInstance()->RemoveIpSecSa(
                        objSaParam.GetIpSecId(), objPolicy.GetSpi(), nSocketId, GetSlotId());

                objPolicy.RemoveAcceptedSocketId(nSocketId);
            }
        }
    }
}

PUBLIC VIRTUAL void OsNetworkIpSec::SetSdbFlushCapability(IN IMS_BOOL bCapability)
{
    // Do not flush SDB always...
    (void)bCapability;
}

PRIVATE
IMS_SINT32 OsNetworkIpSec::GetAvailableId()
{
    IMS_SINT32 nNewId = m_nNextAvailableId;

    m_nNextAvailableId++;

    if (m_nNextAvailableId == 0x7FFFFFFF)
    {
        m_nNextAvailableId = 1;
    }

    if (m_objPolicy.IsEmpty())
    {
        return nNewId;
    }

    IIpSecPolicy* piPolicy = GetPolicy(nNewId);

    while (piPolicy != IMS_NULL)
    {
        nNewId = m_nNextAvailableId;
        m_nNextAvailableId++;

        if (m_nNextAvailableId == 0x7FFFFFFF)
        {
            m_nNextAvailableId = 1;
        }

        piPolicy = GetPolicy(nNewId);
    }

    return nNewId;
}

PRIVATE GLOBAL IMS_SINT32 OsNetworkIpSec::GetSlotId()
{
    return ThreadService::GetCurrentSlotId(
            SystemConfig::IsMultiSimEnabled() ? IMS_SLOT_ANY : IMS_SLOT_0);
}
