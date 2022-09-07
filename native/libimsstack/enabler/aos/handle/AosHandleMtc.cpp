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
#include "ServiceNetworkPolicy.h"
#include "CarrierConfig.h"
#include "INetworkWatcher.h"

#include "AosReason.h"

#include "IImsAosListener.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosService.h"

#include "provider/AosProvider.h"
#include "provider/AosUtil.h"
#include "handle/AosHandleMtc.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

/*

Remarks

*/
PUBLIC
AosHandleMtc::AosHandleMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
        IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
        AosHandle(piAppContext, strAppId, strServiceId, nServiceType)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleMtc = %" PFLS_u "/%" PFLS_x, strAppId.GetStr(),
            sizeof(AosHandleMtc), this);

    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));

    m_objServiceFeatures.Append(ImsAosFeature::MMTEL);
    m_objServiceFeatures.Append(ImsAosFeature::VIDEO);
    m_objServiceFeatures.Append(ImsAosFeature::TEXT);
    m_objServiceFeatures.Append(ImsAosFeature::USSI);
    m_objServiceFeatures.Append(ImsAosFeature::VERSTAT);

    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_VOLTE_CAPABILITY);
    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_VILTE_CAPABILITY);
    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_VOPS);

    m_objHoldingBlocksPolicyForWifi.Append(BLOCK_VOWIFI_CAPABILITY);
    m_objHoldingBlocksPolicyForWifi.Append(BLOCK_VIWIFI_CAPABILITY);
}

/*

Remarks

*/
PUBLIC VIRTUAL AosHandleMtc::~AosHandleMtc()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleMtc = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleMtc), this);
}

