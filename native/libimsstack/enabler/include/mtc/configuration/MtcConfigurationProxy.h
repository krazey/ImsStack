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

#ifndef MTC_CONFIGURATION_PROXY_H_
#define MTC_CONFIGURATION_PROXY_H_

#include "AString.h"
#include "CarrierConfig.h"
#include "ImsTypeDef.h"
#include "ImsVector.h"
#include "configuration/ConfigCache.h"
#include "configuration/ConfigDef.h"
#include <memory>
#include <optional>
#include <variant>

class ConfigCache;
class ICarrierConfig;

using ConfigAssets = CarrierConfig::Assets;
using ConfigEmergency = CarrierConfig::ImsEmergency;
using ConfigIms = CarrierConfig::Ims;
using ConfigRtt = CarrierConfig::ImsRtt;
using ConfigVoice = CarrierConfig::ImsVoice;
using ConfigVt = CarrierConfig::ImsVt;
using ConfigWfc = CarrierConfig::ImsWfc;

class MtcConfigurationProxy
{
public:
    explicit MtcConfigurationProxy();
    virtual ~MtcConfigurationProxy();
    MtcConfigurationProxy(IN const MtcConfigurationProxy&) = delete;
    MtcConfigurationProxy& operator=(IN const MtcConfigurationProxy&) = delete;

    virtual IMS_BOOL GetBoolean(IN const IMS_CHAR* pszKey) const;
    virtual IMS_SINT32 GetInt(IN const IMS_CHAR* pszKey) const;
    virtual AString GetString(IN const IMS_CHAR* pszKey) const;
    virtual ImsVector<IMS_SINT32> GetIntArray(IN const IMS_CHAR* pszKey) const;
    virtual ImsVector<AString> GetStringArray(IN const IMS_CHAR* pszKey) const;

    virtual IMS_SINT32 GetIntFromArray(IN const IMS_CHAR* pszKey, IN const IMS_UINT32 nIndex) const;
    virtual AString GetStringFromArray(IN const IMS_CHAR* pszKey, IN const IMS_UINT32 nIndex) const;

    virtual IMS_BOOL Contains(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue) const;
    virtual IMS_BOOL Contains(IN const IMS_CHAR* pszKey, IN const IMS_CHAR* pszValue) const;

    virtual void PutCache(IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue);
    virtual void PutCache(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue);
    virtual void PutCache(IN const IMS_CHAR* pszKey, IN const IMS_CHAR* pszValue);

    virtual void OnRegistrationRefreshed();

private:
    std::optional<ConfigValue> GetCachedValue(const IMS_CHAR* pszKey) const;
    void PutCache(IN const IMS_CHAR* pszKey, IN const ConfigValue& objValue);
    IMS_BOOL ValidateSuffix(const IMS_CHAR* pszKey, const IMS_CHAR* pszSuffix) const;

    std::unique_ptr<ConfigCache> m_pCache;
    ICarrierConfig* m_piCc;
};

#endif
