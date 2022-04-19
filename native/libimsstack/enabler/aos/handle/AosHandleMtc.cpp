#include "ServiceTrace.h"
#include "ServiceEvent.h"
#include "ServiceNetworkPolicy.h"

#include "AoSReason.h"

#include "IImsAosListener.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosService.h"

#include "provider/AosProvider.h"
#include "handle/AosHandleMtc.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosHandleMtc::AosHandleMtc
    (
        IN IAosAppContext* piAppContext,
        IN CONST AString& strAppId,
        IN CONST AString& strServiceId,
        IN CONST IMS_SINT32 nServiceType
    )
    : AosHandle(piAppContext, strAppId, strServiceId, nServiceType)
    , m_nVops(IMS_VOICE_OVER_PS_SUPPORTED)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleMtc = %" PFLS_u "/%" PFLS_x,
            strAppId.GetStr(), sizeof(AosHandleMtc), this);

    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
            static_cast<IMS_UINT32>(AosCapability::VIDEO));
}

/*

Remarks

*/
PUBLIC VIRTUAL
AosHandleMtc::~AosHandleMtc()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleMtc = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleMtc), this);
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandleMtc::App_Notify()
{
    AosHandle::App_Notify();
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandleMtc::CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    if (nType != IAosCallTracker::TYPE_NORMAL)
    {
        return;
    }

    if (nState == IAosCallTracker::STATE_IDLE)
    {
        if (m_nHoldingVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
        {
            A_IMS_TRACE_D(ServiceTypeToString(),
                    "CallTracker_StateChanged :: handle vops block , state (%d)",
                    m_nHoldingVopsState, 0, 0);

            ProcessBlock(BLOCK_VOPS, IMS_TRUE);
            m_nHoldingVopsState = IMS_VOICE_OVER_PS_SUPPORTED;
        }
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL
void AosHandleMtc::NetTracker_StatusChanged()
{
    IMS_SINT32 nCnxType = m_piAppContext->GetConnection()->GetConnectionType();

    if (nCnxType == NetworkPolicy::APN_WIFI)
    {
        A_IMS_TRACE_D(APPPROFILE,
                "NetTracker_StatusChanged :: nCnxType is WiFi, RAT change is ignored", 0, 0, 0);
        return;
    }

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();
    IMS_UINT32 nCurrNetworkType = piNetTracker->GetNetworkType();

    IMS_CHAR acLog[256+1] = {0, };
    IMS_Sprintf(acLog, 256,
            "m_bNetSrvIn(%s) -> bCurrSrvIn(%s) , m_nNetworkType(%x) -> nCurrNetworkType(%x) (%s)",
            (m_bNetSrvIn)? "IN SRV" : "NO SRV", (bCurrSrvIn)? "IN SRV" : "NO SRV",
            m_nNetworkType, nCurrNetworkType,
            (IsSupportedNetworkTypeForCellular(nCurrNetworkType)) ? "LTE/NR" : "NOT LTE/NR");

    A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: %s", acLog, 0, 0);

    AosHandle::NetTracker_StatusChanged();

    if ((bCurrSrvIn) && (nCurrNetworkType != m_nNetworkType))
    {
        // 3G to 4G/5G/WLAN
        if (IsSupportedNetworkType(nCurrNetworkType))
        {
            A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: LTE/NR Coverage", 0, 0, 0);
            ProcessImsResumed(AoSReason::SUSPEND_NO_LTE_COVERAGE);
        }
        // 4G/5G/WLAN to 3G, etc
        else
        {
            A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: Out Of LTE Coverage", 0, 0, 0);
            ProcessImsSuspended(AoSReason::SUSPEND_NO_LTE_COVERAGE);
        }

        ProcessNetworkChanged();

        m_nNetworkType = nCurrNetworkType;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::InitializeServiceBlock()
{
    // VOPS
    if (!GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled())
    {
        if (m_nVops == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
        {
            A_IMS_TRACE_D(APPPROFILE, "InitializeServiceBlock :: VoPS is not supported", 0, 0, 0);
            AddBlock(BLOCK_VOPS, m_nBlocks);
        }
    }

    m_bBlocked = IsHandleBlocked();

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceBlock :: block(%s)", _TRACE_B_(m_bBlocked), 0, 0);
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::InitializeServiceFeature()
{
    if (!IsHandleBlocked())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    }

    if (!AosHandle::IsHandleBlocked(BLOCK_VILTE_CAPABILITY))
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::VIDEO);
    }

    if (GET_N_CONFIG(m_nSlotId)->IsRttSupported())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::TEXT);
    }

    // jryou: TODO: USSI/VERSTAT check

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceFeature :: Features(%x)",
            m_objFeatureTagList.GetFeatures(), 0, 0);

    InitializeFeatureTags();
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::InitializeFeatureTags()
{
    if (GET_N_CONFIG(m_nSlotId)->IsUsedGGsmaRcsTelephonyFeatureTagToSpecifyAvailableVoiceCallType())
    {
        if (m_objFeatureTagList.HasFeature(ImsAosFeature::MMTEL))
        {
            m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
            m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "volte");
        }
    }

    m_objFeatureTagList.PrintFeatureTagList();
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::UpdateFeatureTags()
{
    AosHandle::UpdateFeatureTags();

    if (GET_N_CONFIG(m_nSlotId)->IsUsedGGsmaRcsTelephonyFeatureTagToSpecifyAvailableVoiceCallType())
    {
        if (m_objFeatureTagList.HasFeature(ImsAosFeature::MMTEL))
        {
            if (IsEpdgEnabled() && m_objFeatureTagList.HasFeature(ImsAosFeature::VIDEO) &&
                    GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
            {
                m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
            }

            m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
            m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "volte");
        }
        else
        {
            m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "cs");
            m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "volte");

            if (IsEpdgEnabled() && m_objFeatureTagList.HasFeature(ImsAosFeature::VIDEO) &&
                    GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
            {
                m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
            }
        }
    }

    m_objFeatureTagList.PrintFeatureTagList();
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::ProcessImsSuspended(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsImsConnected() == IMS_FALSE)
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsSuspended :: nReason(%d)", nReason, 0, 0);

    if (nReason == AoSReason::SUSPEND_NO_SERVICE)
    {
        SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    }
    else if (nReason == AoSReason::SUSPEND_NO_LTE_COVERAGE)
    {
        SetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    }
    else
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsSuspended :: Unknown Suspended Reason", 0, 0, 0);
    }

    if (IsImsSuspended() == IMS_TRUE)
    {
        m_nReason = nReason;
        m_piListener->ImsAos_Suspended(GetImsAosReasonForSuspend(m_nReason));
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::ProcessImsResumed(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsImsConnected() == IMS_FALSE)
    {
        return;
    }

    if (IsImsSuspended() == IMS_FALSE)
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: nReason(%d)", nReason, 0, 0);

    if (nReason == AoSReason::SUSPEND_NO_LTE_COVERAGE)
    {
        ResetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    }
    else if (nReason == AoSReason::SUSPEND_NO_SERVICE)
    {
        ResetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);

        IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
        IMS_UINT32 nCurrNetworkType = piNetTracker->GetNetworkType();

        if (IsSupportedNetworkType(nCurrNetworkType))
        {
            A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: Srv In, LTE or NR", 0, 0, 0);
            ResetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
        }
        else
        {
            A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: Srv In, But Not LTE", 0, 0, 0);
            SetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
        }

        m_nNetworkType = nCurrNetworkType;
    }
    else
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsResumed :: Unknown Resumed Reason", 0, 0, 0);
    }

    if (IsImsSuspended() == IMS_FALSE)
    {
        m_nReason = nReason;
        m_piListener->ImsAos_Resumed();
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::CheckSuspended()
{
    IMS_SINT32 nCnxType = m_piAppContext->GetConnection()->GetConnectionType();

    if (nCnxType == NetworkPolicy::APN_WIFI)
    {
        A_IMS_TRACE_D(APPPROFILE, "CheckSuspended :: nCnxType is WiFi, RAT check is skipped",
                0, 0, 0);
        return;
    }

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();

    A_IMS_TRACE_I(APPPROFILE, "CheckSuspended :: service (%s) >> (%s)",
            (m_bNetSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE",
            (bCurrSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE", 0);

    if (bCurrSrvIn == IMS_FALSE)
    {
        SetSuspendedReason(AoSReason::SUSPEND_NO_SERVICE);
    }

    IMS_UINT32 nCurrNetworkType = piNetTracker->GetNetworkType();

    if (!IsSupportedNetworkType(nCurrNetworkType))
    {
        A_IMS_TRACE_I(APPPROFILE, "IMS_CONNECTED, but Network is not in LTE or NR", 0, 0, 0);
        SetSuspendedReason(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    }

    m_bNetSrvIn = bCurrSrvIn;
    m_nNetworkType = nCurrNetworkType;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::SetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AoSReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason |= AoSReason::SUSPEND_NO_SERVICE;
    }
    else if (nReason == AoSReason::SUSPEND_NO_LTE_COVERAGE)
    {
        m_nSuspendedReason |= AoSReason::SUSPEND_NO_LTE_COVERAGE;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::ResetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AoSReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason &= ~(AoSReason::SUSPEND_NO_SERVICE);
    }
    else if (nReason == AoSReason::SUSPEND_NO_LTE_COVERAGE)
    {
        m_nSuspendedReason &= ~(AoSReason::SUSPEND_NO_LTE_COVERAGE);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    AosHandle::Init();

    IAosNConfiguration* piConfig = GET_N_CONFIG(m_nSlotId);

    if (!piConfig->IsVopsIgnoredForVolteEnabled())
    {
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, this, m_nSlotId);

        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCallTracker != IMS_NULL)
        {
            piCallTracker->SetListener(this);
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    AosHandle::CleanUp();

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, this, m_nSlotId);

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL)
    {
        piCallTracker->RemoveListener(this);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandleMtc::IsHandleBlocked() const
{
    if (IsEpdgEnabled())
    {
        IMS_BOOL bBlocked = AosHandle::IsHandleBlocked(BLOCK_VOWIFI_CAPABILITY);

        if (GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
        {
            bBlocked = bBlocked && AosHandle::IsHandleBlocked(BLOCK_VIWIFI_CAPABILITY);
        }

        return bBlocked;
    }

    return AosHandle::IsHandleBlocked(BLOCK_VOPS | BLOCK_VOLTE_CAPABILITY | BLOCK_NETWORK);
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandleMtc::IsBlockForMobile(IN IMS_UINT32 nBlock) const
{
    switch (nBlock)
    {
        case BLOCK_VOWIFI_CAPABILITY: // FALL-THROUGH
        case BLOCK_VIWIFI_CAPABILITY:
            return IMS_FALSE;

        default:
            break;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
IMS_BOOL AosHandleMtc::IsBlockForWifi(IN IMS_UINT32 nBlock) const
{
    switch (nBlock)
    {
        case BLOCK_VOLTE_CAPABILITY: // FALL-THROUGH
        case BLOCK_VILTE_CAPABILITY: // FALL-THROUGH
        case BLOCK_VOPS: // FALL-THROUGH
        case BLOCK_NETWORK:
            return IMS_FALSE;

        default:
            break;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::ProcessCapabilitiesChanged(
        IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
{
    A_IMS_TRACE_I(APPPROFILE, "ProcessCapabilitiesChanged :: Size[%d]",
            objNewCapabilities.GetSize(), 0, 0);

    IMS_UINT32 nCurrentRat = GetNetworkType();

    for (IMS_UINT32 i = 0; i < m_objCapabilities.GetSize(); i++)
    {
        IMS_UINT32 nNetworkType = m_objCapabilities.GetKeyAt(i);

        // Network type is not existed in the new capabilities (=no capabilities)
        if (objNewCapabilities.GetIndexOfKey(nNetworkType) < 0)
        {
            if (IsNetworkTypeMatchedToRat(nNetworkType, nCurrentRat))
            {
                ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ?
                        BLOCK_VOWIFI_CAPABILITY : BLOCK_VOLTE_CAPABILITY, IMS_TRUE);
                ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ?
                        BLOCK_VIWIFI_CAPABILITY : BLOCK_VILTE_CAPABILITY, IMS_TRUE);
            }

            m_objCapabilities.SetValue(nNetworkType, 0);
            continue;
        }

        IMS_UINT32 nNewCapabilities = objNewCapabilities.GetValue(nNetworkType);

        A_IMS_TRACE_D(APPPROFILE, "ProcessCapabilitiesChanged :: \
                nNetworkType[%d], nNewCapabilities[%d], nCurrentRat[%d]",
                nNetworkType, nNewCapabilities, nCurrentRat);

        if (IsNetworkTypeMatchedToRat(nNetworkType, nCurrentRat))
        {
            ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ?
                    BLOCK_VOWIFI_CAPABILITY : BLOCK_VOLTE_CAPABILITY,
                    !IsCapabilityExisted(nNewCapabilities, AosCapability::VOICE));
            ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ?
                    BLOCK_VIWIFI_CAPABILITY : BLOCK_VILTE_CAPABILITY,
                    !IsCapabilityExisted(nNewCapabilities, AosCapability::VIDEO));
        }

        m_objCapabilities.SetValue(nNetworkType, nNewCapabilities);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::ProcessNetworkChanged()
{
    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_UINT32 nNewNetwork = piNetTracker->GetNetworkType();
    IMS_UINT32 nCapabilities = static_cast<IMS_UINT32>(AosCapability::NONE);

    switch (nNewNetwork)
    {
        case NW_REPORT_RADIO_LTE:
            nCapabilities = m_objCapabilities.GetValue(
                    static_cast<IMS_UINT32>(AosNetworkType::LTE));
            break;

        case NW_REPORT_RADIO_NR:
            nCapabilities = m_objCapabilities.GetValue(
                    static_cast<IMS_UINT32>(AosNetworkType::NR));
            break;

        case NW_REPORT_RADIO_WLAN:
            nCapabilities = m_objCapabilities.GetValue(
                    static_cast<IMS_UINT32>(AosNetworkType::IWLAN));
            break;

        default:
            return;
    }

    ProcessBlock((nNewNetwork == NW_REPORT_RADIO_WLAN) ?
            BLOCK_VOWIFI_CAPABILITY : BLOCK_VOLTE_CAPABILITY,
            !IsCapabilityExisted(nCapabilities, AosCapability::VOICE));

    ProcessBlock((nNewNetwork == NW_REPORT_RADIO_WLAN) ?
            BLOCK_VIWIFI_CAPABILITY : BLOCK_VILTE_CAPABILITY,
            !IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
}

/*

Remarks

*/
PROTECTED VIRTUAL
void AosHandleMtc::ProcessVopsStateChanged(IN IMS_UINT32 nState)
{
    if (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);

        if (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive())
        {
            A_IMS_TRACE_I(ServiceTypeToString(),
                    "ProcessVopsStateChanged :: pending the block feature, state (%d)",
                    nState, 0, 0);
            m_nHoldingVopsState = nState;
            return;
        }
    }
    else // IMS_VOICE_OVER_PS_SUPPORTED
    {
        if (m_nHoldingVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
        {
            m_nHoldingVopsState = nState;
            return;
        }
    }

    ProcessBlock(BLOCK_VOPS, (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED));
}