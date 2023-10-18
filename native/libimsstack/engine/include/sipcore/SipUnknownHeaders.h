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
#ifndef SIP_UNKNOWN_HEADERS_H_
#define SIP_UNKNOWN_HEADERS_H_

#include "AString.h"

class SipUnknownHeaders
{
private:
    class Header
    {
    public:
        explicit Header(IN const AString& strName);
        Header(IN const Header& other);
        ~Header();

    public:
        Header& operator=(IN const Header& other);

    public:
        inline void Clear() { objBodys.Clear(); }
        IMS_BOOL Equals(IN const AString& strName) const;

    public:
        AString strCompactName;
        AString strName;
        ImsList<AString> objBodys;
    };

public:
    SipUnknownHeaders();
    SipUnknownHeaders(IN const SipUnknownHeaders& other);
    ~SipUnknownHeaders();

public:
    SipUnknownHeaders& operator=(IN const SipUnknownHeaders& other);

public:
    IMS_RESULT AddHeader(IN const AString& strName, IN const AString& strBody);
    void Clear();
    inline IMS_SINT32 GetCount() const { return m_objHeaders.GetSize(); }
    AString GetHeader(IN const AString& strName, IN IMS_SINT32 nIndex = 0) const;
    AString GetHeaderBodys(IN IMS_SINT32 nPos) const;
    AString GetHeaderBodys(IN const AString& strName) const;
    IMS_SINT32 GetHeaderCount(IN const AString& strName) const;
    const AString& GetHeaderName(IN IMS_SINT32 nPos, IN IMS_BOOL bCompactForm = IMS_FALSE) const;
    ImsList<AString> GetHeaders(IN const AString& strName) const;
    inline IMS_BOOL IsHeaderPresent(IN const AString& strName) const
    {
        return (FindHeader(strName) != IMS_NULL);
    }
    IMS_BOOL OverwriteHeaders(IN const SipUnknownHeaders& objOther);
    IMS_RESULT PrependHeader(IN const AString& strName, IN const AString& strBody);
    void RemoveHeader(IN const AString& strName);
    IMS_RESULT SetHeader(IN const AString& strName, IN const AString& strBody);

private:
    void DeleteHeader(IN const AString& strName);
    Header* FindHeader(IN const AString& strName) const;

private:
    ImsList<Header*> m_objHeaders;
};

#endif
