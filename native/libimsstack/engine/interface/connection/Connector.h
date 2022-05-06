#ifndef _CONNECTOR_H_
#define _CONNECTOR_H_

#include "AString.h"

class IConnection;

/**
 * @brief This class is a factory class to create IConnection interface.
 */
class Connector
{
private:
    Connector();

public:
    /**
     * @brief Creates IConnection interface using a specified name.
     *
     * @param strName specify the connection scheme and parameters to be opened
     * @return Pointer to IConnection or null
     * @see IConnection
     */
    static IConnection* Open(IN const AString& strName);

    /**
     * @brief Creates IConnection interface using a specified scheme, target, and parameters.
     *
     * @param strScheme a scheme to identify IConnection category
     *                  (i.e. "imscore"/"sip")
     * @param strTarget a target information to be opened. It depends on the scheme.
     * @param strParams any parameters for this scheme.
     *                  Multiple parameters are separated by semi-colon
     * @return Pointer to IConnection or null
     * @see IConnection
     */
    static IConnection* Open(
            IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams);
};

#endif  // _CONNECTOR_H_
