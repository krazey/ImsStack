/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090531  toastops@                 Created
    </table>

    Description

*/

#ifndef _SERVICE_IDENTIFIER_H_
#define _SERVICE_IDENTIFIER_H_

#include "AString.h"

class ServiceIdentifierPrivate;

/*

Class ServiceIdentifier

Example

See Also

*/
class ServiceIdentifier
{
public:
    ServiceIdentifier();
    ServiceIdentifier(IN const ServiceIdentifier& objRHS);
    virtual ~ServiceIdentifier();

private:
    ServiceIdentifier(IN const AString& strName, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire);

public:
    ServiceIdentifier& operator=(IN const ServiceIdentifier& objRHS);

public:
    const AString& GetName() const;
    IMS_BOOL IsExplicitPresent() const;
    IMS_BOOL IsRequirePresent() const;
    AString ToString() const;
    static ServiceIdentifier Create(IN const AString& strValue);
    static IMS_BOOL CheckFeatureFlags(
            IN const AString& strValue, IN IMS_BOOL bAllowExplicitRequire);

private:
    ServiceIdentifierPrivate* pServiceIdentifierP;
};

#endif  // _SERVICE_IDENTIFIER_H_
