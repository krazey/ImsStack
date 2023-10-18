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
#ifndef CALLER_CAPABILITY_H_
#define CALLER_CAPABILITY_H_

#include "AString.h"

class AppConfig;
class Feature;
class FeatureSet;
class ICoreServiceConfig;
class ISipConfigV;

class CallerCapability
{
public:
    explicit CallerCapability(IN IMS_UINT32 nId);
    ~CallerCapability();

    CallerCapability(IN const CallerCapability&) = delete;
    CallerCapability& operator=(IN const CallerCapability&) = delete;

public:
    IMS_BOOL AddFeature(IN const Feature* pFeature);
    IMS_BOOL AddFeature(IN const FeatureSet* pFeatureSet);
    IMS_BOOL AddFeatures(IN const CallerCapability* pCc);
    void Clear();
    IMS_BOOL Create(IN const AppConfig* pAppConfig, IN const ICoreServiceConfig* piServiceConfig,
            IN const ISipConfigV* piSipConfigV);
    IMS_BOOL Equals(IN const CallerCapability* pCc) const;
    inline const ImsList<FeatureSet*>& GetFeatures() const { return m_objContactFeatures; }
    IMS_BOOL HasFeature(IN const Feature* pFeature) const;
    inline IMS_BOOL IsEmpty() const { return m_objContactFeatures.IsEmpty(); }
    IMS_BOOL RemoveFeature(IN const Feature* pFeature);
    IMS_BOOL RemoveFeature(IN const FeatureSet* pFeatureSet);
    IMS_BOOL RemoveFeatures(IN const CallerCapability* pCc, IN IMS_BOOL bRemoveRef = IMS_TRUE);
    AString ToString() const;

private:
    IMS_BOOL Attach(IN const AString& strTag, IN const AString& strValue = AString::ConstNull());
    void Detach(IN const AString& strTag);
    FeatureSet* Lookup(IN const AString& strTag) const;

    IMS_SINT32 AddFeature(IN const AString& strTag);
    IMS_SINT32 AddFeature(IN const AString& strTag, IN const AString& strValue);
    IMS_SINT32 RemoveFeature(IN const AString& strTag);
    IMS_SINT32 RemoveFeature(IN const AString& strTag, IN const AString& strValue);

private:
    enum
    {
        FEATURE_UNCHANGED = 0x01,
        FEATURE_CHANGED = 0x02
    };

    IMS_UINT32 m_nId;
    ImsList<FeatureSet*> m_objContactFeatures;
};

#endif
