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
#include "subscribe/UceRlmiComposer.h"

#include "IXmlStreamWriter.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"
#include "XmlFactory.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

PUBLIC
UceRlmiComposer::UceRlmiComposer()
{
    IMS_TRACE_MEM("UCE_MEM", "UCE_M : UceRlmiComposer = %" PFLS_u, sizeof(UceRlmiComposer), 0, 0);
    m_pXMLOutputFactory = XmlFactory::GetInstance();
}

UceRlmiComposer::~UceRlmiComposer()
{
    IMS_TRACE_MEM("UCE_MEM", "UCE_F : UceRlmiComposer = %" PFLS_u, sizeof(UceRlmiComposer), 0, 0);
    IMS_TRACE_I("~UceRlmiComposer", 0, 0, 0);
}

PUBLIC
AString UceRlmiComposer::ComposeRLMIList(IN ImsList<AString>& pContactInfoList)
{
    IMS_TRACE_D("ComposeRLMIList ", 0, 0, 0);
    IXmlStreamWriter* piWriter = IMS_NULL;

    //---------------------------------------------------------------------------------------------
    if (IMS_NULL == m_pXMLOutputFactory)
    {
        IMS_TRACE_I("ComposeRLMIList:XMLOutputFactory Creation Failed", 0, 0, 0);
        return AString::ConstNull();
    }

    // Create XML Stream Writer
    piWriter = m_pXMLOutputFactory->CreateStreamWriter();
    if (piWriter != IMS_NULL)
    {
        // XML Encoding method
        piWriter->WriteStartDocument("UTF-8", "1.0");
        piWriter->WriteCharacters(TextParser::STR_LF);

        // XML Presence Name Space
        EncodeResourceXMLNameSpace(piWriter);
        piWriter->WriteStartElement("list");
        piWriter->WriteAttribute("name", "dummy-rfc5367");
        piWriter->WriteCharacters(TextParser::STR_LF);

        for (IMS_UINT32 n = 0; n < pContactInfoList.GetSize(); n++)
        {
            AString szUserID = pContactInfoList.GetAt(n);
            piWriter->WriteStartElement("entry");
            if (szUserID.StartsWith("sip:") == IMS_FALSE &&
                    szUserID.StartsWith("tel:") == IMS_FALSE)
            {
                szUserID = "tel:" + szUserID;
            }
            piWriter->WriteAttribute("uri", szUserID);
            piWriter->WriteEndElement();
            piWriter->WriteCharacters(TextParser::STR_LF);
        }
        /*End of list element*/
        piWriter->WriteEndElement();
        piWriter->WriteCharacters(TextParser::STR_LF);
        /*End of Resouse list element*/
        piWriter->WriteEndElement();
        piWriter->WriteCharacters(TextParser::STR_LF);

        IMS_CHAR* pszValue = piWriter->Flush();
        AString strSubXml(pszValue);

        if (pszValue != IMS_NULL)
        {
            IMS_MEM_Free(pszValue);
        }
        piWriter->Close();
        m_pXMLOutputFactory->DestroyStreamWriter(piWriter);
        return strSubXml;
    }
    return AString::ConstNull();
}

PRIVATE
void UceRlmiComposer::EncodeResourceXMLNameSpace(IN IXmlStreamWriter*& piWriter)
{
    IMS_TRACE_D("EncodeResourceXMLNameSpace ", 0, 0, 0);
    //---------------------------------------------------------------------------------------------
    piWriter->SetDefaultNamespace("urn:ietf:params:xml:ns:resource-lists");
    piWriter->SetPrefix("xsi", "urn:ietf:params:xml:ns:pidf:data-model");

    piWriter->WriteStartElement("resource-lists");
    piWriter->WriteDefaultNamespace("urn:ietf:params:xml:ns:resource-lists");
    piWriter->WriteNamespace("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    piWriter->WriteCharacters(TextParser::STR_LF);
}
