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
#ifndef PREFERENCE_HEADER_H_
#define PREFERENCE_HEADER_H_

#include "AStringArray.h"

class FeatureSet;
class ISipHeader;

class PreferenceHeader
{
public:
    explicit PreferenceHeader(IN const AString& strHeader);
    explicit PreferenceHeader(IN const ISipHeader* piHeader);
    PreferenceHeader(IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire);
    ~PreferenceHeader();

    PreferenceHeader(IN const PreferenceHeader&) = delete;
    PreferenceHeader& operator=(IN const PreferenceHeader&) = delete;

public:
    IMS_BOOL AddFeature(IN const AString& strTag);
    IMS_BOOL AddFeature(IN const AString& strTag, IN const AString& strValue);
    IMS_BOOL Contains(IN const AString& strTag) const;
    IMS_BOOL Contains(IN const AString& strTag, IN const AString& strValue) const;
    inline const IMSList<FeatureSet*>& GetFeatureSets() const { return m_objPreferenceFeatures; }
    inline IMS_BOOL IsExplicitPresent() const { return m_bExplicit; }
    inline IMS_BOOL IsRequirePresent() const { return m_bRequire; }
    AString ToString() const;

private:
    IMS_BOOL Attach(IN const AString& strTag, IN const AString& strValue = AString::ConstNull());
    void Detach(IN const AString& strTag);
    FeatureSet* Lookup(IN const AString& strTag) const;

    void ExtractProperties(IN const AString& strFeatureSet);

private:
    IMSList<FeatureSet*> m_objPreferenceFeatures;
    IMS_BOOL m_bExplicit;
    IMS_BOOL m_bRequire;
};

#endif
