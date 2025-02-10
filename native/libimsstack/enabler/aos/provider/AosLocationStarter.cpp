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
#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosBlock.h"
#include "provider/AosLocationStarter.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosLocationStarter::AosLocationStarter() :
        m_nSlotId(IMS_SLOT_0),
        m_bInitialized(IMS_FALSE),
        m_bWfcSetting(IMS_FALSE),
        m_nDefaultUpdateInterval(DEFAULT_UPDATE_INTERVAL),
        m_objVolteBlockReasons(ImsList<IMS_UINT32>()),
        m_objWfcBlockReasons(ImsList<IMS_UINT32>()),
        m_piStopDelayTimer(IMS_NULL),
        m_piAppContext(IMS_NULL)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosLocationStarter = %" PFLS_u "/%" PFLS_x,
            sizeof(AosLocationStarter), this, 0);
}

PUBLIC VIRTUAL AosLocationStarter::~AosLocationStarter()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosLocationStarter = %" PFLS_u "/%" PFLS_x,
            sizeof(AosLocationStarter), this, 0);

    m_objVolteBlockReasons.Clear();
    m_objWfcBlockReasons.Clear();

    StopDelayTimer();

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_WFC_SETTING_CHANGED, this, m_nSlotId);

    m_piAppContext->GetBlock()->RemoveListener(this);
    m_piAppContext->GetBlock()->RemoveSilentListener(this);

    ILocationInfo* piLocation = PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(m_nSlotId);
    if (piLocation != IMS_NULL)
    {
        piLocation->StopListeningForLocation();
    }
}

PUBLIC VIRTUAL IMS_SINT32 AosLocationStarter::GetSlotId() const
{
    return m_nSlotId;
}

PUBLIC VIRTUAL void AosLocationStarter::SetSlotId(IN IMS_SINT32 nSlotId)
{
    m_nSlotId = nSlotId;

    m_strTag.Sprintf("%d", m_nSlotId);
}

PUBLIC VIRTUAL void AosLocationStarter::Init(IN IAosAppContext* piContext,
        IN IMS_UINT32 nPolicy /* = POLICY_START_ON_WFC_AVAILABILITY */)
{
    if (m_bInitialized)
    {
        return;
    }

    m_bInitialized = IMS_TRUE;

    m_piAppContext = piContext;

    InitFeatures(nPolicy);

    m_piAppContext->GetBlock()->SetListener(this);
    m_piAppContext->GetBlock()->SetSilentListener(this);

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_WFC_SETTING_CHANGED, this, m_nSlotId);
}

PUBLIC VIRTUAL IMS_BOOL AosLocationStarter::SetPolicy(
        IN IMS_UINT32 nPolicy, IN IMS_SINT32 nOperation /* = 0 (add) */)
{
    if (nOperation == 0)
    {
        if (IsFeatureDisabled(nPolicy))
        {
            EnableFeature(nPolicy);
            return IMS_TRUE;
        }
    }
    else
    {
        if (IsFeatureEnabled(nPolicy))
        {
            DisableFeature(nPolicy);
            return IMS_TRUE;
        }
    }

    A_IMS_TRACE_I(AOSTAG, "set policy (%0x)", GetFeatures(), 0, 0);
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosLocationStarter::IsPolicyEnabled(IN IMS_UINT32 nPolicy)
{
    IMS_BOOL bResult = ((GetFeatures() & nPolicy) == nPolicy);
    A_IMS_TRACE_I(AOSTAG, "Is policy (%0x) enabled [%s]", GetFeatures(), _TRACE_B_(bResult), 0);

    return bResult;
}

PUBLIC VIRTUAL void AosLocationStarter::AddBlockReason(
        IN BLOCK_REASON eReason, IN IMS_SINT32 nType /* (0: VoLTE, 1: WFC) */)
{
    if (nType == TYPE_VOLTE)
    {
        AosUtil::GetInstance()->AddElementToList(eReason, m_objVolteBlockReasons);
    }
    else if (nType == TYPE_WFC)
    {
        AosUtil::GetInstance()->AddElementToList(eReason, m_objWfcBlockReasons);
    }
}

PUBLIC VIRTUAL IMS_BOOL AosLocationStarter::SetUpdateInterval(IN IMS_UINT32 nInterval)
{
    if (nInterval < DEFAULT_SHORT_UPDATE_INTERVAL)
    {
        A_IMS_TRACE_D(AOSTAG, "Update interval is too short!!", 0, 0, 0);
        return IMS_FALSE;
    }

    m_nDefaultUpdateInterval = nInterval;
    return IMS_TRUE;
}

PUBLIC VIRTUAL void AosLocationStarter::StartLocationInfoUpdate()
{
    Start();
}

PUBLIC VIRTUAL void AosLocationStarter::StopLocationInfoUpdate()
{
    Stop(DEFAULT_STOP_DELAY);
}

PROTECTED VIRTUAL void AosLocationStarter::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer != m_piStopDelayTimer)
    {
        return;
    }

    StopDelayTimer();
    Stop(0);
}

