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

#include "utility/MtsSipFormUtils.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsConfigurationManager::MtsConfigurationManager() :
        m_objCarrierConfig(MtsCarrierConfigItems()),
        m_objAsset(MtsAssetItems())
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

    UpdateMtsCarrierConfig(piCc);
    UpdateMtsAsset(piCc);
}

PUBLIC
void MtsConfigurationManager::UpdateMtsCarrierConfig(IN const ICarrierConfig* piCc)
{
    IMS_TRACE_I("UpdateMtsCarrierConfig", 0, 0, 0);

    m_objCarrierConfig.nRequestUriType =
            piCc->GetInt(CarrierConfig::Ims::KEY_REQUEST_URI_TYPE_INT);
    m_objCarrierConfig.nPolicyOfLocalNumbers =
            piCc->GetInt(CarrierConfig::ImsVoice::KEY_POLICY_OF_LOCAL_NUMBERS_INT);
    m_objCarrierConfig.bSmsCsfbRetryOnFailure =
            piCc->GetInt(CarrierConfig::ImsSms::KEY_SMS_CSFB_RETRY_ON_FAILURE_BOOL);
    m_objCarrierConfig.nSmsOverImsFormat =
            piCc->GetBoolean(CarrierConfig::ImsSms::KEY_SMS_OVER_IMS_FORMAT_INT);
}

PUBLIC
void MtsConfigurationManager::UpdateMtsAsset(IN const ICarrierConfig* piCc)
{
    IMS_TRACE_I("UpdateMtsAsset", 0, 0, 0);

    m_objAsset.nSmsRequestUriType =
            piCc->GetInt(CarrierConfig::Assets::KEY_SMS_REQUEST_URI_TYPE_INT);
    m_objAsset.bUseDialedNumber =
            piCc->GetBoolean(CarrierConfig::Assets::KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL);
}

PUBLIC VIRTUAL void MtsConfigurationManager::CarrierConfig_NotifyConfigChanged(
        IN IMS_SINT32 nSlotId)
{
    if (nSlotId == ThreadService::GetCurrentSlotId())
    {
        ICarrierConfig* piCc = ConfigService::GetConfigService()->GetCarrierConfig(
                ThreadService::GetCurrentSlotId());
        UpdateMtsCarrierConfig(piCc);
        UpdateMtsAsset(piCc);
    }
}

// IMS Public Carrier Config
PUBLIC
IMS_SINT32 MtsConfigurationManager::GetRequestUriType() const
{
    if (m_objAsset.nSmsRequestUriType == MtsSipFormUtils::SCHEME_UNKNOWN)
    {
        return m_objCarrierConfig.nRequestUriType;
    }
    else
    {
        return m_objAsset.nSmsRequestUriType;
    }
}

PUBLIC
IMS_SINT32 MtsConfigurationManager::GetPolicyOfLocalNumbers() const
{
    return m_objCarrierConfig.nPolicyOfLocalNumbers;
}

// Carrier Config
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

PUBLIC
IMS_BOOL MtsConfigurationManager::IsUseDialedNumber() const
{
    return m_objAsset.bUseDialedNumber;
}
