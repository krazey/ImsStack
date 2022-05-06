#ifndef DOCUMENT_BUILDER_FACTORY_H_
#define DOCUMENT_BUILDER_FACTORY_H_

class DocumentBuilder;

/**
 * @brief This class provides a factory API that enables the applications to obtain a parser
 *        that produces DOM object trees from XML documents.
 */
class DocumentBuilderFactory
{
protected:
    inline DocumentBuilderFactory() {}

public:
    inline virtual ~DocumentBuilderFactory() {}

public:
    /**
     * @brief Creates a new instance of a DocumentBuilder using the currently configured
     *        parameters.
     *
     * @return A new instance of a DocumentBuilder.
     */
    virtual DocumentBuilder* NewDocumentBuilder() = 0;
};

#endif
