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
#include "ImsTypeDef.h"
#include "configuration/ConfigDef.h"
#include "configuration/IMtcConfigurationManager.h"
#include <memory>

class ConfigCache;
class IMtcConfigurationManager;

class MtcConfigurationProxy
{
public:
    MtcConfigurationProxy(IN IMtcConfigurationManager* pManager);
    ~MtcConfigurationProxy();
    MtcConfigurationProxy(IN const MtcConfigurationProxy&) = delete;
    MtcConfigurationProxy& operator=(IN const MtcConfigurationProxy&) = delete;

    void Init();

    IMS_BOOL Is(IN Feature eFeature) const;
    IMS_BOOL Is(IN Feature eFeature, IN const AString& strAdditionalInfo) const;
    IMS_BOOL Is(IN Feature eFeature, IN IMS_SINT32 nAdditionalInfo) const;
    IMS_SINT32 GetInt(IN Feature eFeature) const;
    IMS_SINT32 GetInt(IN Feature eFeature, IN IMS_BOOL bWParam, IN IMS_BOOL bLParam) const;
    const AString GetStr(IN Feature eFeature, IN IMS_SINT32 nAdditionalInfo) const;

    void PutConfigCache(IN Feature eFeature, IN IMS_SINT32 nValue);
    void PutConfigCache(IN Feature eFeature, IN IMS_BOOL bValue);
    void PutConfigCache(IN Feature eFeature, IN const AString& strValue);

    void OnRegistrationRefreshed();  // called by MtcService.

private:
    std::unique_ptr<IMtcConfigurationManager> m_pManager;
    ConfigCache* m_pCache;
};

#endif
