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

#include "MtcDef.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "configuration/ConfigDef.h"
#include "ussi/UssiDataCreator.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiData.h"
#include "ussi/UssiDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL void UssiDataCreator::GetXmlBody(IN const AString& strUssdString,
        OUT AStringBuffer& objXml, IN UssiModeType eUssiModeType /*= UssiModeType::NONE*/,
        IN UssiError eErrorCode /*= UssiError::CODE_NONE*/)
{
    objXml.Append(UssiConstants::XML_PROCESSING_INSTRUCTION);
    objXml.Append(TextParser::CHAR_LF);

    objXml.Append(CreateStartElement(UssiConstants::ELEMENT_USSD_DATA));
    objXml.Append(TextParser::CHAR_HTAB);

    objXml.Append(
            CreateAttribute(UssiConstants::ELEMENT_LANGUAGE, UssiConstants::ELEMENT_LANGUAGE_EN));

    // ussd-string
    objXml.Append(TextParser::CHAR_HTAB);
    objXml.Append(CreateAttribute(UssiConstants::ELEMENT_USSD_STRING, strUssdString));

    // error-code
    AString strErrorCode = AString::ConstEmpty();
    GetErrorCode(eErrorCode, strErrorCode);

    if (strErrorCode.GetLength() > 0)
    {
        objXml.Append(TextParser::CHAR_HTAB);
        objXml.Append(CreateAttribute(UssiConstants::ELEMENT_ERROR_CODE, strErrorCode));
    }

    // append extension of request type
    if (eUssiModeType != UssiModeType::NONE)
    {
        objXml.Append(TextParser::CHAR_HTAB);
        objXml.Append(CreateStartElement(UssiConstants::ELEMENT_ANYEXT));

        objXml.Append(TextParser::CHAR_HTAB);
        objXml.Append(TextParser::CHAR_HTAB);

        AString strAttribute;
        strAttribute.Append(TextParser::CHAR_LAQUOT);
        // <UnstructuredSS-Request/>
        if (eUssiModeType == UssiModeType::REQUEST)
        {
            strAttribute.Append(UssiConstants::ELEMENT_USS_REQUEST);
        }
        // <UnstructuredSS-Notify/>
        else if (eUssiModeType == UssiModeType::NOTIFY)
        {
            strAttribute.Append(UssiConstants::ELEMENT_USS_NOTIFY);
        }
        strAttribute.Append(TextParser::CHAR_SLASH);
        strAttribute.Append(TextParser::CHAR_RAQUOT);
        strAttribute.Append(TextParser::CHAR_LF);

        objXml.Append(strAttribute);
        objXml.Append(TextParser::CHAR_HTAB);
        objXml.Append(CreateEndElement(UssiConstants::ELEMENT_ANYEXT));
    }

    objXml.Append(CreateEndElement(UssiConstants::ELEMENT_USSD_DATA));

    IMS_TRACE_D("GetXmlBody : [%s]", strUssdString.GetStr(), 0, 0);
}

PRIVATE GLOBAL const AString UssiDataCreator::CreateStartElement(
        IN const AString& strStartElementName)
{
    if (strStartElementName.GetLength() <= 0)
    {
        IMS_TRACE_D("CreateStartElement : Empty", 0, 0, 0);
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

PRIVATE GLOBAL const AString UssiDataCreator::CreateAttribute(
        IN const AString& strAttributeName, IN const AString& strValue)
{
    if (strAttributeName.GetLength() <= 0 || strValue.GetLength() <= 0)
    {
        IMS_TRACE_D("CreateAttribute : Empty", 0, 0, 0);
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

PRIVATE GLOBAL const AString UssiDataCreator::CreateEndElement(IN const AString& strEndElementName)
{
    if (strEndElementName.GetLength() <= 0)
    {
        IMS_TRACE_D("CreateEndElement : Empty", 0, 0, 0);
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

PRIVATE GLOBAL void UssiDataCreator::GetErrorCode(
        IN UssiError eErrorCode, OUT AString& strErrorCode)
{
    if (eErrorCode == UssiError::CODE_NONE)
    {
        return;
    }

    switch (eErrorCode)
    {
        case UssiError::CODE_1:
            strErrorCode = "1";
            break;
        case UssiError::CODE_2:
            strErrorCode = "2";
            break;
        case UssiError::CODE_3:
            strErrorCode = "3";
            break;
        case UssiError::CODE_4:
            strErrorCode = "4";
            break;
        default:
            break;
    }
}
