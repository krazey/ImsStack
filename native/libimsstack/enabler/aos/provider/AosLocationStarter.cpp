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

__IMS_TRACE_TAG_USER_DECL__("AOS");

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
        m_piAppContext(IMS_NULL),
        m_piBlock(IMS_NULL)
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

    StopTimer(TIMER_STOP_DELAY);

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_WFC_SETTING_CHANGED, this, m_nSlotId);

    if (m_piBlock != IMS_NULL)
    {
        m_piBlock->RemoveListener(this);
    }

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

PUBLIC VIRTUAL IMS_BOOL AosLocationStarter::Init(IN IAosAppContext* piContext,
        IN IMS_UINT32 nPolicy /* = POLICY_START_ON_WFC_AVAILABILITY */)
{
    if (m_bInitialized)
    {
        return IMS_FALSE;
    }

    m_bInitialized = IMS_TRUE;

    m_piAppContext = piContext;

    InitFeatures(nPolicy);

    m_piBlock = m_piAppContext->GetBlock();
    if (m_piBlock != IMS_NULL)
    {
        m_piBlock->SetListener(this);
    }

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_WFC_SETTING_CHANGED, this, m_nSlotId);

    return IMS_TRUE;
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

PUBLIC VIRTUAL IMS_BOOL AosLocationStarter::StartLocationInfoUpdate()
{
    return Start();
}

PUBLIC VIRTUAL IMS_BOOL AosLocationStarter::StopLocationInfoUpdate()
{
    return Stop(DEFAULT_STOP_DELAY);
}

PRIVATE VIRTUAL void AosLocationStarter::OnFeatureEnabled(IN IMS_UINT32 nFeature)
{
    (void)nFeature;

    HandleStartConditionChanged();
}

PRIVATE VIRTUAL void AosLocationStarter::OnFeatureDisabled(IN IMS_UINT32 nFeature)
{
    (void)nFeature;

    HandleStartConditionChanged();
}

PRIVATE VIRTUAL void AosLocationStarter::Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
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

PRIVATE VIRTUAL void AosLocationStarter::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == m_piStopDelayTimer)
    {
        StopTimer(TIMER_STOP_DELAY);
        Stop(0);
        return;
    }
}

PRIVATE VIRTUAL void AosLocationStarter::Event_NotifyEvent(
        IN IMS_SINT32 nEvent, IN IMS_UINT32 nWParam, IN IMS_UINT32 /* nLParam */)
{
    switch (nEvent)
    {
        case IMS_EVENT_WFC_SETTING_CHANGED:
            m_bWfcSetting = IMS_FALSE;

            if (nWParam == IMS_WFC_ON)
            {
                m_bWfcSetting = IMS_TRUE;
            }

            if (IsFeatureEnabled(POLICY_START_ON_WFC_SETTING))
            {
                HandleStartConditionChanged();
            }
            break;

        default:
            break;
    }
}

PRIVATE
IMS_BOOL AosLocationStarter::HandleStartConditionChanged()
{
    IMS_BOOL bStart = IMS_FALSE;

    if (m_piBlock == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_ON_WFC_AVAILABILITY))
    {
        if (m_piBlock->IsCleared(SERVICE_WIFI) ||
                m_piBlock->IsReasonBlocked(
                        BLOCK_WIFI_COUNTRY_CODE_UNAVAILABLE, IMS_TRUE, SERVICE_WIFI))
        {
            bStart = IMS_TRUE;
        }
    }

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_ON_VOLTE_AVAILABLE))
    {
        if (m_piBlock->IsCleared(SERVICE_CELLULAR))
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
        m_piBlock->GetBlockReasons(objBlocks, SERVICE_CELLULAR);
        if (!AosUtil::GetInstance()->IsElementExistInList(m_objVolteBlockReasons, objBlocks))
        {
            bStart = IMS_TRUE;
        }
    }

    // TODO : Need to Config
    if (IsFeatureEnabled(POLICY_START_AFTER_CHECKING_WFC_BLOCK_REASON))
    {
        m_piBlock->GetBlockReasons(objBlocks, SERVICE_WIFI);
        if (!AosUtil::GetInstance()->IsElementExistInList(m_objWfcBlockReasons, objBlocks))
        {
            bStart = IMS_TRUE;
        }
    }

    return (bStart) ? Start() : Stop(DEFAULT_STOP_DELAY);
}

PRIVATE
IMS_BOOL AosLocationStarter::Start()
{
    A_IMS_TRACE_I(AOSTAG, "Start", 0, 0, 0);

    StopTimer(TIMER_STOP_DELAY);

    ILocationInfo* piLocation = PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(m_nSlotId);
    if (piLocation != IMS_NULL)
    {
        piLocation->StartListeningForLocation(m_nDefaultUpdateInterval);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AosLocationStarter::Stop(IN IMS_UINT32 nDelayTime)
{
    A_IMS_TRACE_I(AOSTAG, "Stop :: delay (%d)", nDelayTime, 0, 0);

    if (nDelayTime > 0)
    {
        StartTimer(TIMER_STOP_DELAY, nDelayTime * 1000);
        return IMS_FALSE;
    }

    ILocationInfo* piLocation = PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(m_nSlotId);
    if (piLocation != IMS_NULL)
    {
        piLocation->StopListeningForLocation();
    }

    return IMS_TRUE;
}

PRIVATE
void AosLocationStarter::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_STOP_DELAY:
            ppiTimer = &m_piStopDelayTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer != IMS_NULL)
    {
        return;
    }

    *ppiTimer = AosUtil::GetInstance()->StartTimer(nDuration, this, AString("TIMER_LOCATION_STOP"));
}

PRIVATE
void AosLocationStarter::StopTimer(IN IMS_UINT32 nType)
{
    ITimer** ppiTimer = IMS_NULL;

    switch (nType)
    {
        case TIMER_STOP_DELAY:
            ppiTimer = &m_piStopDelayTimer;
            break;

        default:
            return;
    }

    if (*ppiTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(*ppiTimer, AString("TIMER_LOCATION_STOP"));
}