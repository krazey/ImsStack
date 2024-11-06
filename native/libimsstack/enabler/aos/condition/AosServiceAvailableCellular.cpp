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
#include "ServiceTrace.h"
#include "AosReason.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "provider/AosProvider.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailableCellular.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosServiceAvailableCellular::AosServiceAvailableCellular() :
        AosServiceAvailable("AosServiceAvailableCellular"),
        m_bVopsState(IMS_FALSE),
        m_bNetworkServiceIn(IMS_FALSE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosServiceAvailableCellular = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailableCellular), this, 0);
}

PUBLIC VIRTUAL AosServiceAvailableCellular::~AosServiceAvailableCellular()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosServiceAvailableCellular = %" PFLS_u "/%" PFLS_x,
            sizeof(AosServiceAvailableCellular), this, 0);
}

PUBLIC IMS_BOOL AosServiceAvailableCellular::IsVopsSupported()
{
    return m_bVopsState;
}

PROTECTED VIRTUAL void AosServiceAvailableCellular::HandleNetworkStateChanged()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    if (piNetTracker != IMS_NULL)
    {
        m_bNetworkServiceIn = piNetTracker->IsServiceIn(IAosNetTracker::TYPE_MOBILE);
    }

    A_IMS_TRACE_I(AOSTAG, "HandleNetworkStateChanged :: Is Service In - (%s)",
            _TRACE_B_(m_bNetworkServiceIn), 0, 0);

    if (m_piBlock == IMS_NULL)
    {
        return;
    }

    if (m_bNetworkServiceIn)
    {
        m_piBlock->ResetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    }
    else
    {
        m_piBlock->SetBlockReason(BLOCK_CELLULAR_OUT_OF_SERVICE);
    }
}

PROTECTED VIRTUAL void AosServiceAvailableCellular::HandleRoamingChanged(IN IMS_UINT32 nState)
{
    AosServiceAvailable::HandleRoamingChanged(nState);

    if (GET_N_CONFIG(m_nSlotId) == IMS_NULL || GET_N_CONFIG(m_nSlotId)->IsVoLteRoamingAvailable())
    {
        return;
    }

    if (m_bRoamingState)
    {
        RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, AosReason::NOT_SPECIFIED);

        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
        }
    }
    else
    {
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->ResetBlockReason(BLOCK_CELLULAR_ROAMING);
        }
    }
}

PROTECTED VIRTUAL void AosServiceAvailableCellular::HandleAirplaneModeChanged(IN IMS_UINT32 nState)
{
    AosServiceAvailable::HandleAirplaneModeChanged(nState);

    if (m_bAirplaneMode)
    {
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
        }
    }
    else
    {
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->ResetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
        }
    }
}

PROTECTED
void AosServiceAvailableCellular::HandleVopsChanged(IN IMS_UINT32 nState)
{
    m_bVopsState = (nState == IMS_VOICE_OVER_PS_SUPPORTED) ? IMS_TRUE : IMS_FALSE;

    if (m_bVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, AosReason::NOT_SPECIFIED);
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
        }
    }
    else
    {
        if (m_piBlock != IMS_NULL)
        {
            m_piBlock->ResetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
        }
    }
}

PROTECTED VIRTUAL IMS_BOOL AosServiceAvailableCellular::CheckServiceAvailable()
{
    if (GET_N_CONFIG(m_nSlotId) == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!GET_N_CONFIG(m_nSlotId)->IsVoLteAvailable())
    {
        A_IMS_TRACE_I(AOSTAG, "CheckServiceAvailable :: Cellular service config is not available",
                0, 0, 0);
        return IMS_FALSE;
    }

    if (m_piBlock == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_piBlock->IsCleared(SERVICE_CELLULAR);
}
