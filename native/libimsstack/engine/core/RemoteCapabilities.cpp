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
#include "ServiceIdentifier.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"

#include "RemoteCapabilities.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
RemoteCapabilities::RemoteCapabilities() :
        m_bAudioSupported(IMS_FALSE),
        m_bVideoSupported(IMS_FALSE),
        m_bFramedMediaSupported(IMS_FALSE),
        m_bApplicationSupported(IMS_FALSE),
        m_objAppSubTypes(ImsList<FeatureSet*>()),
        m_objEvents(ImsList<FeatureSet*>()),
        m_objIcsis(ImsList<FeatureSet*>()),
        m_objIaris(ImsList<FeatureSet*>()),
        m_objFeatureTags(ImsList<FeatureSet*>())
{
}

PUBLIC
RemoteCapabilities::~RemoteCapabilities()
{
    RemoveAllFeatureSets(m_objAppSubTypes);
    RemoveAllFeatureSets(m_objEvents);
    RemoveAllFeatureSets(m_objIaris);
    RemoveAllFeatureSets(m_objIcsis);
    RemoveAllFeatureSets(m_objFeatureTags);
}

PUBLIC
IMS_BOOL RemoteCapabilities::Create(IN const ImsList<AString>& objCapabilities)
{
    FeatureSet* pFeatureSet;
    AString strName;
    AString strValue;

    for (IMS_UINT32 i = 0; i < objCapabilities.GetSize(); ++i)
    {
        const AString& strTemp = objCapabilities.GetAt(i);
        AString strElement = strTemp.MakeLower();
        IMS_SINT32 nCount = strElement.SplitF(TextParser::CHAR_EQUAL, strName, strValue);

        pFeatureSet = IMS_NULL;

        if (nCount == 1)
        {
            pFeatureSet = new FeatureSet(strName);

            if (pFeatureSet == IMS_NULL)
            {
                IMS_TRACE_E(0, "Allocating FeatureSet failed", 0, 0, 0);
                goto EXIT_Create;
            }

            if (pFeatureSet->Add(strName) == FeatureSet::OP_FAIL)
            {
                delete pFeatureSet;
                IMS_TRACE_E(0, "Adding Feature (%s) failed", strName.GetStr(), 0, 0);
                goto EXIT_Create;
            }
        }
        else if (nCount == 2)
        {
            pFeatureSet = new FeatureSet(strName, strValue);

            if (pFeatureSet == IMS_NULL)
            {
                IMS_TRACE_E(0, "Allocating FeatureSet failed", 0, 0, 0);
                goto EXIT_Create;
            }
        }
        else
        {
            continue;
        }

        // ICSIs
        if (pFeatureSet->GetTag().Equals(Feature::OTHER_G_3GPP_ICSI_REF))
        {
            if (!m_objIcsis.Append(pFeatureSet))
            {
                delete pFeatureSet;
                goto EXIT_Create;
            }
        }
        // IARIs
        else if (pFeatureSet->GetTag().Equals(Feature::OTHER_G_3GPP_IARI_REF))
        {
            if (!m_objIaris.Append(pFeatureSet))
            {
                delete pFeatureSet;
                goto EXIT_Create;
            }
        }
        // audio
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_AUDIO]))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                m_bAudioSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // video
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_VIDEO]))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                m_bVideoSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // message
        else if (pFeatureSet->GetTag().Equals(Feature::OTHER_MESSAGE))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                m_bFramedMediaSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // application
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                m_bApplicationSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // app_subtype
        else if (pFeatureSet->GetTag().Equals(Feature::OTHER_APP_SUBTYPE))
        {
            if (!m_objAppSubTypes.Append(pFeatureSet))
            {
                delete pFeatureSet;
                goto EXIT_Create;
            }
        }
        // events
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_EVENTS]))
        {
            if (!m_objEvents.Append(pFeatureSet))
            {
                delete pFeatureSet;
                goto EXIT_Create;
            }
        }
        // Feature-Tags
        else
        {
            // If the name is a feature-tag, then appends it to the list of feature-tag
            if (Feature::IsFeatureTag(pFeatureSet->GetTag()))
            {
                if (!m_objFeatureTags.Append(pFeatureSet))
                {
                    delete pFeatureSet;
                    goto EXIT_Create;
                }
            }
            else
            {
                delete pFeatureSet;
            }
        }
    }

    return IMS_TRUE;

