/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20141117  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SERVICE_PROTOCOL_H_
#define _SERVICE_PROTOCOL_H_

#include "Protocol.h"

class IService;

class ServiceProtocol : public Protocol
{
public:
    ServiceProtocol();
    virtual ~ServiceProtocol();

public:
    virtual IConnection* OpenPrim(IN const AString& strName);
    virtual IConnection* OpenPrim(
            IN const AString& strScheme, IN const AString& strTarget, IN const AString& strParams);

protected:
    virtual IService* CreateService(IN const AString& strAppId, IN const AString& strServiceId,
            IN const AString& strUserId);
    virtual const IMS_CHAR* GetConnectionScheme() const;

public:
    // "userId"
    static const IMS_CHAR CONNECTION_PARAM_USER_ID[];
    // "serviceId"
    static const IMS_CHAR CONNECTION_PARAM_SERVICE_ID[];
};

#endif  // _SERVICE_PROTOCOL_H_
