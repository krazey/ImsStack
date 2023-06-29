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
#define IMS_STL_USE

#include <utils/String8.h>
#include "ServiceTrace.h"
#include "ImsProcess.h"
#include "JniAosServiceThread.h"
#include "JniAosService.h"
#include "JniEnablerConnector.h"
#include "IIAosService.h"
#include "IAosService.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.AOS");

JniAosService::JniAosService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId) :
        BaseService(nSlotId),
        m_pJniAosServiceThread(IMS_NULL)
{
    IMS_TRACE_D("+JniAosService SlotId[%d]", nSlotId, 0, 0);

    Initialize(pfnSendDataToJava);
}

JniAosService::~JniAosService()
{
    IMS_TRACE_D("~JniAosService SlotId[%d]", GetSlotId(), 0, 0);

    JniEnablerConnector::GetInstance().SetJniEnabler(
            GetSlotId(), EnablerType::AOS_SERVICE, IMS_NULL);

    if (m_pJniAosServiceThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_pJniAosServiceThread->GetName());
        m_pJniAosServiceThread = IMS_NULL;
    }
}

PUBLIC
int JniAosService::SendData(const Parcel& objParcel)
{
    int nMessage = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMessage))
    {
        SendDataUsingEnablerThread(objParcel);
    }
    else
    {
        HandleMessage(nMessage, objParcel);
    }

    return 1;
}

PUBLIC
void JniAosService::Initialize(IN Jni_SendDataToJava pfnSendDataToJava)
{
    if (pfnSendDataToJava == NULL)
    {
        return;
    }

    AString strThreadName;
    strThreadName.Sprintf("JniAosServiceThread_%d", GetSlotId());

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniAosServiceThread();
    };

    ImsProcess::GetInstance()->LoadThread(strThreadName, fnEntry, GetSlotId());
    m_pJniAosServiceThread =
            DYNAMIC_CAST(JniAosServiceThread*, ImsProcess::GetInstance()->GetThread(strThreadName));

    if (m_pJniAosServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniAosService : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniAosServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnSendDataToJava);

    Attach();
}

PUBLIC VIRTUAL IJniEnablerThread* JniAosService::GetJniThread() const
{
    return DYNAMIC_CAST(IJniEnablerThread*, m_pJniAosServiceThread);
}

