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

#include "CarrierConfig.h"
#include "IAosService.h"
#include "INetworkWatcher.h"
#include "ServiceEvent.h"
#include "ServiceImsRadio.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

#include "AosReason.h"

#include "IImsAosListener.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosApplication.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosNConfiguration.h"
#include "interface/IAosNetTracker.h"

#include "provider/AosProvider.h"
#include "provider/AosString.h"
#include "provider/AosUtil.h"

#include "handle/AosHandleMtc.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosHandleMtc::AosHandleMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
        IN const AString& strServiceId, IN const IMS_SINT32 nServiceType) :
        AosHandle(piAppContext, strAppId, strServiceId, nServiceType),
        m_piImsRadio(IMS_NULL),
        m_piVolteHysTimer(IMS_NULL),
        m_bSsacBarred(IMS_FALSE),
        m_bSsacHeld(IMS_FALSE)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleMtc = %" PFLS_u "/%" PFLS_x, strAppId.GetStr(),
            sizeof(AosHandleMtc), this);

    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::LTE),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::IWLAN),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO));
    m_objCapabilities.Add(static_cast<IMS_UINT32>(AosNetworkType::NR),
            static_cast<IMS_UINT32>(AosCapability::VOICE) |
                    static_cast<IMS_UINT32>(AosCapability::VIDEO) |
                    static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER));
}

PUBLIC VIRTUAL AosHandleMtc::~AosHandleMtc()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleMtc = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleMtc), this);
}

PUBLIC VIRTUAL IMS_BOOL AosHandleMtc::App_Notify()
{
    return AosHandle::App_Notify();
}

PUBLIC VIRTUAL void AosHandleMtc::CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState)
{
    if (nType != IAosCallTracker::TYPE_NORMAL)
    {
        return;
    }

    if (eState == CallState::IDLE)
    {
        IMS_BOOL bIsHoldingVopsChanged = IMS_FALSE;
        IMS_BOOL bIsHoldingSsacChanged = IMS_FALSE;

        if (!m_bVopsIgnoredForVolteEnabled)
        {
            if (m_nHoldingVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
            {
                A_IMS_TRACE_D(APPPROFILE,
                        "CallTracker_StateChanged :: handle vops block, state(%d)",
                        m_nHoldingVopsState, 0, 0);

                m_nVopsState = m_nHoldingVopsState;
                m_nHoldingVopsState = IMS_VOICE_OVER_PS_SUPPORTED;
                bIsHoldingVopsChanged = IMS_TRUE;
            }
        }

        if (m_bSsacHeld)
        {
            A_IMS_TRACE_D(APPPROFILE,
                    "CallTracker_StateChanged :: handle ssac voice block, state(%s)",
                    _TRACE_B_(m_bSsacHeld), 0, 0);

            m_bSsacBarred = IMS_TRUE;
            m_bSsacHeld = IMS_FALSE;
            bIsHoldingSsacChanged = IMS_TRUE;
        }

        if (bIsHoldingVopsChanged || bIsHoldingSsacChanged)
        {
            if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
            {
                ReevaluateUnavailableFeature();
                return;
            }

            if (IsPlmnBlockCondition())
            {
                A_IMS_TRACE_I(APPPROFILE,
                        "CallTracker_StateChanged :: PLMN is blocked with timeout", 0, 0, 0);
                m_piAppContext->GetApp()->RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT);
            }

            if (bIsHoldingVopsChanged)
            {
                ProcessBlock(BLOCK_VOPS, IMS_TRUE);
            }

            if (bIsHoldingSsacChanged)
            {
                ProcessBlock(BLOCK_SSAC, IMS_TRUE);
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

PUBLIC VIRTUAL void AosHandleMtc::NetTracker_StatusChanged()
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    IMS_BOOL bCurrSrvIn = !m_piAppContext->GetNetTracker()->IsSuspended();
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

    if (nCurrNetworkType != m_nNetworkType)
    {
        if (bCurrSrvIn)
        {
            // 3G to 4G/5G/WLAN
            if (IsSupportedNetworkType(nCurrNetworkType))
            {
                A_IMS_TRACE_I(
                        APPPROFILE, "NetTracker_StatusChanged :: LTE/NR/WLAN Coverage", 0, 0, 0);
                ProcessImsResumed(AosReason::SUSPEND_NO_LTE_COVERAGE);
            }
            // 4G/5G/WLAN to 3G, etc
            else
            {
                A_IMS_TRACE_I(
                        APPPROFILE, "NetTracker_StatusChanged :: Out Of LTE Coverage", 0, 0, 0);
                ProcessImsSuspended(AosReason::SUSPEND_NO_LTE_COVERAGE);
            }
        }

        m_nNetworkType = nCurrNetworkType;

        ProcessNetworkChanged();
    }
}

PROTECTED VIRTUAL void AosHandleMtc::InitializeHoldingBlocksPolicy()
{
    AosHandle::InitializeHoldingBlocksPolicy();

    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_VOLTE_CAPABILITY);
    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_VILTE_CAPABILITY);
    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_VOPS);
    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_SSAC);
    m_objHoldingBlocksPolicyForMobile.Append(BLOCK_NETWORK);

    m_objHoldingBlocksPolicyForWifi.Append(BLOCK_VOWIFI_CAPABILITY);
    m_objHoldingBlocksPolicyForWifi.Append(BLOCK_VIWIFI_CAPABILITY);
}

