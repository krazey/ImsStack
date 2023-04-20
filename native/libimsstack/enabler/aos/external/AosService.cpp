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
#include "interface/IAosRegistrationControlListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosServiceSettingListener.h"
#include "external/AosService.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

#define AOSTAG m_strTag.GetStr()
#define TO_BOOLEAN(n) ((n) > 0)

PUBLIC
AosService::AosService(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_objAosRegistrationControlListeners(ImsList<IAosRegistrationControlListener*>()),
        m_objAosServiceSettingListeners(ImsList<IAosServiceSettingListener*>()),
        m_objAosServicePhoneListeners(ImsList<IAosServicePhoneListener*>()),
        m_objCapabilities(ImsMap<IMS_UINT32, IMS_UINT32>())
{
    IMS_TRACE_I("+AosService [slot_%d]", m_nSlotId, 0, 0);
    Init();
}

PUBLIC VIRTUAL AosService::~AosService()
{
    IMS_TRACE_I("~AosService [slot_%d]", m_nSlotId, 0, 0);

    JniEnablerConnector::GetInstance().SetNativeEnabler(
            m_nSlotId, EnablerType::AOS_SERVICE, IMS_NULL);

    CleanUp();
}

PUBLIC VIRTUAL IMS_BOOL AosService::AddListener(IN IAosRegistrationControlListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piTempListener =
                m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return IMS_FALSE;
        }
    }

    m_objAosRegistrationControlListeners.Append(piListener);
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RemoveListener(IN IAosRegistrationControlListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAosRegistrationControlListeners.GetSize(); ++i)
    {
        IAosRegistrationControlListener* piTempListener =
                m_objAosRegistrationControlListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosRegistrationControlListeners.RemoveAt(i);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::AddListener(IN IAosServiceSettingListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piTempListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return IMS_FALSE;
        }
    }

    m_objAosServiceSettingListeners.Append(piListener);
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RemoveListener(IN IAosServiceSettingListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piTempListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosServiceSettingListeners.RemoveAt(i);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::AddListener(IN IAosServicePhoneListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piTempListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            return IMS_FALSE;
        }
    }

    m_objAosServicePhoneListeners.Append(piListener);
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RemoveListener(IN IAosServicePhoneListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piTempListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener == piTempListener)
        {
            m_objAosServicePhoneListeners.RemoveAt(i);
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL void AosService::UpdateSipDelegateRegistration()
{
    IMS_TRACE_I("UpdateSipDelegateRegistration", 0, 0, 0);
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
    IMS_TRACE_I("TriggerSipDelegateDeregistration", 0, 0, 0);
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
    IMS_TRACE_I("TriggerFullNetworkRegistration :: nSipCode(%d), strSipReason(%s)", nSipCode,
            strSipReason.GetStr(), 0);
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
    IMS_TRACE_I("NotifyCapabilitiesChanged", 0, 0, 0);
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
    IMS_TRACE_I("ControlRegistration :: nRequestType(%d), nPcscfOrder(%d), nControlCause(%d)",
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

PUBLIC VIRTUAL void AosService::NotifyAirplaneSetting(IN IMS_UINT32 nIsOn)
{
    IMS_TRACE_I("NotifyAirplaneSetting :: nIsOn(%d)", nIsOn, 0, 0);
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
    IMS_TRACE_I("NotifyDataRoamingSetting :: nIsAllowed(%d)", nIsAllowed, 0, 0);
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
    IMS_TRACE_I("NotifyMobileDataSetting :: nIsOn(%d)", nIsOn, 0, 0);
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
    IMS_TRACE_I("NotifyRoamingPreferredVoiceNetwork :: nState(%d)", nState, 0, 0);
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
    IMS_TRACE_I("NotifyServiceSetting :: nState(%d), nServiceBits(%d)", nState, nServiceBits, 0);
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
    IMS_TRACE_I("NotifyTtySetting :: nIsOn(%d)", nIsOn, 0, 0);
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
    IMS_TRACE_I("NotifyVideoSetting :: nIsOn(%d)", nIsOn, 0, 0);
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
    IMS_TRACE_I("NotifyVolteSetting :: nIsOn(%d)", nIsOn, 0, 0);
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
    IMS_TRACE_I("NotifyWfcSetting :: nIsOn(%d)", nIsOn, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServiceSettingListeners.GetSize(); ++i)
    {
        IAosServiceSettingListener* piListener = m_objAosServiceSettingListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServiceSetting_WfcChanged(TO_BOOLEAN(nIsOn));
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyAosStart()
{
    IMS_TRACE_I("NotifyAosStart", 0, 0, 0);
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
    IMS_TRACE_I("NotifyIpcanHandoverFailure :: nTargetNetwork(%d), nCauseCode(%d)", nTargetNetwork,
            nCauseCode, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_notifyIpcanHandoverFailure(nTargetNetwork, nCauseCode);
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyIsimState(IN IMS_UINT32 nState)
{
    IMS_TRACE_I("NotifyIsimState :: nState(%d)", nState, 0, 0);
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
    IMS_TRACE_I("NotifyLocationInfo :: nState(%d)", nState, 0, 0);
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
    IMS_TRACE_I("NotifyMobileDataLimit :: nIsLimited(%d)", nIsLimited, 0, 0);
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
    IMS_TRACE_I("NotifyNetworkVideoCapability :: nIsOn(%d)", nIsOn, 0, 0);
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
    IMS_TRACE_I("NotifyPhoneNumberState :: nIsRefresh(%d), nState(%d)", nIsRefresh, nState, 0);
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

PUBLIC VIRTUAL void AosService::NotifyPlmnChanged()
{
    IMS_TRACE_I("NotifyPlmnChanged", 0, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PlmnChanged();
        }
    }
}

PUBLIC VIRTUAL void AosService::NotifyPowerOff()
{
    IMS_TRACE_I("NotifyPowerOff", 0, 0, 0);
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
    IMS_TRACE_I("NotifyPreciseCallState :: nState(%d)", nState, 0, 0);
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
    IMS_TRACE_I("NotifyCarrierSignalPcoValueChanged :: nValue(%d)", nValue, 0, 0);
    for (IMS_UINT32 i = 0; i < m_objAosServicePhoneListeners.GetSize(); ++i)
    {
        IAosServicePhoneListener* piListener = m_objAosServicePhoneListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ServicePhone_PcoValueChanged(nValue);
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyRegistered(IN AosNetworkType eNetworkType,
        IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags)
{
    IMS_TRACE_I("NotifyRegistered", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyRegistered(
                static_cast<IMS_SINT32>(eNetworkType), nFeatureTagBits, objFeatureTags);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyRegistering(IN AosNetworkType eNetworkType,
        IN IMS_UINT32 nFeatureTagBits, IN const ImsList<AString>& objFeatureTags)
{
    IMS_TRACE_I("NotifyRegistering", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyRegistering(
                static_cast<IMS_SINT32>(eNetworkType), nFeatureTagBits, objFeatureTags);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyDeregistered(
        IN AosNetworkType eNetworkType, IN AosReasonCode eReason)
{
    IMS_TRACE_I("NotifyDeregistered - network(%d), reason(%d)",
            static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason), 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyDeregistered(
                static_cast<IMS_SINT32>(eNetworkType), static_cast<IMS_SINT32>(eReason));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyTechnologyChangeFailed(
        IN AosNetworkType eNetworkType, IN IMS_SINT32 nCauseCode)
{
    IMS_TRACE_I("NotifyTechnologyChangeFailed", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyTechnologyChangeFailed(
                static_cast<IMS_SINT32>(eNetworkType), nCauseCode);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris)
{
    IMS_TRACE_I("NotifyAssociatedUriChanged", 0, 0, 0);
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
    IMS_TRACE_I("NotifyCapabilitiesUpdateFailed", 0, 0, 0);
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
    IMS_TRACE_I("NotifyAosIsimState", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyAosIsimState(static_cast<IMS_SINT32>(eState));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::NotifyRegEventState(IN AosRegEvent eState)
{
    IMS_TRACE_I("NotifyRegEventState", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->NotifyRegEventState(static_cast<IMS_SINT32>(eState));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RequestPhoneNumberRetry(IN AosPhoneNumberRetryCommand eCommand)
{
    IMS_TRACE_I("RequestPhoneNumberRetry", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->RequestPhoneNumberRetry(static_cast<IMS_SINT32>(eCommand));
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL AosService::RequestWifiService(IN IMS_BOOL bIsOn)
{
    IMS_TRACE_I("RequestWifiService", 0, 0, 0);
    IJniAosServiceThread* piJniThread = GetJniThread();
    if (piJniThread)
    {
        piJniThread->RequestWifiService(bIsOn);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL ImsMap<IMS_UINT32, IMS_UINT32>& AosService::GetCapabilities()
{
    IMS_TRACE_I("GetCapabilities", 0, 0, 0);
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

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::OPTIONS_UCE))
    {
        strCapabilities.Append("OPTIONS_UCE ");
    }

    if (nCapabilities & static_cast<IMS_UINT32>(AosCapability::PRESENCE_UCE))
    {
        strCapabilities.Append("PRESENCE_UCE ");
    }

    strCapabilities.Append("] ");

    return strCapabilities;
}

PRIVATE
void AosService::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);
    Attach();
}

PRIVATE
void AosService::CleanUp()
{
    IMS_TRACE_I("CleanUp", 0, 0, 0);
}

PRIVATE
void AosService::Attach()
{
    JniEnablerConnector::GetInstance().SetNativeEnabler(m_nSlotId, EnablerType::AOS_SERVICE, this);
}

PRIVATE
IJniAosServiceThread* AosService::GetJniThread()
{
    IJniEnabler* piJniEnabler =
            JniEnablerConnector::GetInstance().GetJniEnabler(m_nSlotId, EnablerType::AOS_SERVICE);
    if (piJniEnabler == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniAosServiceThread is null", 0, 0, 0);
        return IMS_NULL;
    }

    return reinterpret_cast<IJniAosServiceThread*>(piJniEnabler->GetJniThread());
}
