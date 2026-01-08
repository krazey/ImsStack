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
#ifndef XML_RESPONSE_H_
#define XML_RESPONSE_H_

#include "IXmlResponse.h"

class XmlResponse : public IXmlResponse
{
public:
    inline XmlResponse() :
            m_piDocument(IMS_NULL),
            m_nResponseCode(RESPONSE_CODE_SUCCESS)
    {
    }
    ~XmlResponse() override = default;

    XmlResponse(IN const XmlResponse&) = delete;
    XmlResponse& operator=(IN const XmlResponse&) = delete;

    inline IDocument* GetDocument() const override { return m_piDocument; }
    inline IMS_SINT32 GetResponseCode() const override { return m_nResponseCode; }

    inline void SetDocument(IN IDocument* piDocument) { m_piDocument = piDocument; }
    inline void SetResponseCode(IN IMS_SINT32 nResponseCode) { m_nResponseCode = nResponseCode; }

private:
    IDocument* m_piDocument;
    IMS_SINT32 m_nResponseCode;
};

#endif
