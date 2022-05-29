#include "DocumentBuilder.h"
#include "DomDocumentBuilderFactory.h"
#include "IDocument.h"
#include "IElement.h"
#include "IText.h"
#include "ServiceTrace.h"

#include "ussi/UssiConstants.h"
#include "ussi/UssiData.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
UssiData::UssiData() :
        m_strLanguage(AString::ConstNull()),
        m_strUssdString(AString::ConstNull()),
        m_eErrorCode(UssiError::CODE_NONE)
{
    IMS_TRACE_I("+UssiData", 0, 0, 0);
}

PUBLIC
UssiData::~UssiData()
{
    IMS_TRACE_I("~UssiData", 0, 0, 0);
}

PUBLIC
const UssiData::AnyExtension& UssiData::GetAnyExtension() const
{
    return objAnyExtension;
}

PUBLIC
IMS_BOOL UssiData::Parse(IN const AString& strUssiBody)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();
    if (pDocumentBuilder == IMS_NULL)
    {
        IMS_TRACE_E(0, "DocumentBuilder is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strUssiBody);
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
    if (!strTagName.EqualsIgnoreCase(UssiConstants::ELEMENT_USSD_DATA))
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
        if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_LANGUAGE))
        {
            INode* piNodeChild = piNodeEvent->GetFirstChild();
            if (piNodeChild != IMS_NULL && piNodeChild->GetNodeType() == INode::TEXT_NODE)
            {
                IText* piText = DYNAMIC_CAST(IText*, piNodeChild);
                m_strLanguage = piText->GetData();
            }
        }
        else if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_USSD_STRING))
        {
            INode* piNodeChild = piNodeEvent->GetFirstChild();
            if (piNodeChild != IMS_NULL && piNodeChild->GetNodeType() == INode::TEXT_NODE)
            {
                IText* piText = DYNAMIC_CAST(IText*, piNodeChild);
                m_strUssdString = piText->GetData();
            }
        }
        else if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_ERROR_CODE))
        {
            INode* piNodeChild = piNodeEvent->GetFirstChild();
            if (piNodeChild != IMS_NULL)
            {
                m_eErrorCode = (UssiError)piNodeChild->GetNodeValue().ToInt32();
            }
        }
        else if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_ANYEXT))
        {
            CreateAnyExtension(piNodeEvent);
        }
    }

    piDocument->DestroyDocument();
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    IMS_TRACE_D("Parse : Done", 0, 0, 0);

    return IMS_TRUE;
}

PRIVATE
void UssiData::CreateAnyExtension(IN INode* piNode)
{
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

        if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_USS_REQUEST))
        {
            objAnyExtension.m_eUssiModeType = UssiModeType::REQUEST;
        }
        else if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_USS_NOTIFY))
        {
            objAnyExtension.m_eUssiModeType = UssiModeType::NOTIFY;
        }
        else if (strName.EqualsIgnoreCase(UssiConstants::ELEMENT_ALERTING_PATTERN))
        {
            INode* piNode_Value = piElement->GetFirstChild();

            if (piNode_Value != IMS_NULL)
            {
                objAnyExtension.m_nAlertingPattern = piNode_Value->GetNodeValue().ToInt32();
            }
        }

        piElement = piElement->GetNextSibling();
    }

    IMS_TRACE_D("CreateAnyExtension : type=%d, alertingPattern=%d", objAnyExtension.m_eUssiModeType,
            objAnyExtension.m_nAlertingPattern, 0);
}
