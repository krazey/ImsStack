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
#ifndef XML_FACTORY_H_
#define XML_FACTORY_H_

#include "ImsTypeDef.h"

class IXmlStreamWriter;
class IXmlTransactionProvider;

class XmlFactory
{
protected:
    XmlFactory();

public:
    ~XmlFactory();

public:
    /**
     * @brief Creates a new IXmlStreamWriter that writes to a stream.
     *
     * @return Pointer to a new IXmlStreamWriter.
     */
    IXmlStreamWriter* CreateStreamWriter();

    /**
     * @brief Destroys a specified IXmlStreamWriter.
     *
     * @param piWriter An XML stream writer to destroy
     */
    void DestroyStreamWriter(IN IXmlStreamWriter*& piWriter);

    /**
     * @brief Creates a new IXmlTransactionProvider.
     *
     * @return Pointer to a new IXmlTransactionProvider.
     */
    IXmlTransactionProvider* CreateTransactionProvider();

    /**
     * @brief Destroys a specified IXmlTransactionProvider.
     *
     * @param piProvider An XML transaction provider to destroy
     */
    void DestroyTransactionProvider(IN IXmlTransactionProvider*& piProvider);

    /**
     * @brief Gets a new instance of this factory.
     *
     * @return An instance of XmlFactory.
     */
    static XmlFactory* GetInstance();
};

#endif