PROTECTED VIRTUAL void AosLocationStarter::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 /* nLParam */)
{
    if (nEvent == IMS_EVENT_WFC_SETTING_CHANGED)
    {
        m_bWfcSetting = (nWParam == IMS_WFC_ON) ? IMS_TRUE : IMS_FALSE;

        if (IsFeatureEnabled(POLICY_START_ON_WFC_SETTING))
        {
            HandleStartConditionChanged();
        }
    }
}

PROTECTED VIRTUAL void AosLocationStarter::Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
{
    (void)nType;
    (void)nParam;

    if (IsFeatureEnabled(POLICY_START_ON_WFC_AVAILABILITY) ||
            IsFeatureEnabled(POLICY_START_ON_VOLTE_AVAILABLE) ||
            IsFeatureEnabled(POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON) ||
            IsFeatureEnabled(POLICY_START_AFTER_CHECKING_VOLTE_BLOCK_REASON))
    {
        HandleStartConditionChanged();
    }
}

PROTECTED VIRTUAL void AosLocationStarter::Block_SilentChanged(
        IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
{
    Block_Changed(nType, nParam);
}

PROTECTED VIRTUAL void AosLocationStarter::OnFeatureEnabled(IN IMS_UINT32 nFeature)
{
    (void)nFeature;

    HandleStartConditionChanged();
}

PROTECTED VIRTUAL void AosLocationStarter::OnFeatureDisabled(IN IMS_UINT32 nFeature)
{
    (void)nFeature;

    HandleStartConditionChanged();
}

PROTECTED
void AosLocationStarter::HandleStartConditionChanged()
{
    IMS_BOOL bStart = IMS_FALSE;
    IAosBlock* piBlock = m_piAppContext->GetBlock();

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_ON_WFC_AVAILABILITY))
    {
        if (piBlock->IsCleared(SERVICE_WIFI) ||
                piBlock->IsReasonBlocked(
                        BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, IMS_TRUE, SERVICE_WIFI))
        {
            bStart = IMS_TRUE;
        }
    }

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_ON_VOLTE_AVAILABLE))
    {
        if (piBlock->IsCleared(SERVICE_CELLULAR))
        {
            bStart = IMS_TRUE;
        }
    }

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_ON_WFC_SETTING))
    {
        if (m_bWfcSetting)
        {
            bStart = IMS_TRUE;
        }
    }

    ImsList<IMS_UINT32> objBlocks;

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_AFTER_CHECKING_VOLTE_BLOCK_REASON))
    {
        piBlock->GetBlockReasons(objBlocks, SERVICE_CELLULAR);
        if (!AosUtil::GetInstance()->IsElementExistInList(m_objVolteBlockReasons, objBlocks))
        {
            bStart = IMS_TRUE;
        }
    }

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON))
    {
        piBlock->GetBlockReasons(objBlocks, SERVICE_WIFI);
        if (!AosUtil::GetInstance()->IsElementExistInList(m_objWfcBlockReasons, objBlocks))
        {
            bStart = IMS_TRUE;
        }
    }

    (bStart) ? Start() : Stop(DEFAULT_STOP_DELAY);
}

PROTECTED
void AosLocationStarter::Start()
{
    A_IMS_TRACE_I(AOSTAG, "Start", 0, 0, 0);

    StopDelayTimer();

    ILocationInfo* piLocation = PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(m_nSlotId);
    if (piLocation != IMS_NULL)
    {
        piLocation->StartListeningForLocation(m_nDefaultUpdateInterval);
    }
}

PROTECTED
void AosLocationStarter::Stop(IN IMS_UINT32 nDelayTime)
{
    A_IMS_TRACE_I(AOSTAG, "Stop :: delay (%d)", nDelayTime, 0, 0);

    if (nDelayTime > 0)
    {
        StartDelayTimer(nDelayTime * 1000);
        return;
    }

    ILocationInfo* piLocation = PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(m_nSlotId);
    if (piLocation != IMS_NULL)
    {
        piLocation->StopListeningForLocation();
    }
}

PROTECTED
IMS_BOOL AosLocationStarter::StartDelayTimer(IN IMS_UINT32 nDuration)
{
    if (m_piStopDelayTimer != IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_piStopDelayTimer =
            AosUtil::GetInstance()->StartTimer(nDuration, this, AString("TIMER_LOCATION_STOP"));

    return IMS_TRUE;
}

PROTECTED
IMS_BOOL AosLocationStarter::StopDelayTimer()
{
    if (m_piStopDelayTimer == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AosUtil::GetInstance()->StopTimer(m_piStopDelayTimer, AString("TIMER_LOCATION_STOP"));
    m_piStopDelayTimer = IMS_NULL;

    return IMS_TRUE;
}
