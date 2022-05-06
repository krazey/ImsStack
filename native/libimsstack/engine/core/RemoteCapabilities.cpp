/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091201  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "Feature.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"
#include "ServiceIdentifier.h"
#include "RemoteCapabilities.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
RemoteCapabilities::RemoteCapabilities() :
        bAudioSupported(IMS_FALSE),
        bVideoSupported(IMS_FALSE),
        bFramedMediaSupported(IMS_FALSE),
        bApplicationSupported(IMS_FALSE),
        objAppSubTypes(IMSList<FeatureSet*>()),
        objEvents(IMSList<FeatureSet*>()),
        objICSIs(IMSList<FeatureSet*>()),
        objIARIs(IMSList<FeatureSet*>()),
        objFeatureTags(IMSList<FeatureSet*>())
{
}

PUBLIC
RemoteCapabilities::~RemoteCapabilities()
{
    //---------------------------------------------------------------------------------------------

    RemoveAllFeatureSets(objAppSubTypes);
    RemoveAllFeatureSets(objEvents);
    RemoveAllFeatureSets(objIARIs);
    RemoveAllFeatureSets(objICSIs);
    RemoveAllFeatureSets(objFeatureTags);
}

PUBLIC
IMS_BOOL RemoteCapabilities::Create(IN CONST IMSList<AString>& objCapabilities)
{
    FeatureSet* pFeatureSet;
    AString strName;
    AString strValue;

    //---------------------------------------------------------------------------------------------

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
            if (!objICSIs.Append(pFeatureSet))
            {
                delete pFeatureSet;
                goto EXIT_Create;
            }
        }
        // IARIs
        else if (pFeatureSet->GetTag().Equals(Feature::OTHER_G_3GPP_IARI_REF))
        {
            if (!objIARIs.Append(pFeatureSet))
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
                bAudioSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // video
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_VIDEO]))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                bVideoSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // message
        else if (pFeatureSet->GetTag().Equals(Feature::OTHER_MESSAGE))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                bFramedMediaSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // application
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_APPLICATION]))
        {
            if (pFeatureSet->Contains(TextParser::STR_TRUE))
            {
                bApplicationSupported = IMS_TRUE;
            }

            delete pFeatureSet;
        }
        // app_subtype
        else if (pFeatureSet->GetTag().Equals(Feature::OTHER_APP_SUBTYPE))
        {
            if (!objAppSubTypes.Append(pFeatureSet))
            {
                delete pFeatureSet;
                goto EXIT_Create;
            }
        }
        // events
        else if (pFeatureSet->GetTag().Equals(Feature::BASE_TAG[Feature::BASE_EVENTS]))
        {
            if (!objEvents.Append(pFeatureSet))
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
                if (!objFeatureTags.Append(pFeatureSet))
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

    bAudioSupported = IMS_FALSE;
    bVideoSupported = IMS_FALSE;
    bFramedMediaSupported = IMS_FALSE;
    bApplicationSupported = IMS_FALSE;

    RemoveAllFeatureSets(objAppSubTypes);
    RemoveAllFeatureSets(objEvents);
    RemoveAllFeatureSets(objIARIs);
    RemoveAllFeatureSets(objICSIs);
    RemoveAllFeatureSets(objFeatureTags);

    return IMS_FALSE;
}

