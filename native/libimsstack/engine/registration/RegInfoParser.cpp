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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "DocumentBuilder.h"
#include "DocumentBuilderFactory.h"
#include "DomDocumentBuilderFactory.h"
#include "IDocument.h"
#include "IRegInfoParserListener.h"
#include "RegInfoParser.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfoParser::RegInfoParser(IN const RegKey& objRegKey) :
        m_objRegKey(objRegKey),
        m_piListener(IMS_NULL)
{
}

PUBLIC
IMS_BOOL RegInfoParser::Parse(IN const AString& strRegInfo)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strRegInfo);

    if (piDocument == IMS_NULL)
    {
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

        IMS_TRACE_E(0, "Parsing a 'reginfo' XML failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Notify the XML parsing result for "reginfo"
    if (m_piListener != IMS_NULL)
    {
        m_piListener->RegInfoParser_ParsingCompleted(this, piDocument);
    }

    piDocument->DestroyDocument();
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    return IMS_TRUE;
}
