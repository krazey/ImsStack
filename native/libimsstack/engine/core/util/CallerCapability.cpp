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
#include "AStringArray.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Feature.h"
#include "ISipConfigV.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"

#include "util/CallerCapability.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CallerCapability::CallerCapability(IN IMS_UINT32 nId) :
        m_nId(nId),
        m_objContactFeatures(ImsList<FeatureSet*>())
{
}

PUBLIC
CallerCapability::~CallerCapability()
{
    Clear();
}

PUBLIC
IMS_BOOL CallerCapability::AddFeature(IN const Feature* pFeature)
{
    if (pFeature == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    if (pFeature->IsTagOnly())
    {
        nUpdateFlag = AddFeature(pFeature->GetTag());
    }
    else
    {
        nUpdateFlag = AddFeature(pFeature->GetTag(), pFeature->GetValue());
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::AddFeature(IN const FeatureSet* pFeatureSet)
{
    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;
    const ImsList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        if (pFeature->IsTagOnly())
        {
            nUpdateFlag |= AddFeature(pFeature->GetTag());
        }
        else
        {
            nUpdateFlag |= AddFeature(pFeature->GetTag(), pFeature->GetValue());
        }
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::AddFeatures(IN const CallerCapability* pCc)
{
    if (pCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    for (IMS_UINT32 i = 0; i < pCc->m_objContactFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = pCc->m_objContactFeatures.GetAt(i);

        const ImsList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

        for (IMS_UINT32 j = 0; j < objFeatures.GetSize(); ++j)
        {
            const Feature* pFeature = objFeatures.GetAt(j);

            if (pFeature->IsTagOnly())
            {
                nUpdateFlag |= AddFeature(pFeature->GetTag());
            }
            else
            {
                nUpdateFlag |= AddFeature(pFeature->GetTag(), pFeature->GetValue());
            }
        }
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
void CallerCapability::Clear()
{
    // Clear the existing caller capabilities
    if (!m_objContactFeatures.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objContactFeatures.GetSize(); ++i)
        {
            FeatureSet* pFeatureSet = m_objContactFeatures.GetAt(i);

            if (pFeatureSet != IMS_NULL)
            {
                delete pFeatureSet;
            }
        }

        m_objContactFeatures.Clear();
    }
}

PUBLIC
IMS_BOOL CallerCapability::Create(IN const AppConfig* pAppConfig,
        IN const ICoreServiceConfig* piServiceConfig, IN const ISipConfigV* piSipConfigV)
{
    if (pAppConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "Error occurred - AppConfig is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_UINT32 nFeatureTags = ISipConfigV::FEATURE_TAG_DEFAULT;

    if (piSipConfigV != IMS_NULL)
    {
        nFeatureTags = piSipConfigV->GetFeatureTagOptions();
    }

    // CoreService property
    // Check the core service and add a feature first for IARI / ICSI / Feature-Tags
    if (piServiceConfig != IMS_NULL)
    {
        // IARI : "+g.3gpp.iari-ref"
        if (piServiceConfig->IsIariSupported())
        {
            AddFeature(Feature::OTHER_G_3GPP_IARI_REF, piServiceConfig->GetIari().GetName());
        }

        // ICSIs : "+g.3gpp.icsi-ref"
        const ImsList<ServiceIdentifier>& objIcsis = piServiceConfig->GetIcsis();

        if (!objIcsis.IsEmpty())
        {
            AString strTag(Feature::OTHER_G_3GPP_ICSI_REF);

            for (IMS_UINT32 j = 0; j < objIcsis.GetSize(); ++j)
            {
                AddFeature(strTag, objIcsis.GetAt(j).GetName());
            }
        }

        // Feature tags
        const ImsList<ServiceIdentifier>& objFeatureTags = piServiceConfig->GetFeatureTags();

        if (!objFeatureTags.IsEmpty())
        {
            AString strTag;
            AString strValue;

            for (IMS_UINT32 j = 0; j < objFeatureTags.GetSize(); ++j)
            {
                const AString& strFTag = objFeatureTags.GetAt(j).GetName();
                IMS_SINT32 nCount = strFTag.SplitF(TextParser::CHAR_EQUAL, strTag, strValue);

                if (nCount == 1)
                {
                    AddFeature(strTag);
                }
                else if (nCount == 2)
                {
                    AddFeature(strTag, strValue);
                }
            }
        }
    }

    // StreamMedia property
    if (pAppConfig->IsStreamMediaSupported())
    {
        if (!pAppConfig->IsStreamMediaAudioSupported() &&
                !pAppConfig->IsStreamMediaVideoSupported() &&
                !pAppConfig->IsStreamMediaTextSupported())
        {
            if ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_AUDIO) != 0)
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_AUDIO]);
            }

            if ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO) != 0)
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_VIDEO]);
            }

            if ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT) != 0)
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_TEXT]);
            }
        }
        else
        {
            if (pAppConfig->IsStreamMediaAudioSupported() &&
                    ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_AUDIO) != 0))
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_AUDIO]);
            }

            if (pAppConfig->IsStreamMediaVideoSupported() &&
                    ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO) != 0))
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_VIDEO]);
            }

            if (pAppConfig->IsStreamMediaTextSupported() &&
                    ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_TEXT) != 0))
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_TEXT]);
            }
        }
    }

    // FramedMedia property
    if (pAppConfig->IsFramedMediaSupported() &&
            ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_FRAMED) != 0))
    {
        AddFeature(Feature::OTHER_MESSAGE);
    }

    // BasicMedia property
    if (pAppConfig->IsBasicMediaSupported() &&
            ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_BASIC) != 0))
    {
        ImsList<AString> objTokens;
        const AStringArray& objBasicMediaMimeTypes = pAppConfig->GetBasicMediaMimeTypes();

        for (IMS_SINT32 j = 0; j < objBasicMediaMimeTypes.GetCount(); ++j)
        {
            objTokens = objBasicMediaMimeTypes.GetElementAt(j).Split(TextParser::CHAR_SLASH);

            const AString& strTopLevelType = objTokens.GetAt(0);

            if (strTopLevelType.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_APPLICATION]);
                AddFeature(Feature::OTHER_APP_SUBTYPE, objTokens.GetAt(1));
            }
            else if (strTopLevelType.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_VIDEO]))
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_VIDEO]);
            }
            else if (strTopLevelType.EqualsIgnoreCase(Feature::BASE_TAG[Feature::BASE_AUDIO]))
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_AUDIO]);
            }
        }
    }

    // Event property
    if ((nFeatureTags & ISipConfigV::FEATURE_TAG_EVENT) != 0)
    {
        const AStringArray& objEventPackages = pAppConfig->GetSupportedEventPackages();

        for (IMS_SINT32 i = 0; i < objEventPackages.GetCount(); ++i)
        {
            AddFeature(Feature::BASE_TAG[Feature::BASE_EVENTS], objEventPackages.GetElementAt(i));
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL CallerCapability::Equals(IN const CallerCapability* pCc) const
{
    if (pCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nId != pCc->m_nId)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL CallerCapability::HasFeature(IN const Feature* pFeature) const
{
    if (pFeature == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const FeatureSet* pFeatureSet = Lookup(pFeature->GetTag());

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pFeatureSet->Contains(pFeature);
}

PUBLIC
IMS_BOOL CallerCapability::RemoveFeature(IN const Feature* pFeature)
{
    if (pFeature == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    if (pFeature->IsTagOnly())
    {
        nUpdateFlag = RemoveFeature(pFeature->GetTag());
    }
    else
    {
        nUpdateFlag = RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::RemoveFeature(IN const FeatureSet* pFeatureSet)
{
    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;
    const ImsList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        if (pFeature->IsTagOnly())
        {
            nUpdateFlag |= RemoveFeature(pFeature->GetTag());
        }
        else
        {
            nUpdateFlag |= RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
        }
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::RemoveFeatures(
        IN const CallerCapability* pCc, IN IMS_BOOL bRemoveRef /*= IMS_TRUE*/)
{
    if (pCc == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    for (IMS_UINT32 i = 0; i < pCc->m_objContactFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = pCc->m_objContactFeatures.GetAt(i);

        const ImsList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

        for (IMS_UINT32 j = 0; j < objFeatures.GetSize(); ++j)
        {
            const Feature* pFeature = objFeatures.GetAt(j);

            if (bRemoveRef)
            {
                if (pFeature->IsTagOnly())
                {
                    nUpdateFlag |= RemoveFeature(pFeature->GetTag());
                }
                else
                {
                    nUpdateFlag |= RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
                }
            }
            else
            {
                while (HasFeature(pFeature))
                {
                    if (pFeature->IsTagOnly())
                    {
                        nUpdateFlag |= RemoveFeature(pFeature->GetTag());
                    }
                    else
                    {
                        nUpdateFlag |= RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
                    }
                }
            }
        }
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
AString CallerCapability::ToString() const
{
    if (!m_objContactFeatures.IsEmpty())
    {
        AString strContactFeatures = m_objContactFeatures.GetAt(0)->ToString();

        for (IMS_UINT32 i = 1; i < m_objContactFeatures.GetSize(); ++i)
        {
            const FeatureSet* pFeatureSet = m_objContactFeatures.GetAt(i);

            strContactFeatures.Append(TextParser::CHAR_SEMICOLON);
            strContactFeatures.Append(pFeatureSet->ToString());
        }

        return strContactFeatures;
    }

    return AString::ConstNull();
}

PRIVATE
IMS_BOOL CallerCapability::Attach(
        IN const AString& strTag, IN const AString& strValue /*= AString::ConstNull()*/)
{
    FeatureSet* pFeatureSet = new FeatureSet(strTag);

    if (strValue.IsNULL())
    {
        pFeatureSet->Add(strTag);
    }
    else
    {
        pFeatureSet->Add(strTag, strValue);
    }

    if (!m_objContactFeatures.Append(pFeatureSet))
    {
        delete pFeatureSet;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void CallerCapability::Detach(IN const AString& strTag)
{
    for (IMS_UINT32 i = 0; i < m_objContactFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objContactFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            m_objContactFeatures.RemoveAt(i);
            return;
        }
    }
}

PRIVATE
FeatureSet* CallerCapability::Lookup(IN const AString& strTag) const
{
    for (IMS_UINT32 i = 0; i < m_objContactFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = m_objContactFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            return pFeatureSet;
        }
    }

    // If not found,
    return IMS_NULL;
}

PRIVATE
IMS_SINT32 CallerCapability::AddFeature(IN const AString& strTag)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet)
    {
        if (pFeatureSet->Add(strTag) == FeatureSet::OP_ADD)
        {
            return FEATURE_CHANGED;
        }
    }
    else
    {
        // TODO:: if failed, what to do ???
        if (Attach(strTag, AString::ConstNull()))
        {
            return FEATURE_CHANGED;
        }
    }

    return FEATURE_UNCHANGED;
}

PRIVATE
IMS_SINT32 CallerCapability::AddFeature(IN const AString& strTag, IN const AString& strValue)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Add(strTag, strValue) == FeatureSet::OP_ADD)
        {
            return FEATURE_CHANGED;
        }
    }
    else
    {
        // TODO:: if failed, what to do ???
        if (Attach(strTag, strValue))
        {
            return FEATURE_CHANGED;
        }
    }

    return FEATURE_UNCHANGED;
}

PRIVATE
IMS_SINT32 CallerCapability::RemoveFeature(IN const AString& strTag)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Remove(strTag) == FeatureSet::OP_REMOVE)
        {
            if (pFeatureSet->IsEmpty())
            {
                Detach(strTag);
                delete pFeatureSet;
            }

            return FEATURE_CHANGED;
        }
    }

    return FEATURE_UNCHANGED;
}

PRIVATE
IMS_SINT32 CallerCapability::RemoveFeature(IN const AString& strTag, IN const AString& strValue)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Remove(strTag, strValue) == FeatureSet::OP_REMOVE)
        {
            if (pFeatureSet->IsEmpty())
            {
                Detach(strTag);
                delete pFeatureSet;
            }

            return FEATURE_CHANGED;
        }
    }

    return FEATURE_UNCHANGED;
}
