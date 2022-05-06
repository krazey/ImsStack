#ifndef XML_REQUEST_H_
#define XML_REQUEST_H_

#include "IXmlRequest.h"

class XmlRequest : public IXmlRequest
{
public:
    inline XmlRequest() :
            m_strRawXml(AString::ConstNull())
    {
    }
    inline virtual ~XmlRequest() {}

public:
    inline void SetRawXml(IN const IMS_CHAR* pszRawXml) override { m_strRawXml = pszRawXml; }
    inline void SetRawXml(IN const AString& strRawXml) override { m_strRawXml = strRawXml; }
    inline const AString& GetRawXml() const override { return m_strRawXml; }

private:
    AString m_strRawXml;
};

#endif
