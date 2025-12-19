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
#include "ImsTypeDef.h"
#include <optional>
#include <variant>

using ConfigValue = std::variant<IMS_SINT32, IMS_BOOL, AString>;

class ConfigCache final
{
public:
    inline explicit ConfigCache() {}
    inline ~ConfigCache() = default;
    ConfigCache(IN const ConfigCache&) = delete;
    ConfigCache& operator=(IN const ConfigCache&) = delete;

    inline void PutCache(IN const IMS_CHAR* pszKey, const ConfigValue& objValue)
    {
        objConfigMap.Add(pszKey, objValue);
    }

    inline std::optional<ConfigValue> GetCache(const IMS_CHAR* pszKey) const
    {
        if (objConfigMap.GetIndexOfKey(pszKey) > -1)
        {
            return objConfigMap.GetValue(pszKey);
        }
        return std::nullopt;
    }

private:
    ImsMap<const IMS_CHAR*, ConfigValue> objConfigMap;
};

#endif
