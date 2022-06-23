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
#include "DocumentBuilder.h"
#include "DocumentImpl.h"
#include "ElementImpl.h"
#include "ImsStrLib.h"
#include "ServiceTrace.h"
#include "TextImpl.h"
#include "XmlApiParser.h"
#include "XmlApiTree.h"
#include "XmlApiXPath.h"

__IMS_TRACE_TAG_XML__;

class DocumentBuilderPrivate
{
public:
    DocumentBuilderPrivate();
    ~DocumentBuilderPrivate();

public:
    IDocument* Parse(IN const IMS_CHAR* pszXml, IN IMS_SINT32 nLength);

private:
    IDocument* CreateDocument(IN xmlDocPtr pstDoc);
    IMS_RESULT CreateElement(IN DocumentImpl* pDocument, IN xmlDocPtr pstDoc);
    IMS_RESULT CreateChildNode(
            IN IDocument* pDocument, IN INode* piParentNode, IN xmlNodePtr pstNode);
    INode* CreateNodeInstance(IN xmlNodePtr pstNode);
};

PUBLIC
DocumentBuilderPrivate::DocumentBuilderPrivate() {}

PUBLIC
DocumentBuilderPrivate::~DocumentBuilderPrivate() {}

PUBLIC
IDocument* DocumentBuilderPrivate::Parse(IN const IMS_CHAR* pszXml, IN IMS_SINT32 nLength)
{
    if (pszXml == IMS_NULL)
    {
        IMS_TRACE_E(0, "XML is null", 0, 0, 0);
        return IMS_NULL;
    }

    xmlDocPtr pstDoc = XmlApi_ReadMemory(pszXml, nLength, IMS_NULL, IMS_NULL, XML_PARSE_NOBLANKS);

    if (pstDoc == IMS_NULL)
    {
        IMS_TRACE_E(0, "XmlApi_ReadMemory failed - length=%d", nLength, 0, 0);
        return IMS_NULL;
    }

    IMS_TRACE_D("DocumentBuilder :: Parse done (l=%d, standalone=%d, properties=0x%04x)", nLength,
            pstDoc->standalone, pstDoc->properties);

    return CreateDocument(pstDoc);
}

PRIVATE
IDocument* DocumentBuilderPrivate::CreateDocument(IN xmlDocPtr pstDoc)
{
    xmlXPathContextPtr pstXPathContext = XmlApi_XPathNewContext(pstDoc);

    if (pstXPathContext == IMS_NULL)
    {
        XmlApi_FreeDoc(pstDoc);
        IMS_TRACE_E(0, "XmlApi_XPathNewContext failed", 0, 0, 0);
        return IMS_NULL;
    }

    DocumentImpl* pDocument = new DocumentImpl(pstDoc, pstXPathContext);

    if (pDocument == IMS_NULL)
    {
        XmlApi_XPathFreeContext(pstXPathContext);
        XmlApi_FreeDoc(pstDoc);
        return IMS_NULL;
    }

    if (CreateElement(pDocument, pstDoc) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "CreateElement failed", 0, 0, 0);
        delete pDocument;
        return IMS_NULL;
    }

    return pDocument;
}

PRIVATE
IMS_RESULT DocumentBuilderPrivate::CreateElement(IN DocumentImpl* pDocument, IN xmlDocPtr pstDoc)
{
    xmlNode* pRootElement = pstDoc->children;

    // Find and create a root element (document element)
    if (pRootElement != IMS_NULL)
    {
        // Skip comment node if present
        while (pRootElement != IMS_NULL)
        {
            if ((pRootElement->type == XML_ELEMENT_NODE) || (pRootElement->type == XML_TEXT_NODE))
            {
                break;
            }

            pRootElement = pRootElement->next;
        }
    }

    if (pRootElement == IMS_NULL)
    {
        IMS_TRACE_I("No elements in the document", 0, 0, 0);
        return IMS_SUCCESS;
    }

    INode* piRootNode = CreateNodeInstance(pRootElement);

    if (piRootNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating a root node failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    pDocument->SetChildren(piRootNode);
    piRootNode->SetParent(pDocument);
    piRootNode->SetOwnerDocument(pDocument);

    // Create the child elements recursively
    if (pRootElement->children != IMS_NULL)
    {
        return CreateChildNode(pDocument, piRootNode, pRootElement->children);
    }

    return IMS_SUCCESS;
}

PRIVATE
IMS_RESULT DocumentBuilderPrivate::CreateChildNode(
        IN IDocument* piDocument, IN INode* piParentNode, IN xmlNodePtr pstNode)
{
    INode* piPrevNode = IMS_NULL;

    while (pstNode != IMS_NULL)
    {
        INode* piNode = CreateNodeInstance(pstNode);

        if (piNode == IMS_NULL)
        {
            pstNode = pstNode->next;
            continue;
        }

        if (piPrevNode != IMS_NULL)
        {
            piPrevNode->SetNextSibling(piNode);
            piNode->SetPreviousSibling(piPrevNode);
        }

        piNode->SetParent(piParentNode);
        piNode->SetOwnerDocument(piDocument);
        piParentNode->SetChildren(piNode);

        if (pstNode->children != IMS_NULL)
        {
            CreateChildNode(piDocument, piNode, pstNode->children);
        }

        piPrevNode = piNode;
        pstNode = pstNode->next;
    }

    return IMS_SUCCESS;
}

PRIVATE
INode* DocumentBuilderPrivate::CreateNodeInstance(IN xmlNodePtr pstNode)
{
    if (pstNode->type == XML_ELEMENT_NODE)
    {
        return new ElementImpl(pstNode);
    }
    else if (pstNode->type == XML_TEXT_NODE)
    {
        return new TextImpl(pstNode);
    }

    return IMS_NULL;
}

PROTECTED
DocumentBuilder::DocumentBuilder() :
        pDocumentBuilderPrivate(new DocumentBuilderPrivate())
{
}

PUBLIC VIRTUAL DocumentBuilder::~DocumentBuilder()
{
    if (pDocumentBuilderPrivate != IMS_NULL)
    {
        delete pDocumentBuilderPrivate;
        pDocumentBuilderPrivate = IMS_NULL;
    }
}

PUBLIC
IDocument* DocumentBuilder::Parse(IN const AString& strXml)
{
    return pDocumentBuilderPrivate->Parse(strXml.GetStr(), strXml.GetLength());
}

PUBLIC
IDocument* DocumentBuilder::Parse(IN const IMS_CHAR* pszXml)
{
    return pDocumentBuilderPrivate->Parse(pszXml, IMS_StrLen(pszXml));
}
