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

#include "Feature.h"
#include "SipParameter.h"
#include "SipPrivate.h"
#include "SipUtils.h"

PUBLIC
SipParameter::SipParameter() :
        m_strName(AString::ConstNull())
{
}

PUBLIC
SipParameter::SipParameter(IN const AString& strName) :
        m_strName(strName)
{
}

PUBLIC
SipParameter::SipParameter(IN const AString& strName, IN const AString& strValue) :
        m_strName(strName)
{
    if (!strValue.IsNULL())
    {
        m_objValues.AddElement(strValue);
    }
}

PUBLIC
SipParameter::SipParameter(IN const AString& strName, IN const AStringArray& objValues) :
        m_strName(strName),
        m_objValues(objValues)
{
}

PUBLIC
SipParameter::SipParameter(IN const SipParameter& other) :
        m_strName(other.m_strName),
        m_objValues(other.m_objValues)
{
}

PUBLIC
SipParameter::~SipParameter() {}

PUBLIC
SipParameter& SipParameter::operator=(IN const SipParameter& other)
{
    if (this != &other)
    {
        m_strName = other.m_strName;
        m_objValues = other.m_objValues;
    }

    return (*this);
}

PUBLIC
void SipParameter::AddValue(IN const AString& strValue)
{
    if (strValue.IsNULL())
    {
        return;
    }

    // To prevent the duplicated parameter value
    if (m_objValues.Contains(strValue, IMS_FALSE))
    {
        return;
    }

    m_objValues.AddElement(strValue);
}

PUBLIC
void SipParameter::AddValues(IN const AString& strValues)
{
    if (strValues.IsNULL())
    {
        return;
    }

    // Check if DQUOTE is present
    if (strValues.StartsWith(TextParser::CHAR_DQUOT))
    {
        IMS_SINT32 nLastDquote = strValues.GetLastIndexOf(TextParser::CHAR_DQUOT);
        AString strTmpVal = strValues.GetSubStr(1, nLastDquote - 1);

        if (strTmpVal.Contains(TextParser::CHAR_COMMA))
        {
            IMSList<AString> objTokens = strTmpVal.Split(TextParser::CHAR_COMMA);

            for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
            {
                const AString& strValue = objTokens.GetAt(i);

                // To prevent the duplicated parameter value
                if (m_objValues.Contains(strValue, IMS_FALSE))
                {
                    continue;
                }

                m_objValues.AddElement(strValue);
            }
        }
        else
        {
            // To prevent the duplicated parameter value
            if (m_objValues.Contains(strValues, IMS_FALSE))
            {
                return;
            }

            m_objValues.AddElement(strValues);
        }
    }
    else if (strValues.Contains(TextParser::CHAR_COMMA))
    {
        IMSList<AString> objTokens = strValues.Split(TextParser::CHAR_COMMA);

        for (IMS_UINT32 i = 0; i < objTokens.GetSize(); ++i)
        {
            const AString& strValue = objTokens.GetAt(i);

            // To prevent the duplicated parameter value
            if (m_objValues.Contains(strValue, IMS_FALSE))
            {
                continue;
            }

            m_objValues.AddElement(strValue);
        }
    }
    else
    {
        // To prevent the duplicated parameter value
        if (m_objValues.Contains(strValues, IMS_FALSE))
        {
            return;
        }

        m_objValues.AddElement(strValues);
    }
}

