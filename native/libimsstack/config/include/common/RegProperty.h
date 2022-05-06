/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _REG_PROPERTY_H_
#define _REG_PROPERTY_H_

#include "private/ImsProperty.h"

/*

Class RegProperty

Example

See Also

*/
class RegProperty : public ImsProperty
{
public:
    explicit RegProperty(IN const AString& strServiceId_);
    RegProperty(IN const RegProperty& objRHS);
    virtual ~RegProperty();

public:
    RegProperty& operator=(IN const RegProperty& objRHS);

public:
    IMS_BOOL AddValue(IN const AString& strValue);
    const IMSList<AString>& GetValues() const;

protected:
    virtual IMS_BOOL Equals(IN const ImsProperty& objOther) const;

private:
    AString strServiceId;
    // "Name: Value"
    IMSList<AString> objHeaders;
};

#endif  // _REG_PROPERTY_H_