PROTECTED VIRTUAL void AosHandleMtc::InitializeServiceBlock()
{
    m_bBlocked = IsHandleBlocked();

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceBlock :: block(%s)", _TRACE_B_(m_bBlocked), 0, 0);
}

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

    if (!AosHandle::IsHandleBlocked(BLOCK_CALL_COMPOSER_CAPABILITY))
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::CALL_COMPOSER_VIA_TELEPHONY);
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

PROTECTED VIRTUAL void AosHandleMtc::InitializeFeatureTags()
{
    AosHandle::InitializeFeatureTags();

    if (GET_N_CONFIG(m_nSlotId)->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
    {
        UpdateGGsmaRcsTelephonyFeatureTag();
    }
}

PROTECTED VIRTUAL void AosHandleMtc::CheckSuspended()
{
    if (AosUtil::GetInstance()->IsWifiTest())
    {
        return;
    }

    IMS_BOOL bCurrSrvIn = !m_piAppContext->GetNetTracker()->IsSuspended();

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
        A_IMS_TRACE_I(APPPROFILE, "IMS_CONNECTED, but Network is not LTE or NR. (%s)",
                RadioTypeToString(nCurrNetworkType), 0, 0);

        SetSuspendedReason(AosReason::SUSPEND_NO_LTE_COVERAGE);
    }

    m_bNetSrvIn = bCurrSrvIn;
    m_nNetworkType = nCurrNetworkType;
}

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

PROTECTED VIRTUAL void AosHandleMtc::Init()
{
    A_IMS_TRACE_D(APPPROFILE, "Init", 0, 0, 0);

    m_bVopsIgnoredForVolteEnabled = GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled();
    m_piImsRadio = ImsRadioService::GetImsRadioService()->GetImsRadio(m_nSlotId);

    AosHandle::Init();
}

PROTECTED VIRTUAL void AosHandleMtc::CleanUp()
{
    A_IMS_TRACE_D(APPPROFILE, "CleanUp", 0, 0, 0);

    AosHandle::CleanUp();
}

PROTECTED VIRTUAL void AosHandleMtc::AddListeners()
{
    AosHandle::AddListeners();

    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, this, m_nSlotId);
    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
    IMS_EVENT_AddListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_nSlotId);

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL)
    {
        piCallTracker->SetListener(this);
    }

    if (m_piImsRadio != IMS_NULL)
    {
        m_piImsRadio->AddListenerForSsac(this);
    }

    IAosService* piAosService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piAosService != IMS_NULL)
    {
        piAosService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }
}

PROTECTED VIRTUAL void AosHandleMtc::RemoveListeners()
{
    AosHandle::RemoveListeners();

    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_IMS_VOICE_OVER_PS_STATE, this, m_nSlotId);
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_ROAMING_STATE, this, m_nSlotId);
    IMS_EVENT_RemoveListenerForSlotId(IMS_EVENT_LTE_INFO, this, m_nSlotId);

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL)
    {
        piCallTracker->RemoveListener(this);
    }

    if (m_piImsRadio != IMS_NULL)
    {
        m_piImsRadio->RemoveListenerForSsac(this);
    }

    IAosService* piAosService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (piAosService != IMS_NULL)
    {
        piAosService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }
}

