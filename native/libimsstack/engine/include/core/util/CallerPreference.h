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
#ifndef CALLER_PREFERENCE_H_
#define CALLER_PREFERENCE_H_

#include "AString.h"

class AppConfig;
class CoreServiceConfig;
class Feature;
class FeatureSet;
class ISipConfigV;
class PreferenceHeader;

class CallerPreference
{
public:
    CallerPreference() = delete;

public:
    static IMS_BOOL CreateAcceptContactHeaders(IN const AppConfig* pAppConfig,
            IN const CoreServiceConfig* pServiceConfig, IN const ISipConfigV* piSipConfigV,
            OUT IMSList<PreferenceHeader*>& objHeaders);
    static IMS_SINT32 GetCandidateScore(IN const AppConfig* pAppConfig,
            IN const CoreServiceConfig* pServiceConfig,
            IN const IMSList<PreferenceHeader*>& objHeaders,
            IN const IMSList<FeatureSet*>& objExtraFeatures);

private:
    static void AddFeature(IN const AString& strTag, OUT IMSList<PreferenceHeader*>& objHeaders);
    static void AddFeature(IN const AString& strTag, IN const AString& strValue,
            OUT IMSList<PreferenceHeader*>& objHeaders);
    static void AddFeature(IN const AString& strTag, IN const AString& strValue,
            IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire,
            OUT IMSList<PreferenceHeader*>& objHeaders);
    static void AddFeature(IN IMS_BOOL bBooleanFeature, IN const AString& strTag,
            IN const AString& strValue, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire,
            OUT IMSList<PreferenceHeader*>& objHeaders);

public:
    enum
    {
        SCORE_INVALID = -1,
        SCORE_CALLEE_PREFERENCE = 100
    };
};

#endif
