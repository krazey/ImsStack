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
#ifndef DOCUMENT_BUILDER_FACTORY_H_
#define DOCUMENT_BUILDER_FACTORY_H_

class DocumentBuilder;

/**
 * @brief This class provides a factory API that enables the applications to obtain a parser
 *        that produces DOM object trees from XML documents.
 */
class DocumentBuilderFactory
{
protected:
    DocumentBuilderFactory() = default;

public:
    virtual ~DocumentBuilderFactory() = default;

    DocumentBuilderFactory(IN const DocumentBuilderFactory&) = delete;
    DocumentBuilderFactory& operator=(IN const DocumentBuilderFactory&) = delete;

public:
    /**
     * @brief Creates a new instance of a DocumentBuilder using the currently configured
     *        parameters.
     *
     * @return A new instance of a DocumentBuilder.
     */
    virtual DocumentBuilder* NewDocumentBuilder() = 0;
};

#endif