PROTECTED VIRTUAL IMS_BOOL AosHandleMtc::IsHandleBlocked() const
{
    if (IsEpdgEnabled())
    {
        IMS_BOOL bBlocked = AosHandle::IsHandleBlocked(BLOCK_VOWIFI_CAPABILITY);

        if (GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
        {
            // VZW Reqs. - VZ_REQ_VOWIFI_6230394
            bBlocked = bBlocked && AosHandle::IsHandleBlocked(BLOCK_VIWIFI_CAPABILITY);
        }

        return bBlocked;
    }

    return AosHandle::IsHandleBlocked(
            BLOCK_VOLTE_CAPABILITY | BLOCK_VOPS | BLOCK_SSAC | BLOCK_NETWORK | BLOCK_3G);
}

PROTECTED VIRTUAL void AosHandleMtc::ProcessFeatureBlock(
        IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked)
{
    AosHandle::ProcessFeatureBlock(nFeature, bBlocked);

    if (GET_N_CONFIG(m_nSlotId)->IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType())
    {
        UpdateGGsmaRcsTelephonyFeatureTag();
    }
}

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

PROTECTED VIRTUAL void AosHandleMtc::ProcessCapabilitiesChanged(
        IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities)
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
        // Manage current blocks
        ProcessBlock(GetVoiceBlockReasonForIpcan(),
                !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VOICE));

        ProcessBlock(GetVideoBlockReasonForIpcan(),
                !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VIDEO));

        ProcessBlock(BLOCK_CALL_COMPOSER_CAPABILITY,
                !IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::CALL_COMPOSER),
                IMS_FALSE);

        // Manage holding blocks
        if (IsEpdgEnabled())
        {
            IMS_UINT32 nMobileNetworkType = GetMobileNetworkType();

            if (IsSupportedNetworkTypeForCellular(nMobileNetworkType))
            {
                ProcessBlock(BLOCK_VOLTE_CAPABILITY,
                        !IsCapabilityExistedForNetworkType(
                                nMobileNetworkType, AosCapability::VOICE));
                ProcessBlock(BLOCK_VILTE_CAPABILITY,
                        !IsCapabilityExistedForNetworkType(
                                nMobileNetworkType, AosCapability::VIDEO));
            }
        }
        else
        {
            if (GET_N_CONFIG(m_nSlotId)->IsWfcImsAvailable())
            {
                ProcessBlock(BLOCK_VOWIFI_CAPABILITY,
                        !IsCapabilityExistedForNetworkType(
                                NW_REPORT_RADIO_WLAN, AosCapability::VOICE));

                ProcessBlock(BLOCK_VIWIFI_CAPABILITY,
                        !IsCapabilityExistedForNetworkType(
                                NW_REPORT_RADIO_WLAN, AosCapability::VIDEO));
            }
        }
    }
}

