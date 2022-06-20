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
#ifndef XML_APP_H_
#define XML_APP_H_

#include "AString.h"
#include "ImsMessageDef.h"

class DocumentBuilder;
class IDocument;
class IXmlTransactionProvider;
class XmlTransaction;

class XmlApp
{
public:
    enum class XmlResult
    {
        XML_RESULT_FAILURE = 0,
        XML_RESULT_SUCCESS,
    };

    class AttachResponseParam
    {
    public:
        XmlResult eResult;
    };

    class ParseResponseParam
    {
    public:
        XmlResult eResult;
        IDocument* piDocument;
        DocumentBuilder* pDocumentBuilder;
    };

public:
    XmlApp(IN IXmlTransactionProvider* piTransactionProvider);
    ~XmlApp();

    XmlApp(IN const XmlApp&) = delete;
    XmlApp& operator=(IN const XmlApp&) = delete;

public:
    void Attach(IN const AString& strTargetName);
    void Detach();
    IMS_RESULT Parse(IN XmlTransaction* pTransaction);

private:
    IMS_RESULT Parse(IN const AString& strRawXml);
    IMS_RESULT SendParseResponse(
            IN XmlResult eResult, IN IDocument* piDocument, IN DocumentBuilder* pDocumentBuilder);

public:
    enum
    {
        AMSG_XML_ATTACH_RESPONSE = IMS_MSG_XML + 1,
        AMSG_XML_PARSE_RESPONSE
    };

private:
    IXmlTransactionProvider* m_piTransactionProvider;
    AString m_strTargetName;
};

#endif
