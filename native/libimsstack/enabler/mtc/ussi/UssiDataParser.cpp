/*
 * author : aromi.kwak@
 * date : 201610
 * brief : Refactoring USSIHelper.
 *         Create USSIBodyParser
 */

#include "DomDocumentBuilderFactory.h"
#include "DocumentBuilder.h"
#include "IDocument.h"
#include "IElement.h"
#include "IText.h"

#include "ussi/UssiDataParser.h"
#include "ussi/UssiConstants.h"

__IMS_TRACE_TAG_COM_UC__;

PUBLIC
USSDDataParser::USSDDataParser() :
        m_strLanguage(AString::ConstNull()),
        m_strUSSDString(AString::ConstNull()),
        m_nErrorCode(ERROR_CODE_NONE)
{
    IMS_TRACE_MEM("uc", "uc_M : USSDDataParser[%" PFLS_u "][%" PFLS_x "]", sizeof(USSDDataParser),
            this, 0);
}

PUBLIC
USSDDataParser::USSDDataParser(IN const AString& strUSSIBody) :
        m_strLanguage(AString::ConstNull()),
        m_strUSSDString(AString::ConstNull()),
        m_nErrorCode(ERROR_CODE_NONE)
{
    IMS_TRACE_MEM("uc", "uc_M : USSDDataParser[%" PFLS_u "][%" PFLS_x "]", sizeof(USSDDataParser),
            this, 0);

    Parse(strUSSIBody);
}

PUBLIC
USSDDataParser::~USSDDataParser()
{
    IMS_TRACE_MEM("uc", "uc_F : USSDDataParser[%" PFLS_u "][%" PFLS_x "]", sizeof(USSDDataParser),
            this, 0);
}

/*

Remarks

*/
PUBLIC
const USSDDataParser::AnyExtension& USSDDataParser::GetAnyExtension() const
{
    //---------------------------------------------------------------------------------------------

    return objAnyExtension;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL USSDDataParser::Parse(IN const AString& strUSSIBody)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    //---------------------------------------------------------------------------------------------

    if (pDocumentBuilder == IMS_NULL)
    {
        IMS_TRACE_E(0, "DocumentBuilder is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strUSSIBody);

    if (piDocument == IMS_NULL)
    {
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

        IMS_TRACE_E(0, "Parsing a \'ussd-data\' XML failed", 0, 0, 0);
        return IMS_FALSE;
    }

    IElement* piElement = piDocument->GetDocumentElement();

    if (piElement == IMS_NULL)
    {
        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

        IMS_TRACE_E(0, "No root element", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strTagName = piElement->GetTagName();

    if (!strTagName.EqualsIgnoreCase(USSDConstants::ELEMENT_USSD_DATA))
    {
        IMS_TRACE_E(
                0, "Root element (%s) is not matched in 'ussd-data'", strTagName.GetStr(), 0, 0);

        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FALSE;
    }

    INodeList* piNodeListEvents = piElement->GetChildNodes();
    if (piNodeListEvents == IMS_NULL || piNodeListEvents->GetLength() <= 0)
    {
        IMS_TRACE_E(0, "no 'ussd-data' NodeList", 0, 0, 0);
        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < piNodeListEvents->GetLength(); i++)
    {
        INode* piNodeEvent = piNodeListEvents->Item(i);
        if (piNodeEvent == IMS_NULL)
        {
            continue;
        }

        AString strName = piNodeEvent->GetLocalName();

        if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_LANGUAGE))
        {
            INode* piNodeChild = piNodeEvent->GetFirstChild();
            if (piNodeChild != IMS_NULL && piNodeChild->GetNodeType() == INode::TEXT_NODE)
            {
                IText* piText = DYNAMIC_CAST(IText*, piNodeChild);
                m_strLanguage = piText->GetData();
            }
        }
        else if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_USSD_STRING))
        {
            INode* piNodeChild = piNodeEvent->GetFirstChild();
            if (piNodeChild != IMS_NULL && piNodeChild->GetNodeType() == INode::TEXT_NODE)
            {
                IText* piText = DYNAMIC_CAST(IText*, piNodeChild);
                m_strUSSDString = piText->GetData();
            }
        }
        else if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_ERROR_CODE))
        {
            INode* piNodeChild = piNodeEvent->GetFirstChild();
            if (piNodeChild != IMS_NULL)
            {
                m_nErrorCode = piNodeChild->GetNodeValue().ToInt32();
            }
        }
        else if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_ANYEXT))
        {
            CreateAnyExtension(piNodeEvent);
        }
    }

    piDocument->DestroyDocument();
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    IMS_TRACE_I("Parse : Done", 0, 0, 0);

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE
void USSDDataParser::CreateAnyExtension(IN INode* piNode)
{
    //---------------------------------------------------------------------------------------------

    if (piNode == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateAnyExtension : piNode is NULL", 0, 0, 0);
        return;
    }

    INode* piElement = piNode->GetFirstChild();
    AString strName;

    while (piElement != IMS_NULL)
    {
        strName = piElement->GetLocalName();

        if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_USS_REQUEST))
        {
            objAnyExtension.m_nUSSType = AnyExtension::USS_TYPE_REQUEST;
        }
        else if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_USS_NOTIFY))
        {
            objAnyExtension.m_nUSSType = AnyExtension::USS_TYPE_NOTIFY;
        }
        else if (strName.EqualsIgnoreCase(USSDConstants::ELEMENT_ALERTING_PATTERN))
        {
            INode* piNode_Value = piElement->GetFirstChild();

            if (piNode_Value != IMS_NULL)
            {
                objAnyExtension.m_nAlertingPattern = piNode_Value->GetNodeValue().ToInt32();
            }
        }

        piElement = piElement->GetNextSibling();
    }

    IMS_TRACE_I("USSDDataParser :: anyExt :: type=%d, alertingPattern=%d",
            objAnyExtension.m_nUSSType, objAnyExtension.m_nAlertingPattern, 0);
}
