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

#include "configuration/MtsConfigurationManager.h"
#include "configuration/MtsCarrierConfigItems.h"
#include "CarrierConfig.h"
#include "ICarrierConfig.h"
#include "IMSTypeDef.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ServiceConfig.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsConfigurationManager::MtsConfigurationManager() :
        m_objCarrierConfig(MtsCarrierConfigItems())
{
}

PUBLIC
MtsConfigurationManager::~MtsConfigurationManager() {}

PUBLIC
void MtsConfigurationManager::Init()
{
    IMS_TRACE_I("Init", 0, 0, 0);
    ICarrierConfig* piCc =
            ConfigService::GetConfigService()->GetCarrierConfig(ThreadService::GetCurrentSlotId());
    piCc->AddListener(this);

    UpdateMtsConfig(piCc);
}

PUBLIC
void MtsConfigurationManager::UpdateMtsConfig(IN const ICarrierConfig* piCc)
{
    IMS_TRACE_I("UpdateMtsConfig", 0, 0, 0);

    m_objCarrierConfig.bSmsCsfbRetryOnFailure =
            piCc->GetInt(CarrierConfig::ImsSms::KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL);
    m_objCarrierConfig.nSmsOverImsFormat =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_FORMAT_INT);
}

PUBLIC VIRTUAL void MtsConfigurationManager::CarrierConfig_NotifyConfigChanged(
        IN IMS_SINT32 nSlotId)
{
    if (nSlotId == ThreadService::GetCurrentSlotId())
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(
                ThreadService::GetCurrentSlotId());
        UpdateMtsConfig(piCc);
    }
}

PUBLIC
IMS_BOOL MtsConfigurationManager::IsSmsCsfbRetryOnFailure() const
{
    return m_objCarrierConfig.bSmsCsfbRetryOnFailure;
}

PUBLIC
IMS_SINT32 MtsConfigurationManager::GetSmsOverImsFormat() const
{
    return m_objCarrierConfig.nSmsOverImsFormat;
}
