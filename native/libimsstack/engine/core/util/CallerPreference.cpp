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
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Feature.h"
#include "ISipConfigV.h"
#include "ServiceIdentifier.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"

#include "util/CallerPreference.h"
#include "util/PreferenceHeader.h"

__IMS_TRACE_TAG_IMS_CORE__;

class PreferenceScore
{
public:
    inline PreferenceScore() :
            m_nNpf(0),
            m_nNcf(0),
            m_nNvm(0),
            m_nNvmForCs(0)
            //, m_nNfForCs(0)
            ,
            m_nScore(0)
    {
    }
    inline ~PreferenceScore() {}

public:
    inline void Clear()
    {
        m_nNpf = 0;
        m_nNcf = 0;
        m_nNvm = 0;
        m_nNvmForCs = 0;
        // m_nNfForCs = 0;
        m_nScore = 0;
    }
    inline IMS_SINT32 GetNpf() const { return m_nNpf; }
    inline IMS_SINT32 GetNvm() const { return m_nNvm; }
    inline IMS_SINT32 GetNvmCs() const { return m_nNvmForCs; }
    inline IMS_SINT32 GetScore() const { return m_nScore; }
    inline void IncreaseNpf() { ++m_nNpf; }
    inline void IncreaseNcf() { ++m_nNcf; }
    inline void IncreaseNvm() { ++m_nNvm; }
    inline void IncreaseNvmCs() { ++m_nNvmForCs; }
    inline void SetScore(IN IMS_SINT32 nScore) { m_nScore = nScore; }

    void ProcessAcceptContact(IN const AppConfig* pAppConfig,
            IN const CoreServiceConfig* pServiceConfig, IN const PreferenceHeader* pHeader,
            IN const ImsList<FeatureSet*>& objExtraFeatures);

    /**
     * @brief Finds a matched value between the Registry and Accept-Contact header.
     *
     * @param objVr The properties from Registry
     * @param pVa The feature sets from Accept-Contact header
     * @return true if one of them is matched, false otherwise.
     */
    static IMS_BOOL FindMatchFor(IN const AStringArray& objVr, IN const FeatureSet* pVa);

    /**
     * @brief Check if there are any matches between the Registry and Accept-Contact header.
     *
     * @param pVa The feature sets from Accept-Contact header
     * @param objVr The properties from Registry
     * @param bCaseSensitive The flag specifying whether the comparison is case-sensitive or not
     * @return true if the Registry contains any values of Accept-Contact header, false otherwise.
     */
    static IMS_BOOL IsIn(
            IN const FeatureSet* pVa, IN const AStringArray& objVr, IN IMS_BOOL bCaseSensitive);
    /**
     * @brief Check if there are any matches between the Registry and Accept-Contact header.
     *
     * The comparison is case-sensitive.
     *
     * @param pVa The feature sets from Accept-Contact header
     * @param objVr The properties from Registry
     * @return true if the Registry contains any values of Accept-Contact header, false otherwise.
     */
    static IMS_BOOL IsIn(IN const FeatureSet* pVa, IN const FeatureSet* pVr);

private:
    enum
    {
        SCORE_MULTIPLIER = 1000000
    };

    // Number of features in "Accept-Contact" header
    IMS_SINT32 m_nNpf;
    // Number of "Accept-Contact" feature tags where the Registry implies the same tag
    IMS_SINT32 m_nNcf;
    // Number of value matches between "Accept-Contact" and Registry for a feature tag
    IMS_SINT32 m_nNvm;
    // Number of matches for IARI, ICSI and Feature Tags fields in the CoreService property
    // for a given CS
    IMS_SINT32 m_nNvmForCs;
    // Total number of features implied from IARI, ICSI and Feature Tags fields
    // in the CoreService property
    // for a given CS
    // IMS_SINT32 m_nNfForCs;
    // Score of an "Accept-Contact" header for the given CS
    IMS_SINT32 m_nScore;
};

