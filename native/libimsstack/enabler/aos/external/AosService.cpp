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
#include "JniEnablerConnector.h"
#include "IJniAosServiceThread.h"
#include "IJniEnabler.h"

#include "interface/IAosEmergencyListener.h"
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosServiceSettingListener.h"
#include "external/AosService.h"

__IMS_TRACE_TAG_AOS__;

#define AOSTAG m_strTag.GetStr()
#define TO_BOOLEAN(n) ((n) > 0)

PUBLIC
AosService::AosService(IN IMS_SINT32 nSlotId) :
        m_objAosEmergencyListeners(ImsList<IAosEmergencyListener*>()),
        m_objAosRegistrationControlListeners(ImsList<IAosRegistrationControlListener*>()),
        m_objAosServiceSettingListeners(ImsList<IAosServiceSettingListener*>()),
        m_objAosServicePhoneListeners(ImsList<IAosServicePhoneListener*>()),
        m_nSlotId(nSlotId),
        m_bNullNasSecAlgo(IMS_FALSE),
        m_objCapabilities(ImsMap<IMS_UINT32, IMS_UINT32>())
{
    m_strTag.Sprintf("%d", m_nSlotId);
    A_IMS_TRACE_I(AOSTAG, "+AosService [slot_%d]", m_nSlotId, 0, 0);
    Init();
}

PUBLIC VIRTUAL AosService::~AosService()
{
    A_IMS_TRACE_I(AOSTAG, "~AosService [slot_%d]", m_nSlotId, 0, 0);

    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::AOS_SERVICE, IMS_NULL);

    CleanUp();
}

PUBLIC VIRTUAL void AosService::AddListener(IN IAosRegistrationControlListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        const IAosRegistrationControlListener* piTempListener =
                m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return;
        }
    }

    m_objAosRegistrationControlListeners.Append(piListener);
}

PUBLIC VIRTUAL void AosService::RemoveListener(IN IAosRegistrationControlListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        const IAosRegistrationControlListener* piTempListener =
                m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosRegistrationControlListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void AosService::AddListener(IN IAosServiceSettingListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        const IAosServiceSettingListener* piTempListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return;
        }
    }

    m_objAosServiceSettingListeners.Append(piListener);
}

PUBLIC VIRTUAL void AosService::RemoveListener(IN IAosServiceSettingListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        const IAosServiceSettingListener* piTempListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosServiceSettingListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void AosService::AddListener(IN IAosServicePhoneListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        const IAosServicePhoneListener* piTempListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return;
        }
    }

    m_objAosServicePhoneListeners.Append(piListener);
}

PUBLIC VIRTUAL void AosService::RemoveListener(IN IAosServicePhoneListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        const IAosServicePhoneListener* piTempListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosServicePhoneListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void AosService::AddListener(IN IAosEmergencyListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosEmergencyListeners.GetSize(); ++i)
    {
        const IAosEmergencyListener* piTempListener = m_objAosEmergencyListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return;
        }
    }

    m_objAosEmergencyListeners.Append(piListener);
}

PUBLIC VIRTUAL void AosService::RemoveListener(IN IAosEmergencyListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objAosEmergencyListeners.GetSize(); ++i)
    {
        const IAosEmergencyListener* piTempListener = m_objAosEmergencyListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosEmergencyListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyEmergencyCallbackModeChanged(
        IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_ULONG nDuration)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyEmergencyCallbackModeChanged nType(%d), nState(%d), nDuration(%d)",
            nType, nState, nDuration);
    for (IMS_UINT32 i = 0; i < m_objAosEmergencyListeners.GetSize(); ++i)
    {
        IAosEmergencyListener* piListener = m_objAosEmergencyListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->CallbackModeChanged(static_cast<EmergencyCallbackModeType>(nType),
                    static_cast<EmergencyCallbackMode>(nState), nDuration);
        }
    }
}

PUBLIC VIRTUAL void AosService::UpdateSipDelegateRegistration()
{
    A_IMS_TRACE_I(AOSTAG, "UpdateSipDelegateRegistration", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piListener = m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->RegistrationControl_UpdateSipDelegateRegistration();
        }
    }
}

