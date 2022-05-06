#ifndef DOM_DOCUMENT_BUILDER_FACTORY_H_
#define DOM_DOCUMENT_BUILDER_FACTORY_H_

#include "DocumentBuilder.h"
#include "DocumentBuilderFactory.h"

/**
 * @brief This class provides a factory API that enables XML applications to obtain a parser
 *        that produces DOM object trees from XML documents.
 */
class DomDocumentBuilderFactory : public DocumentBuilderFactory
{
private:
    DomDocumentBuilderFactory();
    virtual ~DomDocumentBuilderFactory();

public:
    DocumentBuilder* NewDocumentBuilder() override;
    void DestroyDocumentBuilder(IN DocumentBuilder*& pDocumentBuilder);

    static DomDocumentBuilderFactory* GetInstance();
};

#endif
