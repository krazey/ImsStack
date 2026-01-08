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
#include "TextParser.h"

#include "Feature.h"

#include "ISipHeader.h"
#include "SipParameter.h"
#include "util/PreferenceHeader.h"

PUBLIC
PreferenceHeader::PreferenceHeader(IN const AString& strHeader) :
        m_objPreferenceFeatures(ImsList<FeatureSet*>()),
        m_bExplicit(IMS_FALSE),
        m_bRequire(IMS_FALSE)
{
    ImsList<AString> objTokens = strHeader.Split(TextParser::CHAR_SEMICOLON);

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        const AString& strToken = objTokens.GetAt(i);

        if (strToken.Equals(TextParser::CHAR_ASTERISK))
        {
            // Ignore this string, mandatory field
        }
        else if (strToken.EqualsIgnoreCase(Feature::FLAG_EXPLICIT))
        {
            m_bExplicit = IMS_TRUE;
        }
        else if (strToken.EqualsIgnoreCase(Feature::FLAG_REQUIRE))
        {
            m_bRequire = IMS_TRUE;
        }
        else
        {
            ExtractProperties(strToken);
        }
    }
}

PUBLIC
PreferenceHeader::PreferenceHeader(IN const ISipHeader* piHeader) :
        m_objPreferenceFeatures(ImsList<FeatureSet*>()),
        m_bExplicit(IMS_FALSE),
        m_bRequire(IMS_FALSE)
{
    if (piHeader == IMS_NULL)
    {
        return;
    }

    const ImsList<SipParameter*>& objParameters = piHeader->GetParameters();

    for (IMS_UINT32 i = 0; i < objParameters.GetSize(); ++i)
    {
        const SipParameter* pParameter = objParameters.GetAt(i);

        if (pParameter == IMS_NULL)
        {
            continue;
        }

        const AString& strTag = pParameter->GetName();

        if (strTag.EqualsIgnoreCase(Feature::FLAG_EXPLICIT))
        {
            m_bExplicit = IMS_TRUE;
        }
        else if (strTag.EqualsIgnoreCase(Feature::FLAG_REQUIRE))
        {
            m_bRequire = IMS_TRUE;
        }
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
PreferenceHeader::PreferenceHeader(IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire) :
        m_objPreferenceFeatures(ImsList<FeatureSet*>()),
        m_bExplicit(bExplicit),
        m_bRequire(bRequire)
{
}

PUBLIC
PreferenceHeader::~PreferenceHeader()
{
    for (IMS_UINT32 i = 0; i < m_objPreferenceFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = m_objPreferenceFeatures.GetAt(i);

        if (pFeatureSet != IMS_NULL)
        {
            delete pFeatureSet;
        }
    }
}

PUBLIC
IMS_BOOL PreferenceHeader::AddFeature(IN const AString& strTag)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Add(strTag) == FeatureSet::OP_FAIL)
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!Attach(strTag, AString::ConstNull()))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PreferenceHeader::AddFeature(IN const AString& strTag, IN const AString& strValue)
{
    FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet != IMS_NULL)
    {
        if (pFeatureSet->Add(strTag, strValue) == FeatureSet::OP_FAIL)
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!Attach(strTag, strValue))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PreferenceHeader::Contains(IN const AString& strTag) const
{
    const FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL PreferenceHeader::Contains(IN const AString& strTag, IN const AString& strValue) const
{
    const FeatureSet* pFeatureSet = Lookup(strTag);

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!pFeatureSet->Contains(strValue))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
AString PreferenceHeader::ToString() const
{
    AString strHeader(TextParser::STR_ASTERISK, 1);

    for (IMS_UINT32 i = 0; i < m_objPreferenceFeatures.GetSize(); ++i)
    {
        const FeatureSet* pFeatureSet = m_objPreferenceFeatures.GetAt(i);

        strHeader.Append(TextParser::CHAR_SEMICOLON);
        strHeader.Append(pFeatureSet->ToString());
    }

    if (m_bRequire)
    {
        strHeader.Append(TextParser::CHAR_SEMICOLON);
        strHeader.Append(Feature::FLAG_REQUIRE);
    }

    if (m_bExplicit)
    {
        strHeader.Append(TextParser::CHAR_SEMICOLON);
        strHeader.Append(Feature::FLAG_EXPLICIT);
    }

    return strHeader;
}

PRIVATE
IMS_BOOL PreferenceHeader::Attach(
        IN const AString& strTag, IN const AString& strValue /*= AString::ConstNull()*/)
{
    FeatureSet* pFeatureSet = new FeatureSet(strTag);

    if (pFeatureSet == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (strValue.IsNULL())
    {
        pFeatureSet->Add(strTag);
    }
    else
    {
        pFeatureSet->Add(strTag, strValue);
    }

    if (!m_objPreferenceFeatures.Append(pFeatureSet))
    {
        delete pFeatureSet;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
FeatureSet* PreferenceHeader::Lookup(IN const AString& strTag) const
{
    for (IMS_UINT32 i = 0; i < m_objPreferenceFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = m_objPreferenceFeatures.GetAt(i);

        if (pFeatureSet->GetTag().EqualsIgnoreCase(strTag))
        {
            return pFeatureSet;
        }
    }

    // If not found,
    return IMS_NULL;
}

PRIVATE
void PreferenceHeader::ExtractProperties(IN const AString& strFeatureSet)
{
    AString strTag;
    AString strValue;
    IMS_SINT32 nCount = strFeatureSet.SplitF(TextParser::CHAR_EQUAL, strTag, strValue);

    if (nCount == 1)
    {
        AddFeature(strTag);
    }
    else if (nCount == 2)
    {
        strValue = TextParser::TrimDquot(strValue);

        if (strValue.Contains(TextParser::CHAR_COMMA))
        {
            ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_COMMA);

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
