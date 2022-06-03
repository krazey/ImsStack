#ifndef USSI_DATA_CREATOR_H_
#define USSI_DATA_CREATOR_H_

#include "AStringBuffer.h"
#include "ussi/UssiData.h"

class UssiDataCreator
{
public:
    static void GetXmlBody(IN const AString& strUssdString, OUT AStringBuffer& objXml,
            IN UssiModeType eUssiModeType = UssiModeType::NONE,
            IN UssiError eErrorCode = UssiError::CODE_NONE);

private:
    static const AString CreateStartElement(IN const AString& strElementName);
    static const AString CreateAttribute(
            IN const AString& strAttributeName, IN const AString& strValue);
    static const AString CreateEndElement(IN const AString& strEndElementName);

    static void GetErrorCode(IN UssiError eErrorCode, OUT AString& strErrorCode);
};

#endif