PUBLIC VIRTUAL void AosService::TriggerSipDelegateDeregistration()
{
    A_IMS_TRACE_I(AOSTAG, "TriggerSipDelegateDeregistration", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piListener = m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->RegistrationControl_TriggerSipDelegateDeregistration();
        }
    }
}

PUBLIC VIRTUAL void AosService::TriggerFullNetworkRegistration(
        IN IMS_SINT32 nSipCode, IN const AString& strSipReason)
{
    A_IMS_TRACE_I(AOSTAG, "TriggerFullNetworkRegistration :: nSipCode(%d), strSipReason(%s)",
            nSipCode, strSipReason.GetStr(), 0);
    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piListener = m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->RegistrationControl_TriggerFullNetworkRegistration(nSipCode, strSipReason);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyCapabilitiesChanged(
        IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyCapabilitiesChanged", 0, 0, 0);
    PrintCapabilities(objCapabilities);
    m_objCapabilities = objCapabilities;

    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piListener = m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->RegistrationControl_NotifyCapabilitiesChanged(objCapabilities);
        }
    }
}

PUBLIC VIRTUAL void AosService::ControlRegistration(
        IN IMS_SINT32 nRequestType, IN IMS_SINT32 nPcscfOrder, IN IMS_SINT32 nControlCause)
{
    A_IMS_TRACE_I(AOSTAG,
            "ControlRegistration :: nRequestType(%d), nPcscfOrder(%d), nControlCause(%d)",
            nRequestType, nPcscfOrder, nControlCause);
    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piListener = m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->RegistrationControl_ControlRegistration(
                    static_cast<AosRegRequestType>(nRequestType),
                    static_cast<AosPcscfOrder>(nPcscfOrder),
                    static_cast<AosControlCause>(nControlCause));
        }
    }
}

