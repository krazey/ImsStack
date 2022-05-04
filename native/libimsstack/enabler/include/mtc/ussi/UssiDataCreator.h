/*
 * author : aromi.kwak
 * version : 1.0
 * date : 2016.10
 * brief : Create USSDDataCreator
 */

#ifndef USSD_DATA_CREATOR_H_
#define USSD_DATA_CREATOR_H_

#include "AStringBuffer.h"
#include "ussi/UssiDataParser.h"

class USSDDataCreator
{
public:
    static void GetXMLBody(IN const AString& strUSSDStr, OUT AStringBuffer& objXML,
            IN IMS_SINT32 nSlotID,
            IN IMS_UINT32 nUSSType = USSDDataParser::AnyExtension::USS_TYPE_NONE,
            IN IMS_UINT32 nErrorCode = USSDDataParser::ERROR_CODE_NONE);

private:
    static const AString CreateStartElement(IN const AString& strElementName);
    static const AString CreateAttribute(
            IN const AString& strAttributeName, IN const AString& strValue);
    static const AString CreateEndElement(IN const AString& strEndElementName);

    static void GetErrorCode(IN IMS_UINT32 nErrorCode, IN AString& strErrorCode);
};

#endif /*  USSD_DATA_CREATOR_H_ */
