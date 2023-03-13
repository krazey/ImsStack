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
#include "AStringBuffer.h"
#include "ImsLib.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Feature.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC GLOBAL const IMS_CHAR* Feature::BASE_TAG[Feature::BASE_MAX] = {
        "audio",
        "application",
        "data",
        "control",
        "video",
        "text",
        "automata",
        "class",
        "duplex",
        "mobility",
        "description",
        "events",
        "priority",
        "methods",
        "extensions",
        "schemes",
        "actor",
        "isfocus",
        "language",
        "type",
};

PUBLIC GLOBAL const IMS_CHAR Feature::OTHER_APP_SUBTYPE[] = "app_subtype";
PUBLIC GLOBAL const IMS_CHAR Feature::OTHER_MESSAGE[] = "message";
PUBLIC GLOBAL const IMS_CHAR Feature::OTHER_G_3GPP_IARI_REF[] = "+g.3gpp.iari-ref";
PUBLIC GLOBAL const IMS_CHAR Feature::OTHER_G_3GPP_ICSI_REF[] = "+g.3gpp.icsi-ref";
PUBLIC GLOBAL const IMS_CHAR Feature::FLAG_EXPLICIT[] = "explicit";
PUBLIC GLOBAL const IMS_CHAR Feature::FLAG_REQUIRE[] = "require";
PUBLIC GLOBAL const IMS_CHAR Feature::PREFIX_3GPP_IARI[] = "urn:urn-7:3gpp-application.ims.iari";
PUBLIC GLOBAL const IMS_CHAR Feature::PREFIX_3GPP_ICSI[] = "urn:urn-7:3gpp-service.ims.icsi";

PUBLIC
Feature::Feature(IN const AString& strFeature) :
        RcObject(),
        m_nType(NONE),
        m_bCaseSensitivity(IMS_FALSE),
        m_strTag(AString::ConstNull()),
        m_strValue(AString::ConstNull())
{
    ExtractProperties(strFeature);
}

PUBLIC
Feature::Feature(IN const AString& strTag, IN const AString& strValue) :
        RcObject(),
        m_nType(NONE),
        m_bCaseSensitivity(IMS_FALSE),
        m_strTag(AString::ConstNull()),
        m_strValue(AString::ConstNull())
{
    if (strValue.GetLength() == 0)
    {
        ExtractProperties(strTag);
    }
    else
    {
        ExtractProperties(strTag + TextParser::CHAR_EQUAL + strValue);
    }
}

PUBLIC
Feature::Feature(IN const Feature& other) :
        RcObject(other),
        m_nType(other.m_nType),
        m_bCaseSensitivity(other.m_bCaseSensitivity),
        m_strTag(other.m_strTag),
        m_strValue(other.m_strValue)
{
}

PUBLIC VIRTUAL Feature::~Feature() {}

PUBLIC
Feature& Feature::operator=(IN const Feature& other)
{
    if (this != &other)
    {
        RcObject::operator=(other);
        m_nType = other.m_nType;
        m_bCaseSensitivity = other.m_bCaseSensitivity;
        m_strTag = other.m_strTag;
        m_strValue = other.m_strValue;
    }

    return (*this);
}

