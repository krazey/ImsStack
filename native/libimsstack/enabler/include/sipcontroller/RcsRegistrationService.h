/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef RCS_REGISTRATION_SERVICE_H_
#define RCS_REGISTRATION_SERVICE_H_

#include "ImsService.h"
#include "IMSTypeDef.h"
#include "IImsAosListener.h"
#include "ImsMap.h"

class IImsAos;
class IJniSipControllerServiceThread;

class RcsRegistrationService : public ImsService, public IImsAosListener
{
public:
    RcsRegistrationService(IN const AString& strAppName, IN const IMS_SINT32 nSlotId);
    RcsRegistrationService(IN IImsAos* piImsAos, IN const IMS_SINT32 nSlotId);
    virtual ~RcsRegistrationService();
    IMS_BOOL UpdateDelegateRegistration(IN IMS_UINTP nParam);
    IMS_BOOL TriggerDelegateDeregistration(void);

private:
    void RcsFeatureTags();
    IMS_BOOL isPreDefindedFeature(IN const AString& feature);
    void EnableAoS();
    IJniSipControllerServiceThread* GetJniThread();
    IMSList<AString> GetFeatureTags(IN IMS_UINT32 nFeatures);
    IMS_SINT32 GetReason(IN IMS_UINT32 nReason);

protected:
    virtual void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAos_Connecting() override;
    virtual void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Resumed() override;

private:
    IMS_SINT32 m_nSlotId;
    IImsAos* m_piImsAos;
    IMSMap<AString, IMS_UINT32> m_objDefinedFeatures;
    IMSList<AString> m_objCurrentFeatures;
};
#endif  // RCS_REGISTRATION_SERVICE_H_