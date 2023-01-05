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
#ifndef CONFIG_SECTION_DATA_H_
#define CONFIG_SECTION_DATA_H_

#include "conf/ConfigComment.h"

class ConfigSectionData
{
public:
    explicit ConfigSectionData(IN const AString& strKey);
    ConfigSectionData(IN const AString& strKey, IN const AString& strValue);
    inline ~ConfigSectionData() {}

    ConfigSectionData(IN const ConfigSectionData&) = delete;
    ConfigSectionData& operator=(IN const ConfigSectionData&) = delete;

public:
    inline void AddComment(IN const AString& strComment) { m_objComment.Add(strComment); }
    inline const AString& GetKey() const { return m_strKey; }
    inline const AString& GetValue() const { return m_strValue; }
    inline void SetValue(IN const AString& strValue) { m_strValue = strValue; }
    AString ToString() const;

private:
    AString m_strKey;
    AString m_strValue;
    ConfigComment m_objComment;
};

#endif
