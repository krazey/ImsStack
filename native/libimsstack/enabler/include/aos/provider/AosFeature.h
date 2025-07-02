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
#ifndef AOS_FEATURE_H_
#define AOS_FEATURE_H_

#include "ServiceTrace.h"

class AosFeature
{
public:
    AosFeature() :
            nFeatures(FEATURE_NONE)
    {
        IMS_TRACE_MEM("AOS_MEM", "AOS_M : AosFeature = %" PFLS_u "/%" PFLS_x, sizeof(AosFeature),
                this, 0);
    }
    virtual ~AosFeature()
    {
        IMS_TRACE_MEM("AOS_MEM", "AOS_F : AosFeature = %" PFLS_u "/%" PFLS_x, sizeof(AosFeature),
                this, 0);
    }
    void EnableFeature(IN IMS_UINT32 nFeatures)
    {
        this->nFeatures |= nFeatures;
        OnFeatureEnabled(nFeatures);
    }
    void DisableFeature(IN IMS_UINT32 nFeatures)
    {
        this->nFeatures &= (~nFeatures);
        OnFeatureDisabled(nFeatures);
    }
    IMS_BOOL IsFeatureEnabled(IN IMS_UINT32 nFeature) { return (nFeatures & nFeature) == nFeature; }
    IMS_BOOL IsFeatureDisabled(IN IMS_UINT32 nFeature) { return !IsFeatureEnabled(nFeature); }
    void InitFeatures(IN IMS_UINT32 nFeatures) { this->nFeatures = nFeatures; }
    IMS_UINT32 GetFeatures() { return nFeatures; }
    virtual void OnFeatureEnabled(IN IMS_UINT32 nFeatures) { (void)nFeatures; }
    virtual void OnFeatureDisabled(IN IMS_UINT32 nFeatures) { (void)nFeatures; }

    enum
    {
        FEATURE_NONE = 0
    };

private:
    IMS_UINT32 nFeatures;
};

#endif  // AOS_FEATURE_H_
