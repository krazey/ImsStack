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
#ifndef DOCUMENT_BUILDER_H_
#define DOCUMENT_BUILDER_H_

#include "IDocument.h"

class DocumentBuilderPrivate;

/**
 * @brief This class provides the API to obtain DOM document instances from an XML document.
 *
 * Using this class, an application can obtain a Document from XML content.
 */
class DocumentBuilder
{
protected:
    DocumentBuilder();
    virtual ~DocumentBuilder();

public:
    DocumentBuilder(IN const DocumentBuilder&) = delete;
    DocumentBuilder& operator=(IN const DocumentBuilder&) = delete;

public:
    /**
     * @brief Parses the content of the given string as an XML document and
     *        returns a new DOM Document object.
     *
     * @param strXml String containing the content to be parsed.
     * @return A new instance of IDocument.
     */
    IDocument* Parse(IN const AString& strXml);

    /**
     * @brief Parses the content of the given string as an XML document and
     *        returns a new DOM Document object.
     *
     * @param pszXml String containing the content to be parsed.
     * @return A new instance of IDocument.
     */
    IDocument* Parse(IN const IMS_CHAR* pszXml);

private:
    DocumentBuilderPrivate* pDocumentBuilderPrivate;
};

#endif
