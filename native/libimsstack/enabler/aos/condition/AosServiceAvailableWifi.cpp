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
#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetwork.h"
#include "AosReason.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailableWifi.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosServiceAvailableWifi::AosServiceAvailableWifi() :
        AosServiceAvailable("AosServiceAvailableWifi"),
        m_strCountry(AString::ConstEmpty()),
        m_nBadNetworkState(STATE_BAD_NETWORK_NONE),
        m_bWifiState(IMS_FALSE),
        m_piNetPing(IMS_NULL),
        m_piTestLocation(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosServiceAvailableWifi = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailableWifi), this, 0);
}

PUBLIC VIRTUAL AosServiceAvailableWifi::~AosServiceAvailableWifi()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosServiceAvailableWifi = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailableWifi), this, 0);
}

PUBLIC VIRTUAL IMS_BOOL AosServiceAvailableWifi::StartToCheckNetworkConnection()
{
    if (m_piCallTracker == IMS_NULL || m_piCallTracker->IsEmergencyCallActive())
    {
        return IMS_FALSE;
    }

    if (m_piRegistration == IMS_NULL || m_piRegistration->IsRegistered() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (m_nBadNetworkState == STATE_BAD_NETWORK_CHECKING ||
            m_nBadNetworkState == STATE_BAD_NETWORK_DETECTED)
    {
        A_IMS_TRACE_D(AOSTAG, "Checking ePDG connection is ongoing or detected as bad", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piCallTracker->IsNormalCallActive() && m_piConnection != IMS_NULL &&
            m_piConnection->IsEpdgEnabled())
    {
        A_IMS_TRACE_D(AOSTAG, "There is ongoing WiFi call. Checking ePDG connection", 0, 0, 0);

        if (RequestNetPing() == INetworkPing::PING_STATUS_NOK)
        {
            ProcessBadConnectionReported();
        }

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosServiceAvailableWifi::StopToCheckNetworkConnection(
        IN IMS_BOOL bNeedToCheckAvailable /*= IMS_TRUE*/)
{
    if (m_nBadNetworkState == STATE_BAD_NETWORK_NONE)
    {
        // Do not need to clear
        return IMS_FALSE;
    }

    IMS_BOOL bWasDetected =
            (m_nBadNetworkState == STATE_BAD_NETWORK_DETECTED) ? IMS_TRUE : IMS_FALSE;

    ClearBadNetworkState();

    if (bNeedToCheckAvailable)
    {
        Notify();
    }

    return bWasDetected;
}

PUBLIC
void AosServiceAvailableWifi::SetLocation(IN ILocationProperties* piTestLocation)
{
    m_piTestLocation = piTestLocation;
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::RegisterListener()
{
    AosServiceAvailable::RegisterListener();

    IWifiWatcher* piWifiWatcher = PhoneInfoService::GetPhoneInfoService()->GetWifiWatcher();
    piWifiWatcher->RegisterObserver(this);

    if (piWifiWatcher->GetState() == IWifiWatcher::STATE_CONNECTED)
    {
        m_bWifiState = IMS_TRUE;
    }
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::DeregisterListener()
{
    IWifiWatcher* piWifiWatcher = PhoneInfoService::GetPhoneInfoService()->GetWifiWatcher();
    piWifiWatcher->RemoveObserver(this);

    AosServiceAvailable::DeregisterListener();
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::WifiWatcher_NotifyStateChanged(
        IN IWifiWatcher* pIWifiWatcher)
{
    if (pIWifiWatcher == IMS_NULL)
    {
        return;
    }

    if (pIWifiWatcher->GetState() == IWifiWatcher::STATE_CONNECTED)
    {
        if (m_bWifiState)
        {
            return;
        }

        m_bWifiState = IMS_TRUE;
    }
    else
    {
        if (m_bWifiState == IMS_FALSE)
        {
            return;
        }

        m_bWifiState = IMS_FALSE;

        ClearBadNetworkState();
    }

    HandleEvent(EVENT_WIFI_STATE, 0, -1);
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::NetworkPing_NotifyResult(
        IN INetworkPing* piPing, IN IMS_SINT32 nResult)
{
    A_IMS_TRACE_D(AOSTAG, "NetPing_NotifyResult :: result=%s", PingResultToString(nResult), 0, 0);

    if (piPing != IMS_NULL)
    {
        piPing->Destroy();
    }
    m_piNetPing = IMS_NULL;

    if (m_nBadNetworkState != STATE_BAD_NETWORK_CHECKING)
    {
        A_IMS_TRACE_D(AOSTAG, "NetPing_NotifyResult :: It was not requested", 0, 0, 0);
        return;
    }

    ClearBadNetworkState();

    if (nResult == INetworkPing::PING_STATUS_DEAD_PEER ||
            nResult == INetworkPing::PING_STATUS_TIMEDOUT)
    {
        ProcessBadConnectionReported();
    }
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::HandleCallStateChanged(
        IN IMS_UINT32 nState, IN IMS_SINT32 nStateEx)
{
    A_IMS_TRACE_D(AOSTAG, "HandleCallStateChanged :: nType(%d), nState(%d)", nState, nStateEx, 0);

    if (nState == IAosCallTracker::TYPE_NORMAL)
    {
        if (static_cast<CallState>(nStateEx) == CallState::IDLE)
        {
            ClearBadNetworkState();
        }
    }
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::HandleAirplaneModeChanged(IN IMS_UINT32 nState)
{
    AosServiceAvailable::HandleAirplaneModeChanged(nState);

    if (!GET_N_CONFIG(m_nSlotId)->IsRequiredWfcBlockByAirplaneMode())
    {
        return;
    }

    if (m_bAirplaneMode)
    {
        m_piBlock->SetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    }
    else
    {
        m_piBlock->ResetBlockReason(BLOCK_WIFI_AIRPLANE_MODE_ON);
    }
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::HandleWifiConnectionChanged()
{
    if (m_piBlock == IMS_NULL)
    {
        return;
    }

    if (m_nBadNetworkState == STATE_BAD_NETWORK_DETECTED)
    {
        m_piBlock->SetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    }
    else
    {
        m_piBlock->ResetBlockReason(BLOCK_WIFI_BAD_CONNECTION);
    }
}

PROTECTED VIRTUAL void AosServiceAvailableWifi::HandleLocationInfoChanged()
{
    AString strNewCountry;

    if (m_piTestLocation == IMS_NULL)
    {
        const ILocationProperties* piLocation =
                PhoneInfoService::GetPhoneInfoService()
                        ->GetLocationInfo(m_nSlotId)
                        ->GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY);

        if (piLocation == IMS_NULL)
        {
            return;
        }

        strNewCountry = piLocation->GetCountry();
    }
    else
    {
        strNewCountry = m_piTestLocation->GetCountry();
    }

    A_IMS_TRACE_I(AOSTAG, "HandleLocationInfoChanged :: FROM (%s) TO (%s)", m_strCountry.GetStr(),
            strNewCountry.GetStr(), 0);

    IMS_BOOL bIsCountrySame = strNewCountry.Equals(m_strCountry);
    m_strCountry = strNewCountry;

    if (!bIsCountrySame)
    {
        m_piBlock->ResetBlockReason(BLOCK_IMS_DISABLED);
        m_piBlock->ResetBlockReason(BLOCK_AUTHENTICATION_FAILED);
    }
}

PROTECTED VIRTUAL IMS_BOOL AosServiceAvailableWifi::CheckServiceAvailable()
{
    if (GET_N_CONFIG(m_nSlotId) == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
    {
        A_IMS_TRACE_I(
                AOSTAG, "CheckServiceAvailable :: Wifi Service config is not available", 0, 0, 0);
        return IMS_FALSE;
    }

    return m_piBlock->IsCleared(SERVICE_WIFI);
}

PROTECTED
void AosServiceAvailableWifi::ProcessBadConnectionReported()
{
    m_nBadNetworkState = STATE_BAD_NETWORK_DETECTED;

    Notify();
}

PROTECTED
void AosServiceAvailableWifi::ClearBadNetworkState()
{
    m_nBadNetworkState = STATE_BAD_NETWORK_NONE;

    if (m_piNetPing != IMS_NULL)
    {
        m_piNetPing->Destroy();
        m_piNetPing = IMS_NULL;
    }
}

PROTECTED
IMS_SINT32 AosServiceAvailableWifi::RequestNetPing()
{
    IMS_SINT32 nResult = INetworkPing::PING_STATUS_OK;

    IMS_UINT32 nNA;
    AString strIPA;

    m_piRegistration->GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS, nNA, strIPA);

    IpAddress objSrcIP(strIPA);
    INetworkConnection* piNetCon = NetworkService::GetNetworkService()->FindConnection(objSrcIP);

    if (piNetCon != IMS_NULL)
    {
        AString strPCSCF;
        m_piAppContext->GetRegistration()->GetProperty(
                IAosRegistration::PROPERTY_PCSCF_ADDRESS, nNA, strPCSCF);

        IpAddress objDstIP(strPCSCF);
        IMS_SINT32 nPCSCFPort = 5060;

        INetworkPing* piPing = piNetCon->CreatePing();
        piPing->SetListener(this);
        nResult = piPing->Ping(objSrcIP, objDstIP, nPCSCFPort, TIME_BAD_NETWORK_CHECK);

        A_IMS_TRACE_D(AOSTAG, "RequestNetPing :: Result=%s", PingResultToString(nResult), 0, 0);

        if (nResult == INetworkPing::PING_STATUS_PENDING)
        {
            m_nBadNetworkState = STATE_BAD_NETWORK_CHECKING;
            m_piNetPing = piPing;
        }
        else
        {
            piPing->Destroy();
        }
    }

    return nResult;
}

PROTECTED
const IMS_CHAR* AosServiceAvailableWifi::PingResultToString(IN IMS_SINT32 nResult)
{
    switch (nResult)
    {
        case INetworkPing::PING_STATUS_OK:
            return "PING_STATUS_OK";

        case INetworkPing::PING_STATUS_PENDING:
            return "PING_STATUS_PENDING";

        case INetworkPing::PING_STATUS_NOK:
            return "PING_STATUS_NOK";

        case INetworkPing::PING_STATUS_DEAD_PEER:
            return "PING_STATUS_DEAD_PEER";

        case INetworkPing::PING_STATUS_TIMEDOUT:
            return "PING_STATUS_TIMEDOUT";

        default:
            return "__INVALID__";
    }
}
