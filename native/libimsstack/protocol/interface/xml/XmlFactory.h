#ifndef XML_FACTORY_H_
#define XML_FACTORY_H_

#include "ImsTypeDef.h"

class IXmlStreamWriter;
class IXmlTransactionProvider;

class XmlFactory
{
protected:
    XmlFactory();

public:
    ~XmlFactory();

public:
    /**
     * @brief Creates a new IXmlStreamWriter that writes to a stream.
     *
     * @return Pointer to a new IXmlStreamWriter.
     */
    IXmlStreamWriter* CreateStreamWriter();

    /**
     * @brief Destroys a specified IXmlStreamWriter.
     *
     * @param piWriter An XML stream writer to destroy
     */
    void DestroyStreamWriter(IN IXmlStreamWriter*& piWriter);

    /**
     * @brief Creates a new IXmlTransactionProvider.
     *
     * @return Pointer to a new IXmlTransactionProvider.
     */
    IXmlTransactionProvider* CreateTransactionProvider();

    /**
     * @brief Destroys a specified IXmlTransactionProvider.
     *
     * @param piProvider An XML transaction provider to destroy
     */
    void DestroyTransactionProvider(IN IXmlTransactionProvider*& piProvider);

    /**
     * @brief Gets a new instance of this factory.
     *
     * @return An instance of XmlFactory.
     */
    static XmlFactory* GetInstance();
};

#endif
