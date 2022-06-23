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

#ifndef CONFIG_CACHE_H_
#define CONFIG_CACHE_H_

#include "AString.h"
#include "ImsMap.h"
#include "ConfigDef.h"

class ConfigCache final
{
public:
    explicit ConfigCache();
    ~ConfigCache();
    ConfigCache(IN const ConfigCache&) = delete;
    ConfigCache& operator=(IN const ConfigCache&) = delete;

    // no additional info is allowed.
    void PutCache(IN Feature eFeature, IN IMS_BOOL bValue);
    void PutCache(IN Feature eFeature, IN IMS_SINT32 nValue);
    void PutCache(IN Feature eFeature, const IN AString& strValue);

    IMS_BOOL ResetCache(IN Feature eFeature);  // necessary?

    IMS_BOOL GetBooleanCache(IN Feature eFeature) const;
    IMS_SINT32 GetIntegerCache(IN Feature eFeature) const;
    const AString GetStringCache(IN Feature eFeature) const;

    IMS_BOOL HasBooleanCache(IN Feature eFeature) const;
    IMS_BOOL HasIntegerCache(IN Feature eFeature) const;
    IMS_BOOL HasStringCache(IN Feature eFeature) const;
    IMS_BOOL IsEmpty() const;  // necessary?

private:
    IMSMap<Feature, IMS_BOOL> m_objBooleanCache;
    IMSMap<Feature, IMS_SINT32> m_objIntegerCache;
    IMSMap<Feature, AString> m_objStringCache;
};

#endif
