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
#ifndef XML_REQUEST_H_
#define XML_REQUEST_H_

#include "IXmlRequest.h"

class XmlRequest : public IXmlRequest
{
public:
    inline XmlRequest() :
            m_strRawXml(AString::ConstNull())
    {
    }
    ~XmlRequest() override = default;

    XmlRequest(IN const XmlRequest&) = delete;
    XmlRequest& operator=(IN const XmlRequest&) = delete;

public:
    inline void SetRawXml(IN const IMS_CHAR* pszRawXml) override { m_strRawXml = pszRawXml; }
    inline void SetRawXml(IN const AString& strRawXml) override { m_strRawXml = strRawXml; }
    inline const AString& GetRawXml() const override { return m_strRawXml; }

private:
    AString m_strRawXml;
};

#endif
