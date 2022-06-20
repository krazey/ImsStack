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
#include "XmlFactory.h"
#include "XmlStreamWriter.h"
#include "XmlTransactionProvider.h"

PROTECTED
XmlFactory::XmlFactory() {}

PUBLIC
XmlFactory::~XmlFactory() {}

PUBLIC
IXmlStreamWriter* XmlFactory::CreateStreamWriter()
{
    return new XmlStreamWriter();
}

PUBLIC
void XmlFactory::DestroyStreamWriter(IN IXmlStreamWriter*& piWriter)
{
    XmlStreamWriter* pWriter = DYNAMIC_CAST(XmlStreamWriter*, piWriter);

    if (pWriter != IMS_NULL)
    {
        delete pWriter;
    }

    piWriter = IMS_NULL;
}

PUBLIC
IXmlTransactionProvider* XmlFactory::CreateTransactionProvider()
{
    return new XmlTransactionProvider();
}

PUBLIC
void XmlFactory::DestroyTransactionProvider(IN IXmlTransactionProvider*& piProvider)
{
    XmlTransactionProvider* pProvider = DYNAMIC_CAST(XmlTransactionProvider*, piProvider);

    if (pProvider != IMS_NULL)
    {
        delete pProvider;
    }

    piProvider = IMS_NULL;
}

PUBLIC GLOBAL XmlFactory* XmlFactory::GetInstance()
{
    static XmlFactory* pXmlFactory = IMS_NULL;

    if (pXmlFactory == IMS_NULL)
    {
        pXmlFactory = new XmlFactory();
    }

    return pXmlFactory;
}
