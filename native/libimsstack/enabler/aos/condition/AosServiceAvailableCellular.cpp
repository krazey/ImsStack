#include "ServiceEvent.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "AoSReason.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "provider/AosProvider.h"
#include "provider/AosUtil.h"
#include "condition/AosCondition.h"
#include "condition/AosServiceAvailableCellular.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG m_strTag.GetStr()

PUBLIC
AosServiceAvailableCellular::AosServiceAvailableCellular() :
        AosServiceAvailable("AosServiceAvailableCellular"),
        m_bVolteSetting(IMS_FALSE),
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

PRIVATE VIRTUAL void AosServiceAvailableCellular::HandleNetworkStateChanged()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    if (piNetTracker != IMS_NULL)
    {
        m_bNetworkServiceIn = piNetTracker->IsServiceIN(IAosNetTracker::TYPE_MOBILE);
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

PRIVATE VIRTUAL void AosServiceAvailableCellular::HandleRoamingChanged(IN IMS_UINT32 nState)
{
    AosServiceAvailable::HandleRoamingChanged(nState);

    if (GET_N_CONFIG(m_nSlotId)->IsVoLteRoamingAvailable())
    {
        return;
    }

    if (m_bRoamingState)
    {
        RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, AoSReason::NOT_SPECIFIED);
        m_piBlock->SetBlockReason(BLOCK_CELLULAR_ROAMING);
    }
    else
    {
        m_piBlock->ResetBlockReason(BLOCK_CELLULAR_ROAMING);
    }
}

PRIVATE VIRTUAL void AosServiceAvailableCellular::HandleAirplaneModeChanged(IN IMS_UINT32 nState)
{
    AosServiceAvailable::HandleAirplaneModeChanged(nState);

    if (!GET_N_CONFIG(m_nSlotId)->IsRequiredVolteBlockByAirplaneMode())
    {
        return;
    }

    if (m_bAirplaneMode)
    {
        m_piBlock->SetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    }
    else
    {
        m_piBlock->ResetBlockReason(BLOCK_CELLULAR_AIRPLANE_MODE_ON);
    }
}

PRIVATE
void AosServiceAvailableCellular::HandleVolteSettingChanged(IN IMS_UINT32 nState)
{
    m_bVolteSetting = (nState == IMS_VOLTE_SETTING_ON) ? IMS_TRUE : IMS_FALSE;

    if (!GET_N_CONFIG(m_nSlotId)->IsRequiredVolteBlockBySetting())
    {
        return;
    }

    if (m_bVolteSetting == IMS_FALSE)
    {
        RequestCommand(AosCondition::REQUEST_STOP, AoSReason::NOT_SPECIFIED);
        m_piBlock->SetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    }
    else
    {
        m_piBlock->ResetBlockReason(BLOCK_CELLULAR_VOLTE_OFF);
    }
}

PRIVATE
void AosServiceAvailableCellular::HandleVopsChanged(IN IMS_UINT32 nState)
{
    m_bVopsState = (nState == IMS_VOICE_OVER_PS_SUPPORTED) ? IMS_TRUE : IMS_FALSE;

    if (GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled())
    {
        return;
    }

    if (!IsAvailableRatForVops(m_bVopsState))
    {
        return;
    }

    if (m_bVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        RequestCommand(AosCondition::REQUEST_PDN_DISCONNECT, AoSReason::NOT_SPECIFIED);
        m_piBlock->SetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    }
    else
    {
        m_piBlock->ResetBlockReason(BLOCK_CELLULAR_VOPS_OFF);
    }
}

PRIVATE VIRTUAL IMS_BOOL AosServiceAvailableCellular::CheckServiceAvailable()
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

    return m_piBlock->IsCleared(SERVICE_CELLULAR);
}

PRIVATE
IMS_BOOL AosServiceAvailableCellular::IsAvailableRatForVops(IN IMS_UINT32 nState)
{
    IMS_UINT32 nNetworkType = m_piAppContext->GetNetTracker()->GetNetworkType();
    IMS_UINT32 nNetVoiceNetworkType = m_piAppContext->GetNetTracker()->GetMobileVoiceNetworkType();
    IMS_BOOL bResult = IMS_TRUE;

    if (nState == IMS_VOICE_OVER_PS_SUPPORTED)
    {
        if (!AosUtil::GetInstance()->IsSupportedNetworkTypeForCellular(nNetworkType))
        {
            bResult = IMS_FALSE;
        }
    }
    else
    {
        if (!AosUtil::GetInstance()->IsSupportedNetworkTypeForCellular(nNetworkType) ||
                !AosUtil::GetInstance()->IsSupportedNetworkTypeForCellular(nNetVoiceNetworkType))
        {
            bResult = IMS_FALSE;
        }
    }

    A_IMS_TRACE_D(AOSTAG, "IsAvailableRatForVops :: voice(0x%08X), data(0x%08X), result(%s)",
            nNetVoiceNetworkType, nNetworkType, _TRACE_B_(bResult));

    return bResult;
}
