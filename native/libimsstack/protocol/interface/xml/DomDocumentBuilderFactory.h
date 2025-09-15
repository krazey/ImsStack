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
#ifndef DOM_DOCUMENT_BUILDER_FACTORY_H_
#define DOM_DOCUMENT_BUILDER_FACTORY_H_

#include "DocumentBuilder.h"
#include "DocumentBuilderFactory.h"

/**
 * @brief This class provides a factory API that enables XML applications to obtain a parser
 *        that produces DOM object trees from XML documents.
 */
class DomDocumentBuilderFactory : public DocumentBuilderFactory
{
private:
    DomDocumentBuilderFactory();
    ~DomDocumentBuilderFactory() override;

    DomDocumentBuilderFactory(IN const DomDocumentBuilderFactory&) = delete;
    DomDocumentBuilderFactory& operator=(IN const DomDocumentBuilderFactory&) = delete;

public:
    DocumentBuilder* NewDocumentBuilder() override;
    static void DestroyDocumentBuilder(IN DocumentBuilder*& pDocumentBuilder);

    static DomDocumentBuilderFactory* GetInstance();
};

#endif
