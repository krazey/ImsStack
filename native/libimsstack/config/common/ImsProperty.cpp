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
#include "IMSLib.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "private/ImsProperty.h"

__IMS_TRACE_TAG_CONF__;

PUBLIC GLOBAL const IMS_CHAR ImsProperty::STREAM_MEDIA_TYPE_AUDIO[] = "Audio";
PUBLIC GLOBAL const IMS_CHAR ImsProperty::STREAM_MEDIA_TYPE_VIDEO[] = "Video";

PUBLIC GLOBAL const IMS_CHAR* ImsProperty::PKEY_STRING[ImsProperty::PKEY_MAX] = {
        "",
        "Stream",
        "Framed",
        "Basic",
        "Event",
        "CoreService",
        "Qos",
        "Reg",
        "Write",
        "Read",
        "Cap",
        "Mprof",
        "Connection",
};

PUBLIC
ImsProperty::ImsProperty(IN IMS_SINT32 nKey, IN const AString& strKey /*= AString::ConstNull()*/) :
        m_nKey(nKey),
        m_strKey(strKey)
{
    if ((m_nKey > PKEY_CUSTOM) && (m_nKey < PKEY_MAX))
    {
        m_strKey = PKEY_STRING[m_nKey];
    }
}

PUBLIC
ImsProperty::ImsProperty(IN const ImsProperty& other) :
        m_nKey(other.m_nKey),
        m_strKey(other.m_strKey)
{
}

PUBLIC VIRTUAL ImsProperty::~ImsProperty() {}

