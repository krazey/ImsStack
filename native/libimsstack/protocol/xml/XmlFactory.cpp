#include "XmlFactory.h"
#include "XmlStreamWriter.h"
#include "XmlTransactionProvider.h"

PROTECTED
XmlFactory::XmlFactory() {}

PUBLIC
XmlFactory::~XmlFactory() {}

PUBLIC
IXmlStreamWriter* XmlFactory::CreateStreamWriter()
{
    return new XmlStreamWriter();
}

PUBLIC
void XmlFactory::DestroyStreamWriter(IN IXmlStreamWriter*& piWriter)
{
    XmlStreamWriter* pWriter = DYNAMIC_CAST(XmlStreamWriter*, piWriter);

    if (pWriter != IMS_NULL)
    {
        delete pWriter;
    }

    piWriter = IMS_NULL;
}

PUBLIC
IXmlTransactionProvider* XmlFactory::CreateTransactionProvider()
{
    return new XmlTransactionProvider();
}

PUBLIC
void XmlFactory::DestroyTransactionProvider(IN IXmlTransactionProvider*& piProvider)
{
    XmlTransactionProvider* pProvider = DYNAMIC_CAST(XmlTransactionProvider*, piProvider);

    if (pProvider != IMS_NULL)
    {
        delete pProvider;
    }

    piProvider = IMS_NULL;
}

PUBLIC GLOBAL XmlFactory* XmlFactory::GetInstance()
{
    static XmlFactory* pXmlFactory = IMS_NULL;

    if (pXmlFactory == IMS_NULL)
    {
        pXmlFactory = new XmlFactory();
    }

    return pXmlFactory;
}
