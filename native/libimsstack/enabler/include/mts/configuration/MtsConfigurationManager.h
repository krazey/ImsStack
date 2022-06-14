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

#ifndef MTS_CONFIGURATION_MANAGER_H_
#define MTS_CONFIGURATION_MANAGER_H_

#include "configuration/MtsCarrierConfigItems.h"
#include "ICarrierConfigListener.h"

class ICarrierConfig;

class MtsConfigurationManager final : public ICarrierConfigListener
{
public:
    MtsConfigurationManager();
    ~MtsConfigurationManager();
    MtsConfigurationManager(IN const MtsConfigurationManager&) = delete;
    MtsConfigurationManager& operator=(IN const MtsConfigurationManager&) = delete;

    void Init();
    void UpdateMtsConfig(IN const ICarrierConfig* piCc);

    // ICarrierConfigListener
    void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId) override;

    // sms configurations
    IMS_BOOL IsSmsCsfbRetryOnFailure() const;  // KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL
    IMS_SINT32 GetSmsOverImsFormat() const;    // KEY_SMS_OVER_IMS_FORMAT_INT

private:
    MtsCarrierConfigItems m_objCarrierConfig;
};

#endif
