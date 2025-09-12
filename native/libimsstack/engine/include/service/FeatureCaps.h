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
#ifndef FEATURE_CAPS_H_
#define FEATURE_CAPS_H_

#include "ImsMap.h"

#include "IFeatureCaps.h"

class CallerCapability;

class FeatureCaps : public IFeatureCaps
{
public:
    FeatureCaps();
    ~FeatureCaps() override;

    FeatureCaps(IN const FeatureCaps&) = delete;
    FeatureCaps& operator=(IN const FeatureCaps&) = delete;

public:
    // IFeatureCaps class
    void AddFeature(IN const AString& strName, IN const AString& strValue) override;
    void AddFeature(IN const AString& strName, IN const AString& strValue, IN IMS_SINT32 nSipMethod,
            IN IMS_SINT32 nMessageType = ISipMessage::TYPE_ANY) override;
    void RemoveFeature(IN const AString& strName, IN const AString& strValue) override;
    void RemoveFeature(IN const AString& strName, IN const AString& strValue,
            IN IMS_SINT32 nSipMethod, IN IMS_SINT32 nMessageType = ISipMessage::TYPE_ANY) override;
    void RemoveAllFeatures() override;

    void AddExcludedFeatureForRegCaps(
            IN const AString& strName, IN const AString& strValue) override;
    void RemoveExcludedFeatureForRegCaps(
            IN const AString& strName, IN const AString& strValue) override;
    void RemoveAllExcludedFeaturesForRegCaps() override;

    IMS_BOOL FormContactFeatures(
            IN IMS_SINT32 nSipMethod, IN IMS_BOOL bRequest, OUT AString& strContactFeatures);
    void UpdateRegCaps(IN CallerCapability* pRegCaps);

private:
    CallerCapability* GetExcludedFeaturesForRegCaps(IN IMS_BOOL bCreate = IMS_FALSE);
    CallerCapability* GetFeaturesForAllMessage(IN IMS_BOOL bCreate = IMS_FALSE);
    CallerCapability* GetFeaturesForRequest(
            IN IMS_SINT32 nSipMethod, IN IMS_BOOL bCreate = IMS_FALSE);
    CallerCapability* GetFeaturesForResponse(
            IN IMS_SINT32 nSipMethod, IN IMS_BOOL bCreate = IMS_FALSE);
    IMS_BOOL HasAdditionalFeatures(IN IMS_SINT32 nSipMethod, IN IMS_BOOL bRequest);

private:
    CallerCapability* m_pExcludedFeaturesForRegCaps;
    CallerCapability* m_pFeaturesForAllMessage;
    ImsMap<IMS_SINT32, CallerCapability*>* m_pFeaturesForRequest;
    ImsMap<IMS_SINT32, CallerCapability*>* m_pFeaturesForResponse;

    CallerCapability* m_pRegCaps;
};

#endif
