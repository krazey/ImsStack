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
#ifndef AOS_FEATURE_TAG_H_
#define AOS_FEATURE_TAG_H_

#include "IMSTypeDef.h"
#include "ImsList.h"
#include "AString.h"

#include "ImsAosParameter.h"

class AosFeatureTag
{
public:
    AosFeatureTag(IN const AString& strName, IN const AString& strValue = AString::ConstNull(),
            IN IMS_UINT32 nType = 0, IN IMS_UINT32 nOption = OPTION_HEADER_PARAMETER);
    virtual ~AosFeatureTag();

    void SetFeatureTag(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull());

    IMS_BOOL Equals(IN AosFeatureTag* pFeatureTag);
    IMS_BOOL Equals(IN const AString& strName, IN const AString& strValue = AString::ConstNull());

    AString& GetName();
    AString& GetValue();
    IMS_UINT32 GetType();
    IMS_UINT32 GetOption();

    enum
    {
        OPTION_HEADER_PARAMETER = 0,
        OPTION_EXTRA_CAPABILITY
    };

private:
    AString m_strName;
    AString m_strValue;
    IMS_UINT32 m_nType;
    IMS_UINT32 m_nOption;
};

class AosFeatureTagList
{
public:
    AosFeatureTagList();
    virtual ~AosFeatureTagList();

    IMS_BOOL AddFeatureTag(IN const AString& strName,
            IN const AString& strValue = AString::ConstNull(), IN IMS_UINT32 nType = 0,
            IN IMS_UINT32 nOption = AosFeatureTag::OPTION_HEADER_PARAMETER);
    IMS_BOOL RemoveFeatureTag(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull());

    void AddFeature(IN IMS_UINT32 nFeature);
    void RemoveFeature(IN IMS_UINT32 nFeature);

    void AddUnavailableFeature(IN IMS_UINT32 nFeature);
    void RemoveUnavailableFeature(IN IMS_UINT32 nFeature);

    IMS_UINT32 GetFeatures();
    IMS_UINT32 GetUnavailableFeatures();
    void ClearFeatures();
    void ClearFeatureTags();
    void Clear();

    IMS_BOOL Equals(IN AosFeatureTagList& objTargetList);
    void Copy(IN AosFeatureTagList& objSourceList);
    void CopyFeatures(IN AosFeatureTagList& objSourceList);
    void CopyFeatureTags(IN IMSList<ImsAosFeatureTag*>& objFeatureTag);

    IMS_UINT32 GetSize();
    AosFeatureTag* GetAt(IN IMS_UINT32 nIndex);

    IMS_BOOL HasUnavailableFeature(IN IMS_UINT32 nFeature) const;
    IMS_BOOL HasFeature(IN IMS_UINT32 nFeature) const;
    IMS_BOOL HasFeatureTag(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) const;

    void PrintFeatureTagList();

private:
    IMSList<AosFeatureTag*> m_objFeatureTagList;
    IMS_UINT32 m_nFeatures;
    IMS_UINT32 m_nUnavailableFeatures;

private:
    friend class AosFeatureTagTest;
};
#endif  // AOS_FEATURE_TAG_H_
