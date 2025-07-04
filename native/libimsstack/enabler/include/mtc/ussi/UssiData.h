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

#ifndef USSI_DATA_H_
#define USSI_DATA_H_

#include "AString.h"
#include "INodeList.h"
#include "ImsTypeDef.h"
#include "ussi/UssiDef.h"

class UssiData
{
public:
    class AnyExtension
    {
    public:
        inline AnyExtension() :
                m_eUssiModeType(UssiModeType::NONE),
                m_nAlertingPattern(-1)
        {
        }
        inline virtual ~AnyExtension() {}
        AnyExtension(IN const AnyExtension&) = delete;
        AnyExtension& operator=(IN const AnyExtension&) = delete;

    public:
        inline virtual UssiModeType GetUssiModeType() const { return m_eUssiModeType; }
        inline virtual IMS_SINT32 GetAlertingPattern() const { return m_nAlertingPattern; }

    private:
        friend class UssiData;

        UssiModeType m_eUssiModeType;
        // unsignedByte. only in network initiated USSD request or USSD notification.
        IMS_SINT32 m_nAlertingPattern;
    };

public:
    UssiData();
    virtual ~UssiData();
    UssiData(IN const UssiData&) = delete;
    UssiData& operator=(IN const UssiData&) = delete;

    inline virtual const AString& GetLanguage() const { return m_strLanguage; }
    inline virtual const AString& GetUssdString() const { return m_strUssdString; }
    inline virtual UssiError GetErrorCode() const { return m_eErrorCode; }
    virtual const AnyExtension& GetAnyExtension() const;
    virtual IMS_BOOL Parse(IN const AString& strUssiBody);

private:
    void CreateAnyExtension(IN const INode* piNode);

    AnyExtension objAnyExtension;
    AString m_strLanguage;
    AString m_strUssdString;
    UssiError m_eErrorCode;
};

class UssiDataParser
{
public:
    inline UssiDataParser() {}
    inline virtual ~UssiDataParser() {}
    UssiDataParser(IN const UssiDataParser&) = delete;
    UssiDataParser& operator=(IN const UssiDataParser&) = delete;

    inline virtual UssiData* Parse(IN const AString& strUssiBody)
    {
        UssiData* pData = new UssiData();
        if (!pData->Parse(strUssiBody))
        {
            delete pData;
            return IMS_NULL;
        }

        return pData;
    }
};

#endif
