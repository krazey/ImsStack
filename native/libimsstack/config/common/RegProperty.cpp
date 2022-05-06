/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "RegProperty.h"

PUBLIC
RegProperty::RegProperty(IN const AString& strServiceId_) :
        ImsProperty(ImsProperty::PKEY_REG),
        strServiceId(strServiceId_)
{
}

PUBLIC
RegProperty::RegProperty(IN const RegProperty& objRHS) :
        ImsProperty(objRHS),
        strServiceId(objRHS.strServiceId),
        objHeaders(objRHS.objHeaders)
{
}

PUBLIC VIRTUAL RegProperty::~RegProperty() {}

PUBLIC
RegProperty& RegProperty::operator=(IN const RegProperty& objRHS)
{
    if (this != &objRHS)
    {
        ImsProperty::operator=(objRHS);

        strServiceId = objRHS.strServiceId;
        objHeaders = objRHS.objHeaders;
    }

    return (*this);
}

PUBLIC
IMS_BOOL RegProperty::AddValue(IN const AString& strValue)
{
    return objHeaders.Append(strValue);
}

PUBLIC
const IMSList<AString>& RegProperty::GetValues() const
{
    return objHeaders;
}

PROTECTED VIRTUAL IMS_BOOL RegProperty::Equals(IN const ImsProperty& objOther) const
{
    const RegProperty& objRegOther = DYNAMIC_CAST(const RegProperty&, objOther);

    if (nKey != objRegOther.nKey)
    {
        return IMS_FALSE;
    }

    if (!strServiceId.Equals(objRegOther.strServiceId))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
