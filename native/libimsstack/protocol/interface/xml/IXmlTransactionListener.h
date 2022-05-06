#ifndef INTERFACE_XML_TRANSACTION_LISTENER_H_
#define INTERFACE_XML_TRANSACTION_LISTENER_H_

#include "IXmlTransaction.h"

class IXmlTransactionListener
{
public:
    /**
     * @brief Notifies the application that parsing XML document is completed.
     *
     * @param piTransaction an XML transaction
     * @return If the notification is successfully processed, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT XmlTransaction_NotifyParsingCompleted(IN IXmlTransaction* piTransaction) = 0;
};

#endif
