#ifndef XML_RESPONSE_H_
#define XML_RESPONSE_H_

#include "IXmlResponse.h"

class XmlResponse : public IXmlResponse
{
public:
    inline XmlResponse() :
            m_piDocument(IMS_NULL),
            m_nResponseCode(RESPONSE_CODE_SUCCESS)
    {
    }
    inline virtual ~XmlResponse() {}

    inline IDocument* GetDocument() const override { return m_piDocument; }
    inline IMS_SINT32 GetResponseCode() const override { return m_nResponseCode; }

    inline void SetDocument(IN IDocument* piDocument) { m_piDocument = piDocument; }
    inline void SetResponseCode(IN IMS_SINT32 nResponseCode) { m_nResponseCode = nResponseCode; }

private:
    IDocument* m_piDocument;
    IMS_SINT32 m_nResponseCode;
};

#endif
