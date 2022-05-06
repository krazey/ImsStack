#ifndef _INTERFACE_CONNECTION_H_
#define _INTERFACE_CONNECTION_H_

#include "IMSTypeDef.h"

/**
 * @brief This class is a basic interface class to connect with IMS engine.
 *
 * This is the most basic connection type that can only be opened and closed.\n
 * The Open() method is not included in the interface because connections are always
 * opened using the static Open() method of the Connector class.
 *
 * @see Connector
 */
class IConnection
{
public:
    /**
     * @brief Destroys IConnection interface.
     */
    virtual void Close() = 0;
};

#endif  // _INTERFACE_CONNECTION_H_
