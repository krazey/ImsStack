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
                        "CallTracker_StateChanged :: handle vops block, state(%d)",
                        m_nHoldingVopsState, 0, 0);

                m_nVopsState = m_nHoldingVopsState;
                m_nHoldingVopsState = IMS_VOICE_OVER_PS_SUPPORTED;

                if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
                {
                    ReevaluateUnavailableFeature();
                }
                else
                {
                    ProcessBlock(BLOCK_VOPS, IMS_TRUE);
                }
            }
        }
    }

    if (GET_N_CONFIG(m_nSlotId)->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
    {
        if (eState == CallState::IDLE || eState == CallState::OFFHOOK)
        {
            UpdateGGsmaRcsTelephonyFeatureTag();
            ProcessFeatureTagChange();
        }
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
            "m_bNetSrvIn(%s) -> bCurrSrvIn(%s) , m_nNetworkType(%s) -> nCurrNetworkType(%s)",
            (m_bNetSrvIn) ? "IN SRV" : "NO SRV", (bCurrSrvIn) ? "IN SRV" : "NO SRV",
            RadioTypeToString(m_nNetworkType), RadioTypeToString(nCurrNetworkType));

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

        m_nNetworkType = nCurrNetworkType;

        ProcessNetworkChanged();
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

    if (GET_N_CONFIG(m_nSlotId)->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
    {
        UpdateGGsmaRcsTelephonyFeatureTag();
    }
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

    if (!GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled())
    {
        IMS_EVENT_AddListenerForSlotId(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, this, m_nSlotId);
    }

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

    return AosHandle::IsHandleBlocked(BLOCK_VOLTE_CAPABILITY | BLOCK_VOPS | BLOCK_NETWORK);
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ProcessFeatureBlock(
        IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked)
{
    AosHandle::ProcessFeatureBlock(nFeature, bBlocked);

    if (GET_N_CONFIG(m_nSlotId)->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
    {
        UpdateGGsmaRcsTelephonyFeatureTag();
    }
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

    for (IMS_UINT32 i = 0; i < m_objCapabilities.GetSize(); i++)
    {
        IMS_UINT32 nCapaNetworkType = m_objCapabilities.GetKeyAt(i);

        // Network type is not existed in the new capabilities (=no capabilities)
        if (objNewCapabilities.GetIndexOfKey(nCapaNetworkType) < 0)
        {
            m_objCapabilities.SetValue(
                    nCapaNetworkType, static_cast<IMS_UINT32>(AosCapability::NONE));
            continue;
        }

        IMS_UINT32 nNewCapabilities = objNewCapabilities.GetValue(nCapaNetworkType);

        A_IMS_TRACE_D(APPPROFILE, "ProcessCapabilitiesChanged :: \
                nCapaNetworkType[%d], nNewCapabilities[%x], m_nNetworkType[%s]",
                nCapaNetworkType, nNewCapabilities, RadioTypeToString(m_nNetworkType));

        m_objCapabilities.SetValue(nCapaNetworkType, nNewCapabilities);
    }

    if (IsSupportedNetworkType(m_nNetworkType))
    {
        ProcessBlock(GetVoiceBlockReasonForIpcan(),
                !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VOICE));

        ProcessBlock(GetVideoBlockReasonForIpcan(),
                !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VIDEO));
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ProcessNetworkChanged()
{
    if (IsSupportedNetworkType(m_nNetworkType))
    {
        if (AosHandle::IsHandleBlocked(BLOCK_NETWORK))
        {
            ProcessBlock(BLOCK_NETWORK, IMS_FALSE);
        }

        IMS_BOOL bIsVoiceCapable =
                IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VOICE);
        IMS_BOOL bIsVideoCapable =
                IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VIDEO);

        if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
        {
            ReevaluateUnavailableFeature();
            ProcessBlock(GetVideoBlockReasonForIpcan(), !bIsVideoCapable);
        }
        else
        {
            ProcessBlock(GetVoiceBlockReasonForIpcan(), !bIsVoiceCapable);
            ProcessBlock(GetVideoBlockReasonForIpcan(), !bIsVideoCapable);
        }
    }
    else if (Is3G(m_nNetworkType))
    {
        if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
        {
            ReevaluateUnavailableFeature();
        }
        else
        {
            ProcessBlock(BLOCK_NETWORK, IMS_TRUE);
        }
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
                    "ProcessVopsStateChanged :: pending the block feature, state(%d)", nState, 0,
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

    m_nVopsState = nState;

    if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
    {
        ReevaluateUnavailableFeature();
    }
    else
    {
        ProcessBlock(BLOCK_VOPS, (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED));
    }
}

/*

Remarks

*/
PROTECTED VIRTUAL void AosHandleMtc::ReevaluateUnavailableFeature()
{
    IMS_BOOL bIsVoiceUnavailable = IMS_FALSE;
    IMS_BOOL bIsVideoUnavailable = IMS_FALSE;
    IMS_UINT32 nOldUnavailableFeature = m_objFeatureTagList.GetUnavailableFeatures();

    if (IsSupportedNetworkTypeForCellular(m_nNetworkType))
    {
        if (!GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled())
        {
            bIsVoiceUnavailable = (m_nVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED);
        }
    }
    else if (Is3G(m_nNetworkType))
    {
        bIsVoiceUnavailable = IMS_TRUE;
    }
    else
    {
        return;
    }

    bIsVideoUnavailable =
            bIsVoiceUnavailable && !AosHandle::IsHandleBlocked(GetVideoBlockReasonForIpcan());

    A_IMS_TRACE_D(APPPROFILE,
            "ReevaluateUnavailableFeature :: bIsVoiceUnavailable(%s), bIsVideoUnavailable(%s)",
            _TRACE_B_(bIsVoiceUnavailable), _TRACE_B_(bIsVideoUnavailable), 0);

    ProcessUnavailableFeature(ImsAosFeature::MMTEL, bIsVoiceUnavailable);
    ProcessUnavailableFeature(ImsAosFeature::VIDEO, bIsVideoUnavailable);

    if (nOldUnavailableFeature != m_objFeatureTagList.GetUnavailableFeatures())
    {
        ProcessUnavailableFeatureChanged();
    }
}

/*

Remarks

*/
PRIVATE
void AosHandleMtc::UpdateGGsmaRcsTelephonyFeatureTag()
{
    /* VZW Req. - VZ_REQ_IMS_22939, VZ_REQ_VOWIFI_6230394
                  VZ_REQ_VOWIFI_6258874, VZ_REQ_VOWIFI_6258951
    */

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

    m_objFeatureTagList.PrintFeatureTagList();
}

PRIVATE
IMS_UINT32 AosHandleMtc::GetVoiceBlockReasonForIpcan()
{
    return (m_nNetworkType == NW_REPORT_RADIO_WLAN) ? BLOCK_VOWIFI_CAPABILITY
                                                    : BLOCK_VOLTE_CAPABILITY;
}

PRIVATE
IMS_UINT32 AosHandleMtc::GetVideoBlockReasonForIpcan()
{
    return (m_nNetworkType == NW_REPORT_RADIO_WLAN) ? BLOCK_VIWIFI_CAPABILITY
                                                    : BLOCK_VILTE_CAPABILITY;
}