PUBLIC VIRTUAL void AosService::UpdateDataFailureReason(IN IMS_SINT32 nReason)
{
    A_IMS_TRACE_I(AOSTAG, "UpdateDataFailureReason :: nReason(%d)", nReason, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piListener = m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->RegistrationControl_UpdateDataFailureReason(nReason);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyAirplaneSetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyAirplaneSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_AirplaneChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyDataRoamingSetting(IN IMS_UINT32 nIsAllowed)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyDataRoamingSetting :: nIsAllowed(%d)", nIsAllowed, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_DataRoamingChanged(TO_BOOLEAN(nIsAllowed));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyMobileDataSetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyMobileDataSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_MobileDataChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyRoamingPreferredVoiceNetwork(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyRoamingPreferredVoiceNetwork :: nState(%d)", nState, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_RoamingPreferredVoiceNetworkChanged(
                    static_cast<RoamingPreferredVoiceNetwork>(nState));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyServiceSetting(
        IN IMS_UINT32 nState, IN IMS_UINT32 nServiceBits)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyServiceSetting :: nState(%d), nServiceBits(%d)", nState,
            nServiceBits, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_ServiceChanged(
                    static_cast<ServiceSetting>(nState), nServiceBits);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyTtySetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyTtySetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_TtyChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyVideoSetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyVideoSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_VideoChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyVolteSetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyVolteSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_VolteChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyWfcSetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyWfcSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_WfcChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyWifiSetting(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyWifiSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_WifiChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyAosStart()
{
    A_IMS_TRACE_I(AOSTAG, "NotifyAosStart", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_AosStart();
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyIpcanHandoverFailure(
        IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyIpcanHandoverFailure :: nTargetNetwork(%d), nCauseCode(%d)",
            nTargetNetwork, nCauseCode, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_notifyIpcanHandoverFailure(nTargetNetwork, nCauseCode);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyIsimState(IN IMS_SINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyIsimState :: nState(%d)", nState, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_IsimStateChanged(static_cast<IsimState>(nState));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyLocationInfo(IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyLocationInfo :: nState(%d)", nState, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_LocationInfoChanged(static_cast<LocationInfo>(nState));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyMobileDataLimit(IN IMS_UINT32 nIsLimited)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyMobileDataLimit :: nIsLimited(%d)", nIsLimited, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_MobileDataLimitChanged(TO_BOOLEAN(nIsLimited));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyNetworkVideoCapability(IN IMS_UINT32 nIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyNetworkVideoCapability :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_NetworkVideoCapabilityChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyPhoneNumberState(
        IN IMS_UINT32 nIsRefresh, IN IMS_UINT32 nState)
{
    A_IMS_TRACE_I(
            AOSTAG, "NotifyPhoneNumberState :: nIsRefresh(%d), nState(%d)", nIsRefresh, nState, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PhoneNumberStateChanged(
                    TO_BOOLEAN(nIsRefresh), static_cast<PhoneNumberState>(nState));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyPlmnChanged(IN const AString& strPlmn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyPlmnChanged :: strPlmn(%s)", strPlmn.GetStr(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PlmnChanged(strPlmn);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyVopsStateChanged(
        IN IMS_UINT32 nState, IN const AString& strPlmn)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyVopsStateChanged :: nState(%d), strPlmn(%s)", nState,
            strPlmn.GetStr(), 0);

    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_VopsStateChanged(nState, strPlmn);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyPowerOff()
{
    A_IMS_TRACE_I(AOSTAG, "NotifyPowerOff", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PowerOff();
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyPreciseCallState(IN IMS_SINT32 nState)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyPreciseCallState :: nState(%d)", nState, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PreciseCallStateChanged(static_cast<PreciseCallState>(nState));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyCarrierSignalPcoValueChanged(IN IMS_SINT32 nValue)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyCarrierSignalPcoValueChanged :: nValue(%d)", nValue, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PcoValueChanged(nValue);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyCrossSimStatus(IN IMS_SINT32 nIsConnected)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyCrossSimStatus :: nIsConnected(%d)", nIsConnected, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_CrossSimStatusChanged(TO_BOOLEAN(nIsConnected));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyNasSecurityAlgorithmChanged(IN IMS_UINT32 nIsNullAlgo)
{
    A_IMS_TRACE_I(
            AOSTAG, "NotifyNasSecurityAlgorithmChanged :: nIsNullAlgo(%d)", nIsNullAlgo, 0, 0);

    m_bNullNasSecAlgo = TO_BOOLEAN(nIsNullAlgo);
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyRegistered(IN IMS_SINT32 nRegType,
        IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
        IN const ImsList<AString>& objFeatureTags)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyRegistered", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyRegistered(
                nRegType, static_cast<IMS_SINT32>(eNetworkType), nFeatureTagBits, objFeatureTags);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyRegistering(IN IMS_SINT32 nRegType,
        IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
        IN const ImsList<AString>& objFeatureTags)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyRegistering", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyRegistering(
                nRegType, static_cast<IMS_SINT32>(eNetworkType), nFeatureTagBits, objFeatureTags);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyDeregistered(
        IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN AosReasonCode eReason)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyDeregistered - regType(%d), network(%d), reason(%d)", nRegType,
            static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason));
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyDeregistered(
                nRegType, static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyDeregistering(IN IMS_SINT32 nRegType)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyDeregistering", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyDeregistering(nRegType);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyTechnologyChangeFailed(
        IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN AosReasonCode eReason)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyTechnologyChangeFailed - regType(%d), network(%d), reason(%d)",
            nRegType, static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason));
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyTechnologyChangeFailed(
                nRegType, static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyAssociatedUriChanged", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyAssociatedUriChanged(objUris);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyCapabilitiesUpdateFailed(
        IN AosCapability eCapabilities, IN AosNetworkType eNetworkType, IN AosReasonCode eReason)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyCapabilitiesUpdateFailed", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyCapabilitiesUpdateFailed(static_cast<IMS_UINT32>(eCapabilities),
                static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyAosIsimState(IN AosIsimState eState)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyAosIsimState", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyAosIsimState(static_cast<IMS_SINT32>(eState));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyRegEventState(
        IN IMS_UINT32 nStatusCode, IN const ImsList<AString>& objImpus /*= ImsList<AString>()*/)
{
    A_IMS_TRACE_I(AOSTAG, "NotifyRegEventState", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyRegEventState(nStatusCode, objImpus);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RequestPhoneNumberRetry(IN AosPhoneNumberRetryCommand eCommand)
{
    A_IMS_TRACE_I(AOSTAG, "RequestPhoneNumberRetry", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->RequestPhoneNumberRetry(static_cast<IMS_SINT32>(eCommand));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RequestWifiService(IN IMS_BOOL bIsOn)
{
    A_IMS_TRACE_I(AOSTAG, "RequestWifiService", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->RequestWifiService(bIsOn);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL ImsMap<IMS_UINT32, IMS_UINT32>& AosService::GetCapabilities()
{
    A_IMS_TRACE_I(AOSTAG, "GetCapabilities", 0, 0, 0);
    return m_objCapabilities;
}

PUBLIC VIRTUAL IMS_UINT32 AosService::GetCapabilitiesForNetwork(AosNetworkType eNetworkType)
{
    if (m_objCapabilities.GetIndexOfKey(static_cast<IMS_UINT32>(eNetworkType)) < 0)
    {
        return 0;
    }

    return m_objCapabilities.GetValue(static_cast<IMS_UINT32>(eNetworkType));
}

PUBLIC VIRTUAL IMS_BOOL AosService::IsSupportCapabilitiesForNetwork(
        AosNetworkType eNetworkType, AosCapability eCapability)
{
    return (GetCapabilitiesForNetwork(eNetworkType) & static_cast<IMS_UINT32>(eCapability)) > 0
            ? IMS_TRUE
            : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::IsNasSecurityAlgorithmNull()
{
    return m_bNullNasSecAlgo;
}

PUBLIC GLOBAL AString AosService::PrintCapabilities(
        IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities)
{
    AString strCapabilities;

    IMS_UINT32 nSize = objCapabilities.GetSize();
    for (IMS_UINT32 i = 0; i < nSize; i++)
    {
        strCapabilities.Append(NetworkTypeToString(objCapabilities.GetKeyAt(i)));
        strCapabilities.Append(" : ");
        strCapabilities.Append(CapabilitiesToString(objCapabilities.GetValueAt(i)));
    }
    IMS_TRACE_I(
            "PrintCapabilities :: size(%d), Capabilities(%s)", nSize, strCapabilities.GetStr(), 0);

    return strCapabilities;
}

PUBLIC GLOBAL const IMS_CHAR* AosService::NetworkTypeToString(IN IMS_SINT32 nType)
{
    switch (static_cast<AosNetworkType>(nType))
    {
        case AosNetworkType::NONE:
            return "NONE";

        case AosNetworkType::LTE:
            return "LTE";

        case AosNetworkType::IWLAN:
            return "IWLAN";

        case AosNetworkType::CROSS_SIM:
            return "CROSS_SIM";

        case AosNetworkType::NR:
            return "NR";

        case AosNetworkType::UTRAN:
            return "UTRAN";

        default:
            return "INVALID";
    }
}

PUBLIC GLOBAL const AString AosService::CapabilitiesToString(IN IMS_UINT32 nCapabilities)
{
    AString strCapabilities = "[ ";

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::VOICE))
    {
        strCapabilities.Append("VOICE ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::VIDEO))
    {
        strCapabilities.Append("VIDEO ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::UT))
    {
        strCapabilities.Append("UT ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::SMS))
    {
        strCapabilities.Append("SMS ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER))
    {
        strCapabilities.Append("CALL_COMPOSER ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::CALL_COMPOSER_BUSINESS_ONLY))
    {
        strCapabilities.Append("CALL_COMPOSER_BUSINESS_ONLY ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE))
    {
        strCapabilities.Append("OPTIONS_UCE ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE))
    {
        strCapabilities.Append("PRESENCE_UCE ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::TEXT))
    {
        strCapabilities.Append("TEXT ");
    }

    strCapabilities.Append("] ");

    return strCapabilities;
}

PRIVATE
void AosService::Init()
{
    A_IMS_TRACE_I(AOSTAG, "Init", 0, 0, 0);
    Attach();
}

PRIVATE
void AosService::CleanUp()
{
    A_IMS_TRACE_I(AOSTAG, "CleanUp", 0, 0, 0);
}

PRIVATE
void AosService::Attach()
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::AOS_SERVICE, this);
}

PRIVATE
IJniAosServiceThread* AosService::GetJniThread()
{
    const IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSlotId, EnablerType::AOS_SERVICE);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniAosServiceThread is null", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniAosServiceThread*>(piJniEnabler->GetJniThread());
}
