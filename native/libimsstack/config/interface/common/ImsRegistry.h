/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    20091106  toastops@                 Renamed (RegistryProperties -> ImsRegistry)
    </table>

    Description

*/

#ifndef _IMS_REGISTRY_H_
#define _IMS_REGISTRY_H_

#include "AStringArray.h"

class ImsRegistry
{
public:
    ImsRegistry();
    ImsRegistry(IN const ImsRegistry& objRHS);
    ~ImsRegistry();

public:
    ImsRegistry& operator=(IN const ImsRegistry& objRHS);

public:
    IMS_BOOL Add(IN const AStringArray& objProperty);
    const AStringArray& GetAt(IN IMS_SINT32 i) const;
    IMS_SINT32 GetCount() const;

private:
    IMSList<AStringArray> objProperties;
};

#endif  // _IMS_REGISTRY_H_
