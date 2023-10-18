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
#include "DomDocumentBuilderFactory.h"
#include "IXmlTransactionProvider.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServiceTrace.h"
#include "XmlApp.h"
#include "XmlRequest.h"
#include "XmlTransaction.h"

__IMS_TRACE_TAG_XML__;

PUBLIC
XmlApp::XmlApp(IN IXmlTransactionProvider* piTransactionProvider) :
        m_piTransactionProvider(piTransactionProvider),
        m_strTargetName(AString::ConstNull())
{
}

PUBLIC
XmlApp::~XmlApp() {}

PUBLIC
void XmlApp::Attach(IN const AString& strTargetName)
{
    IMS_TRACE_D("Attach :: app=%s", strTargetName.GetStr(), 0, 0);

    m_strTargetName = strTargetName;

    AttachResponseParam* pParam = new AttachResponseParam();
    pParam->eResult = XmlResult::XML_RESULT_SUCCESS;

    IMS_MSG_CreateNPostActivityMessageByName(strTargetName, AMSG_XML_ATTACH_RESPONSE, 0, pParam);
}

PUBLIC
void XmlApp::Detach()
{
    IMS_TRACE_D("Detach", 0, 0, 0);

    m_strTargetName = AString::ConstNull();
}

PUBLIC
IMS_RESULT XmlApp::Parse(IN XmlTransaction* pTransaction)
{
    if (pTransaction == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    IXmlRequest* piRequest = pTransaction->GetRequest();

    if (piRequest == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parse :: IXmlRequest is null", 0, 0, 0);
        return IMS_FAILURE;
    }

    AString strRawXml = piRequest->GetRawXml();

    m_piTransactionProvider->Push(pTransaction);

    Parse(strRawXml);

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT XmlApp::Parse(IN const AString& strRawXml)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        SendParseResponse(XmlResult::XML_RESULT_FAILURE, IMS_NULL, IMS_NULL);
        return IMS_FAILURE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strRawXml);

    if (piDocument == IMS_NULL)
    {
        SendParseResponse(XmlResult::XML_RESULT_FAILURE, IMS_NULL, IMS_NULL);
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FAILURE;
    }

    if (SendParseResponse(XmlResult::XML_RESULT_SUCCESS, piDocument, pDocumentBuilder) !=
            IMS_SUCCESS)
    {
        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT XmlApp::SendParseResponse(IN XmlApp::XmlResult eResult, IN IDocument* piDocument,
        IN DocumentBuilder* pDocumentBuilder)
{
    if (m_strTargetName.GetLength() == 0)
    {
        return IMS_FAILURE;
    }

    ParseResponseParam* pParam = new ParseResponseParam();

    pParam->eResult = eResult;
    pParam->piDocument = piDocument;
    pParam->pDocumentBuilder = pDocumentBuilder;

    IMS_MSG_CreateNPostActivityMessageByName(m_strTargetName, AMSG_XML_PARSE_RESPONSE, 0, pParam);

    return IMS_SUCCESS;
}