PROTECTED VIRTUAL void AosHandleMtc::ProcessNetworkChanged()
{
    if (IsSupportedNetworkType(m_nNetworkType))
    {
        if (!IsEpdgEnabled())
        {
            if (AosHandle::IsHandleBlocked(BLOCK_NETWORK) ||
                    AosHandle::IsHandleBlocked(m_nHoldingBlocksForMobile, BLOCK_NETWORK))
            {
                ProcessBlock(BLOCK_NETWORK, IMS_FALSE);
            }
        }

        IMS_BOOL bIsVoiceCapable =
                IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VOICE);
        IMS_BOOL bIsVideoCapable =
                IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::VIDEO);
        IMS_BOOL bIsCallComposerCapable =
                IsCapabilityExistedForNetworkType(m_nNetworkType, AosCapability::CALL_COMPOSER);

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

        ProcessBlock(BLOCK_CALL_COMPOSER_CAPABILITY, !bIsCallComposerCapable, IMS_FALSE);

        if (GET_N_CONFIG(m_nSlotId)->IsRequiredVolteBlockBySsac())
        {
            if (m_nNetworkType == NW_REPORT_RADIO_LTE)
            {
                SsacInfo objSsacInfo = m_piImsRadio->GetSsacInfo();
                if (m_bSsacBarred != (objSsacInfo.nBarringFactorForVoice == 0))
                {
                    ImsRadio_OnSsacChanged(objSsacInfo);
                }
            }
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

PROTECTED VIRTUAL void AosHandleMtc::ProcessVopsStateChanged(
        IN IMS_UINT32 nState, IN IMS_BOOL bUpdateState /* = IMS_TRUE */)
{
    if (m_bVopsIgnoredForVolteEnabled && bUpdateState)
    {
        m_nVopsState = nState;
        return;
    }

    if (ProcessHoldingVopsState(nState))
    {
        A_IMS_TRACE_I(APPPROFILE,
                "ProcessVopsStateChanged :: handled for holding state. m_nHoldingVopsState(%d)",
                m_nHoldingVopsState, 0, 0);
        return;
    }

    if (bUpdateState)
    {
        m_nVopsState = nState;
    }

    if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
    {
        ReevaluateUnavailableFeature();
        return;
    }

    if (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        StopVolteHysTimer();

        if (IsPlmnBlockCondition())
        {
            A_IMS_TRACE_I(
                    APPPROFILE, "ProcessVopsStateChanged :: PLMN is blocked with timeout", 0, 0, 0);
            m_piAppContext->GetApp()->RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT);
        }

        ProcessBlock(BLOCK_VOPS, IMS_TRUE);
    }
    else
    {
        if (AosHandle::IsHandleBlocked(BLOCK_VOPS))
        {
            IMS_SINT32 nVolteHysTime = GET_N_CONFIG(m_nSlotId)->GetVolteHysTime();
            if (nVolteHysTime > 0 && IsSupportedNetworkTypeForCellular(m_nNetworkType) &&
                    !AosHandle::IsHandleBlocked(BLOCK_SSAC))
            {
                StartVolteHysTimer(nVolteHysTime);
                return;
            }
        }

        ProcessBlock(BLOCK_VOPS, IMS_FALSE);
    }
}

