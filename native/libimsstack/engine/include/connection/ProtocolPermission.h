/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091027  toastops@                 Created
    </table>

    Description

*/

#ifndef _PROTOCOL_PERMISSION_H_
#define _PROTOCOL_PERMISSION_H_

#include "AString.h"

class Protocol;
class ProtocolPermissionPrivate;

class ProtocolPermission
{
private:
    ProtocolPermission();
    ProtocolPermission(IN const ProtocolPermission& objRHS);

public:
    ~ProtocolPermission();

private:
    ProtocolPermission& operator=(IN const ProtocolPermission& objRHS);

public:
    Protocol* Lookup(IN const AString& strName);
    IMS_BOOL Register(IN const AString& strName, IN Protocol* pProtocol);
    IMS_BOOL Deregister(IN const AString& strName);

    static ProtocolPermission* GetInstance();

private:
    ProtocolPermissionPrivate* pProtocolPermissionP;
};

#endif  // _PROTOCOL_PERMISSION_H_
