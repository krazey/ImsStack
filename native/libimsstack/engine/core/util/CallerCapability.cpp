/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090609  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "AStringArray.h"
#include "TextParser.h"
#include "private/AppConfig.h"
#include "private/CoreServiceConfig.h"
#include "Feature.h"
#include "ISipConfigV.h"
#include "util/CallerCapability.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
CallerCapability::CallerCapability(IN IMS_UINT32 nID_) :
        nID(nID_),
        objContactFeatures(IMSList<FeatureSet*>())
{
}

/*
PUBLIC
CallerCapability::CallerCapability(IN CONST CallerCapability &objRHS)
    : nID(objRHS.nID)
{
    FeatureSet *pFeatureSet;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objRHS.objContactFeatures.GetSize(); ++i)
    {
        if (objRHS.objContactFeatures.GetAt(i, pFeatureSet))
        {
            FeatureSet *pNewFeatureSet = new FeatureSet(*pFeatureSet);

            objContactFeatures.Append(pNewFeatureSet);
        }
    }
}*/

PUBLIC
CallerCapability::~CallerCapability()
{
    //---------------------------------------------------------------------------------------------

    Clear();
}

/*
PUBLIC
CallerCapability& CallerCapability::operator=(IN CONST CallerCapability &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        FeatureSet *pFeatureSet;

        for (IMS_UINT32 i = 0; i < objRHS.objContactFeatures.GetSize(); ++i)
        {
            if (objRHS.objContactFeatures.GetAt(i, pFeatureSet))
            {
                FeatureSet *pNewFeatureSet = new FeatureSet(*pFeatureSet);

                objContactFeatures.Append(pNewFeatureSet);
            }
        }

        nID = objRHS.nID;
    }

    return (*this);
}*/

