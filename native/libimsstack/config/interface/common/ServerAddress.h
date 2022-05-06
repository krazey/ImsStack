/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100905  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SERVER_ADDRESS_H_
#define _SERVER_ADDRESS_H_

#include "AString.h"

class ServerAddress
{
public:
    ServerAddress(IN const AString& strAddress_, IN IMS_SINT32 nPort_ = PORT_UNSPECIFIED);
    ~ServerAddress();

private:
    ServerAddress(IN const ServerAddress& objRHS);
    ServerAddress& operator=(IN const ServerAddress& objRHS);

public:
    /*
     Returns the address (FQDN or IP address) of the IMS server.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    AString                 Address of IMS server
    </table>
    */
    const AString& GetAddress() const;

    /*
     Returns the port number of the IMS server.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IMS_SINT32              Port number
    </table>
    */
    IMS_SINT32 GetPort() const;

private:
    /*
     Sets the address (FQDN or IP address) of the IMS server.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    strAddress              Address of IMS server (FQDN or IP-based string)
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    void SetAddress(IN const AString& strAddress);

    /*
     Sets the port number of the IMS server.

    Parameters
    <table>
    parameter               description
    ----------              ----------
    nPort                   Port number
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>
    */
    void SetPort(IN IMS_SINT32 nPort);

public:
    enum
    {
        PORT_UNSPECIFIED = (-1)
    };

private:
    friend class SubscriberConfig;

    AString strAddress;
    IMS_SINT32 nPort;
};

#endif  // _SERVER_ADDRESS_H_
