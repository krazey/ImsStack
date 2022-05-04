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
#include "ServiceMemory.h"
#include "ServiceThread.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ServiceEvent.h"
#include "ServiceNetworkPolicy.h"
#include "CarrierConfig.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNetTrackerListener.h"

#include "provider/AosNConfiguration.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"

#include "network/AosNetTracker.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define CNXID strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosNetTracker::AosNetTracker(IN IAosAppContext* piAppContext_) :
        nCnxPolicy(0),
        nCnxPolicyInRoaming(0),
        piNetWatcherInfo(IMS_NULL),
        piWifiWatcher(IMS_NULL)
        //    , piAppContext(piAppContext_)
        ,
        piConnection(piAppContext_->GetConnection()),
        pUtil(IMS_NULL),
        nSlotId(piAppContext_->GetSlotId()),
        nNetServiceType(NW_REPORT_SRV_NOSRV),
        nNetRadioType(NW_REPORT_RADIO_NOSRV),
        nChangingRat(NW_REPORT_RADIO_NOSRV),
        nNetVoiceRadioType(NW_REPORT_RADIO_NOSRV),
        nNetChangingVoiceRadioType(NW_REPORT_RADIO_NOSRV),
        bIsNetAvailable(IMS_FALSE),
        bIsRoaming(IMS_FALSE),
        bIsEpdgEnabled(IMS_FALSE),
        bIsWifiConnected(IMS_FALSE),
        bIsDataConnected(IMS_FALSE),
        nFeature(FEATURE_NONE),
        nServiceInTime(SERVICE_IN_TIME_MILLI_SEC),
        nServiceOutTime(SERVICE_OUT_TIME_MILLI_SEC),
        nRatTime(0),
        nVoiceRatGuardTime(0),
        piServiceInTimer(IMS_NULL),
        piServiceOutTimer(IMS_NULL),
        piRatTimer(IMS_NULL),
        piVoiceRatTimer(IMS_NULL)
{
    strTag.Sprintf("%d:%s.cnx", nSlotId, piAppContext_->GetStaticProfile()->GetId().GetStr());

    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosNetTracker = %" PFLS_u "/%" PFLS_x, strTag.GetStr(),
            sizeof(AosNetTracker), this);

    InitConfig();
    InitObject();

    Update();
}

