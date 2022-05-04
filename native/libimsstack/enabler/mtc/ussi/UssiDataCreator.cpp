/*
 * author : aromi.kwak
 * version : 1.0
 * date : 2016.10
 * brief : Create USSDDataCreator
 */

#include "ServiceTrace.h"
#include "TextParser.h"

#include "configuration/ConfigDef.h"
#include "ussi/UssiDataCreator.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiDataParser.h"
#include "MtcDef.h"

__IMS_TRACE_TAG_COM_UC__;

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC GLOBAL void USSDDataCreator::GetXMLBody(IN const AString& strUSSDStr,
        OUT AStringBuffer& objXML, IN IMS_SINT32 nSlotID,
        IN IMS_UINT32 nUSSType /*= USSDDataParser::AnyExtension::USS_TYPE_NONE*/,
        IN IMS_UINT32 nErrorCode /*= USSDDataParser::ERROR_CODE_NONE*/)
{
    objXML.Append(USSDConstants::XML_PROCESSING_INSTRUCTION);
    objXML.Append(TextParser::CHAR_LF);

    objXML.Append(CreateStartElement(USSDConstants::ELEMENT_USSD_DATA));
    objXML.Append(TextParser::CHAR_HTAB);

    // TODO, MTC BUILD
    UNUSED_PARAM(nSlotID);
    objXML.Append(
            CreateAttribute(USSDConstants::ELEMENT_LANGUAGE, USSDConstants::ELEMENT_LANGUAGE_EN));

    // ussd-string
    objXML.Append(TextParser::CHAR_HTAB);
    objXML.Append(CreateAttribute(USSDConstants::ELEMENT_USSD_STRING, strUSSDStr));

    // error-code
    AString strErrorCode = AString::ConstEmpty();
    GetErrorCode(nErrorCode, strErrorCode);

    if (strErrorCode.GetLength() > 0)
    {
        objXML.Append(TextParser::CHAR_HTAB);
        objXML.Append(CreateAttribute(USSDConstants::ELEMENT_ERROR_CODE, strErrorCode));
    }

    // append extension of request type
    if (nUSSType != USSDDataParser::AnyExtension::USS_TYPE_NONE)
    {
        objXML.Append(TextParser::CHAR_HTAB);
        objXML.Append(CreateStartElement(USSDConstants::ELEMENT_ANYEXT));

        objXML.Append(TextParser::CHAR_HTAB);
        objXML.Append(TextParser::CHAR_HTAB);

        AString strAttribute;
        strAttribute.Append(TextParser::CHAR_LAQUOT);
        // <UnstructuredSS-Request/>
        if (nUSSType == USSDDataParser::AnyExtension::USS_TYPE_REQUEST)
        {
            strAttribute.Append(USSDConstants::ELEMENT_USS_REQUEST);
        }
        // <UnstructuredSS-Notify/>
        else if (nUSSType == USSDDataParser::AnyExtension::USS_TYPE_NOTIFY)
        {
            strAttribute.Append(USSDConstants::ELEMENT_USS_NOTIFY);
        }
        strAttribute.Append(TextParser::CHAR_SLASH);
        strAttribute.Append(TextParser::CHAR_RAQUOT);
        strAttribute.Append(TextParser::CHAR_LF);

        objXML.Append(strAttribute);
        objXML.Append(TextParser::CHAR_HTAB);
        objXML.Append(CreateEndElement(USSDConstants::ELEMENT_ANYEXT));
    }

    objXML.Append(CreateEndElement(USSDConstants::ELEMENT_USSD_DATA));

    IMS_TRACE_D("GetXMLBody : [%s]", strUSSDStr.GetStr(), 0, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE GLOBAL const AString USSDDataCreator::CreateStartElement(
        IN const AString& strStartElementName)
{
    if (strStartElementName.GetLength() <= 0)
    {
        IMS_TRACE_I("CreateStartElement : Empty", 0, 0, 0);
        return AString::ConstEmpty();
    }

    AString strStartElement;

    strStartElement.Append(TextParser::CHAR_LAQUOT);
    strStartElement.Append(strStartElementName);
    strStartElement.Append(TextParser::CHAR_RAQUOT);

    strStartElement.Append(TextParser::CHAR_LF);

    IMS_TRACE_D("CreateStartElement : [%s]", strStartElement.GetStr(), 0, 0);
    return strStartElement;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE GLOBAL const AString USSDDataCreator::CreateAttribute(
        IN const AString& strAttributeName, IN const AString& strValue)
{
    if (strAttributeName.GetLength() <= 0 || strValue.GetLength() <= 0)
    {
        IMS_TRACE_I("CreateAttribute : Empty", 0, 0, 0);
        return AString::ConstEmpty();
    }

    AString strAttribute;

    strAttribute.Append(TextParser::CHAR_LAQUOT);
    strAttribute.Append(strAttributeName);
    strAttribute.Append(TextParser::CHAR_RAQUOT);
    strAttribute.Append(strValue);
    strAttribute.Append(TextParser::CHAR_LAQUOT);
    strAttribute.Append(TextParser::CHAR_SLASH);
    strAttribute.Append(strAttributeName);
    strAttribute.Append(TextParser::CHAR_RAQUOT);

    strAttribute.Append(TextParser::CHAR_LF);

    IMS_TRACE_D("CreateAttribute : [%s]", strAttribute.GetStr(), 0, 0);
    return strAttribute;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE GLOBAL const AString USSDDataCreator::CreateEndElement(IN const AString& strEndElementName)
{
    if (strEndElementName.GetLength() <= 0)
    {
        IMS_TRACE_I("CreateEndElement : Empty", 0, 0, 0);
        return AString::ConstEmpty();
    }

    AString strEndElement;

    strEndElement.Append(TextParser::CHAR_LAQUOT);
    strEndElement.Append(TextParser::CHAR_SLASH);
    strEndElement.Append(strEndElementName);
    strEndElement.Append(TextParser::CHAR_RAQUOT);

    strEndElement.Append(TextParser::CHAR_LF);

    IMS_TRACE_D("CreateEndElement : [%s]", strEndElement.GetStr(), 0, 0);
    return strEndElement;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PRIVATE GLOBAL void USSDDataCreator::GetErrorCode(
        IN IMS_UINT32 nErrorCode, IN AString& strErrorCode)
{
    if (nErrorCode == USSDDataParser::ERROR_CODE_NONE)
    {
        return;
    }

    switch (nErrorCode)
    {
        case USSDDataParser::ERROR_CODE_1:
            strErrorCode = "1";
            break;
        case USSDDataParser::ERROR_CODE_2:
            strErrorCode = "2";
            break;
        case USSDDataParser::ERROR_CODE_3:
            strErrorCode = "3";
            break;
        case USSDDataParser::ERROR_CODE_4:
            strErrorCode = "4";
            break;
        default:
            break;
    }
}