/*

Checks if a certain FeatureTag is supported by the remote device.

*/
PUBLIC
IMS_BOOL RemoteCapabilities::IsCompatible(
        IN CONST AppConfig* pAppConfig, IN CONST AString& strServiceId) const
{
    IMS_BOOL bIsCompatible = IMS_TRUE;

    //---------------------------------------------------------------------------------------------

    if (pAppConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // StreamAudio
    if (pAppConfig->IsStreamMediaAudioSupported() && !IsAudioSupported())
    {
        bIsCompatible = IMS_FALSE;
    }
    // StreamVideo
    else if (pAppConfig->IsStreamMediaVideoSupported() && !IsVideoSupported())
    {
        bIsCompatible = IMS_FALSE;
    }
    // FramedMedia
    else if (pAppConfig->IsFramedMediaSupported() && !IsFramedMediaSupported())
    {
        bIsCompatible = IMS_FALSE;
    }
    // BasicMedia
    else if (pAppConfig->IsBasicMediaSupported() && !IsBasicMediaCompatible(pAppConfig))
    {
        bIsCompatible = IMS_FALSE;
    }
    // Event packages
    else if (!pAppConfig->GetSupportedEventPackages().IsEmpty() && !IsEventCompatible(pAppConfig))
    {
        bIsCompatible = IMS_FALSE;
    }
    // CoreService capabilities
    else if (!pAppConfig->GetCoreServiceConfigs().IsEmpty() &&
            !IsCoreServiceCompatible(pAppConfig, strServiceId))
    {
        bIsCompatible = IMS_FALSE;
    }

    return bIsCompatible;
}

/*

Checks if the remote device supports streaming audio or audio content-type.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsAudioSupported() const
{
    //---------------------------------------------------------------------------------------------

    return bAudioSupported;
}

/*

Checks if the remote device supports streaming video or video content-type.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsVideoSupported() const
{
    //---------------------------------------------------------------------------------------------

    return bVideoSupported;
}

/*

Checks if the remote device supports Framed Media, MSRP.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsFramedMediaSupported() const
{
    //---------------------------------------------------------------------------------------------

    return bFramedMediaSupported;
}

/*

Checks if a certain application subtype is supported by the remote device.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsAppSubTypeSupported(IN CONST AString& strAppSubType) const
{
    //---------------------------------------------------------------------------------------------

    if (objAppSubTypes.IsEmpty())
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objAppSubTypes.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objAppSubTypes.GetAt(i);

        if (pFeatureSet->Contains(strAppSubType))
            return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Checks if a certain event is supported by the remote device.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsEventSupported(IN CONST AString& strEvent) const
{
    //---------------------------------------------------------------------------------------------

    if (objEvents.IsEmpty())
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objEvents.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objEvents.GetAt(i);

        if (pFeatureSet->Contains(strEvent))
            return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Checks if a certain IARI is supported by the remote device.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsIARISupported(IN CONST AString& strIARI) const
{
    //---------------------------------------------------------------------------------------------

    if (objIARIs.IsEmpty())
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objIARIs.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objIARIs.GetAt(i);

        if (pFeatureSet->Contains(strIARI))
            return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Checks if a certain ICSI is supported by the remote device.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsICSISupported(IN CONST AString& strICSI) const
{
    //---------------------------------------------------------------------------------------------

    if (objICSIs.IsEmpty())
        return IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objICSIs.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objICSIs.GetAt(i);

        if (pFeatureSet->Contains(strICSI))
            return IMS_TRUE;
    }

    return IMS_FALSE;
}

/*

Checks if a certain FeatureTag is supported by the remote device.

*/
PRIVATE
IMS_BOOL RemoteCapabilities::IsFeatureTagSupported(IN CONST AString& strFeatureTag) const
{
    //---------------------------------------------------------------------------------------------

    if (objFeatureTags.IsEmpty())
        return IMS_FALSE;

    Feature objFeature(strFeatureTag);

    for (IMS_UINT32 i = 0; i < objFeatureTags.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objFeatureTags.GetAt(i);

        if (pFeatureSet->Contains(&objFeature))
            return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL RemoteCapabilities::IsBasicMediaCompatible(IN CONST AppConfig* pAppConfig) const
{
    AString strLowerMimeType;
    const AStringArray& objMimeTypes = pAppConfig->GetBasicMediaMimeTypes();

    //---------------------------------------------------------------------------------------------

    if (objMimeTypes.IsEmpty())
        return IMS_TRUE;

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
                    return IMS_FALSE;
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
IMS_BOOL RemoteCapabilities::IsEventCompatible(IN CONST AppConfig* pAppConfig) const
{
    const AStringArray& objEvents = pAppConfig->GetSupportedEventPackages();

    //---------------------------------------------------------------------------------------------

    if (objEvents.IsEmpty())
        return IMS_TRUE;

    for (IMS_SINT32 i = 0; i < objEvents.GetCount(); ++i)
    {
        if (!IsEventSupported(objEvents.GetElementAt(i)))
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL RemoteCapabilities::IsCoreServiceCompatible(
        IN CONST AppConfig* pAppConfig, IN CONST AString& strServiceId) const
{
    const IMSList<CoreServiceConfig*>& objCoreServiceConfigs = pAppConfig->GetCoreServiceConfigs();

    //---------------------------------------------------------------------------------------------

    if (objCoreServiceConfigs.IsEmpty())
        return IMS_TRUE;

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
        IN CONST CoreServiceConfig* pServiceConfig) const
{
    /// Evaluate the ICSIs
    const IMSList<ServiceIdentifier>& objLocalICSIs = pServiceConfig->GetICSIs();

    //---------------------------------------------------------------------------------------------

    if (!objLocalICSIs.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objLocalICSIs.GetSize(); ++i)
        {
            const ServiceIdentifier& objICSI = objLocalICSIs.GetAt(i);

            if (!IsICSISupported(objICSI.GetName()))
                return IMS_FALSE;
        }
    }

    /// Evaluates the IARI
    if (pServiceConfig->IsIARISupported())
    {
        const ServiceIdentifier& objIARI = pServiceConfig->GetIARI();

        if (!IsIARISupported(objIARI.GetName()))
            return IMS_FALSE;
    }

    /// Evaluates the feature-tags
    const IMSList<ServiceIdentifier>& objLocalFeatureTags = pServiceConfig->GetFeatureTags();

    if (!objLocalFeatureTags.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objLocalFeatureTags.GetSize(); ++i)
        {
            const ServiceIdentifier& objFTag = objLocalFeatureTags.GetAt(i);

            if (!IsFeatureTagSupported(objFTag.GetName()))
                return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL void RemoteCapabilities::RemoveAllFeatureSets(
        IN_OUT IMSList<FeatureSet*>& objFeatureSets)
{
    //---------------------------------------------------------------------------------------------

    if (objFeatureSets.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objFeatureSets.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objFeatureSets.GetAt(i);

        if (pFeatureSet != IMS_NULL)
            delete pFeatureSet;
    }

    objFeatureSets.Clear();
}
