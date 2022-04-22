#ifndef INTERFACE_XML_RESPONSE_H_
#define INTERFACE_XML_RESPONSE_H_

#include "ImsTypeDef.h"

class IDocument;

class IXmlResponse
{
public:
    /**
     * @brief Returns a parsed XML document.
     *
     * @return Pointer to a new IDocument.
     */
    virtual IDocument* GetDocument() const = 0;

    /**
     * @brief Returns a response code.
     *
     * @return The response code of this response.\n
     *         #RESPONSE_CODE_FAILURE\n
     *         #RESPONSE_CODE_SUCCESS\n
     */
    virtual IMS_SINT32 GetResponseCode() const = 0;

public:
    enum
    {
        RESPONSE_CODE_FAILURE = 0,
        RESPONSE_CODE_SUCCESS,
    };
};

#endif