PUBLIC
IMS_BOOL CallerCapability::AddFeature(IN CONST Feature* pFeature)
{
    //---------------------------------------------------------------------------------------------

    if (pFeature == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    if (pFeature->IsTagOnly())
        nUpdateFlag = AddFeature(pFeature->GetTag());
    else
        nUpdateFlag = AddFeature(pFeature->GetTag(), pFeature->GetValue());

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::AddFeature(IN CONST FeatureSet* pFeatureSet)
{
    //---------------------------------------------------------------------------------------------

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;
    const IMSList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        if (pFeature->IsTagOnly())
            nUpdateFlag |= AddFeature(pFeature->GetTag());
        else
            nUpdateFlag |= AddFeature(pFeature->GetTag(), pFeature->GetValue());
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::AddFeatures(IN CONST CallerCapability* pCC)
{
    //---------------------------------------------------------------------------------------------

    if (pCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    for (IMS_UINT32 i = 0; i < pCC->objContactFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = pCC->objContactFeatures.GetAt(i);

        const IMSList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

        for (IMS_UINT32 j = 0; j < objFeatures.GetSize(); ++j)
        {
            const Feature* pFeature = objFeatures.GetAt(j);

            if (pFeature->IsTagOnly())
                nUpdateFlag |= AddFeature(pFeature->GetTag());
            else
                nUpdateFlag |= AddFeature(pFeature->GetTag(), pFeature->GetValue());
        }
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
void CallerCapability::Clear()
{
    //---------------------------------------------------------------------------------------------

    // Clear the existing caller capabilities
    if (!objContactFeatures.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < objContactFeatures.GetSize(); ++i)
        {
            FeatureSet* pFeatureSet = objContactFeatures.GetAt(i);

            if (pFeatureSet != IMS_NULL)
                delete pFeatureSet;
        }

        objContactFeatures.Clear();
    }
}

PUBLIC
IMS_BOOL CallerCapability::Create(IN CONST AppConfig* pAppConfig,
        IN CONST CoreServiceConfig* pServiceConfig, IN CONST ISipConfigV* piSipConfigV)
{
    //---------------------------------------------------------------------------------------------

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
    if (pServiceConfig != IMS_NULL)
    {
        // IARI : "+g.3gpp.iari-ref"
        if (pServiceConfig->IsIARISupported())
        {
            AddFeature(Feature::OTHER_G_3GPP_IARI_REF, pServiceConfig->GetIARI().GetName());
        }

        // ICSIs : "+g.3gpp.icsi-ref"
        IMS_UINT32 j = 0;
        const IMSList<ServiceIdentifier>& objICSIs = pServiceConfig->GetICSIs();

        if (!objICSIs.IsEmpty())
        {
            AString strTag(Feature::OTHER_G_3GPP_ICSI_REF);

            for (j = 0; j < objICSIs.GetSize(); ++j)
            {
                AddFeature(strTag, objICSIs.GetAt(j).GetName());
            }
        }

        // Feature tags
        const IMSList<ServiceIdentifier>& objFeatureTags = pServiceConfig->GetFeatureTags();

        if (!objFeatureTags.IsEmpty())
        {
            AString strTag;
            AString strValue;

            for (j = 0; j < objFeatureTags.GetSize(); ++j)
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
        if ((!pAppConfig->IsStreamMediaVideoSupported() &&
                    !pAppConfig->IsStreamMediaAudioSupported()) ||
                (pAppConfig->IsStreamMediaVideoSupported() &&
                        pAppConfig->IsStreamMediaAudioSupported()))
        {
            if ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_AUDIO) != 0)
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_AUDIO]);
            }

            if ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO) != 0)
            {
                AddFeature(Feature::BASE_TAG[Feature::BASE_VIDEO]);
            }
        }
        else if (pAppConfig->IsStreamMediaVideoSupported() &&
                ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_VIDEO) != 0))
        {
            AddFeature(Feature::BASE_TAG[Feature::BASE_VIDEO]);
        }
        else if (pAppConfig->IsStreamMediaAudioSupported() &&
                ((nFeatureTags & ISipConfigV::FEATURE_TAG_MEDIA_STREAM_AUDIO) != 0))
        {
            AddFeature(Feature::BASE_TAG[Feature::BASE_AUDIO]);
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
        IMSList<AString> objTokens;
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
IMS_BOOL CallerCapability::Equals(IN CONST CallerCapability* pCC) const
{
    //---------------------------------------------------------------------------------------------

    if (pCC == IMS_NULL)
        return IMS_FALSE;

    if (nID != pCC->nID)
        return IMS_FALSE;

    return IMS_TRUE;
}

PUBLIC
const IMSList<FeatureSet*>& CallerCapability::GetFeatures() const
{
    //---------------------------------------------------------------------------------------------

    return objContactFeatures;
}

PUBLIC
IMS_BOOL CallerCapability::HasFeature(IN CONST Feature* pFeature) const
{
    //---------------------------------------------------------------------------------------------

    if (pFeature == IMS_NULL)
    {
        return IMS_FALSE;
    }

    FeatureSet* pFeatureSet = Lookup(pFeature->GetTag());

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pFeatureSet->Contains(pFeature);
}

PUBLIC
IMS_BOOL CallerCapability::IsEmpty() const
{
    //---------------------------------------------------------------------------------------------

    return objContactFeatures.IsEmpty();
}

PUBLIC
IMS_BOOL CallerCapability::RemoveFeature(IN CONST Feature* pFeature)
{
    //---------------------------------------------------------------------------------------------

    if (pFeature == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    if (pFeature->IsTagOnly())
        nUpdateFlag = RemoveFeature(pFeature->GetTag());
    else
        nUpdateFlag = RemoveFeature(pFeature->GetTag(), pFeature->GetValue());

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::RemoveFeature(IN CONST FeatureSet* pFeatureSet)
{
    //---------------------------------------------------------------------------------------------

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;
    const IMSList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        const Feature* pFeature = objFeatures.GetAt(i);

        if (pFeature->IsTagOnly())
            nUpdateFlag |= RemoveFeature(pFeature->GetTag());
        else
            nUpdateFlag |= RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
IMS_BOOL CallerCapability::RemoveFeatures(
        IN CONST CallerCapability* pCC, IN IMS_BOOL bRemoveRef /* = IMS_TRUE */)
{
    //---------------------------------------------------------------------------------------------

    if (pCC == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nUpdateFlag = FEATURE_UNCHANGED;

    for (IMS_UINT32 i = 0; i < pCC->objContactFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = pCC->objContactFeatures.GetAt(i);

        const IMSList<Feature*>& objFeatures = pFeatureSet->GetFeatures();

        for (IMS_UINT32 j = 0; j < objFeatures.GetSize(); ++j)
        {
            const Feature* pFeature = objFeatures.GetAt(j);

            if (bRemoveRef)
            {
                if (pFeature->IsTagOnly())
                    nUpdateFlag |= RemoveFeature(pFeature->GetTag());
                else
                    nUpdateFlag |= RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
            }
            else
            {
                while (HasFeature(pFeature))
                {
                    if (pFeature->IsTagOnly())
                        nUpdateFlag |= RemoveFeature(pFeature->GetTag());
                    else
                        nUpdateFlag |= RemoveFeature(pFeature->GetTag(), pFeature->GetValue());
                }
            }
        }
    }

    return ((nUpdateFlag & FEATURE_CHANGED) == FEATURE_CHANGED);
}

PUBLIC
AString CallerCapability::ToString() const
{
    //---------------------------------------------------------------------------------------------

    if (!objContactFeatures.IsEmpty())
    {
        AString strContactFeatures = objContactFeatures.GetAt(0)->ToString();

        for (IMS_UINT32 i = 1; i < objContactFeatures.GetSize(); ++i)
        {
            const FeatureSet* pFeatureSet = objContactFeatures.GetAt(i);

            strContactFeatures.Append(TextParser::CHAR_SEMICOLON);
            strContactFeatures.Append(pFeatureSet->ToString());
        }

        return strContactFeatures;
    }

    return AString::ConstNull();
}

PRIVATE
IMS_BOOL CallerCapability::Attach(
        IN CONST AString& strTag, IN CONST AString& strValue /* = AString::ConstNull() */)
{
    FeatureSet* pFeatureSet = new FeatureSet(strTag);

    //---------------------------------------------------------------------------------------------

    if (strValue.IsNULL())
        pFeatureSet->Add(strTag);
    else
        pFeatureSet->Add(strTag, strValue);

    if (!objContactFeatures.Append(pFeatureSet))
    {
        delete pFeatureSet;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void CallerCapability::Detach(IN CONST AString& strTag)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContactFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = objContactFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            objContactFeatures.RemoveAt(i);
            return;
        }
    }
}

PRIVATE
FeatureSet* CallerCapability::Lookup(IN CONST AString& strTag) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objContactFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objContactFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            return pFeatureSet;
        }
    }

    // If not found,
    return IMS_NULL;
}

PRIVATE
IMS_SINT32 CallerCapability::AddFeature(IN CONST AString& strTag)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet)
    {
        if (pFeatureSet->Add(strTag) == FeatureSet::OP_ADD)
            return FEATURE_CHANGED;
    }
    else
    {
        // TODO:: if failed, what to do ???
        if (Attach(strTag, AString::ConstNull()))
            return FEATURE_CHANGED;
    }

    return FEATURE_UNCHANGED;
}

PRIVATE
IMS_SINT32 CallerCapability::AddFeature(IN CONST AString& strTag, IN CONST AString& strValue)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Add(strTag, strValue) == FeatureSet::OP_ADD)
            return FEATURE_CHANGED;
    }
    else
    {
        // TODO:: if failed, what to do ???
        if (Attach(strTag, strValue))
            return FEATURE_CHANGED;
    }

    return FEATURE_UNCHANGED;
}

PRIVATE
IMS_SINT32 CallerCapability::RemoveFeature(IN CONST AString& strTag)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

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
IMS_SINT32 CallerCapability::RemoveFeature(IN CONST AString& strTag, IN CONST AString& strValue)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

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