PUBLIC GLOBAL IMS_BOOL PreferenceScore::FindMatchFor(
        IN const AStringArray& objVr, IN const FeatureSet* pVa)
{
    const ImsList<Feature*>& objFeatures = pVa->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        // If there is a value r in VR such that a == r, then SUCCESS
        if (objVr.Contains(pFeature->GetValue(), IMS_FALSE))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL PreferenceScore::IsIn(
        IN const FeatureSet* pVa, IN const AStringArray& objVr, IN IMS_BOOL bCaseSensitive)
{
    const ImsList<Feature*>& objFeatures = pVa->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        // If there is a value r in VR such that a == r, then SUCCESS
        if (objVr.Contains(pFeature->GetValue(), bCaseSensitive))
        {
            return IMS_TRUE;
        }

        // If a is on the form !b (negation),
        if (pFeature->GetValue().StartsWith('!'))
        {
            AString strTmp = pFeature->GetValue().GetSubStr(1);

            if (!objVr.Contains(strTmp, bCaseSensitive))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL PreferenceScore::IsIn(IN const FeatureSet* pVa, IN const FeatureSet* pVr)
{
    const ImsList<Feature*>& objFeatures = pVa->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        // If there is a value r in VR such that a == r, then SUCCESS
        if (pVr->Contains(pFeature))
        {
            return IMS_TRUE;
        }

        // If a is on the form !b (negation),
        if (pFeature->GetValue().StartsWith('!'))
        {
            AString strTmp = pFeature->GetValue().GetSubStr(1);

            if (!pVr->Contains(strTmp))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PUBLIC
void PreferenceScore::ProcessAcceptContact(IN const AppConfig* pAppConfig,
        IN const CoreServiceConfig* pServiceConfig, IN const PreferenceHeader* pHeader,
        IN const ImsList<FeatureSet*>& objExtraFeatures)
{
    const AStringArray& objBasicMimeTypes = pAppConfig->GetBasicMediaMimeTypes();
    AStringArray objTopLevelMimeTypes;
    AStringArray objAppSubLevelMimeTypes;
    ImsList<AString> objTokens;

    for (IMS_SINT32 i = 0; i < objBasicMimeTypes.GetCount(); ++i)
    {
        objTokens = objBasicMimeTypes.GetElementAt(i).Split(TextParser::CHAR_SLASH);

        if (objTokens.GetAt(0).EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
        {
            objAppSubLevelMimeTypes.AddElement(objTokens.GetAt(1));
        }

        objTopLevelMimeTypes.AddElement(objTokens.GetAt(0));
    }

    AStringArray objSupportedIcsis;
    ImsList<FeatureSet*> objSupportedFeatureSets;

    if (pServiceConfig != IMS_NULL)
    {
        const ImsList<ServiceIdentifier>& objIcsis = pServiceConfig->GetIcsis();

        for (IMS_UINT32 i = 0; i < objIcsis.GetSize(); ++i)
        {
            AString strValue = TextParser::DoPercentDecoding(objIcsis.GetAt(i).GetName());

            objSupportedIcsis.AddElement(strValue);
        }

        objSupportedFeatureSets = pServiceConfig->GetFeatureSets();

        // To support the dynamic feature sets
        for (IMS_UINT32 i = 0; i < objExtraFeatures.GetSize(); ++i)
        {
            objSupportedFeatureSets.Append(objExtraFeatures.GetAt(i));
        }
    }

    const ImsList<FeatureSet*>& objAcFeatureSets = pHeader->GetFeatureSets();

    for (IMS_UINT32 i = 0; i < objAcFeatureSets.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = objAcFeatureSets.GetAt(i);
        const AString& strTag = pFeatureSet->GetTag();

        IncreaseNpf();

        if (strTag.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_VIDEO]))
        {
            if (pAppConfig->IsStreamMediaVideoSupported() ||
                    objTopLevelMimeTypes.Contains(
                            Feature::BASE_TAG[Feature::BASE_VIDEO], IMS_FALSE))
            {
                IncreaseNcf();
                IncreaseNvm();
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_AUDIO]))
        {
            if (pAppConfig->IsStreamMediaAudioSupported() ||
                    objTopLevelMimeTypes.Contains(
                            Feature::BASE_TAG[Feature::BASE_AUDIO], IMS_FALSE))
            {
                IncreaseNcf();
                IncreaseNvm();
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::OTHER_MESSAGE))
        {
            if (pAppConfig->IsFramedMediaSupported())
            {
                IncreaseNcf();
                IncreaseNvm();
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_EVENTS]))
        {
            const AStringArray& objEventPackages = pAppConfig->GetSupportedEventPackages();

            if (objEventPackages.GetCount() != 0)
            {
                IncreaseNcf();

                if (IsIn(pFeatureSet, objEventPackages, IMS_TRUE))
                {
                    IncreaseNvm();
                }
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
        {
            if (objTopLevelMimeTypes.Contains(
                        Feature::BASE_TAG[Feature::BASE_APPLICATION], IMS_FALSE))
            {
                IncreaseNcf();
                IncreaseNvm();
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::OTHER_APP_SUBTYPE))
        {
            if (objTopLevelMimeTypes.Contains(
                        Feature::BASE_TAG[Feature::BASE_APPLICATION], IMS_FALSE))
            {
                IncreaseNcf();

                if (FindMatchFor(objAppSubLevelMimeTypes, pFeatureSet))
                {
                    IncreaseNvm();
                }
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::OTHER_G_3GPP_IARI_REF))
        {
            if (pServiceConfig != IMS_NULL)
            {
                if (pServiceConfig->GetIari().GetName().GetLength() != 0)
                {
                    AString strValue =
                            TextParser::DoPercentDecoding(pServiceConfig->GetIari().GetName());

                    IncreaseNcf();

                    AStringArray objIari;

                    objIari.AddElement(strValue);

                    if (IsIn(pFeatureSet, objIari, IMS_FALSE))
                    {
                        IncreaseNvm();
                        IncreaseNvmCs();
                    }
                }
            }
        }
        else if (strTag.EqualsIgnoreCase(Feature::OTHER_G_3GPP_ICSI_REF))
        {
            if (objSupportedIcsis.GetCount() != 0)
            {
                IncreaseNcf();

                if (IsIn(pFeatureSet, objSupportedIcsis, IMS_FALSE))
                {
                    IncreaseNvm();
                    IncreaseNvmCs();
                }
            }
        }
        else
        {
            for (IMS_UINT32 j = 0; j < objSupportedFeatureSets.GetSize(); ++j)
            {
                const FeatureSet* pSupportedFeatureSet = objSupportedFeatureSets.GetAt(j);

                if (pFeatureSet->GetTag().EqualsIgnoreCase(pSupportedFeatureSet->GetTag()))
                {
                    IncreaseNcf();

                    if (IsIn(pFeatureSet, pSupportedFeatureSet))
                    {
                        IncreaseNvm();
                        IncreaseNvmCs();
                    }
                }
            }
        }

        // NOTE: The check for if the feature is handled by the device is not implemented.
    }

    if (GetNpf() > 0)
    {
        SetScore((GetNvm() * SCORE_MULTIPLIER) / GetNpf());

        if (GetNvm() < GetNpf())
        {
            if (pHeader->IsRequirePresent())
            {
                // CoreService will be dropped, and next candidate CS is tried.
                SetScore(CallerPreference::SCORE_INVALID);
            }
            else if (pHeader->IsExplicitPresent())
            {
                SetScore(0);
            }
        }
    }
}

PUBLIC GLOBAL IMS_BOOL CallerPreference::CreateAcceptContactHeaders(IN const AppConfig* pAppConfig,
        IN const CoreServiceConfig* pServiceConfig, IN const ISipConfigV* piSipConfigV,
        OUT ImsList<PreferenceHeader*>& objHeaders)
{
    if (pAppConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Application config is an invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nFeatureTags = ISipConfigV::FEATURE_TAG_DEFAULT;

    if (piSipConfigV != IMS_NULL)
    {
        nFeatureTags = piSipConfigV->GetFeatureTagOptions();
    }

    if (pServiceConfig != IMS_NULL)
    {
        // IARI
        if (pServiceConfig->IsIariSupported())
        {
            IMS_BOOL bExplicit = ((nFeatureTags & ISipConfigV::FEATURE_TAG_IARI_EXPLICIT) != 0);
            IMS_BOOL bRequire = ((nFeatureTags & ISipConfigV::FEATURE_TAG_IARI_REQUIRE) != 0);

            AddFeature(Feature::OTHER_G_3GPP_IARI_REF, pServiceConfig->GetIari().GetName(),
                    bExplicit, bRequire, objHeaders);
        }

        // ICSI
        const ImsList<ServiceIdentifier>& objIcsis = pServiceConfig->GetIcsis();

        for (IMS_UINT32 i = 0; i < objIcsis.GetSize(); ++i)
        {
            const ServiceIdentifier& objIcsi = objIcsis.GetAt(i);

            AddFeature(Feature::OTHER_G_3GPP_ICSI_REF, objIcsi.GetName(),
                    objIcsi.IsExplicitPresent(), objIcsi.IsRequirePresent(), objHeaders);
        }

        // Feature-Tags
        const ImsList<ServiceIdentifier>& objFeatureTags = pServiceConfig->GetFeatureTags();

        if (!objFeatureTags.IsEmpty())
        {
            AString strTag;
            AString strValue;

            for (IMS_UINT32 i = 0; i < objFeatureTags.GetSize(); ++i)
            {
                const ServiceIdentifier& objFTag = objFeatureTags.GetAt(i);
                IMS_SINT32 nCount =
                        objFTag.GetName().SplitF(TextParser::CHAR_EQUAL, strTag, strValue);

                if (nCount == 1)
                {
                    AddFeature(IMS_TRUE, strTag, AString::ConstNull(), objFTag.IsExplicitPresent(),
                            objFTag.IsRequirePresent(), objHeaders);
                }
                else if (nCount == 2)
                {
                    AddFeature(IMS_FALSE, strTag, strValue, objFTag.IsExplicitPresent(),
                            objFTag.IsRequirePresent(), objHeaders);
                }
            }
        }
    }

    if (pAppConfig->IsStreamMediaAudioSupported() &&
            ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_AUDIO) != 0))
    {
        AddFeature(Feature::BASE_TAG[Feature::BASE_AUDIO], objHeaders);
    }

    if (pAppConfig->IsStreamMediaVideoSupported() &&
            ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO) != 0))
    {
        AddFeature(Feature::BASE_TAG[Feature::BASE_VIDEO], objHeaders);
    }

    if (pAppConfig->IsFramedMediaSupported() &&
            ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_FRAMED) != 0))
    {
        AddFeature(Feature::OTHER_MESSAGE, objHeaders);
    }

    if (pAppConfig->IsBasicMediaSupported() &&
            ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_BASIC) != 0))
    {
        ImsList<AString> objTokens;
        const AStringArray& objMimeTypes = pAppConfig->GetBasicMediaMimeTypes();

        for (IMS_SINT32 i = 0; i < objMimeTypes.GetCount(); ++i)
        {
            objTokens = objMimeTypes.GetElementAt(i).Split(TextParser::CHAR_SLASH);

            const AString& strType = objTokens.GetAt(0);

            if (strType.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
            {
                AddFeature(strType, objHeaders);
                AddFeature(Feature::OTHER_APP_SUBTYPE, objTokens.GetAt(1), objHeaders);
            }
            else if (strType.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_AUDIO]) ||
                    strType.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_VIDEO]))
            {
                AddFeature(strType, objHeaders);
            }
            // Currently, "text"/"image" is not supported for a IMS service.
            // else if (strType.EqualsIgnoreCase("text"))
            //{
            //    AddFeature(strType, objHeaders);
            //}
            // else if (strType.EqualsIgnoreCase("image"))
            //{
            //    AddFeature(strType, objHeaders);
            //}
        }
    }

    // Events
    if ((nFeatureTags & ISipConfigV::FEATURE_TAG_EVENT) != 0)
    {
        const AStringArray& objEventPackages = pAppConfig->GetSupportedEventPackages();

        for (IMS_SINT32 i = 0; i < objEventPackages.GetCount(); ++i)
        {
            AddFeature(Feature::BASE_TAG[Feature::BASE_EVENTS], objEventPackages.GetElementAt(i),
                    objHeaders);
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_SINT32 CallerPreference::GetCandidateScore(IN const AppConfig* pAppConfig,
        IN const CoreServiceConfig* pServiceConfig, IN const ImsList<PreferenceHeader*>& objHeaders,
        IN const ImsList<FeatureSet*>& objExtraFeatures)
{
    IMS_SINT32 nScore = 0;
    IMS_SINT32 nNumberOfHeaders = static_cast<IMS_SINT32>(objHeaders.GetSize());
    IMS_SINT32 nSumOfNvmCs = 0;
    IMS_SINT32 nNfCs = 0;

    if (pServiceConfig != IMS_NULL)
    {
        if (pServiceConfig->IsIariSupported())
        {
            ++nNfCs;
        }

        nNfCs += pServiceConfig->GetIcsis().GetSize();
        nNfCs += pServiceConfig->GetFeatureTags().GetSize();
    }

    PreferenceScore objScore;

    for (IMS_SINT32 i = 0; i < nNumberOfHeaders; ++i)
    {
        const PreferenceHeader* pHeader = objHeaders.GetAt(i);

        if (pHeader == IMS_NULL)
        {
            continue;
        }

        objScore.Clear();

        objScore.ProcessAcceptContact(pAppConfig, pServiceConfig, pHeader, objExtraFeatures);

        // TRACE("Candidated (%d) score (%d)", i, objPrefScore.GetScore());

        if (objScore.GetScore() == SCORE_INVALID)
        {
            IMS_TRACE_D("CallerPreference :: SERVICE (%s, %s) IS DROPPED ...",
                    pAppConfig->GetAppId().GetStr(),
                    (pServiceConfig != IMS_NULL) ? pServiceConfig->GetServiceId().GetStr()
                                                 : "__NULL__",
                    0);

            return SCORE_INVALID;
        }
        else
        {
            nSumOfNvmCs += objScore.GetNvmCs();
            nScore += objScore.GetScore();
        }
    }

    if (nSumOfNvmCs < nNfCs)
    {
        IMS_TRACE_D("CallerPreference :: SERVICE (%s, %s) IS DROPPED ...",
                pAppConfig->GetAppId().GetStr(),
                (pServiceConfig != IMS_NULL) ? pServiceConfig->GetServiceId().GetStr() : "__NULL__",
                0);

        return SCORE_INVALID;
    }
    else
    {
        if (nNumberOfHeaders == 0)
        {
            return (nNfCs > 0) ? SCORE_INVALID : 0;
        }

        return IMS_SINT32(nScore / nNumberOfHeaders);
    }
}

PRIVATE GLOBAL void CallerPreference::AddFeature(
        IN const AString& strTag, OUT ImsList<PreferenceHeader*>& objHeaders)
{
    AddFeature(IMS_TRUE, strTag, AString::ConstNull(), IMS_FALSE, IMS_FALSE, objHeaders);
}

PRIVATE GLOBAL void CallerPreference::AddFeature(IN const AString& strTag,
        IN const AString& strValue, OUT ImsList<PreferenceHeader*>& objHeaders)
{
    AddFeature(IMS_FALSE, strTag, strValue, IMS_FALSE, IMS_FALSE, objHeaders);
}

PRIVATE GLOBAL void CallerPreference::AddFeature(IN const AString& strTag,
        IN const AString& strValue, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire,
        OUT ImsList<PreferenceHeader*>& objHeaders)
{
    AddFeature(IMS_FALSE, strTag, strValue, bExplicit, bRequire, objHeaders);
}

PRIVATE GLOBAL void CallerPreference::AddFeature(IN IMS_BOOL bBooleanFeature,
        IN const AString& strTag, IN const AString& strValue, IN IMS_BOOL bExplicit,
        IN IMS_BOOL bRequire, OUT ImsList<PreferenceHeader*>& objHeaders)
{
    // Implement "If the feature T is already part of some member in Fs, then Fs is unchanged."
    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        const PreferenceHeader* pHeader = objHeaders.GetAt(i);

        if (bBooleanFeature)
        {
            // Exact match found; the current header already contains the given feature-param.
            if (pHeader->Contains(strTag))
            {
                IMS_TRACE_D("CallerPreference::AddFeature() - Already Exists: %s", strTag.GetStr(),
                        0, 0);
                return;
            }
        }
        else
        {
            // Exact match found; the current header already contains the given feature-param.
            if (pHeader->Contains(strTag, strValue))
            {
                IMS_TRACE_D("CallerPreference::AddFeature() - Already Exists: %s=%s",
                        strTag.GetStr(), strValue.GetStr(), 0);
                return;
            }
        }
    }

    // Implements "Search for a header h in Fs that is free from T.tag and has the same explicit
    // and require flags, then add T to h. Done."
    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        PreferenceHeader* pHeader = objHeaders.GetAt(i);

        if ((pHeader->IsExplicitPresent() == bExplicit) &&
                (pHeader->IsRequirePresent() == bRequire))
        {
            if (bBooleanFeature)
            {
                if (!pHeader->Contains(strTag))
                {
                    pHeader->AddFeature(strTag);
                    return;
                }
            }
            else
            {
                if (!pHeader->Contains(strTag, strValue))
                {
                    pHeader->AddFeature(strTag, strValue);
                    return;
                }
            }
        }
    }

    // Implements "If the search was not successful, then create a new header h with T
    // and add that to Fs.
    // Mark with the explicit and require flags associated with T."
    PreferenceHeader* pHeader = new PreferenceHeader(bExplicit, bRequire);

    if (bBooleanFeature)
    {
        pHeader->AddFeature(strTag);
    }
    else
    {
        pHeader->AddFeature(strTag, strValue);
    }

    if (!objHeaders.Append(pHeader))
    {
        delete pHeader;
    }
}
