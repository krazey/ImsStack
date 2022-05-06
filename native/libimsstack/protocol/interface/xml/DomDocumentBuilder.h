#ifndef DOM_DOCUMENT_BUILDER_H_
#define DOM_DOCUMENT_BUILDER_H_

#include "DocumentBuilder.h"

/**
 * @brief This class provides the APIs to obtain DOM Document instances from an XML document.
 */
class DomDocumentBuilder : public DocumentBuilder
{
public:
    inline DomDocumentBuilder() :
            DocumentBuilder()
    {
    }
    inline virtual ~DomDocumentBuilder() {}
};

#endif
