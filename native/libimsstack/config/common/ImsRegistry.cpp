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

#include "ServiceMemory.h"
#include "ImsRegistry.h"

PUBLIC
ImsRegistry::ImsRegistry() :
        objProperties(IMSList<AStringArray>())
{
}

PUBLIC
ImsRegistry::ImsRegistry(IN const ImsRegistry& objRHS) :
        objProperties(objRHS.objProperties)
{
}

PUBLIC
ImsRegistry::~ImsRegistry() {}

PUBLIC
ImsRegistry& ImsRegistry::operator=(IN const ImsRegistry& objRHS)
{
    if (this != &objRHS)
    {
        objProperties = objRHS.objProperties;
    }

    return (*this);
}

PUBLIC
IMS_BOOL ImsRegistry::Add(IN const AStringArray& objProperty)
{
    return objProperties.Append(objProperty);
}

PUBLIC
const AStringArray& ImsRegistry::GetAt(IN IMS_SINT32 i) const
{
    return objProperties.GetAt(i);
}

PUBLIC
IMS_SINT32 ImsRegistry::GetCount() const
{
    return static_cast<IMS_SINT32>(objProperties.GetSize());
}
