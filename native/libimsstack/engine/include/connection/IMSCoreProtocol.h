/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091126  toastops@                 Created
    </table>

    Description

*/

#ifndef _IMS_CORE_PROTOCOL_H_
#define _IMS_CORE_PROTOCOL_H_

#include "ServiceProtocol.h"
#include "IMSCore.h"

class AppConfig;
class CoreServiceConfig;

class IMSCoreProtocol : public ServiceProtocol
{
private:
    IMSCoreProtocol();
    IMSCoreProtocol(IN const IMSCoreProtocol& objRHS);

public:
    virtual ~IMSCoreProtocol();

public:
    static IMSCoreProtocol* GetInstance();

private:
    // ServiceProtocol class
    virtual IService* CreateService(IN const AString& strAppId, IN const AString& strServiceId,
            IN const AString& strUserId);
    inline virtual const IMS_CHAR* GetConnectionScheme() const
    {
        return IMSCore::CONNECTION_SCHEME;
    }

    static IMS_BOOL IsRegistryConsistent(
            IN const AppConfig* pAppConfig, IN const CoreServiceConfig* pServiceConfig);
};

#endif  // _IMS_CORE_PROTOCOL_H_