PUBLIC
IMS_BOOL Feature::Equals(IN const Feature* pOther) const
{
    if (pOther == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (m_nType != pOther->m_nType)
    {
        return IMS_FALSE;
    }

    if (!m_strTag.EqualsIgnoreCase(pOther->m_strTag))
    {
        return IMS_FALSE;
    }

    if (m_nType & BOOLEAN)
    {
        if ((m_strValue.GetLength() == 0) && (pOther->m_strValue.GetLength() == 0))
        {
            return IMS_TRUE;
        }

        if (m_bCaseSensitivity && pOther->m_bCaseSensitivity)
        {
            if ((m_strValue.GetLength() == 0) && pOther->m_strValue.Equals(TextParser::STR_TRUE))
            {
                return IMS_TRUE;
            }

            if ((pOther->m_strValue.GetLength() == 0) && m_strValue.Equals(TextParser::STR_TRUE))
            {
                return IMS_TRUE;
            }

            if (m_strValue.Equals(pOther->m_strValue))
            {
                return IMS_TRUE;
            }
        }
        else
        {
            if ((m_strValue.GetLength() == 0) &&
                    pOther->m_strValue.EqualsIgnoreCase(TextParser::STR_TRUE))
            {
                return IMS_TRUE;
            }

            if ((pOther->m_strValue.GetLength() == 0) &&
                    m_strValue.EqualsIgnoreCase(TextParser::STR_TRUE))
            {
                return IMS_TRUE;
            }

            if (m_strValue.EqualsIgnoreCase(pOther->m_strValue))
            {
                return IMS_TRUE;
            }
        }

        return IMS_FALSE;
    }

    if (m_bCaseSensitivity)
    {
        if (!m_strValue.Equals(pOther->m_strValue))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        if (!m_strValue.EqualsIgnoreCase(pOther->m_strValue))
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL Feature::IsSameTag(IN const Feature* pOther) const
{
    if (pOther == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return m_strTag.EqualsIgnoreCase(pOther->m_strTag);
}

PUBLIC
IMS_BOOL Feature::IsTagOnly() const
{
    if (m_nType == BOOLEAN)
    {
        return IMS_TRUE;
    }

    if (m_strValue.GetLength() == 0)
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC
AString Feature::ToString() const
{
    if (m_nType & BOOLEAN)
    {
        return m_strTag;
    }

    if (m_strValue.GetLength() == 0)
    {
        return m_strTag;
    }

    AString strFeature(m_strTag);

    // If the equal('=') is present, DQUOTE MUST be present.
    strFeature.Append(TextParser::CHAR_EQUAL);
    strFeature.Append(TextParser::CHAR_DQUOT);

    if ((m_nType & IARI) || (m_nType & ICSI))
    {
        strFeature.Append(DoPercentEncoding(m_strValue));
    }
    else
    {
        strFeature.Append(m_strValue);
    }

    strFeature.Append(TextParser::CHAR_DQUOT);

    return strFeature;
}

PUBLIC
AString Feature::ToStringValueOnly() const
{
    if ((m_nType & IARI) || (m_nType & ICSI))
    {
        return DoPercentEncoding(m_strValue);
    }

    return m_strValue;
}

PUBLIC GLOBAL const IMS_CHAR* Feature::GetFeatureTag(IN IMS_SINT32 nBaseTag)
{
    return IsValidBaseTag(nBaseTag) ? BASE_TAG[nBaseTag] : IMS_NULL;
}

PUBLIC GLOBAL IMS_BOOL Feature::IsFeatureTag(IN const AString& strName)
{
    if (IsBaseTag(strName))
    {
        return IMS_TRUE;
    }

    if (!strName.StartsWith(TextParser::CHAR_PLUS))
    {
        return IMS_FALSE;
    }

    if (strName.GetLength() < 2)
    {
        return IMS_FALSE;
    }

    if (!IMS_ISALPHA(strName[1]))
    {
        return IMS_FALSE;
    }

    // ftag-name = ALPHA * (ALPHA / DIGIT / "!" / "'" / "." / "-" / "%")
    // 0x21, 0x27, 0x2E, 0x2D, 0x25
    const IMS_SINT32 nLen = strName.GetLength();

    for (IMS_SINT32 i = 2; i < nLen; ++i)
    {
        if (IMS_ISALPHA(strName[i]) || IMS_ISDIGIT(strName[i]) || (strName[i] == 0x21) ||
                (strName[i] == 0x27) || (strName[i] == 0x2E) || (strName[i] == 0x2D) ||
                (strName[i] == 0x25))
        {
            continue;
        }
        else
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PRIVATE
void Feature::ExtractProperties(IN const AString& strFeature)
{
    IMS_SINT32 nCount = strFeature.SplitF(TextParser::CHAR_EQUAL, m_strTag, m_strValue);

    if (nCount == 0)
    {
        m_strTag = AString::ConstNull();
        m_strValue = AString::ConstNull();

        IMS_TRACE_D("Feature is null or empty", 0, 0, 0);
        return;
    }

    if (nCount == 1)
    {
        m_nType = BOOLEAN;
        m_strValue = AString::ConstNull();
    }
    else if (nCount == 2)
    {
        // feature-param := enc-feature-tag [EQUAL LDQUOT (tag-value-list/string-value) RDQUOT]

        // If the value has a DQUOT, then remove it.
        m_strValue = TextParser::TrimDquot(m_strValue);

        m_nType = TOKEN_NOBANG;

        // string-value := '<value>'
        if (m_strValue.StartsWith(TextParser::CHAR_LAQUOT) &&
                m_strValue.EndsWith(TextParser::CHAR_RAQUOT))
        {
            m_nType = STRING_VALUE;

            // If the value is a string-value, the value is compared in case-sensitively.
            m_bCaseSensitivity = IMS_TRUE;
        }
        // numeric := "#" numeric-relation number
        else if (m_strValue.StartsWith("!#") || m_strValue.StartsWith(TextParser::CHAR_SHARP))
        {
            m_nType = NUMERIC;
        }

        // "boolean" value can be treated as tokens.
        // So, case-insensitive string comparison will be done.
        if (m_strValue.EqualsIgnoreCase(TextParser::STR_TRUE) ||
                m_strValue.EqualsIgnoreCase(TextParser::STR_FALSE))
        {
            m_nType = BOOLEAN;
        }
    }

    // Check if the tag is the base tag
    // If it is a base tag, then check the prefix and if present, strip the prefix.
    if (IsBaseTag(m_strTag))
    {
        m_strTag = StripPrefixInSipTree(m_strTag);
    }
    else
    {
        // Other tags: it starts with '+'
        if (m_strTag.StartsWith(TextParser::CHAR_PLUS))
        {
            if (m_strTag.EqualsIgnoreCase(OTHER_G_3GPP_IARI_REF))
            {
                m_nType = IARI;

                // IMS_TRACE_D(".... IARI (before decoding) : %s", strValue.GetStr(), 0, 0);
                m_strValue = DoPercentDecoding(m_strValue);
                // IMS_TRACE_D(".... IARI (after decoding) : %s", strValue.GetStr(), 0, 0);
            }
            else if (m_strTag.EqualsIgnoreCase(OTHER_G_3GPP_ICSI_REF))
            {
                m_nType = ICSI;
                m_strValue = DoPercentDecoding(m_strValue);
            }
        }
    }

    // In case of "methods" feature-tag, case-sensitive string comparison will be done.
    if (m_strTag.EqualsIgnoreCase(BASE_TAG[BASE_METHODS]))
    {
        m_bCaseSensitivity = IMS_TRUE;
    }
}

PRIVATE GLOBAL AString Feature::DoPercentDecoding(IN const AString& strValue)
{
    AString strNewValue;
    const IMS_SINT32 nLen = strValue.GetLength();

    for (IMS_SINT32 i = 0; i < nLen; ++i)
    {
        if (strValue[i] != TextParser::CHAR_PERCENT)
        {
            strNewValue.Append(strValue[i]);
        }
        else
        {
            AString strHexDigs = strValue.GetSubStr(i + 1, 2);
            IMS_SINT32 ch = TextParser::HexStringToChar(strHexDigs);

            if ((ch < 0x00) || (ch > 0xFF))
            {
                strNewValue.Append(TextParser::CHAR_PERCENT);
                strNewValue.Append(strHexDigs);
            }
            else
            {
                strNewValue.Append(static_cast<const IMS_CHAR>(ch));
            }

            i += 2;
        }
    }

    return strNewValue;
}

PRIVATE GLOBAL AString Feature::DoPercentEncoding(IN const AString& strValue)
{
    // tag-value = ["!"] (token-nobang / boolean / numeric)
    // token-nobang = 1*(alphanum / "-" / "." / "%" / "*" / "_" / "+" / "`" / "'" / "~")
    //    0x21("!"), 0x2D, 0x2E, 0x45, 0x2A, 0x5F, 0x2B, 0x60, 0x47, 0x7E
    AString strNewValue;
    const IMS_SINT32 nLen = strValue.GetLength();

    for (IMS_SINT32 i = 0; i < nLen; ++i)
    {
        if (IMS_ISALPHA(strValue[i]) || IMS_ISDIGIT(strValue[i]) || (strValue[i] == 0x21) ||
                (strValue[i] == 0x2D) || (strValue[i] == 0x2E) || (strValue[i] == 0x45) ||
                (strValue[i] == 0x2A) || (strValue[i] == 0x5F) || (strValue[i] == 0x2B) ||
                (strValue[i] == 0x60) || (strValue[i] == 0x47) || (strValue[i] == 0x7E))
        {
            strNewValue.Append(strValue[i]);
        }
        else
        {
            strNewValue.Append(TextParser::CHAR_PERCENT);
            strNewValue.Append(TextParser::CharToHexString(strValue[i]));
        }
    }

    return strNewValue;
}

PRIVATE GLOBAL IMS_BOOL Feature::IsBaseTag(IN const AString& strName)
{
    AString strTemp = strName.MakeLower();

    if (strTemp.StartsWith(TextParser::CHAR_PLUS))
    {
        return IMS_FALSE;
    }

    if (strTemp.StartsWith("sip."))
    {
        for (IMS_SINT32 i = 0; i < BASE_MAX; ++i)
        {
            if (strTemp.EndsWith(BASE_TAG[i]))
            {
                return IMS_TRUE;
            }
        }
    }
    else
    {
        for (IMS_SINT32 i = 0; i < BASE_MAX; ++i)
        {
            if (strTemp.Equals(BASE_TAG[i]))
            {
                return IMS_TRUE;
            }
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL Feature::IsValidBaseTag(IN IMS_SINT32 nBaseTag)
{
    return (BASE_AUDIO <= nBaseTag) && (nBaseTag < BASE_MAX);
}

PRIVATE GLOBAL AString Feature::StripPrefixInSipTree(IN const AString& strName)
{
    IMS_SINT32 nStartIndex = strName.GetIndexOf(TextParser::CHAR_DOT);

    if (nStartIndex == AString::NPOS)
    {
        return strName;
    }

    return strName.GetSubStr(nStartIndex + 1);
}

PUBLIC
FeatureSet::FeatureSet(IN const AString& strTag) :
        m_strTag(strTag),
        m_objFeatures(ImsList<Feature*>())
{
}

PUBLIC
FeatureSet::FeatureSet(IN const AString& strTag, IN const AString& strValues) :
        m_strTag(strTag),
        m_objFeatures(ImsList<Feature*>())
{
    AddFeatures(strValues);
}

PUBLIC
FeatureSet::~FeatureSet()
{
    if (!m_objFeatures.IsEmpty())
    {
        for (IMS_UINT32 i = 0; i < m_objFeatures.GetSize(); ++i)
        {
            Feature* pFeature = m_objFeatures.GetAt(i);

            if (pFeature != IMS_NULL)
            {
                delete pFeature;
            }
        }

        m_objFeatures.Clear();
    }
}

PUBLIC
IMS_SINT32 FeatureSet::Add(IN const AString& strTag)
{
    RcPtr<Feature> rcpFeature = new Feature(strTag);

    return Add(rcpFeature.Get());
}

PUBLIC
IMS_SINT32 FeatureSet::Add(IN const AString& strTag, IN const AString& strValue)
{
    RcPtr<Feature> rcpFeature = new Feature(strTag, strValue);

    return Add(rcpFeature.Get());
}

PUBLIC
IMS_BOOL FeatureSet::Contains(IN const Feature* pFeature) const
{
    for (IMS_UINT32 i = 0; i < m_objFeatures.GetSize(); ++i)
    {
        const Feature* pExFeature = m_objFeatures.GetAt(i);

        if (pExFeature->Equals(pFeature))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
IMS_BOOL FeatureSet::Contains(IN const AString& strValue) const
{
    if (IsEmpty())
    {
        return IMS_FALSE;
    }

    Feature objFeature(m_strTag, strValue);

    for (IMS_UINT32 i = 0; i < m_objFeatures.GetSize(); ++i)
    {
        const Feature* pExFeature = m_objFeatures.GetAt(i);

        if (pExFeature->Equals(&objFeature))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC
Feature* FeatureSet::Lookup(IN const Feature* pFeature, IN IMS_BOOL bDetach /*= IMS_FALSE*/)
{
    for (IMS_UINT32 i = 0; i < m_objFeatures.GetSize(); ++i)
    {
        const Feature* pExFeature = m_objFeatures.GetAt(i);

        if (pExFeature->Equals(pFeature))
        {
            if (bDetach && !pExFeature->IsShared())
            {
                m_objFeatures.RemoveAt(i);
            }

            return const_cast<Feature*>(pExFeature);
        }
    }

    // If match not found,
    return IMS_NULL;
}

PUBLIC
IMS_SINT32 FeatureSet::Remove(IN const AString& strTag)
{
    RcPtr<Feature> rcpFeature = new Feature(strTag);

    return Remove(rcpFeature.Get());
}

PUBLIC
IMS_SINT32 FeatureSet::Remove(IN const AString& strTag, IN const AString& strValue)
{
    RcPtr<Feature> rcpFeature = new Feature(strTag, strValue);

    return Remove(rcpFeature.Get());
}

PUBLIC
AString FeatureSet::ToString() const
{
    if (IsEmpty())
    {
        return AString::ConstNull();
    }

    const Feature* pFeature = m_objFeatures.GetAt(0);

    if (m_objFeatures.GetSize() == 1)
    {
        return pFeature->ToString();
    }
    else
    {
        // If the multiple feature values are present, it will be separated with COMMA.
        AStringBuffer objBuffer(512);

        // Tag
        objBuffer.Append(pFeature->GetTag());

        // EQUAL('=') & DQUOT('"')
        objBuffer.Append(TextParser::CHAR_EQUAL);
        objBuffer.Append(TextParser::CHAR_DQUOT);

        // Values
        objBuffer.Append(pFeature->ToStringValueOnly());

        for (IMS_UINT32 i = 1; i < m_objFeatures.GetSize(); ++i)
        {
            pFeature = m_objFeatures.GetAt(i);

            objBuffer.Append(TextParser::CHAR_COMMA);
            objBuffer.Append(pFeature->ToStringValueOnly());
        }

        // DQUOT('"')
        objBuffer.Append(TextParser::CHAR_DQUOT);

        return static_cast<const AStringBuffer&>(objBuffer).GetString();
    }
}

PUBLIC GLOBAL FeatureSet* FeatureSet::FromServiceIdentifier(
        IN const ServiceIdentifier& objServiceId)
{
    FeatureSet* pFeatureSet = IMS_NULL;
    const AString& strName = objServiceId.GetName();
    IMS_SINT32 nEqualIndex = strName.GetIndexOf(TextParser::CHAR_EQUAL);

    if (nEqualIndex == AString::NPOS)
    {
        pFeatureSet = new FeatureSet(strName);

        pFeatureSet->Add(strName);
    }
    else
    {
        AString strTag = strName.GetSubStr(0, nEqualIndex);
        AString strValue = strName.GetSubStr(nEqualIndex + 1);

        pFeatureSet = new FeatureSet(strTag);

        strValue = TextParser::TrimDquot(strValue);

        if (strValue.Contains(TextParser::CHAR_COMMA))
        {
            ImsList<AString> objTokens = strValue.Split(TextParser::CHAR_COMMA);

            for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
            {
                pFeatureSet->Add(strTag, objTokens.GetAt(i));
            }
        }
        else
        {
            pFeatureSet->Add(strTag, strValue);
        }
    }

    return pFeatureSet;
}

PRIVATE
IMS_SINT32 FeatureSet::Add(IN Feature* pFeature)
{
    if (pFeature == IMS_NULL)
    {
        return OP_FAIL;
    }

    if (!m_strTag.EqualsIgnoreCase(pFeature->GetTag()))
    {
        return OP_FAIL;
    }

    Feature* pExFeature = Lookup(pFeature);

    if (pExFeature != IMS_NULL)
    {
        pExFeature->AddReference();
        return OP_ADD_REF;
    }

    pFeature->AddReference();
    m_objFeatures.Append(pFeature);

    return OP_ADD;
}

PRIVATE
void FeatureSet::AddFeatures(IN const AString& strValues)
{
    ImsList<AString> objTokens;

    if (strValues.StartsWith(TextParser::CHAR_DQUOT) && strValues.EndsWith(TextParser::CHAR_DQUOT))
    {
        AString strUnquotedValues = TextParser::TrimDquot(strValues);

        objTokens = strUnquotedValues.Split(TextParser::CHAR_COMMA);
    }
    else
    {
        objTokens = strValues.Split(TextParser::CHAR_COMMA);
    }

    for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
    {
        Feature* pFeature = new Feature(m_strTag, objTokens.GetAt(i));

        if (pFeature != IMS_NULL)
        {
            pFeature->AddReference();
            m_objFeatures.Append(pFeature);
        }
    }
}

PRIVATE
IMS_SINT32 FeatureSet::Remove(IN const Feature* pFeature)
{
    if (pFeature == IMS_NULL)
    {
        return OP_FAIL;
    }

    if (!m_strTag.EqualsIgnoreCase(pFeature->GetTag()))
    {
        return OP_FAIL;
    }

    Feature* pExFeature = Lookup(pFeature, IMS_TRUE);

    if (pExFeature == IMS_NULL)
    {
        return OP_FAIL;
    }

    if (pExFeature->IsShared())
    {
        pExFeature->RemoveReference();
        return OP_REMOVE_REF;
    }
    else
    {
        pExFeature->RemoveReference();
        return OP_REMOVE;
    }
}
