#ifndef INTERFACE_XML_TRANSACTION_H_
#define INTERFACE_XML_TRANSACTION_H_

#include "ImsTypeDef.h"

class IXmlRequest;
class IXmlResponse;
class IXmlTransactionListener;

class IXmlTransaction
{
public:
    /**
     * @brief Returns an XML request for this transaction.
     *
     * @return Pointer to IXmlRequest.
     */
    virtual IXmlRequest* GetRequest() const = 0;

    /**
     * @brief Returns an XML response for this transaction.
     *
     * @return Pointer to IXmlResponse.
     */
    virtual IXmlResponse* GetResponse() const = 0;

    /**
     * @brief Sends a request to parse an XML document.
     *
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Send() = 0;

    /**
     * @brief Sets an XML transaction listener.
     *
     * @param piListener an XML transaction listener
     */
    virtual void SetListener(IN IXmlTransactionListener* piListener) = 0;
};

#endif
