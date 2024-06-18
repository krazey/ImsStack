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
#ifndef ANSI_STRING_ARRAY_H_
#define ANSI_STRING_ARRAY_H_

#include "AString.h"
#include "TextParser.h"

class AStringArray
{
public:
    AStringArray();
    // cppcheck-suppress noExplicitConstructor
    AStringArray(IN const ImsList<AString>& objElements);
    AStringArray(IN const AStringArray& other);
    ~AStringArray();

public:
    AStringArray& operator=(IN const AStringArray& other);
    AStringArray& operator=(IN const ImsList<AString>& objElements);

public:
    inline void AddElement(IN const AString& strElem) { m_objElements.Append(strElem); }
    IMS_BOOL Contains(IN const AString& strElem, IN IMS_BOOL bCaseSensitive = IMS_TRUE) const;
    inline IMS_SINT32 GetCount() const { return static_cast<IMS_SINT32>(m_objElements.GetSize()); }
    inline const AString& GetElementAt(IN IMS_SINT32 nIndex) const
    {
        return m_objElements.GetAt(nIndex);
    }
    inline const ImsList<AString>& GetElements() const { return m_objElements; }
    const AString& GetFirstElement() const;
    IMS_SINT32 GetIndexOf(IN const AString& strElem, IN IMS_SINT32 nOffset = 0,
            IN IMS_BOOL bCaseSensitive = IMS_TRUE) const;
    const AString& GetLastElement() const;
    IMS_SINT32 GetLastIndexOf(IN const AString& strElem, IN IMS_SINT32 nOffset = 0,
            IN IMS_BOOL bCaseSensitive = IMS_TRUE) const;
    inline void InsertElementAt(IN const AString& strElem, IN IMS_SINT32 nIndex)
    {
        m_objElements.InsertAt(strElem, nIndex);
    }
    inline IMS_BOOL IsEmpty() const { return (m_objElements.GetSize() == 0); }
    inline void RemoveAllElements() { m_objElements.Clear(); }
    IMS_BOOL RemoveElement(IN const AString& strElem, IN IMS_BOOL bCaseSensitive = IMS_TRUE);
    inline void RemoveElementAt(IN IMS_SINT32 nIndex) { m_objElements.RemoveAt(nIndex); }
    inline void SetElementAt(IN const AString& strElem, IN IMS_SINT32 nIndex)
    {
        m_objElements.SetAt(strElem, nIndex);
    }
    AString ToString(IN IMS_CHAR cDelimiter = TextParser::CHAR_COMMA) const;

    static const AStringArray& ConstNull();

private:
    ImsList<AString> m_objElements;
};

#endif
