#ifndef XML_API_H_
#define XML_API_H_

#include "ImsTypeDef.h"

/**
 * @brief This class provides the common XML APIs.
 */
class XmlApi
{
public:
    /**
     * @brief Returns a current XML version.
     *
     * @return XML version string.
     */
    static const IMS_CHAR* GetVersion();
};

#endif
