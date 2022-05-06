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
#include "ServiceTrace.h"
#include "RCObject.h"
#include "TextParser.h"
#include "Feature.h"
#include "ServiceIdentifier.h"

__IMS_TRACE_TAG_CONF__;

class ServiceIdentifierPrivate : public RCObject
{
public:
    inline ServiceIdentifierPrivate() :
            strName(AString::ConstNull()),
            bExplicit(IMS_FALSE),
            bRequire(IMS_FALSE)
    {
    }
    inline virtual ~ServiceIdentifierPrivate() {}

private:
    friend class ServiceIdentifier;

    AString strName;
    IMS_BOOL bExplicit;
    IMS_BOOL bRequire;
};

PUBLIC
ServiceIdentifier::ServiceIdentifier() :
        pServiceIdentifierP(new ServiceIdentifierPrivate())
{
    if (pServiceIdentifierP != IMS_NULL)
    {
        pServiceIdentifierP->AddReference();
    }
}

PUBLIC
ServiceIdentifier::ServiceIdentifier(IN const ServiceIdentifier& objRHS) :
        pServiceIdentifierP(objRHS.pServiceIdentifierP)
{
    if (pServiceIdentifierP != IMS_NULL)
    {
        pServiceIdentifierP->AddReference();
    }
}

PUBLIC VIRTUAL ServiceIdentifier::~ServiceIdentifier()
{
    if (pServiceIdentifierP != IMS_NULL)
    {
        pServiceIdentifierP->RemoveReference();
    }
}

PRIVATE
ServiceIdentifier::ServiceIdentifier(
        IN const AString& strName, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire) :
        pServiceIdentifierP(new ServiceIdentifierPrivate())
{
    if (pServiceIdentifierP != IMS_NULL)
    {
        pServiceIdentifierP->AddReference();

        pServiceIdentifierP->strName = strName.Trim();
        pServiceIdentifierP->bExplicit = bExplicit;
        pServiceIdentifierP->bRequire = bRequire;
    }
}

PUBLIC
ServiceIdentifier& ServiceIdentifier::operator=(IN const ServiceIdentifier& objRHS)
{
    if (this != &objRHS)
    {
        ServiceIdentifierPrivate* pOldServiceIdP = pServiceIdentifierP;

        pServiceIdentifierP = objRHS.pServiceIdentifierP;

        if (pServiceIdentifierP != IMS_NULL)
        {
            pServiceIdentifierP->AddReference();
        }

        if (pOldServiceIdP != IMS_NULL)
        {
            pOldServiceIdP->RemoveReference();
        }
    }

    return (*this);
}

PUBLIC
const AString& ServiceIdentifier::GetName() const
{
    return pServiceIdentifierP->strName;
}

PUBLIC
IMS_BOOL ServiceIdentifier::IsExplicitPresent() const
{
    return pServiceIdentifierP->bExplicit;
}

PUBLIC
IMS_BOOL ServiceIdentifier::IsRequirePresent() const
{
    return pServiceIdentifierP->bRequire;
}

PUBLIC
AString ServiceIdentifier::ToString() const
{
    AString strServiceIdentifier(pServiceIdentifierP->strName);

    if (pServiceIdentifierP->bRequire)
    {
        strServiceIdentifier.Append(TextParser::CHAR_SEMICOLON);
        strServiceIdentifier.Append(Feature::FLAG_REQUIRE);
    }

    if (pServiceIdentifierP->bExplicit)
    {
        strServiceIdentifier.Append(TextParser::CHAR_SEMICOLON);
        strServiceIdentifier.Append(Feature::FLAG_EXPLICIT);
    }

    return strServiceIdentifier;
}

PUBLIC GLOBAL ServiceIdentifier ServiceIdentifier::Create(IN const AString& strValue)
{
    IMS_BOOL bExplicit = IMS_FALSE;
    IMS_BOOL bRequire = IMS_FALSE;
    AStringArray objTokens = strValue.Split(TextParser::CHAR_SEMICOLON);

    // Check validity of feature flags
    for (IMS_SINT32 i = 1; i < objTokens.GetCount(); ++i)
    {
        const AString& strToken = objTokens.GetElementAt(i);

        if (strToken.Equals(Feature::FLAG_EXPLICIT))
        {
            bExplicit = IMS_TRUE;
        }
        else if (strToken.Equals(Feature::FLAG_REQUIRE))
        {
            bRequire = IMS_TRUE;
        }
    }

    return ServiceIdentifier(objTokens.GetElementAt(0), bExplicit, bRequire);
}

PUBLIC GLOBAL IMS_BOOL ServiceIdentifier::CheckFeatureFlags(
        IN const AString& strValue, IN IMS_BOOL bAllowExplicitRequire)
{
    AStringArray objTokens = strValue.Split(TextParser::CHAR_SEMICOLON);

    // Check validity of feature flags
    for (IMS_SINT32 i = 1; i < objTokens.GetCount(); ++i)
    {
        IMS_BOOL bValid = IMS_TRUE;
        const AString& strToken = objTokens.GetElementAt(i);

        if (strToken.Equals(Feature::FLAG_EXPLICIT))
        {
            bValid = bAllowExplicitRequire;
        }
        else if (strToken.Equals(Feature::FLAG_REQUIRE))
        {
            bValid = bAllowExplicitRequire;
        }

        if (!bValid)
        {
            IMS_TRACE_E(0, "Property is malformed, illegal flag in the service identifier(%s)",
                    strValue.GetStr(), 0, 0);
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}
