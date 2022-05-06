#include "DomDocumentBuilder.h"
#include "DomDocumentBuilderFactory.h"
#include "ServiceTrace.h"
#include "XmlApi.h"

PRIVATE
DomDocumentBuilderFactory::DomDocumentBuilderFactory()
{
    ITrace* piTrace = TraceService::GetTraceService()->GetTrace();

    if (piTrace != IMS_NULL)
    {
        piTrace->Out("XmlLibVersion=%s", XmlApi::GetVersion());
    }
}

PRIVATE VIRTUAL DomDocumentBuilderFactory::~DomDocumentBuilderFactory() {}

PUBLIC VIRTUAL DocumentBuilder* DomDocumentBuilderFactory::NewDocumentBuilder()
{
    return new DomDocumentBuilder();
}

PUBLIC GLOBAL DomDocumentBuilderFactory* DomDocumentBuilderFactory::GetInstance()
{
    static DomDocumentBuilderFactory* pDomDocumentBuilderFactory = IMS_NULL;

    if (pDomDocumentBuilderFactory == IMS_NULL)
    {
        pDomDocumentBuilderFactory = new DomDocumentBuilderFactory();
    }

    return pDomDocumentBuilderFactory;
}

PUBLIC VIRTUAL void DomDocumentBuilderFactory::DestroyDocumentBuilder(
        IN DocumentBuilder*& pDocumentBuilder)
{
    DomDocumentBuilder* pBuilder = DYNAMIC_CAST(DomDocumentBuilder*, pDocumentBuilder);

    if (pBuilder != IMS_NULL)
    {
        delete pBuilder;
    }

    pDocumentBuilder = IMS_NULL;
}