EXIT_Create:

    m_bAudioSupported = IMS_FALSE;
    m_bVideoSupported = IMS_FALSE;
    m_bFramedMediaSupported = IMS_FALSE;
    m_bApplicationSupported = IMS_FALSE;

    RemoveAllFeatureSets(m_objAppSubTypes);
    RemoveAllFeatureSets(m_objEvents);
    RemoveAllFeatureSets(m_objIaris);
    RemoveAllFeatureSets(m_objIcsis);
    RemoveAllFeatureSets(m_objFeatureTags);

    return IMS_FALSE;
}

/**
 * @brief Checks if a certain FeatureTag is supported by the remote device.
 */
PUBLIC
IMS_BOOL RemoteCapabilities::IsCompatible(
        IN const AppConfig* pAppConfig, IN const AString& strServiceId) const
{
    if (pAppConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // StreamAudio
    // StreamVideo
    // FramedMedia
    // BasicMedia
    // Event packages
    // CoreService capabilities
    if ((pAppConfig->IsStreamMediaAudioSupported() && !IsAudioSupported()) ||
            (pAppConfig->IsStreamMediaVideoSupported() && !IsVideoSupported()) ||
            (pAppConfig->IsFramedMediaSupported() && !IsFramedMediaSupported()) ||
            (pAppConfig->IsBasicMediaSupported() && !IsBasicMediaCompatible(pAppConfig)) ||
            (!pAppConfig->GetSupportedEventPackages().IsEmpty() &&
                    !IsEventCompatible(pAppConfig)) ||
            (!pAppConfig->GetCoreServiceConfigs().IsEmpty() &&
                    !IsCoreServiceCompatible(pAppConfig, strServiceId)))

    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/**
 * @brief Checks if a certain application subtype is supported by the remote device.
 */
PRIVATE
IMS_BOOL RemoteCapabilities::IsAppSubTypeSupported(IN const AString& strAppSubType) const
{
    if (m_objAppSubTypes.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objAppSubTypes.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objAppSubTypes.GetAt(i);

        if (pFeatureSet->Contains(strAppSubType))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/**
 * @brief Checks if a certain event is supported by the remote device.
 */
PRIVATE
IMS_BOOL RemoteCapabilities::IsEventSupported(IN const AString& strEvent) const
{
    if (m_objEvents.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objEvents.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objEvents.GetAt(i);

        if (pFeatureSet->Contains(strEvent))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/**
 * @brief Checks if a certain IARI is supported by the remote device.
 */
PRIVATE
IMS_BOOL RemoteCapabilities::IsIariSupported(IN const AString& strIari) const
{
    if (m_objIaris.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objIaris.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objIaris.GetAt(i);

        if (pFeatureSet->Contains(strIari))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/**
 * @brief Checks if a certain ICSI is supported by the remote device.
 */
PRIVATE
IMS_BOOL RemoteCapabilities::IsIcsiSupported(IN const AString& strIcsi) const
{
    if (m_objIcsis.IsEmpty())
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objIcsis.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objIcsis.GetAt(i);

        if (pFeatureSet->Contains(strIcsi))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/**
 * @brief Checks if a certain FeatureTag is supported by the remote device.
 */
PRIVATE
IMS_BOOL RemoteCapabilities::IsFeatureTagSupported(IN const AString& strFeatureTag) const
{
    if (m_objFeatureTags.IsEmpty())
    {
        return IMS_FALSE;
    }

    Feature objFeature(strFeatureTag);

    for (IMS_UINT32 i = 0; i < m_objFeatureTags.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objFeatureTags.GetAt(i);

        if (pFeatureSet->Contains(&objFeature))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL RemoteCapabilities::IsBasicMediaCompatible(IN const AppConfig* pAppConfig) const
{
    AString strLowerMimeType;
    const AStringArray& objMimeTypes = pAppConfig->GetBasicMediaMimeTypes();

    if (objMimeTypes.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_SINT32 i = 0; i < objMimeTypes.GetCount(); ++i)
    {
        const AString& strMimeType = objMimeTypes.GetElementAt(i);

        strLowerMimeType = strMimeType.MakeLower();

        if (strLowerMimeType.StartsWith(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
        {
            IMS_SINT32 nStartIndex = strLowerMimeType.GetIndexOf(TextParser::CHAR_SLASH);

            if (nStartIndex != AString::NPOS)
            {
                AString strAppSubType = strLowerMimeType.GetSubStr(nStartIndex + 1);

                if (!IsAppSubTypeSupported(strAppSubType))
                {
                    return IMS_FALSE;
                }
            }
        }
        else if (strLowerMimeType.StartsWith(Feature::BASE_TAG[Feature::BASE_AUDIO]))
        {
            if (!IsAudioSupported())
            {
                return IMS_FALSE;
            }
        }
        else if (strLowerMimeType.StartsWith(Feature::BASE_TAG[Feature::BASE_VIDEO]))
        {
            if (!IsVideoSupported())
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RemoteCapabilities::IsEventCompatible(IN const AppConfig* pAppConfig) const
{
    const AStringArray& objEvents = pAppConfig->GetSupportedEventPackages();

    if (objEvents.IsEmpty())
    {
        return IMS_TRUE;
    }

    for (IMS_SINT32 i = 0; i < objEvents.GetCount(); ++i)
    {
        if (!IsEventSupported(objEvents.GetElementAt(i)))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RemoteCapabilities::IsCoreServiceCompatible(
        IN const AppConfig* pAppConfig, IN const AString& strServiceId) const
{
    const ImsList<CoreServiceConfig*>& objCoreServiceConfigs = pAppConfig->GetCoreServiceConfigs();

    if (objCoreServiceConfigs.IsEmpty())
    {
        return IMS_TRUE;
    }

    // For a dedicated service in the application
    if (!strServiceId.IsNULL())
    {
        const CoreServiceConfig* pServiceConfig = pAppConfig->GetCoreServiceConfigEx(strServiceId);

        if (pServiceConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        return IsCoreServiceCompatible(pServiceConfig);
    }

    // For all the services in the application
    for (IMS_UINT32 i = 0; i < objCoreServiceConfigs.GetSize(); ++i)
    {
        const CoreServiceConfig* pServiceConfig = objCoreServiceConfigs.GetAt(i);

        if (pServiceConfig == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!IsCoreServiceCompatible(pServiceConfig))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RemoteCapabilities::IsCoreServiceCompatible(
        IN const CoreServiceConfig* pServiceConfig) const
{
    /// Evaluate the ICSIs
    const ImsList<ServiceIdentifier>& objLocalIcsis = pServiceConfig->GetIcsis();

    if (!objLocalIcsis.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objLocalIcsis.GetSize(); ++i)
        {
            const ServiceIdentifier& objIcsi = objLocalIcsis.GetAt(i);

            if (!IsIcsiSupported(objIcsi.GetName()))
            {
                return IMS_FALSE;
            }
        }
    }

    /// Evaluates the IARI
    if (pServiceConfig->IsIariSupported())
    {
        const ServiceIdentifier& objIari = pServiceConfig->GetIari();

        if (!IsIariSupported(objIari.GetName()))
        {
            return IMS_FALSE;
        }
    }

    /// Evaluates the feature-tags
    const ImsList<ServiceIdentifier>& objLocalFeatureTags = pServiceConfig->GetFeatureTags();

    if (!objLocalFeatureTags.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objLocalFeatureTags.GetSize(); ++i)
        {
            const ServiceIdentifier& objFTag = objLocalFeatureTags.GetAt(i);

            if (!IsFeatureTagSupported(objFTag.GetName()))
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL void RemoteCapabilities::RemoveAllFeatureSets(
        IN_OUT ImsList<FeatureSet*>& objFeatureSets)
{
    if (objFeatureSets.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objFeatureSets.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objFeatureSets.GetAt(i);

        if (pFeatureSet != IMS_NULL)
        {
            delete pFeatureSet;
        }
    }

    objFeatureSets.Clear();
}