/*

Remarks

*/
PUBLIC VIRTUAL AosNetTracker::~AosNetTracker()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosNetTracker = %" PFLS_u "/%" PFLS_x, strTag.GetStr(),
            sizeof(AosNetTracker), this);

    // Should not use the AosAppContext that is destroyed

    ClearTimers();
    objListeners.Clear();

    if (piWifiWatcher != IMS_NULL)
    {
        piWifiWatcher->RemoveObserver(this);
        piWifiWatcher = IMS_NULL;
    }

    if (piConnection != IMS_NULL)
    {
        piConnection->RemoveListener(this);
        piConnection = IMS_NULL;
    }

    if (piNetWatcherInfo != IMS_NULL)
    {
        piNetWatcherInfo->RemoveObserver(this);
        piNetWatcherInfo = IMS_NULL;
    }

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, nSlotId);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsServiceIN(IN IMS_UINT32 nType /* = TYPE_DEFAULT */)
{
    if (nType == TYPE_WLAN)
    {
        return IsWifiConnected();
    }

    if (nType == TYPE_MOBILE)
    {
        return bIsNetAvailable;
    }

    if (IsWlanEnabled())
    {
        return bIsNetAvailable || IsWifiConnected();
    }

    if (IsEpdgEnabled())
    {
        return IMS_TRUE;
    }

    return bIsNetAvailable;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsDataIN()
{
    return IsServiceAvailable(GetAccessPolicy(), nNetServiceType);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsNetworkIN()
{
    return IsRadioTechAvailable(GetAccessPolicy(), nNetRadioType);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsEmergencyLteAttach()
{
    return piNetWatcherInfo->IsLteEmergencyOnly();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsSuspended()
{
    if (IsEpdgEnabled())
    {
        return IMS_FALSE;
    }

    if (IsDataConnected() && !IsServiceIN(TYPE_MOBILE))
    {
        A_IMS_TRACE_I(CNXID, "Network Suspended", 0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsSessionContinuitySupported()
{
    IMS_UINT32 nPolicy = GetAccessPolicy();

    return ((nPolicy & NW_REPORT_RADIO_WLAN) &&
            ((nPolicy & NW_REPORT_RADIO_LTE) || (nPolicy & NW_REPORT_RADIO_NR)));
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosNetTracker::IsServiceTimerRunning()
{
    return ((piServiceInTimer != IMS_NULL) || (piServiceOutTimer != IMS_NULL));
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::SetListener(IN IAosNetTrackerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosNetTrackerListener* pTmpListener = objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            A_IMS_TRACE_D(CNXID, "SetListener() - Listener (%" PFLS_x ") is already set",
                    piListener, 0, 0);
            return;
        }
    }

    objListeners.Append(piListener);

    A_IMS_TRACE_D(CNXID, "SetListener() - Listener (%" PFLS_x ") is set", piListener, 0, 0);

    piListener->NetTracker_StatusChanged();
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::RemoveListener(IN IAosNetTrackerListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosNetTrackerListener* pTmpListener = objListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            objListeners.RemoveAt(i);

            A_IMS_TRACE_D(
                    CNXID, "RemoveListener - Listener (%" PFLS_x ") is removed", piListener, 0, 0);
            return;
        }
    }
}

/*

Remarks

*/

PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileChangingNetworkType()
{
    return nChangingRat;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileNetworkType()
{
    return nNetRadioType;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_SINT32 AosNetTracker::GetMobileVoiceServiceState()
{
    return PhoneInfoService::GetPhoneInfoService()
            ->GetNetworkWatcher(nSlotId)
            ->GetNetVoiceServiceType();
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetMobileVoiceNetworkType()
{
    A_IMS_TRACE_D(CNXID, "GetMobileVoiceNetworkType :: (%s)", RadioTypeToString(nNetVoiceRadioType),
            0, 0);

    return nNetVoiceRadioType;
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_UINT32 AosNetTracker::GetNetworkType()
{
    if (IsEpdgEnabled())
    {
        return NW_REPORT_RADIO_WLAN;
    }

    return nNetRadioType;
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::SetRATGuardTime(IN IMS_UINT32 nGuardTime)
{
    nRatTime = nGuardTime;

    // Here it uses the same value for RAT change and voice RAT change by default.
    nVoiceRatGuardTime = nGuardTime;

    if (nRatTime > 0)
    {
        nFeature |= FEATURE_RAT_GUARD;
        nFeature |= FEATURE_VOICE_RAT_GUARD;

        /*
            When SetRATGuardTime() is enabled, nChangingRat must be initialized to nNetRadioType.
            When FEATURE_RAT_GUARD is not enabled, it doesn't update nChangingRat.
            When FEATURE_RAT_GUARD become enabled,
            it's possible nChangingRat and nNetRadioType are different.
            If RAT changes but the new RAT is the same are nChangingRat,
            then, the change will not be notified.
        */
        nChangingRat = nNetRadioType;
        nNetChangingVoiceRadioType = nNetVoiceRadioType;
    }
    else
    {
        nFeature &= ~(FEATURE_RAT_GUARD);
        nFeature &= ~(FEATURE_VOICE_RAT_GUARD);
    }

    A_IMS_TRACE_I(CNXID, "SetRATGuardTime() :: Feature(%s)", FeaturesToString().GetStr(), 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::SetSrvOutGuardTime(IN IMS_UINT32 nGuardTime)
{
    nServiceOutTime = nGuardTime;

    if (nServiceOutTime > 0)
    {
        nFeature |= FEATURE_OUT_GUARD;
    }
    else
    {
        nFeature &= ~(FEATURE_OUT_GUARD);
    }

    A_IMS_TRACE_I(CNXID, "SetSrvOutGuardTime() :: Feature(%s)", FeaturesToString().GetStr(), 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::SetSrvInGuardTime(IN IMS_UINT32 nGuardTime)
{
    nServiceInTime = nGuardTime;

    if (nServiceInTime > 0)
    {
        nFeature |= FEATURE_IN_GUARD;
    }
    else
    {
        nFeature &= ~(FEATURE_IN_GUARD);
    }

    A_IMS_TRACE_I(CNXID, "SetSrvInGuardTime() :: Feature(%s)", FeaturesToString().GetStr(), 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo)
{
    if (this->piNetWatcherInfo != piNetWatcherInfo)
    {
        return;
    }

    A_IMS_TRACE_D(CNXID, "NotifyNetWatcherStatus", 0, 0, 0);

    if (nFeature != FEATURE_NONE)
    {
        ProcessNetworkChanged(REASON_NET_STATE_CHANGED);
        ProcessVoiceNetworkChanged();
        return;
    }

    IMS_BOOL bOldNetIN = bIsNetAvailable;
    IMS_UINT32 nOldRadioType = nNetRadioType;
    IMS_UINT32 nOldVoiceRadioType = nNetVoiceRadioType;

    Update();
    UpdateVoiceNetwork();

    if ((bIsNetAvailable != bOldNetIN) || (nOldRadioType != nNetRadioType) ||
            (nOldVoiceRadioType != nNetVoiceRadioType))
    {
        Notify();
    }
    else
    {
        A_IMS_TRACE_D(CNXID, "NotifyNetWatcherStatus :: no changed", 0, 0, 0);
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::WifiWatcher_NotifyStateChanged(IN IWifiWatcher* pIWifiWatcher)
{
    if (piWifiWatcher != pIWifiWatcher)
    {
        return;
    }

    IMS_BOOL bCurrConnected =
            (piWifiWatcher->GetState() == IWifiWatcher::STATE_CONNECTED) ? IMS_TRUE : IMS_FALSE;

    if (IsWifiConnected() != bCurrConnected)
    {
        SetWifiConnected(bCurrConnected);
        Notify();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosNetTracker::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 nLParam)
{
    A_IMS_TRACE_I(CNXID, "Event_NotifyEvent :: E(%X)/W(%d)/L(%d)", nEvent, nWParam, nLParam);

    switch (nEvent)
    {
        case IMS_EVENT_ROAMING_STATE:
        {
            IMS_BOOL bCurrRoaming = (nWParam == IMS_ROAMING_STATE_OFF)
                    ? (nLParam == IMS_ROAMING_STATE_ON)
                    : IMS_TRUE;

            if (bIsRoaming != bCurrRoaming)
            {
                bIsRoaming = bCurrRoaming;

                if (nFeature != FEATURE_NONE)
                {
                    ProcessNetworkChanged(REASON_ROAMING_SATAE_CHANGED);
                    ProcessVoiceNetworkChanged();
                }
                else
                {
                    Update();
                    UpdateVoiceNetwork();
                    Notify();
                }
            }
            break;
        }
        default:
            // No operation
            break;
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::InitConfig()
{
    if (IsCnxTypeEqual(NetworkPolicy::APN_IMS))
    {
        nCnxPolicy |= NW_REPORT_SRV_SRV;

        InitCnxPolicy(GET_N_CONFIG(nSlotId)->GetSupportedRats());

        if (GET_N_CONFIG(nSlotId)->IsSmsOverImsSupported())
        {
            InitCnxPolicy(GET_N_CONFIG(nSlotId)->GetSmsOverImsSupportedRats());
        }

        if (IsVoNRSupported())
        {
            nCnxPolicy |= NW_REPORT_RADIO_NR;
        }

        if (IsRoamingAccessPolicyRequired())
        {
            nCnxPolicyInRoaming |= NW_REPORT_SRV_SRV;
            InitRoamingCnxPolicy(GET_N_CONFIG(nSlotId)->GetSupportedRoamingRats());

            A_IMS_TRACE_I(
                    CNXID, "InitConfig :: nCnxPolicyInRoaming(%X)", nCnxPolicyInRoaming, 0, 0);
            IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, nSlotId);
        }
    }
    else if (IsCnxTypeEqual(NetworkPolicy::APN_EMERGENCY))
    {
        nCnxPolicy = 0xFFFFFFFF;
    }
    else if (IsCnxTypeEqual(NetworkPolicy::APN_WIFI))
    {
        nCnxPolicy = 0x01000000;
    }

    A_IMS_TRACE_I(CNXID, "InitConfig :: nPolicy(%X)", nCnxPolicy, 0, 0);

    if (nServiceInTime > 0)
    {
        nFeature |= FEATURE_IN_GUARD;
    }

    if (nServiceOutTime > 0)
    {
        nFeature |= FEATURE_OUT_GUARD;
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::InitCnxPolicy(IN IMSVector<IMS_SINT32>& objRats)
{
    for (int i = 0; i < objRats.GetSize(); i++)
    {
        if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN)
        {
            nCnxPolicy |= NW_REPORT_RADIO_LTE;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN)
        {
            nCnxPolicy |= NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN)
        {
            nCnxPolicy |= NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE;
        }
        else if (objRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN)
        {
            nCnxPolicy |= NW_REPORT_RADIO_WLAN;
        }
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::InitRoamingCnxPolicy(IN IMSVector<IMS_SINT32>& objRoamingRats)
{
    for (int i = 0; i < objRoamingRats.GetSize(); i++)
    {
        if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN)
        {
            nCnxPolicyInRoaming |= NW_REPORT_RADIO_NR;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN)
        {
            nCnxPolicyInRoaming |= NW_REPORT_RADIO_LTE;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN)
        {
            nCnxPolicyInRoaming |= NW_REPORT_RADIO_WCDMA | NW_REPORT_RADIO_HSPA;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN)
        {
            nCnxPolicyInRoaming |= NW_REPORT_RADIO_GSM | NW_REPORT_RADIO_EDGE;
        }
        else if (objRoamingRats.GetAt(i) == CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN)
        {
            nCnxPolicyInRoaming |= NW_REPORT_RADIO_WLAN;
        }
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::InitObject()
{
    A_IMS_TRACE_D(CNXID, "InitObject", 0, 0, 0);

    pUtil = AosUtil::GetInstance();

    piNetWatcherInfo = PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(nSlotId);
    if (piNetWatcherInfo != IMS_NULL)
    {
        piNetWatcherInfo->RegisterObserver(this);
        if (IsRoamingAccessPolicyRequired())
        {
            bIsRoaming = piNetWatcherInfo->GetRoamingState() ? IMS_TRUE : IMS_FALSE;
            A_IMS_TRACE_I(
                    CNXID, "InitObject :: bIsRoaming(%s)", bIsRoaming ? "TRUE" : "FALSE", 0, 0);
        }
    }

    if (IsCnxTypeEqual(NetworkPolicy::APN_IMS))
    {
        piConnection->SetListener(this);

        if (IsWlanEnabled())
        {
            A_IMS_TRACE_I(CNXID, "InitObject :: wlan is enabled", 0, 0, 0);

            piWifiWatcher = PhoneInfoService::GetPhoneInfoService()->GetWifiWatcher();
            if (piWifiWatcher != IMS_NULL)
            {
                piWifiWatcher->RegisterObserver(this);
            }

            IMS_EVENT_SendEventForSlotId(IMS_EVENT_WIFI_SERVICE, IMS_WIFI_ON, 0, nSlotId);
        }
    }
    else
    {
        if (IsCnxTypeEqual(NetworkPolicy::APN_WIFI))
        {
            A_IMS_TRACE_I(CNXID, "InitObject :: apn is wifi", 0, 0, 0);
            IMS_EVENT_SendEventForSlotId(IMS_EVENT_WIFI_SERVICE, IMS_WIFI_ON, 0, nSlotId);
        }
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::Update()
{
    // update data service & radio state
    IMS_SINT32 nOldService = nNetServiceType;
    IMS_UINT32 nOldRadio = nNetRadioType;
    IMS_BOOL bOldIN = bIsNetAvailable;

    A_IMS_TRACE_I(CNXID, "Old Status :: service(%s), radio(%s), availability(%s)",
            ServiceTypeToString(nOldService), RadioTypeToString(nOldRadio),
            (bOldIN) ? "IN" : "OUT");

    GetStatus(nNetServiceType, nNetRadioType, bIsNetAvailable);
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::UpdateVoiceNetwork()
{
    if (piNetWatcherInfo != IMS_NULL)
    {
        nNetVoiceRadioType = piNetWatcherInfo->GetNetVoiceRadioTechType();
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::Notify()
{
    for (IMS_UINT32 i = 0; i < objListeners.GetSize(); ++i)
    {
        IAosNetTrackerListener* pListener = objListeners.GetAt(i);

        if (pListener == IMS_NULL)
        {
            continue;
        }

        pListener->NetTracker_StatusChanged();
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::GetStatus(
        OUT IMS_SINT32& nService, OUT IMS_UINT32& nRadioTech, OUT IMS_BOOL& bIsIN)
{
    nService = piNetWatcherInfo->GetNetServiceType(
            AString::ConstNull(), piConnection->GetConnectionType());
    nRadioTech = piNetWatcherInfo->GetNetRadioTechType(
            AString::ConstNull(), piConnection->GetConnectionType());

    bIsIN = (IsServiceAvailable(GetAccessPolicy(), nService) &&
                    IsRadioTechAvailable(GetAccessPolicy(), nRadioTech))
            ? IMS_TRUE
            : IMS_FALSE;

    A_IMS_TRACE_I(CNXID, "GetStatus :: service(%s), radio(%s), availability(%s)",
            ServiceTypeToString(nService), RadioTypeToString(nRadioTech), (bIsIN) ? "IN" : "OUT");
}

/*

Remarks

*/
PRIVATE
IMS_UINT32 AosNetTracker::GetAccessPolicy() const
{
    if (IsRoamingAccessPolicyRequired())
    {
        A_IMS_TRACE_I(
                CNXID, "GetAccessPolicy :: bIsRoaming(%s)", bIsRoaming ? "TRUE" : "FALSE", 0, 0);
        return bIsRoaming ? nCnxPolicyInRoaming : nCnxPolicy;
    }

    return nCnxPolicy;
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ProcessNetworkChanged(IMS_SINT32 nReason)
{
    IMS_SINT32 nCurrService = NW_REPORT_SRV_NOSRV;
    IMS_UINT32 nCurrRat = NW_REPORT_RADIO_NOSRV;
    IMS_BOOL bCurrIN = IMS_FALSE;
    IMS_BOOL bNotify = IMS_FALSE;

    GetStatus(nCurrService, nCurrRat, bCurrIN);

    if ((nReason != REASON_ROAMING_SATAE_CHANGED) && !IsNetworkChanged(nCurrRat, nCurrService))
    {
        return;
    }

    // Service Change
    if (nCurrService != nNetServiceType)
    {
        nNetServiceType = nCurrService;
    }

    // Rat Change
    if (pUtil->IsFeatureOn(FEATURE_RAT_GUARD, nFeature))
    {
        if (nNetRadioType != nCurrRat)
        {
            if (nChangingRat != nCurrRat)
            {
                nChangingRat = nCurrRat;
                StartTimer(TIMER_RAT_GUARD, nRatTime);
            }
        }
        else
        {
            nChangingRat = nCurrRat;
            StopTimer(TIMER_RAT_GUARD);
        }
    }
    else
    {
        if (nNetRadioType != nCurrRat)
        {
            nNetRadioType = nCurrRat;
            bNotify = IMS_TRUE;
        }
    }

    // In/Out Change
    if (!pUtil->IsFeatureOn(FEATURE_IN_GUARD, nFeature) &&
            !pUtil->IsFeatureOn(FEATURE_OUT_GUARD, nFeature))
    {
        if (bCurrIN != bIsNetAvailable)
        {
            bIsNetAvailable = bCurrIN;
            bNotify = IMS_TRUE;
        }
    }
    else
    {
        if (bCurrIN != bIsNetAvailable)
        {
            if (bIsNetAvailable)
            {
                // IN -> OUT
                if (pUtil->IsFeatureOn(FEATURE_OUT_GUARD, nFeature))
                {
                    if (piServiceOutTimer == IMS_NULL)
                    {
                        StartTimer(TIMER_OUT_GUARD, nServiceOutTime);
                    }
                }
                else
                {
                    bIsNetAvailable = bCurrIN;
                    bNotify = IMS_TRUE;
                }
            }
            else
            {
                // OUT -> IN
                if (pUtil->IsFeatureOn(FEATURE_IN_GUARD, nFeature))
                {
                    if (piServiceInTimer == IMS_NULL)
                    {
                        StartTimer(TIMER_IN_GUARD, nServiceInTime);
                    }
                }
                else
                {
                    bIsNetAvailable = bCurrIN;
                    bNotify = IMS_TRUE;
                }
            }
        }
        else
        {
            StopTimer(TIMER_IN_GUARD);
            StopTimer(TIMER_OUT_GUARD);
        }
    }

    if (bNotify)
    {
        Notify();
    }
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ProcessVoiceNetworkChanged()
{
    IMS_UINT32 nCurrVoiceRat = (piNetWatcherInfo != IMS_NULL)
            ? piNetWatcherInfo->GetNetVoiceRadioTechType()
            : NW_REPORT_RADIO_NOSRV;

    A_IMS_TRACE_I(CNXID, "nCurrVoiceRat (%s)", RadioTypeToString(nCurrVoiceRat), 0, 0);

    if (pUtil->IsFeatureOn(FEATURE_VOICE_RAT_GUARD, nFeature))
    {
        if (nCurrVoiceRat == nNetVoiceRadioType)
        {
            nNetChangingVoiceRadioType = nCurrVoiceRat;
            StopTimer(TIMER_VOICE_RAT_GUARD);
        }
        else
        {
            if (nNetChangingVoiceRadioType == nCurrVoiceRat)
            {
                return;
            }

            nNetChangingVoiceRadioType = nCurrVoiceRat;
            StartTimer(TIMER_VOICE_RAT_GUARD, nVoiceRatGuardTime);
        }
    }
    else
    {
        // If it doesn't support gaurding timer, update and notify the change.
        if (nCurrVoiceRat != nNetVoiceRadioType)
        {
            nNetVoiceRadioType = nCurrVoiceRat;
            Notify();
        }
    }
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsRadioTechAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nRadioTech)
{
    return ((nPolicy & 0xffff0000) & (nRadioTech & 0xffff0000));
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsServiceAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nService)
{
    return ((nPolicy & 0x000000ff) & (nService & 0x000000ff));
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsDomainAvailable(IN IMS_UINT32 nPolicy, IN IMS_UINT32 nDomain)
{
    return ((nPolicy & 0x0000ff00) & (nDomain & 0x0000ff00));
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsNetworkChanged(IN IMS_UINT32 nCurrRat, IN IMS_SINT32 nCurrService)
{
    if (pUtil->IsFeatureOn(FEATURE_RAT_GUARD, nFeature))
    {
        if ((nNetServiceType == nCurrService) && (nNetRadioType == nCurrRat) &&
                (nChangingRat == nCurrRat))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if ((nNetServiceType == nCurrService) && (nNetRadioType == nCurrRat))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsDataConnected() const
{
    return bIsDataConnected;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsEpdgEnabled() const
{
    return bIsEpdgEnabled;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsCnxTypeEqual(IN IMS_SINT32 nType) const
{
    return (piConnection->GetConnectionType() == nType);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsWlanEnabled() const
{
    return (GetAccessPolicy() & NW_REPORT_RADIO_WLAN);
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsWifiConnected() const
{
    return bIsWifiConnected;
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsVoNRSupported()
{
    return GET_N_CONFIG(nSlotId)->IsImsOverNrEnabled();
}

/*

Remarks

*/
PRIVATE
IMS_BOOL AosNetTracker::IsRoamingAccessPolicyRequired() const
{
    return ((GET_N_CONFIG(nSlotId)->GetSupportedRoamingRats()).GetSize() > 0);
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::SetDataConnected(IN IMS_BOOL bConnected)
{
    if (bIsDataConnected != bConnected)
    {
        A_IMS_TRACE_I(CNXID, "SetDataConnected :: connected (%s)", _TRACE_B_(bConnected), 0, 0);
    }

    bIsDataConnected = bConnected;
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::SetEpdgEnabled(IN IMS_BOOL bEnabled)
{
    if (bIsEpdgEnabled != bEnabled)
    {
        A_IMS_TRACE_I(CNXID, "SetEpdgEnabled :: enabled (%s)", _TRACE_B_(bEnabled), 0, 0);
    }

    bIsEpdgEnabled = bEnabled;
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::SetWifiConnected(IN IMS_BOOL bConnected)
{
    if (bIsWifiConnected != bConnected)
    {
        A_IMS_TRACE_I(CNXID, "SetWifiConnected :: connected (%s)", _TRACE_B_(bConnected), 0, 0);
    }

    bIsWifiConnected = bConnected;
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ProcessInTimerExpired()
{
    StopTimer(TIMER_IN_GUARD);

    bIsNetAvailable = IMS_TRUE;
    Notify();
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ProcessOutTimerExpired()
{
    StopTimer(TIMER_OUT_GUARD);
    bIsNetAvailable = IMS_FALSE;
    Notify();
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ProcessRatTimerExpired()
{
    StopTimer(TIMER_RAT_GUARD);

    nNetRadioType = nChangingRat;
    Notify();
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ProcessVoiceRatTimerExpired()
{
    StopTimer(TIMER_VOICE_RAT_GUARD);

    nNetVoiceRadioType = nNetChangingVoiceRadioType;
    Notify();
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::ClearTimers()
{
    StopTimer(TIMER_IN_GUARD);
    StopTimer(TIMER_OUT_GUARD);
    StopTimer(TIMER_RAT_GUARD);
    StopTimer(TIMER_VOICE_RAT_GUARD);
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_IN_GUARD:
            ppiTimer = &piServiceInTimer;
            break;

        case TIMER_OUT_GUARD:
            ppiTimer = &piServiceOutTimer;
            break;

        case TIMER_RAT_GUARD:
            ppiTimer = &piRatTimer;
            break;

        case TIMER_VOICE_RAT_GUARD:
            ppiTimer = &piVoiceRatTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        StopTimer(nType);
    }

    *ppiTimer = pUtil->StartTimer(nDuration, this, TimerToString(nType));
}

/*

Remarks

*/
PRIVATE
void AosNetTracker::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_IN_GUARD:
            ppiTimer = &piServiceInTimer;
            break;

        case TIMER_OUT_GUARD:
            ppiTimer = &piServiceOutTimer;
            break;

        case TIMER_RAT_GUARD:
            ppiTimer = &piRatTimer;
            break;

        case TIMER_VOICE_RAT_GUARD:
            ppiTimer = &piVoiceRatTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    pUtil->StopTimer(*ppiTimer, TimerToString(nType));
}

/*

Remarks

*/
PRIVATE VIRTUAL void AosNetTracker::AosConnection_StateChanged(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_D(CNXID, "AoSConnection_StateChanged :: nState (%d)", nState, 0, 0);

    IMS_BOOL bCurrConnected = (nState == IAosConnection::STATE_ACTIVE) ? IMS_TRUE : IMS_FALSE;

    if (IsDataConnected() != bCurrConnected)
    {
        SetDataConnected(bCurrConnected);

        // check the epdg connection
        if (bCurrConnected)
        {
            SetEpdgEnabled(piConnection->IsEpdgEnabled());
        }

        Notify();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL void AosNetTracker::AosConnection_IpChanged() {}

/*

Remarks

*/
PRIVATE VIRTUAL void AosNetTracker::AosConnection_IpcanCatChanged()
{
    IMS_BOOL bEpdgEnabled = piConnection->IsEpdgEnabled();

    if (IsEpdgEnabled() == bEpdgEnabled)
    {
        return;
    }

    SetEpdgEnabled(bEpdgEnabled);

    Notify();
}

/*

Remarks

*/
PRIVATE VIRTUAL void AosNetTracker::AosConnection_PcscfChanged() {}

/*

Remarks

*/
PRIVATE VIRTUAL void AosNetTracker::AosConnection_ConnectionFailed() {}

/*

Remarks

*/
PRIVATE VIRTUAL void AosNetTracker::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == piServiceInTimer)
    {
        ProcessInTimerExpired();
        return;
    }

    if (piTimer == piServiceOutTimer)
    {
        ProcessOutTimerExpired();
        return;
    }

    if (piTimer == piRatTimer)
    {
        ProcessRatTimerExpired();
        return;
    }

    if (piTimer == piVoiceRatTimer)
    {
        ProcessVoiceRatTimerExpired();
        return;
    }
}

/*

Remarks

*/
PRIVATE
AString AosNetTracker::FeaturesToString()
{
    AString strFeature = "| ";

    if (nFeature & FEATURE_IN_GUARD)
        strFeature += "FEATURE_IN_GUARD | ";

    if (nFeature & FEATURE_OUT_GUARD)
        strFeature += "FEATURE_OUT_GUARD | ";

    if (nFeature & FEATURE_RAT_GUARD)
        strFeature += "FEATURE_RAT_GUARD | ";

    if (nFeature & FEATURE_VOICE_RAT_GUARD)
        strFeature += "FEATURE_VOICE_RAT_GUARD | ";

    return strFeature;
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* AosNetTracker::DomainTypeToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case (0x0100 << 0):
            return "NW_REPORT_DOMAIN_NOSRV";

        case (0x0100 << 1):
            return "NW_REPORT_DOMAIN_CS";

        case (0x0100 << 2):
            return "NW_REPORT_DOMAIN_PS";

        case (0x0100 << 3):
            return "NW_REPORT_DOMAIN_CSPS";

        case (0x0100 << 4):
            return "NW_REPORT_DOMAIN_CAMPED";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* AosNetTracker::RadioTypeToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case (0x00010000 << 0):
            return "NW_REPORT_RADIO_NOSRV";

        case (0x00010000 << 1):
            return "NW_REPORT_RADIO_AMPS";

        case (0x00010000 << 2):
            return "NW_REPORT_RADIO_CDMA";

        case (0x00010000 << 3):
            return "NW_REPORT_RADIO_GSM";

        case (0x00010000 << 4):
            return "NW_REPORT_RADIO_HDR";

        case (0x00010000 << 5):
            return "NW_REPORT_RADIO_WCDMA";

        case (0x00010000 << 6):
            return "NW_REPORT_RADIO_GPS";

        case (0x00010000 << 7):
            return "NW_REPORT_RADIO_EDGE";

        case (0x00010000 << 8):
            return "NW_REPORT_RADIO_WLAN";

        case (0x00010000 << 9):
            return "NW_REPORT_RADIO_CDMA1X";

        case (0x00010000 << 10):
            return "NW_REPORT_RADIO_EVDODO";

        case (0x00010000 << 11):
            return "NW_REPORT_RADIO_EVDORA";

        case (0x00010000 << 12):
            return "NW_REPORT_RADIO_EHRPD";

        case (0x00010000 << 13):
            return "NW_REPORT_RADIO_LTE";

        case (0x00010000 << 14):
            return "NW_REPORT_RADIO_HSPA";

        case NW_REPORT_RADIO_NR:
            return "NW_REPORT_RADIO_NR";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PRIVATE GLOBAL const IMS_CHAR* AosNetTracker::ServiceTypeToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case (0x01 << 0):
            return "NW_REPORT_SRV_NOSRV";

        case (0x01 << 1):
            return "NW_REPORT_SRV_LIMITED";

        case (0x01 << 2):
            return "NW_REPORT_SRV_SRV";

        case (0x01 << 3):
            return "NW_REPORT_SRV_LIMITEDREGION";

        case (0x01 << 4):
            return "NW_REPORT_SRV_PWRSAVE";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* AosNetTracker::TimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TIMER_IN_GUARD:
            return "TIMER_IN_GUARD";

        case TIMER_OUT_GUARD:
            return "TIMER_OUT_GUARD";

        case TIMER_RAT_GUARD:
            return "TIMER_RAT_GUARD";

        case TIMER_VOICE_RAT_GUARD:
            return "TIMER_VOICE_RAT_GUARD";

        default:
            return "__INVALID__";
    }
}