PUBLIC
IMS_BOOL SipParameter::Create(IN const AString& strParameter)
{
    AString strValue;
    IMS_SINT32 nCount = strParameter.SplitF(TextParser::CHAR_EQUAL, m_strName, strValue);

    if (nCount == 1)
    {
        // Name only parameter
        m_objValues.RemoveAllElements();
    }
    else if (nCount == 2)
    {
        // Extract the parameter values if present
        if (SetValues(strValue) != IMS_SUCCESS)
        {
            return IMS_FALSE;
        }
    }
    else
    {
        m_strName = AString::ConstNull();
        m_objValues.RemoveAllElements();
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipParameter::Equals(IN const SipParameter* pParameter) const
{
    if (pParameter == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strValue = TextParser::DoPercentDecoding(m_strName);
    AString strOtherValue = TextParser::DoPercentDecoding(pParameter->m_strName);

    // 4 Check boolean type parameter

    if (!strValue.EqualsIgnoreCase(strOtherValue))
    {
        return IMS_FALSE;
    }

    IMS_BOOL bFeatureTag = Feature::IsFeatureTag(strValue);

    for (IMS_SINT32 i = 0; i < m_objValues.GetCount(); ++i)
    {
        IMS_BOOL bFound = IMS_FALSE;

        strValue = TextParser::DoPercentDecoding(m_objValues.GetElementAt(i));

        IMS_BOOL bCaseSensitive = strValue.StartsWith(TextParser::CHAR_DQUOT);

        if (bFeatureTag)
        {
            strValue = TextParser::TrimDquot(strValue);
        }

        for (IMS_SINT32 j = 0; j < pParameter->m_objValues.GetCount(); ++j)
        {
            strOtherValue = TextParser::DoPercentDecoding(pParameter->m_objValues.GetElementAt(j));

            if (bFeatureTag)
            {
                strOtherValue = TextParser::TrimDquot(strOtherValue);
            }

            if (bCaseSensitive)
            {
                if (strValue.Equals(strOtherValue))
                {
                    bFound = IMS_TRUE;
                    break;
                }
            }
            else
            {
                if (strValue.EqualsIgnoreCase(strOtherValue))
                {
                    bFound = IMS_TRUE;
                    break;
                }
            }
        }

        if (!bFound)
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
const AString& SipParameter::GetValue() const
{
    if (m_objValues.IsEmpty())
    {
        return AString::ConstNull();
    }

    return m_objValues.GetFirstElement();
}

PUBLIC
void SipParameter::RemoveValue(IN const AString& strValue)
{
    for (IMS_SINT32 i = 0; i < m_objValues.GetCount(); ++i)
    {
        const AString& strExValue = m_objValues.GetElementAt(i);

        if (strExValue.EqualsIgnoreCase(strValue))
        {
            m_objValues.RemoveElementAt(i);
            return;
        }
    }
}

PUBLIC
IMS_RESULT SipParameter::SetValue(IN const AString& strValue)
{
    if (strValue.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    m_objValues.RemoveAllElements();

    if (strValue.IsNULL())
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return IMS_SUCCESS;
    }

    m_objValues.AddElement(strValue);

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT SipParameter::SetValues(IN const AString& strValues)
{
    if (strValues.IsEmpty())
    {
        SipPrivate::SetLastError(SipError::ILLEGAL_ARGUMENT);
        return IMS_FAILURE;
    }

    m_objValues.RemoveAllElements();

    if (strValues.IsNULL())
    {
        SipPrivate::SetLastError(SipError::NO_ERROR);
        return IMS_SUCCESS;
    }

    // Check if DQUOTE is present
    if (strValues.StartsWith(TextParser::CHAR_DQUOT))
    {
        IMS_SINT32 nLastDquote = strValues.GetLastIndexOf(TextParser::CHAR_DQUOT);
        AString strTmpVal = strValues.GetSubStr(1, nLastDquote - 1);

        if (strTmpVal.Contains(TextParser::CHAR_COMMA))
        {
            m_objValues = strTmpVal.Split(TextParser::CHAR_COMMA);
        }
        else
        {
            m_objValues.AddElement(strValues);
        }
    }
    else if (strValues.Contains(TextParser::CHAR_COMMA))
    {
        m_objValues = strValues.Split(TextParser::CHAR_COMMA);
    }
    else
    {
        m_objValues.AddElement(strValues);
    }

    SipPrivate::SetLastError(SipError::NO_ERROR);
    return IMS_SUCCESS;
}

PUBLIC
AString SipParameter::ToString() const
{
    IMS_SINT32 nPValueCount = m_objValues.GetCount();

    if (nPValueCount == 0)
    {
        return m_strName;
    }
    else if (nPValueCount == 1)
    {
        return m_strName + TextParser::CHAR_EQUAL + m_objValues.GetFirstElement();
    }
    else
    {
        AString strParameter(m_strName);

        strParameter += TextParser::CHAR_EQUAL;
        strParameter += TextParser::CHAR_DQUOT;

        if (!m_objValues.IsEmpty())
        {
            strParameter += m_objValues.GetElementAt(0);
        }

        for (IMS_SINT32 i = 1; i < m_objValues.GetCount(); ++i)
        {
            strParameter += TextParser::CHAR_COMMA;
            strParameter += m_objValues.GetElementAt(i);
        }

        strParameter += TextParser::CHAR_DQUOT;

        return strParameter;
    }
}
