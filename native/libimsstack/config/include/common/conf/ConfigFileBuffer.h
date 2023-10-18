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
#ifndef CONFIG_FILE_BUFFER_H_
#define CONFIG_FILE_BUFFER_H_

#include "ConfigBuffer.h"
#include "conf/ConfigSection.h"

class ConfigFileBuffer : public ConfigBuffer
{
public:
    ConfigFileBuffer(IN const AString& strLocator, IN const AString& strName);
    virtual ~ConfigFileBuffer();

    ConfigFileBuffer(IN const ConfigFileBuffer&) = delete;
    ConfigFileBuffer& operator=(IN const ConfigFileBuffer&) = delete;

private:
    ConfigFileBuffer();

public:
    static IConfigBuffer* CreateFileBuffer(IN const AString& strConfigData);

protected:
    // IConfigBuffer interface
    void Destroy() override;

    IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName) override;
    IMS_BOOL CaptureSection(IN const IMS_CHAR* pszSectName, IN IMS_SINT32 nIndex) override;
    void ReleaseSection() override;

    IMS_SINT32 ReadKeyCount(IN const IMS_CHAR* pszKey) const override;
    const AString& ReadValue(IN const IMS_CHAR* pszKey) const override;
    const AString& ReadValue(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex) const override;
    IMS_BOOL ReadValueBoolean(IN const IMS_CHAR* pszKey) const override;
    IMS_SINT32 ReadValueInt(IN const IMS_CHAR* pszKey) const override;

    IMS_BOOL WriteKeyCount(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nCount) override;
    IMS_BOOL WriteValue(IN const IMS_CHAR* pszKey, IN const AString& strValue) override;
    IMS_BOOL WriteValue(
            IN const IMS_CHAR* pszKey, IN IMS_SINT32 nIndex, IN const AString& strValue) override;
    IMS_BOOL WriteValueBoolean(IN const IMS_CHAR* pszKey, IN IMS_BOOL bValue) override;
    IMS_BOOL WriteValueInt(IN const IMS_CHAR* pszKey, IN IMS_SINT32 nValue) override;

    IMS_BOOL WriteToMedium() const override;

    // ConfigBuffer class
    IMS_BOOL Create(IN IMS_SINT32 nId) override;

private:
    void FormConfig(OUT AString& strConfigData) const;
    IMS_BOOL ParseConfig(IN const AString& strConfigData);
    AString ResolveLocator() const;

private:
    static const IMS_CHAR FILE_EXTENSION[];

    IMS_UINT32 m_nIndexOfWorkSection;
    ConfigSection* m_pWorkSection;

    ImsList<ConfigSection*> m_objSections;
    ConfigComment m_objStartComment;
    ConfigComment m_objEndComment;
};

#endif
