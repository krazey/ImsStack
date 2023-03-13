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
#ifndef CONFIG_SECTION_H_
#define CONFIG_SECTION_H_

#include "conf/ConfigComment.h"
#include "conf/ConfigSectionData.h"

class ConfigSection
{
public:
    ConfigSection();
    ~ConfigSection();

    ConfigSection(IN const ConfigSection&) = delete;
    ConfigSection& operator=(IN const ConfigSection&) = delete;

public:
    inline void AddComment(IN const AString& strComment) { m_objComment.Add(strComment); }
    inline const AString& GetName() const { return m_strSectionName; }
    void GetKeys(OUT AStringArray& objKeys) const;
    const AString& GetValue(IN const IMS_CHAR* pszKey) const;
    IMS_BOOL SetValue(IN const IMS_CHAR* pszKey, IN const AString& strValue);
    AString ToString() const;

private:
    IMS_BOOL AddSectionData(IN const AString& strKeyValue);
    ConfigSectionData* GetLastElement() const;
    inline void SetName(IN const AString& strSectName) { m_strSectionName = strSectName; }

private:
    friend class ConfigFileBuffer;

    AString m_strSectionName;
    ImsList<ConfigSectionData*> m_objSectionData;

    ConfigComment m_objComment;
};

#endif
