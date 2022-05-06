/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "TextParser.h"
#include "Feature.h"
#include "ISipHeader.h"
#include "SipParameter.h"
#include "util/PreferenceHeader.h"

PUBLIC
PreferenceHeader::PreferenceHeader(IN CONST AString& strHeader) :
        objPreferenceFeatures(IMSList<FeatureSet*>()),
        bExplicit(IMS_FALSE),
        bRequire(IMS_FALSE)
{
    IMSList<AString> objTokens = strHeader.Split(TextParser::CHAR_SEMICOLON);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        const AString& strToken = objTokens.GetAt(i);

        if (strToken.Equals(TextParser::CHAR_ASTERISK))
        {
            // Ignore this string, mandatory field
        }
        else if (strToken.EqualsIgnoreCase(Feature::FLAG_EXPLICIT))
            bExplicit = IMS_TRUE;
        else if (strToken.EqualsIgnoreCase(Feature::FLAG_REQUIRE))
            bRequire = IMS_TRUE;
        else
        {
            ExtractProperties(strToken);
        }
    }
}

PUBLIC
PreferenceHeader::PreferenceHeader(IN CONST ISipHeader* piHeader) :
        objPreferenceFeatures(IMSList<FeatureSet*>()),
        bExplicit(IMS_FALSE),
        bRequire(IMS_FALSE)
{
    //---------------------------------------------------------------------------------------------

    if (piHeader == IMS_NULL)
    {
        return;
    }

    const IMSList<SipParameter*>& objParameters = piHeader->GetParameters();

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        const SipParameter* pParameter = objParameters.GetAt(i);

        if (pParameter == IMS_NULL)
            continue;

        const AString& strTag = pParameter->GetName();

        if (strTag.EqualsIgnoreCase(Feature::FLAG_EXPLICIT))
            bExplicit = IMS_TRUE;
        else if (strTag.EqualsIgnoreCase(Feature::FLAG_REQUIRE))
            bRequire = IMS_TRUE;
        else
        {
            if (pParameter->IsNameOnly())
            {
                AddFeature(strTag);
            }
            else
            {
                const AStringArray& objValues = pParameter->GetValues();

                for (IMS_SINT32 j = 0; j < objValues.GetCount(); ++j)
                {
                    AddFeature(strTag, objValues.GetElementAt(j));
                }
            }
        }
    }
}

PUBLIC
PreferenceHeader::PreferenceHeader(IN IMS_BOOL bExplicit_, IN IMS_BOOL bRequire_) :
        objPreferenceFeatures(IMSList<FeatureSet*>()),
        bExplicit(bExplicit_),
        bRequire(bRequire_)
{
}

PUBLIC
PreferenceHeader::~PreferenceHeader()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objPreferenceFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objPreferenceFeatures.GetAt(i);

        if (pFeatureSet != IMS_NULL)
            delete pFeatureSet;
    }
}

PUBLIC
IMS_BOOL PreferenceHeader::AddFeature(IN CONST AString& strTag)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Add(strTag) == FeatureSet::OP_FAIL)
            return IMS_FALSE;
    }
    else
    {
        if (!Attach(strTag, AString::ConstNull()))
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PreferenceHeader::AddFeature(IN CONST AString& strTag, IN CONST AString& strValue)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Add(strTag, strValue) == FeatureSet::OP_FAIL)
            return IMS_FALSE;
    }
    else
    {
        if (!Attach(strTag, strValue))
            return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PreferenceHeader::Contains(IN CONST AString& strTag) const
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet == IMS_NULL)
        return IMS_FALSE;

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PreferenceHeader::Contains(IN CONST AString& strTag, IN CONST AString& strValue) const
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet == IMS_NULL)
        return IMS_FALSE;

    if (!pFeatureSet->Contains(strValue))
        return IMS_FALSE;

    return IMS_TRUE;
}

PUBLIC
const IMSList<FeatureSet*>& PreferenceHeader::GetFeatureSets() const
{
    //---------------------------------------------------------------------------------------------

    return objPreferenceFeatures;
}

PUBLIC
IMS_BOOL PreferenceHeader::IsExplicitPresent() const
{
    //---------------------------------------------------------------------------------------------

    return bExplicit;
}

PUBLIC
IMS_BOOL PreferenceHeader::IsRequirePresent() const
{
    //---------------------------------------------------------------------------------------------

    return bRequire;
}

PUBLIC
AString PreferenceHeader::ToString() const
{
    AString strHeader(TextParser::STR_ASTERISK, 1);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objPreferenceFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = objPreferenceFeatures.GetAt(i);

        strHeader.Append(TextParser::CHAR_SEMICOLON);
        strHeader.Append(pFeatureSet->ToString());
    }

    if (bRequire)
    {
        strHeader.Append(TextParser::CHAR_SEMICOLON);
        strHeader.Append(Feature::FLAG_REQUIRE);
    }

    if (bExplicit)
    {
        strHeader.Append(TextParser::CHAR_SEMICOLON);
        strHeader.Append(Feature::FLAG_EXPLICIT);
    }

    return strHeader;
}

PRIVATE
IMS_BOOL PreferenceHeader::Attach(
        IN CONST AString& strTag, IN CONST AString& strValue /* = AString::ConstNull() */)
{
    FeatureSet* pFeatureSet = new FeatureSet(strTag);

    //---------------------------------------------------------------------------------------------

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (strValue.IsNULL())
        pFeatureSet->Add(strTag);
    else
        pFeatureSet->Add(strTag, strValue);

    if (!objPreferenceFeatures.Append(pFeatureSet))
    {
        delete pFeatureSet;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void PreferenceHeader::Detach(IN CONST AString& strTag)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objPreferenceFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = objPreferenceFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            objPreferenceFeatures.RemoveAt(i);
            return;
        }
    }
}

PRIVATE
FeatureSet* PreferenceHeader::Lookup(IN CONST AString& strTag) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objPreferenceFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objPreferenceFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            return pFeatureSet;
        }
    }

    // If not found,
    return IMS_NULL;
}

PRIVATE
void PreferenceHeader::ExtractProperties(IN CONST AString& strFeatureSet)
{
    AString strTag;
    AString strValue;
    IMS_SINT32 nCount = strFeatureSet.SplitF(TextParser::CHAR_EQUAL, strTag, strValue);

    //---------------------------------------------------------------------------------------------

    if (nCount == 1)
    {
        AddFeature(strTag);
    }
    else if (nCount == 2)
    {
        strValue = TextParser::TrimDQUOT(strValue);

        if (strValue.Contains(TextParser::CHAR_COMMA))
        {
            IMSList<AString> objTokens = strValue.Split(TextParser::CHAR_COMMA);

            for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
            {
                AddFeature(strTag, objTokens.GetAt(i));
            }
        }
        else
        {
            AddFeature(strTag, strValue);
        }
    }
}