PRIVATE VIRTUAL void JniAosService::HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IIAosService::J2N_REQUEST_REGISTRATION:
            UpdateSipDelegateRegistration(objParcel);
            break;

        case IIAosService::J2N_REQUEST_DEREGISTRATION:
            TriggerSipDelegateDeregistration(objParcel);
            break;

        case IIAosService::J2N_REQUEST_FULL_REGISTRATION:
            TriggerFullNetworkRegistration(objParcel);
            break;

        case IIAosService::J2N_REQUEST_CAPABILITIES_CHANGED:
            NotifyCapabilitiesChanged(objParcel);
            break;

        case IIAosService::J2N_REQUEST_CONTROL_REGISTRATION:
            ControlRegistration(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_AIRPLANE_SETTING:
            NotifyAirplaneSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_DATA_ROAMING_SETTING:
            NotifyDataRoamingSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_MOBILE_DATA_SETTING:
            NotifyMobileDataSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK:
            NotifyRoamingPreferredVoiceNetwork(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_SERVICE_SETTING:
            NotifyServiceSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_TTY_SETTING:
            NotifyTtySetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_VIDEO_SETTING:
            NotifyVideoSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_VOLTE_SETTING:
            NotifyVolteSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_WFC_SETTING:
            NotifyWfcSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_AOS_START:
            NotifyAosStart(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_IPCAN_HANDOVER_FAILURE:
            NotifyIpcanHandoverFailure(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_ISIM_STATE:
            NotifyIsimState(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_LOCATION_INFO:
            NotifyLocationInfo(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_MOBILE_DATA_LIMIT:
            NotifyMobileDataLimit(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY:
            NotifyNetworkVideoCapability(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_PHONE_NUMBER_STATE:
            NotifyPhoneNumberState(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_PLMN_CHANGED:
            NotifyPlmnChanged(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_POWER_OFF:
            NotifyPowerOff(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_PRECISE_CALL_STATE:
            NotifyPreciseCallState(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_CARRIER_SIGNAL_PCO_VALUE_CHANGED:
            NotifyCarrierSignalPcoValueChanged(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_EMC_CALLBACK_MODE_CHANGED:
            NotifyEmcCallbackModeChanged(objParcel);
            break;

        default:
            break;
    }
}

PRIVATE
void JniAosService::Attach()
{
    IMS_TRACE_I("Attach()", 0, 0, 0);
    JniEnablerConnector::GetInstance().SetJniEnabler(GetSlotId(), EnablerType::AOS_SERVICE, this);
}

PRIVATE
IAosService* JniAosService::GetNativeService()
{
    return DYNAMIC_CAST(IAosService*,
            JniEnablerConnector::GetInstance().GetNativeEnabler(
                    GetSlotId(), EnablerType::AOS_SERVICE));
}

PRIVATE
void JniAosService::UpdateSipDelegateRegistration(IN const Parcel& /*objParcel*/)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->UpdateSipDelegateRegistration();
    }
}

PRIVATE
void JniAosService::TriggerSipDelegateDeregistration(IN const Parcel& /*objParcel*/)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->TriggerSipDelegateDeregistration();
    }
}

PRIVATE
void JniAosService::TriggerFullNetworkRegistration(IN const Parcel& objParcel)
{
    IMS_SINT32 nSipCode = objParcel.readInt32();
    AString strSipReason;
    ConvertString(objParcel.readString16(), strSipReason);

    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->TriggerFullNetworkRegistration(nSipCode, strSipReason);
    }
}

PRIVATE
void JniAosService::NotifyCapabilitiesChanged(IN const Parcel& objParcel)
{
    ImsMap<IMS_UINT32, IMS_UINT32> objCapabilities;

    IMS_SINT32 nSize = objParcel.readInt32();
    for (int i = 0; i < nSize; ++i)
    {
        objCapabilities.Add(objParcel.readInt32(), objParcel.readInt32());
    }

    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyCapabilitiesChanged(objCapabilities);
    }
}

PRIVATE
void JniAosService::ControlRegistration(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->ControlRegistration(
                objParcel.readInt32(), objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyAirplaneSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyAirplaneSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyDataRoamingSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyDataRoamingSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyMobileDataSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyMobileDataSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyRoamingPreferredVoiceNetwork(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyRoamingPreferredVoiceNetwork(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyServiceSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyServiceSetting(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyTtySetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyTtySetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyVideoSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyVideoSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyVolteSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyVolteSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyWfcSetting(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyWfcSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyAosStart(IN const android::Parcel& /*objParcel*/)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyAosStart();
    }
}

PRIVATE
void JniAosService::NotifyIpcanHandoverFailure(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyIpcanHandoverFailure(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyIsimState(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyIsimState(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyLocationInfo(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyLocationInfo(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyMobileDataLimit(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyMobileDataLimit(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyNetworkVideoCapability(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyNetworkVideoCapability(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyPhoneNumberState(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyPhoneNumberState(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyPlmnChanged(IN const android::Parcel& /*objParcel*/)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyPlmnChanged();
    }
}

PRIVATE
void JniAosService::NotifyPowerOff(IN const android::Parcel& /*objParcel*/)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyPowerOff();
    }
}

PRIVATE
void JniAosService::NotifyPreciseCallState(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyPreciseCallState(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyCarrierSignalPcoValueChanged(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyCarrierSignalPcoValueChanged(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyEmcCallbackModeChanged(IN const android::Parcel& objParcel)
{
    IAosService* piAosService = GetNativeService();
    if (piAosService)
    {
        piAosService->NotifyEmcCallbackModeChanged(
                objParcel.readInt32(), objParcel.readInt32(), objParcel.readInt64());
    }
}

PRIVATE GLOBAL void JniAosService::ConvertString(IN const String16& strSource, OUT AString& strDest)
{
    String8 str8(strSource);
    strDest = str8.string();
}
