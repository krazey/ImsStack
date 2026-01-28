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
#ifndef OS_UTIL_H_
#define OS_UTIL_H_

#include "ISystemUtil.h"
#include "ISystemProperty.h"
#include "IZLib.h"

class OsUtil : public ISystemUtil, public ISystemProperty, public IZLib
{
public:
    OsUtil();
    ~OsUtil() override = default;

    OsUtil(IN const OsUtil&) = delete;
    OsUtil& operator=(IN const OsUtil&) = delete;

public:
    void SetDebugOn(IN IMS_BOOL bDebugOn);

    // ISystemProperty class
    const AString& GetChipsetVendor() const override;
    IMS_BOOL IsDebugMode() const override;
    IMS_BOOL IsServerInfoHiddenInLog() const override;
    IMS_BOOL IsUserMode() const override;

    static OsUtil* GetInstance();

private:
    // ISystemUtil class
    void GetUuid(IN IMS_SINT32 nVersion, OUT AString& strUuid,
            IN const AString& strName = AString::ConstNull()) override;

    // ISystemProperty class
    AString Get(IN const AString& strName) override;
    IMS_BOOL Set(IN const AString& strName, IN const AString& strValue) override;

    // IZLib class
    IMS_BOOL Compress(IN const ByteArray& objData, OUT ByteArray& objCompData) override;
    IMS_BOOL Uncompress(IN const ByteArray& objCompData, OUT ByteArray& objData) override;

private:
    static constexpr IMS_SINT32 BUILD_TYPE_UNKNOWN = -1;
    static constexpr IMS_SINT32 BUILD_TYPE_USER = 1;
    static constexpr IMS_SINT32 BUILD_TYPE_DEBUG = 2;

    // READ-ONLY properties
    mutable IMS_SINT32 m_nBuildType;
    // READ-WRITE properties
    IMS_BOOL m_bImsDebugOn;
    // Privacy mode when logging
    IMS_BOOL m_bHidePrivacyLog;
    // Modem chipset vendor
    AString m_strChipsetVendor;
};

#endif
