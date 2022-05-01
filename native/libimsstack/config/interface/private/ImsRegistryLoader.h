/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20091106  toastops@                 Created
    </table>

    Description

*/

#ifndef IMS_REGISTRY_LOADER_H_
#define IMS_REGISTRY_LOADER_H_

#include "ImsRegistry.h"

class ImsRegistryLoader
{
public:
    ImsRegistryLoader() = delete;

public:
    static IMS_BOOL GetRegistry(IN const AString& strAppId, OUT ImsRegistry& objRegistry);
};

#endif
