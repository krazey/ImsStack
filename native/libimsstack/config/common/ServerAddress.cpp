/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100905  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServerAddress.h"

PUBLIC
ServerAddress::ServerAddress(
        IN const AString& strAddress_, IN IMS_SINT32 nPort_ /* = PORT_UNSPECIFIED */) :
        strAddress(strAddress_),
        nPort(nPort_)
{
}

PUBLIC
ServerAddress::~ServerAddress() {}

PUBLIC
const AString& ServerAddress::GetAddress() const
{
    return strAddress;
}

PUBLIC
IMS_SINT32 ServerAddress::GetPort() const
{
    return nPort;
}

PRIVATE
void ServerAddress::SetAddress(IN const AString& strAddress)
{
    this->strAddress = strAddress;
}

PRIVATE
void ServerAddress::SetPort(IN IMS_SINT32 nPort)
{
    this->nPort = nPort;
}