PUBLIC
ImsProperty& ImsProperty::operator=(IN const ImsProperty& other)
{
    if (this != &other)
    {
        m_nKey = other.m_nKey;
        m_strKey = other.m_strKey;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL ImsProperty::Equals(IN const AString& strValue) const
{
    return m_strKey.Equals(strValue);
}

PUBLIC VIRTUAL IMS_BOOL ImsProperty::Equals(IN const ImsProperty& objOther) const
{
    return Equals(objOther.m_strKey);
}

PUBLIC GLOBAL AStringArray ImsProperty::Decode(IN const AString& strValue)
{
    if (strValue.GetLength() == 0)
    {
        return AStringArray();
    }

    AStringArray objTokens;
    AString strCurrentToken;

    for (IMS_SINT32 i = 0; i < strValue.GetLength(); ++i)
    {
        const IMS_CHAR ch = strValue[i];

        if (IMS_ISSPACE(ch))
        {
            if (strCurrentToken.GetLength() > 0)
            {
                objTokens.AddElement(strCurrentToken);
                strCurrentToken.Resize(0);
            }
        }
        else
        {
            strCurrentToken.Append(ch);
        }
    }

    if (strCurrentToken.GetLength() > 0)
    {
        objTokens.AddElement(strCurrentToken);
    }

    return objTokens;
}

PUBLIC GLOBAL AString ImsProperty::Encode(IN const AStringArray& objValues)
{
    if (objValues.IsEmpty())
    {
        return AString::ConstEmpty();
    }

    AString strValue = objValues.GetElementAt(0);

    for (IMS_SINT32 i = 1; i < objValues.GetCount(); ++i)
    {
        strValue.Append(TextParser::CHAR_SP);
        strValue.Append(objValues.GetElementAt(i));
    }

    return strValue;
}

PUBLIC GLOBAL IMS_BOOL ImsProperty::CheckDuplicate(
        IN const AStringArray& objValues, IN IMS_BOOL bCaseSensitive)
{
    IMS_SINT32 nValueCount = objValues.GetCount();

    if (bCaseSensitive)
    {
        for (IMS_SINT32 i = 0; i < nValueCount; ++i)
        {
            const AString& strValue = objValues.GetElementAt(i);

            for (IMS_SINT32 j = i + 1; j < nValueCount; ++j)
            {
                // Check if the two values equals or not
                if (strValue.Equals(objValues.GetElementAt(j)))
                {
                    IMS_TRACE_E(0,
                            "Set value (%s) contains duplicates, "
                            "item (%s) is equal to item (%s)",
                            ToString(objValues).GetStr(), strValue.GetStr(),
                            objValues.GetElementAt(j).GetStr());
                    return IMS_FALSE;
                }
            }
        }
    }
    else
    {
        for (IMS_SINT32 i = 0; i < nValueCount; ++i)
        {
            const AString& strValue = objValues.GetElementAt(i);

            for (IMS_SINT32 j = i + 1; j < nValueCount; ++j)
            {
                // Check if the two values equals or not
                if (strValue.EqualsIgnoreCase(objValues.GetElementAt(j)))
                {
                    IMS_TRACE_E(0,
                            "Set value (%s) contains duplicates, "
                            "item (%s) is equal to item (%s)",
                            ToString(objValues).GetStr(), strValue.GetStr(),
                            objValues.GetElementAt(j).GetStr());
                    return IMS_FALSE;
                }
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL AString ImsProperty::KeyToString(IN IMS_SINT32 nKey)
{
    if ((nKey > PKEY_CUSTOM) && (nKey < PKEY_MAX))
    {
        return AString(PKEY_STRING[nKey]);
    }

    return AString();
}

PUBLIC GLOBAL IMS_SINT32 ImsProperty::StringToKey(IN const AString& strKey)
{
    if (strKey.GetLength() == 0)
    {
        return PKEY_MAX;
    }

    for (IMS_SINT32 i = (PKEY_CUSTOM + 1); i < PKEY_MAX; ++i)
    {
        if (strKey.Equals(PKEY_STRING[i]))
        {
            return i;
        }
    }

    return PKEY_CUSTOM;
}

PUBLIC GLOBAL IMS_BOOL ImsProperty::TrimAndCheckProperties(
        IN const ImsRegistry& objRegistry, OUT ImsRegistry& objNewRegistry)
{
    AStringArray objKeys;
    AStringArray objNewProperty;

    for (IMS_SINT32 i = 0; i < objRegistry.GetCount(); ++i)
    {
        const AStringArray& objProperty = objRegistry.GetAt(i);

        if (objProperty.GetCount() < 1)
        {
            IMS_TRACE_E(0, "Property (%d) is malformed", i, 0, 0);
            return IMS_FALSE;
        }

        const AString& strKey = objProperty.GetElementAt(0);
        IMS_SINT32 nKeyEnum = StringToKey(strKey);

        if (nKeyEnum == PKEY_CUSTOM)
        {
            IMS_TRACE_E(0, "Property key (%s) is malformed", strKey.GetStr(), 0, 0);
            return IMS_FALSE;
        }

        if ((nKeyEnum != PKEY_CORE_SERVICE) && (nKeyEnum != PKEY_QOS) && (nKeyEnum != PKEY_REG) &&
                (nKeyEnum != PKEY_CAP) && (nKeyEnum != PKEY_MPROF) &&
                (nKeyEnum != PKEY_CONNECTION) && objKeys.Contains(strKey))
        {
            IMS_TRACE_E(0, "Property (%d) is malformed, key (%s) appears multiple times.", i,
                    strKey.GetStr(), 0);
            return IMS_FALSE;
        }

        objKeys.AddElement(strKey);

        for (IMS_SINT32 j = 0; j < objProperty.GetCount(); ++j)
        {
            if (j != 0)
            {
                objNewProperty.AddElement(objProperty.GetElementAt(j).Trim());
            }
            else
            {
                objNewProperty.AddElement(objProperty.GetElementAt(j));
            }
        }

        objNewRegistry.Add(objNewProperty);

        objNewProperty.RemoveAllElements();
    }

    return IMS_TRUE;
}

// DEBUG
PUBLIC GLOBAL AString ImsProperty::ToString(IN const AStringArray& objProperty)
{
    if (objProperty.IsEmpty())
    {
        return AString("__NULL__");
    }

    AString strProperty;

    strProperty.Append('{');

    // First element
    strProperty.Append(TextParser::CHAR_DQUOT);
    strProperty.Append(objProperty.GetElementAt(0));
    strProperty.Append(TextParser::CHAR_DQUOT);

    // Other elements from 2nd element
    for (IMS_SINT32 i = 1; i < objProperty.GetCount(); ++i)
    {
        strProperty.Append(TextParser::CHAR_COMMA);
        strProperty.Append(TextParser::CHAR_SP);
        strProperty.Append(TextParser::CHAR_DQUOT);
        strProperty.Append(objProperty.GetElementAt(i));
        strProperty.Append(TextParser::CHAR_DQUOT);
    }

    strProperty.Append('}');

    return strProperty;
}
