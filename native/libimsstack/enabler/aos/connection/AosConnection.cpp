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
#include "ServiceTrace.h"
#include "ServiceEvent.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServicePhoneInfo.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosConnectionListener.h"
#include "interface/IAosTrm.h"

#include "provider/AosProvider.h"
#include "connection/AosConnection.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define CNXID m_strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosConnection::AosConnection(IN IAosAppContext* piAppContext) :
        m_piContext(piAppContext),
        m_nSlotId(piAppContext->GetSlotId()),
        m_nCnxType(NetworkPolicy::APN_NONE),
        m_piConnection(IMS_NULL),
        m_nState(STATE_IDLE),
        m_bActivationRequested(IMS_FALSE)
{
    m_nCnxType = piAppContext->GetStaticProfile()->GetConnectionType();

    m_strTag.Sprintf("%d:%s.cnx", m_nSlotId, piAppContext->GetStaticProfile()->GetId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosConnection = %" PFLS_u "/%" PFLS_x, CNXID,
            sizeof(AosConnection), this);

    m_piConnection = NetworkService::GetNetworkService()->CreateConnection(m_nCnxType, m_nSlotId);
    if (m_piConnection == IMS_NULL)
    {
        A_IMS_TRACE_D(
                CNXID, "AosConnection :: network connection (%d) is failed", m_nCnxType, 0, 0);
        return;
    }

    m_piConnection->SetListener(this);

    A_IMS_TRACE_I(CNXID, "AosConnection :: %d", m_nCnxType, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL AosConnection::~AosConnection()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosConnection = %" PFLS_u "/%" PFLS_x, CNXID,
            sizeof(AosConnection), this);

    // Should not use the AosAppContext that is destroyed

    m_objListeners.Clear();

    if (m_piConnection != IMS_NULL)
    {
        m_piConnection->SetListener(IMS_NULL);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosConnection::Activate()
{
    A_IMS_TRACE_I(CNXID, "Activate :: state(%s)", StateToString(m_nState), 0, 0);

    if (IsConnected())
    {
        Notify();
        return IMS_TRUE;
    }

    if (m_nCnxType != NetworkPolicy::APN_EMERGENCY && IsActivationRequested())
    {
        return IMS_TRUE;
    }

    SetActivationRequested(IMS_TRUE);

    if (m_piConnection->Activate(IMS_TRUE) == INetworkConnection::RESULT_DONE)
    {
        SetState(STATE_ACTIVE);
        Notify();
    }
    else
    {
        SetState(STATE_ACTIVATING);
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosConnection::Deactivate()
{
    A_IMS_TRACE_I(CNXID, "Deactivate :: state(%s)", StateToString(m_nState), 0, 0);

    if (IsActivationRequested())
    {
        m_piConnection->Deactivate(IMS_TRUE);
        SetActivationRequested(IMS_FALSE);
    }

    SetState(STATE_IDLE);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 AosConnection::GetState()
{
    A_IMS_TRACE_D(CNXID, "GetState :: state(%s)", StateToString(m_nState), 0, 0);
    return m_nState;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 AosConnection::GetConnectionType()
{
    return m_nCnxType;
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosConnection::SetListener(IN IAosConnectionListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosConnectionListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(CNXID, "SetListener :: (%" PFLS_x ") is already set", piListener, 0, 0);
            return;
        }
    }

    m_objListeners.Append(piListener);

    A_IMS_TRACE_D(CNXID, "SetListener :: (%" PFLS_x ") is set", piListener, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosConnection::RemoveListener(IN IAosConnectionListener* piListener)
{
    for (IMS_UINT32 i = 0; i < m_objListeners.GetSize(); ++i)
    {
        IAosConnectionListener* pTmpListener = m_objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objListeners.RemoveAt(i);

            A_IMS_TRACE_D(CNXID, "RemoveListener :: (%" PFLS_x ") is removed", piListener, 0, 0);
            return;
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 AosConnection::GetMtu()
{
    if (GetState() == STATE_ACTIVE)
    {
        return m_piConnection->GetMtu();
    }
    else
    {
        return 0;
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL const IPAddress& AosConnection::GetLocalAddress(IN IMS_SINT32 nIpVersion /* = 0 */)
{
    return m_piConnection->GetLocalAddress(nIpVersion);
}

/*

Remarks

*/
PUBLIC VIRTUAL const AStringArray& AosConnection::GetPcscfAddress(
        IN IMS_SINT32 nIpVersion /* = 0 */)
{
    return m_piConnection->GetPcscfAddress(nIpVersion);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 AosConnection::GetHostByName(IN const AString& strHostName,
        OUT IMSList<IPAddress>& objIps, IN IMS_SINT32 nIpVersion /* = 0 */)
{
    return m_piConnection->GetHostByName(strHostName, objIps, nIpVersion);
}

/*

Remarks

*/
PUBLIC VIRTUAL const AString& AosConnection::GetIfaceName()
{
    return m_piConnection->GetIfaceName();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosConnection::IsEpdgEnabled()
{
    return m_piConnection->IsePDGEnabled();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 AosConnection::GetIpcanCategory()
{
    return IsEpdgEnabled() ? IIpcan::CATEGORY_WLAN : IIpcan::CATEGORY_MOBILE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosConnection::SendPingToHostAddress(IN const IPAddress& objHostAddress)
{
    A_IMS_TRACE_D(CNXID, "SendPingToHostAddress :: host address (%s)",
            objHostAddress.ToString().GetStr(), 0, 0);

    return m_piConnection->SendPingToHostAddress(objHostAddress);
}

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* AosConnection::StateToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case STATE_IDLE:
            return "STATE_IDLE";

        case STATE_ACTIVE:
            return "STATE_ACTIVE";

        case STATE_ACTIVATING:
            return "STATE_ACTIVATING";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PROTECTED
void AosConnection::Notify(IN IMS_UINT32 nType /* = TYPE_STATE_CHANGED */)
{
    for (IMS_UINT32 nAt = 0; nAt < m_objListeners.GetSize(); ++nAt)
    {
        IAosConnectionListener* piListener = m_objListeners.GetAt(nAt);

        if (piListener == IMS_NULL)
            continue;

        switch (nType)
        {
            case TYPE_STATE_CHANGED:
                piListener->AosConnection_StateChanged(m_nState);
                break;

            case TYPE_IP_CHANGED:
                piListener->AosConnection_IpChanged();
                break;

            case TYPE_IPCAN_CHANGED:
                piListener->AosConnection_IpcanCatChanged();
                break;

            case TYPE_PCSCF_CHANGED:
                piListener->AosConnection_PcscfChanged();
                break;

            case TYPE_CONNECTION_FAILED:
                piListener->AosConnection_ConnectionFailed();
                break;

            default:
                break;
        }
    }
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosConnection::IsActivationRequested() const
{
    return m_bActivationRequested;
}

/*

Remarks

*/
PROTECTED
IMS_BOOL AosConnection::IsConnected() const
{
    return (m_nState == STATE_ACTIVE);
}

/*

Remarks

*/
PROTECTED
void AosConnection::SetActivationRequested(IN IMS_BOOL bRequest)
{
    m_bActivationRequested = bRequest;
}

/*

Remarks

*/
PROTECTED
void AosConnection::SetState(IN IMS_UINT32 nState)
{
    IMS_UINT32 nOldState = m_nState;

    m_nState = nState;
    A_IMS_TRACE_D(CNXID, "SetState :: OLD(%s) -> NEW(%s)", StateToString(nOldState),
            StateToString(nState), 0);
}

/*

Remarks

*/
PROTECTED
void AosConnection::UpdateIpcanForTrm()
{
    if (m_nCnxType != NetworkPolicy::APN_IMS)
    {
        return;
    }

    static IMS_BOOL bIsUpdated = IMS_FALSE;
    static IMS_BOOL bIsTrmSupported = IMS_FALSE;

    if (!bIsUpdated)
    {
        ITrm* piPhoneTRM = PhoneInfoService::GetPhoneInfoService()->GetTrm();
        if (piPhoneTRM != IMS_NULL)
        {
            bIsTrmSupported = piPhoneTRM->IsTrmSupported();
        }

        bIsUpdated = IMS_TRUE;
    }

    if (bIsTrmSupported)
    {
        IAosTrm* piTrm = AosProvider::GetInstance()->GetTrm(m_nSlotId);
        if (piTrm != IMS_NULL)
        {
            piTrm->SetIPCAN(static_cast<IMS_UINT32>(GetIpcanCategory()));
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosConnection::NetworkConnection_OnConnected(
        IN INetworkConnection* piNetConnection)
{
    if (piNetConnection != m_piConnection)
    {
        return;
    }

    A_IMS_TRACE_I(CNXID, "Connection_Connected", 0, 0, 0);

    SetState(STATE_ACTIVE);

    UpdateIpcanForTrm();

    Notify();
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosConnection::NetworkConnection_OnDisconnected(
        IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode)
{
    if (piNetConnection != m_piConnection)
    {
        return;
    }

    A_IMS_TRACE_I(CNXID, "Connection_Disconnected :: nErrorCode(%d)", nErrorCode, 0, 0);

    SetState(STATE_IDLE);
    Notify();
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosConnection::NetworkConnection_OnConnectionFailed(
        IN INetworkConnection* piNetConnection, IN IMS_SINT32 nErrorCode)
{
    if (piNetConnection != m_piConnection)
    {
        return;
    }

    A_IMS_TRACE_I(CNXID, "Connection_ConnectionFailed :: nErrorCode(%d)", nErrorCode, 0, 0);

    SetState(STATE_IDLE);
    Notify(TYPE_CONNECTION_FAILED);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosConnection::NetworkConnection_OnIpChanged(
        IN INetworkConnection* piNetConnection)
{
    if (piNetConnection != m_piConnection)
    {
        return;
    }

    A_IMS_TRACE_I(CNXID, "Connection_IpChanged", 0, 0, 0);

    if (IsConnected())
    {
        Notify(TYPE_IP_CHANGED);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosConnection::NetworkConnection_OnIpcanChanged(
        IN INetworkConnection* piNetConnection)
{
    if (piNetConnection != m_piConnection)
    {
        return;
    }

    A_IMS_TRACE_I(CNXID, "Connection_IpcanCatChanged", 0, 0, 0);

    UpdateIpcanForTrm();

    Notify(TYPE_IPCAN_CHANGED);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosConnection::NetworkConnection_OnPcscfChanged(
        IN INetworkConnection* piNetConnection)
{
    if (piNetConnection != m_piConnection)
    {
        return;
    }

    A_IMS_TRACE_I(CNXID, "Connection_PcscfChanged", 0, 0, 0);

    Notify(TYPE_PCSCF_CHANGED);
}