/*

Remarks

*/
PUBLIC VIRTUAL IMS_BOOL AosHandleMtc::App_Notify()
{
    return AosHandle::App_Notify();
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosHandleMtc::CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState)
{
    if (nType != IAosCallTracker::TYPE_NORMAL)
    {
        return;
    }

    if (eState == CallState::IDLE)
    {
        if (!GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled())
        {
            if (m_nHoldingVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
            {
                A_IMS_TRACE_D(APPPROFILE,
                        "CallTracker_StateChanged :: handle vops block , state (%d)",
                        m_nHoldingVopsState, 0, 0);

                if (!IsUnavailableFeaturePolicy(
                            CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS) ||
                        !ProcessUnavailableFeatureForVops(m_nHoldingVopsState))
                {
                    ProcessBlock(BLOCK_VOPS, IMS_TRUE);
                }

                m_nHoldingVopsState = IMS_VOICE_OVER_PS_SUPPORTED;
            }
        }
    }

    if (eState == CallState::IDLE || eState == CallState::OFFHOOK)
    {
        // VZW Req. - VZ_REQ_VOWIFI_6258874, VZ_REQ_VOWIFI_6258951
        UpdateFeatureTags();
        ProcessFeatureTagChange();
    }
}

/*

Remarks

*/
PUBLIC VIRTUAL void AosHandleMtc::NetTracker_StatusChanged()
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();
    IMS_UINT32 nCurrNetworkType = GetNetworkType();

    IMS_CHAR acLog[256 + 1] = {
            0,
    };
    IMS_Sprintf(acLog, 256,
            "m_bNetSrvIn(%s) -> bCurrSrvIn(%s) , m_nNetworkType(%x) -> nCurrNetworkType(%x) (%s)",
            (m_bNetSrvIn) ? "IN SRV" : "NO SRV", (bCurrSrvIn) ? "IN SRV" : "NO SRV", m_nNetworkType,
            nCurrNetworkType,
            (IsSupportedNetworkTypeForCellular(nCurrNetworkType)) ? "LTE/NR" : "NOT LTE/NR");

    A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: %s", acLog, 0, 0);

    AosHandle::NetTracker_StatusChanged();

    if ((bCurrSrvIn) && (nCurrNetworkType != m_nNetworkType))
    {
        // 3G to 4G/5G/WLAN
        if (IsSupportedNetworkType(nCurrNetworkType))
        {
            A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: LTE/NR/WLAN Coverage", 0, 0, 0);
            ProcessImsResumed(AosReason::SUSPEND_NO_LTE_COVERAGE);
        }
        // 4G/5G/WLAN to 3G, etc
        else
        {
            A_IMS_TRACE_I(APPPROFILE, "NetTracker_StatusChanged :: Out Of LTE Coverage", 0, 0, 0);
            ProcessImsSuspended(AosReason::SUSPEND_NO_LTE_COVERAGE);
        }

        ProcessNetworkChanged();

        m_nNetworkType = nCurrNetworkType;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::InitializeServiceBlock()
{
    m_bBlocked = IsHandleBlocked();

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceBlock :: block(%s)", _TRACE_B_(m_bBlocked), 0, 0);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::InitializeServiceFeature()
{
    IAosNConfiguration* objConfig = GET_N_CONFIG(m_nSlotId);

    m_objFeatureTagList.Clear();

    if (!IsHandleBlocked())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::MMTEL);
    }

    if (!AosHandle::IsHandleBlocked(BLOCK_VILTE_CAPABILITY))
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::VIDEO);
    }

    if (objConfig->IsRttSupported())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::TEXT);
    }

    if (objConfig->IsVerstatForRegistrationSupported())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::VERSTAT);
    }

    if (objConfig->GetUssdMethod() != CarrierConfig::USSD_OVER_CS_ONLY)
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::USSI);
    }

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceFeature :: Features(%x)",
            m_objFeatureTagList.GetFeatures(), 0, 0);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::InitializeFeatureTags()
{
    AosHandle::InitializeFeatureTags();

    UpdateFeatureTags();
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::UpdateFeatureTags()
{
    AosHandle::UpdateFeatureTags();

    /* VZW Req. - VZ_REQ_IMS_22939, VZ_REQ_VOWIFI_6230394
                  VZ_REQ_VOWIFI_6258874, VZ_REQ_VOWIFI_6258951
    */
    if (GET_N_CONFIG(m_nSlotId)->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive())
        {
            if (m_objBindedFeatureTagList.HasFeatureTag("+g.gsma.rcs.telephony", "\"cs\""))
            {
                m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
                m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
                m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "volte");
            }
        }
        else
        {
            if (m_objFeatureTagList.HasFeature(ImsAosFeature::MMTEL))
            {
                m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
                m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "cs");
                m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "volte");
            }
            else
            {
                m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "cs");
                m_objFeatureTagList.RemoveFeatureTag("+g.gsma.rcs.telephony", "volte");

                if (GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
                {
                    if (m_objFeatureTagList.HasFeature(ImsAosFeature::VIDEO) && IsEpdgEnabled() &&
                            !IsSupportedNetworkTypeForCellular(GetMobileNetworkType()))
                    {
                        m_objFeatureTagList.AddFeatureTag("+g.gsma.rcs.telephony", "\"cs\"");
                    }
                }
            }
        }
    }

    m_objFeatureTagList.PrintFeatureTagList();
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL AosHandleMtc::ProcessImsSuspended(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsImsConnected() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsSuspended :: nReason(%d)", nReason, 0, 0);

    if (nReason == AosReason::SUSPEND_NO_SERVICE)
    {
        SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    }
    else if (nReason == AosReason::SUSPEND_NO_LTE_COVERAGE)
    {
        SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    }
    else
    {
        A_IMS_TRACE_D(APPPROFILE, "ProcessImsSuspended :: Unknown Suspended Reason", 0, 0, 0);
    }

    if (IsImsSuspended() == IMS_TRUE)
    {
        m_nReason = nReason;

        if (m_piListener != IMS_NULL)
        {
            m_piListener->ImsAos_Suspended(GetImsAosReasonForSuspend(m_nReason));

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL AosHandleMtc::ProcessImsResumed(IN IMS_UINT32 nReason /* = 0 */)
{
    if (IsImsConnected() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    if (IsImsSuspended() == IMS_FALSE)
    {
        return IMS_FALSE;
    }

    A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: nReason(%d)", nReason, 0, 0);

    if (nReason == AosReason::SUSPEND_NO_LTE_COVERAGE)
    {
        ResetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    }
    else if (nReason == AosReason::SUSPEND_NO_SERVICE)
    {
        ResetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);

        IMS_UINT32 nCurrNetworkType = GetNetworkType();

        if (IsSupportedNetworkType(nCurrNetworkType))
        {
            A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: Srv In, LTE or NR", 0, 0, 0);
            ResetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
        }
        else
        {
            A_IMS_TRACE_I(APPPROFILE, "ProcessImsResumed :: Srv In, But Not LTE", 0, 0, 0);
            SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
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

        if (m_piListener != IMS_NULL)
        {
            m_piListener->ImsAos_Resumed();

            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::CheckSuspended()
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    IAosNetTracker* piNetTracker = m_piAppContext->GetNetTracker();
    IMS_BOOL bCurrSrvIn = !piNetTracker->IsSuspended();

    A_IMS_TRACE_I(APPPROFILE, "CheckSuspended :: service (%s) >> (%s)",
            (m_bNetSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE",
            (bCurrSrvIn) ? "IN_SERVICE" : "OUT_OF_SERVICE", 0);

    if (bCurrSrvIn == IMS_FALSE)
    {
        SetSuspendedReason(AosReason::SUSPEND_NO_SERVICE);
    }

    IMS_UINT32 nCurrNetworkType = GetNetworkType();

    if (!IsSupportedNetworkType(nCurrNetworkType))
    {
        A_IMS_TRACE_I(APPPROFILE, "IMS_CONNECTED, but Network is not in LTE or NR", 0, 0, 0);
        SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    }

    m_bNetSrvIn = bCurrSrvIn;
    m_nNetworkType = nCurrNetworkType;
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::SetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AosReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason |= AosReason::SUSPEND_NO_SERVICE;
    }
    else if (nReason == AosReason::SUSPEND_NO_LTE_COVERAGE)
    {
        m_nSuspendedReason |= AosReason::SUSPEND_NO_LTE_COVERAGE;
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ResetSuspendedReason(IN IMS_UINT32 nReason)
{
    if (nReason == AosReason::SUSPEND_NO_SERVICE)
    {
        m_nSuspendedReason &= ~(AosReason::SUSPEND_NO_SERVICE);
    }
    else if (nReason == AosReason::SUSPEND_NO_LTE_COVERAGE)
    {
        m_nSuspendedReason &= ~(AosReason::SUSPEND_NO_LTE_COVERAGE);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    AosHandle::Init();

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, this, m_nSlotId);

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL)
    {
        piCallTracker->SetListener(this);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::CleanUp()
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
PROTECTED VIRTUAL IMS_BOOL AosHandleMtc::IsHandleBlocked() const
{
    if (IsEpdgEnabled())
    {
        IMS_BOOL bBlocked = AosHandle::IsHandleBlocked(BLOCK_VOWIFI_CAPABILITY);

        if (GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
        {
            // VZW Reqs. - VZ_REQ_VOWIFI_6230394
            bBlocked = bBlocked &&
                    (AosHandle::IsHandleBlocked(BLOCK_VIWIFI_CAPABILITY) ||
                            IsSupportedNetworkTypeForCellular(GetMobileNetworkType()));
        }

        return bBlocked;
    }

    IMS_UINT32 nBlocks = (BLOCK_VOPS | BLOCK_VOLTE_CAPABILITY);

    if (GET_N_CONFIG(m_nSlotId)->GetRegWithFeatureTagUnavailable().GetSize() > 0)
    {
        if (IsUnavailableFeaturePolicy(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS))
        {
            nBlocks &= ~(BLOCK_VOPS);
        }

        if (IsUnavailableFeaturePolicy(
                    CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY))
        {
            nBlocks &= ~(BLOCK_VOLTE_CAPABILITY);
        }
    }

    return AosHandle::IsHandleBlocked(nBlocks);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ProcessBlockChanged()
{
    if (!GET_N_CONFIG(m_nSlotId)->IsSmsOverImsAvailableWithoutVoiceCapability())
    {
        IAosHandle* piHandleMts = m_piAppContext->GetHandle(ImsAosService::MTS);

        if (piHandleMts != IMS_NULL)
        {
            piHandleMts->Handle_Notify(ImsAosService::MTC, m_bBlocked);
        }
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ProcessCapabilitiesChanged(
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
                if (IsUnavailableFeaturePolicy(
                            CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY))
                {
                    IMS_BOOL bProceeded = IMS_FALSE;

                    if (IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL))
                    {
                        ProcessUnavailableFeature(ImsAosFeature::MMTEL, IMS_TRUE);
                        bProceeded = IMS_TRUE;
                    }
                    else
                    {
                        ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                                           : BLOCK_VOLTE_CAPABILITY,
                                IMS_TRUE);
                    }

                    if (IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO))
                    {
                        ProcessUnavailableFeature(ImsAosFeature::VIDEO, IMS_TRUE);
                        bProceeded = IMS_TRUE;
                    }
                    else
                    {
                        ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                                           : BLOCK_VILTE_CAPABILITY,
                                IMS_TRUE);
                    }

                    if (bProceeded)
                    {
                        ProcessUnavailableFeatureChanged();
                    }
                }
                else
                {
                    ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                                       : BLOCK_VOLTE_CAPABILITY,
                            IMS_TRUE);

                    ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                                       : BLOCK_VILTE_CAPABILITY,
                            IMS_TRUE);
                }
            }

            m_objCapabilities.SetValue(nNetworkType, static_cast<IMS_UINT32>(AosCapability::NONE));
            continue;
        }

        IMS_UINT32 nNewCapabilities = objNewCapabilities.GetValue(nNetworkType);

        A_IMS_TRACE_D(APPPROFILE, "ProcessCapabilitiesChanged :: \
                nNetworkType[%d], nNewCapabilities[%d], nCurrentRat[%s]",
                nNetworkType, nNewCapabilities, RadioTypeToString(nCurrentRat));

        if (IsNetworkTypeMatchedToRat(nNetworkType, nCurrentRat))
        {
            if (IsUnavailableFeaturePolicy(
                        CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY))
            {
                IMS_BOOL bProceeded = IMS_FALSE;

                if (IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL))
                {
                    ProcessUnavailableFeature(ImsAosFeature::MMTEL,
                            !IsCapabilityExisted(nNewCapabilities, AosCapability::VOICE));
                    bProceeded = IMS_TRUE;
                }
                else
                {
                    ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                                       : BLOCK_VOLTE_CAPABILITY,
                            !IsCapabilityExisted(nNewCapabilities, AosCapability::VOICE));
                }

                if (IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO))
                {
                    ProcessUnavailableFeature(ImsAosFeature::VIDEO,
                            !IsCapabilityExisted(nNewCapabilities, AosCapability::VIDEO));
                    bProceeded = IMS_TRUE;
                }
                else
                {
                    ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                                       : BLOCK_VILTE_CAPABILITY,
                            !IsCapabilityExisted(nNewCapabilities, AosCapability::VIDEO));
                }

                if (bProceeded)
                {
                    ProcessUnavailableFeatureChanged();
                }
            }
            else
            {
                ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                                   : BLOCK_VOLTE_CAPABILITY,
                        !IsCapabilityExisted(nNewCapabilities, AosCapability::VOICE));

                ProcessBlock((nCurrentRat == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                                   : BLOCK_VILTE_CAPABILITY,
                        !IsCapabilityExisted(nNewCapabilities, AosCapability::VIDEO));
            }
        }

        m_objCapabilities.SetValue(nNetworkType, nNewCapabilities);
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ProcessNetworkChanged()
{
    IMS_UINT32 nNewNetwork = GetNetworkType();
    IMS_UINT32 nCapabilities = static_cast<IMS_UINT32>(AosCapability::NONE);

    switch (nNewNetwork)
    {
        case NW_REPORT_RADIO_LTE:
            nCapabilities =
                    m_objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::LTE));
            break;

        case NW_REPORT_RADIO_NR:
            nCapabilities = m_objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::NR));
            break;

        case NW_REPORT_RADIO_WLAN:
            nCapabilities =
                    m_objCapabilities.GetValue(static_cast<IMS_UINT32>(AosNetworkType::IWLAN));
            break;

        default:
            return;
    }

    if (IsUnavailableFeaturePolicy(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_CAPABILITY))
    {
        IMS_BOOL bProceeded = IMS_FALSE;

        if (IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_MMTEL))
        {
            ProcessUnavailableFeature(ImsAosFeature::MMTEL,
                    !IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
            bProceeded = IMS_TRUE;
        }
        else
        {
            ProcessBlock((nNewNetwork == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                               : BLOCK_VOLTE_CAPABILITY,
                    !IsCapabilityExisted(nCapabilities, AosCapability::VOICE));
        }

        if (IsUnavailableFeature(CarrierConfig::Assets::UNAVAILABLE_FEATURE_TYPE_VIDEO))
        {
            ProcessUnavailableFeature(ImsAosFeature::VIDEO,
                    !IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
            bProceeded = IMS_TRUE;
        }
        else
        {
            ProcessBlock((nNewNetwork == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                               : BLOCK_VILTE_CAPABILITY,
                    !IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
        }

        if (bProceeded)
        {
            ProcessUnavailableFeatureChanged();
        }
    }
    else
    {
        ProcessBlock((nNewNetwork == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                           : BLOCK_VOLTE_CAPABILITY,
                !IsCapabilityExisted(nCapabilities, AosCapability::VOICE));

        ProcessBlock((nNewNetwork == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                           : BLOCK_VILTE_CAPABILITY,
                !IsCapabilityExisted(nCapabilities, AosCapability::VIDEO));
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ProcessVopsStateChanged(IN IMS_UINT32 nState)
{
    if (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);

        if (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive())
        {
            A_IMS_TRACE_I(APPPROFILE,
                    "ProcessVopsStateChanged :: pending the block feature, state (%d)", nState, 0,
                    0);
            m_nHoldingVopsState = nState;
            return;
        }
    }
    else  // IMS_VOICE_OVER_PS_SUPPORTED
    {
        if (m_nHoldingVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
        {
            m_nHoldingVopsState = nState;
            return;
        }
    }

    if (IsUnavailableFeaturePolicy(CarrierConfig::Assets::UNAVAILABLE_FEATURE_POLICY_VOPS))
    {
        if (ProcessUnavailableFeatureForVops(nState))
        {
            return;
        }
    }

    ProcessBlock(BLOCK_VOPS, (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED));
}

/*

Remarks

*/
PROTECTED VIRTUAL IMS_BOOL AosHandleMtc::ProcessUnavailableFeatureForVops(IN IMS_UINT32 nState)
{
    IMS_BOOL bProceeded = IMS_FALSE;

    IMSVector<IMS_SINT32>& objConfigFeatures =
            GET_N_CONFIG(m_nSlotId)->GetRegWithFeatureTagUnavailable();

    for (IMS_UINT32 i = 0; i < objConfigFeatures.GetSize(); i++)
    {
        IMS_UINT32 nAosFeature = ConvertToAosFeature(objConfigFeatures.GetAt(i));
        if (IsServiceFeature(nAosFeature))
        {
            ProcessUnavailableFeature(nAosFeature, (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED));
            bProceeded = IMS_TRUE;
        }
    }

    if (bProceeded)
    {
        ProcessUnavailableFeatureChanged();
    }

    return bProceeded;
}