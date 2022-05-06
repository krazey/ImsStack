/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090319  toastops@                 Created
    </table>

    Description

*/

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#include "AString.h"

class IConnection;

class Protocol
{
public:
    Protocol();
    virtual ~Protocol() = 0;

public:
    virtual IConnection* OpenPrim(IN const AString& strName);
    virtual IConnection* OpenPrim(
            IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams);

    static void ParseName(IN const AString& strName, OUT AString& strScheme, OUT AString& strTarget,
            OUT AString& strParams);
};

#endif  // _PROTOCOL_H_