PROTECTED VIRTUAL void AosHandleMtc::ReevaluateUnavailableFeature()
{
    IMS_BOOL bIsVoiceUnavailable = IMS_FALSE;
    IMS_UINT32 nOldUnavailableFeature = m_objFeatureTagList.GetUnavailableFeatures();

    if (IsSupportedNetworkTypeForCellular(m_nNetworkType))
    {
        if (!m_bVopsIgnoredForVolteEnabled)
        {
            bIsVoiceUnavailable = (m_nVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED);
        }

        if (m_nNetworkType == NW_REPORT_RADIO_LTE)
        {
            bIsVoiceUnavailable = (bIsVoiceUnavailable || m_bSsacBarred);
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

    IMS_BOOL bIsVideoUnavailable =
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

PRIVATE
void AosHandleMtc::UpdateGGsmaRcsTelephonyFeatureTag()
{
    /* VZW Req. - VZ_REQ_IMS_22939, VZ_REQ_VOWIFI_6230394
                  VZ_REQ_VOWIFI_6258874, VZ_REQ_VOWIFI_6258951
    */

    IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
    if (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive())
    {
        if (m_objBindedFeatureTagList.HasFeatureTag(
                    FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ))
        {
            m_objFeatureTagList.RemoveFeatureTag(
                    FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
            m_objFeatureTagList.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
            m_objFeatureTagList.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);
        }
    }
    else
    {
        if (m_objFeatureTagList.HasFeature(ImsAosFeature::MMTEL))
        {
            m_objFeatureTagList.RemoveFeatureTag(
                    FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
            m_objFeatureTagList.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
            m_objFeatureTagList.AddFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);
        }
        else
        {
            m_objFeatureTagList.RemoveFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_CS);
            m_objFeatureTagList.RemoveFeatureTag(FeatureTags::RCS_TELEPHONY, AosString::STR_VOLTE);

            if (IsCsFeatureTagRequired())
            {
                m_objFeatureTagList.AddFeatureTag(
                        FeatureTags::RCS_TELEPHONY, AosString::STR_CS_WITH_DQ);
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

PRIVATE
IMS_BOOL AosHandleMtc::IsCsFeatureTagRequired() const
{
    if (!GET_N_CONFIG(m_nSlotId)->IsVideoOverWifiSupportedWithoutVoice())
    {
        return IMS_FALSE;
    }

    if (!m_objFeatureTagList.HasFeature(ImsAosFeature::VIDEO))
    {
        return IMS_FALSE;
    }

    if (!IsEpdgEnabled())
    {
        return IMS_FALSE;
    }

    if (!IsInvalidMobileNetwork())
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AosHandleMtc::IsInvalidMobileNetwork() const
{
    if (!IsSupportedNetworkTypeForCellular(GetMobileNetworkType()))
    {
        return IMS_TRUE;
    }

    if (!IsSupportedNetworkTypeForCellular(GetMobileChangingNetworkType()))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AosHandleMtc::IsPlmnBlockCondition() const
{
    if (!GET_N_CONFIG(m_nSlotId)->IsPlmnBlockWithTimeoutOnVoiceCallUnavailable())
    {
        return IMS_FALSE;
    }

    if (!IsSupportedNetworkTypeForCellular(m_nNetworkType))
    {
        return IMS_FALSE;
    }

    if ((m_nNetworkType == NW_REPORT_RADIO_LTE) && m_bCsVoiceAvailable)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AosHandleMtc::ProcessHoldingVopsState(IN IMS_UINT32 nState)
{
    if (nState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive())
        {
            m_nHoldingVopsState = nState;
            return IMS_TRUE;
        }
    }
    else  // IMS_VOICE_OVER_PS_SUPPORTED
    {
        if (m_nHoldingVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
        {
            m_nHoldingVopsState = nState;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AosHandleMtc::ProcessHoldingSsacState(IN IMS_SINT32 nBarringFactorForVoice)
{
    if (nBarringFactorForVoice == 0)
    {
        IAosCallTracker* piCallTracker = AosProvider::GetInstance()->GetCallTracker(m_nSlotId);
        if (piCallTracker != IMS_NULL && piCallTracker->IsNormalCallActive())
        {
            m_bSsacHeld = IMS_TRUE;
            return IMS_TRUE;
        }
    }
    else
    {
        if (m_bSsacHeld)
        {
            m_bSsacHeld = IMS_FALSE;
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
void AosHandleMtc::ProcessVolteHysTimerExpired()
{
    A_IMS_TRACE_D(APPPROFILE, "ProcessVolteHysTimerExpired", 0, 0, 0);

    StopVolteHysTimer();

    if (m_nVopsState == IMS_VOICE_OVER_PS_SUPPORTED && AosHandle::IsHandleBlocked(BLOCK_VOPS))
    {
        ProcessBlock(BLOCK_VOPS, IMS_FALSE);
        return;
    }

    if (!m_bSsacBarred && AosHandle::IsHandleBlocked(BLOCK_SSAC))
    {
        ProcessBlock(BLOCK_SSAC, IMS_FALSE);
    }
}

PRIVATE
IMS_BOOL AosHandleMtc::StartVolteHysTimer(IN IMS_UINT32 nDuration)
{
    if (nDuration == 0)
    {
        return IMS_FALSE;
    }

    if (m_piVolteHysTimer != IMS_NULL)
    {
        return IMS_FALSE;
    }

    m_piVolteHysTimer =
            AosUtil::GetInstance()->StartTimer(nDuration * 1000, this, "TIMER_VOLTE_HYS");

    return IMS_TRUE;
}

PRIVATE
void AosHandleMtc::StopVolteHysTimer()
{
    if (m_piVolteHysTimer == IMS_NULL)
    {
        return;
    }

    AosUtil::GetInstance()->StopTimer(m_piVolteHysTimer, "TIMER_VOLTE_HYS");
}

PRIVATE
IMS_BOOL AosHandleMtc::IsVolteHysTimerRunning() const
{
    return (m_piVolteHysTimer != IMS_NULL);
}

PRIVATE VIRTUAL void AosHandleMtc::NConfiguration_NotifyConfigChanged()
{
    AosHandle::NConfiguration_NotifyConfigChanged();

    IMS_BOOL bIsVopsIgnoredForVolteEnabled =
            GET_N_CONFIG(m_nSlotId)->IsVopsIgnoredForVolteEnabled();

    if (m_bVopsIgnoredForVolteEnabled != bIsVopsIgnoredForVolteEnabled)
    {
        A_IMS_TRACE_D(APPPROFILE, "NConfiguration_NotifyConfigChanged :: \
                IsVopsIgnoredForVolteEnabled(%s), m_nVopsState(%d)",
                _TRACE_B_(bIsVopsIgnoredForVolteEnabled), m_nVopsState, 0);

        if (IsSupportedNetworkTypeForCellular(m_nNetworkType))
        {
            if (m_nVopsState == IMS_VOICE_OVER_PS_NOT_SUPPORTED)
            {
                ProcessVopsStateChanged(bIsVopsIgnoredForVolteEnabled
                                ? IMS_VOICE_OVER_PS_SUPPORTED
                                : IMS_VOICE_OVER_PS_NOT_SUPPORTED,
                        IMS_FALSE);
            }
        }

        m_bVopsIgnoredForVolteEnabled = bIsVopsIgnoredForVolteEnabled;
    }
}

PRIVATE VIRTUAL void AosHandleMtc::ImsRadio_OnSsacChanged(IN const SsacInfo& objSsacInfo)
{
    if (!GET_N_CONFIG(m_nSlotId)->IsRequiredVolteBlockBySsac())
    {
        return;
    }

    A_IMS_TRACE_I(APPPROFILE, "ImsRadio_OnSsacChanged :: BarringFactorForVoice(%d)",
            objSsacInfo.nBarringFactorForVoice, 0, 0);

    if (ProcessHoldingSsacState(objSsacInfo.nBarringFactorForVoice))
    {
        A_IMS_TRACE_D(APPPROFILE, "ImsRadio_OnSsacChanged :: Proceeded holding state", 0, 0, 0);
        return;
    }

    if (objSsacInfo.nBarringFactorForVoice == 0)
    {
        if (m_nNetworkType != NW_REPORT_RADIO_LTE)
        {
            return;
        }

        StopVolteHysTimer();
        m_bSsacBarred = IMS_TRUE;

        if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
        {
            ReevaluateUnavailableFeature();
            return;
        }

        if (IsPlmnBlockCondition())
        {
            A_IMS_TRACE_I(
                    APPPROFILE, "ImsRadio_OnSsacChanged :: PLMN is blocked with timeout", 0, 0, 0);
            m_piAppContext->GetApp()->RequestCmd(ImsAosControl::PLMN_BLOCK_WITH_TIMEOUT);
        }

        ProcessBlock(BLOCK_SSAC, IMS_TRUE);
    }
    else
    {
        m_bSsacBarred = IMS_FALSE;

        if (GET_N_CONFIG(m_nSlotId)->IsRegWithFeatureTagUnavailableSupported())
        {
            ReevaluateUnavailableFeature();
            return;
        }

        if (AosHandle::IsHandleBlocked(BLOCK_SSAC))
        {
            IMS_SINT32 nVolteHysTime = GET_N_CONFIG(m_nSlotId)->GetVolteHysTime();
            if (nVolteHysTime > 0 && m_nNetworkType == NW_REPORT_RADIO_LTE &&
                    !AosHandle::IsHandleBlocked(BLOCK_VOPS))
            {
                A_IMS_TRACE_I(APPPROFILE,
                        "ImsRadio_OnSsacChanged :: Start VoLTE_Hys timer for (%d) secs",
                        nVolteHysTime, 0, 0);
                StartVolteHysTimer(nVolteHysTime);
                return;
            }
        }

        ProcessBlock(BLOCK_SSAC, IMS_FALSE);
    }
}

PRIVATE VIRTUAL void AosHandleMtc::ServicePhone_PlmnChanged()
{
    if (!IsSupportedNetworkTypeForCellular(m_nNetworkType))
    {
        return;
    }

    if (IsVolteHysTimerRunning())
    {
        A_IMS_TRACE_I(APPPROFILE, "ServicePhone_PlmnChanged :: Stop VoLTE_Hys timer", 0, 0, 0);
        ProcessVolteHysTimerExpired();
    }
}

PRIVATE VIRTUAL void AosHandleMtc::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piVolteHysTimer)
    {
        ProcessVolteHysTimerExpired();
        return;
    }
}