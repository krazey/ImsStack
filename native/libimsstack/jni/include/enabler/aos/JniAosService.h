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
#ifndef JNI_AOS_SERVICE_H_
#define JNI_AOS_SERVICE_H_

#include "BaseService.h"

class IJniEnablerThread;
class JniAosServiceThread;
class IAosService;

class JniAosService : public BaseService
{
public:
    JniAosService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId);
    ~JniAosService() override;

    virtual int SendData(const android::Parcel& objParcel) override;

    void Initialize(IN Jni_SendDataToJava pfnSendDataToJava);

    IJniEnablerThread* GetJniThread() const override;

private:
    virtual void HandleMessage(IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel) override;
    void Attach();
    IAosService* GetNativeService();
    void UpdateSipDelegateRegistration(IN const android::Parcel& objParcel);
    void TriggerSipDelegateDeregistration(IN const android::Parcel& objParcel);
    void TriggerFullNetworkRegistration(IN const android::Parcel& objParcel);
    void NotifyCapabilitiesChanged(IN const android::Parcel& objParcel);
    void ControlRegistration(IN const android::Parcel& objParcel);
    void UpdateDataFailureReason(IN const android::Parcel& objParcel);

    void NotifyAirplaneSetting(IN const android::Parcel& objParcel);
    void NotifyDataRoamingSetting(IN const android::Parcel& objParcel);
    void NotifyMobileDataSetting(IN const android::Parcel& objParcel);
    void NotifyRoamingPreferredVoiceNetwork(IN const android::Parcel& objParcel);
    void NotifyServiceSetting(IN const android::Parcel& objParcel);
    void NotifyTtySetting(IN const android::Parcel& objParcel);
    void NotifyVideoSetting(IN const android::Parcel& objParcel);
    void NotifyVolteSetting(IN const android::Parcel& objParcel);
    void NotifyWfcSetting(IN const android::Parcel& objParcel);

    void NotifyAosStart(IN const android::Parcel& objParcel);
    void NotifyIpcanHandoverFailure(IN const android::Parcel& objParcel);
    void NotifyIsimState(IN const android::Parcel& objParcel);
    void NotifyLocationInfo(IN const android::Parcel& objParcel);
    void NotifyMobileDataLimit(IN const android::Parcel& objParcel);
    void NotifyNetworkVideoCapability(IN const android::Parcel& objParcel);
    void NotifyPhoneNumberState(IN const android::Parcel& objParcel);
    void NotifyPlmnChanged(IN const android::Parcel& objParcel);
    void NotifyPowerOff(IN const android::Parcel& objParcel);
    void NotifyPreciseCallState(IN const android::Parcel& objParcel);
    void NotifyCarrierSignalPcoValueChanged(IN const android::Parcel& objParcel);
    void NotifyEmergencyCallbackModeChanged(IN const android::Parcel& objParcel);

    static void ConvertString(IN const android::String16& strSource, OUT AString& strDest);

private:
    JniAosServiceThread* m_pJniAosServiceThread;
};

#endif  // JNI_AOS_SERVICE_H_