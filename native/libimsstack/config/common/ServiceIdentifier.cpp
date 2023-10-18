/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "RcObject.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "TextParser.h"

#include "Feature.h"
#include "ServiceIdentifier.h"

__IMS_TRACE_TAG_CONF__;

class ServiceIdentifierPrivate : public RcObject
{
public:
    inline ServiceIdentifierPrivate() :
            m_strName(AString::ConstNull()),
            m_bExplicit(IMS_FALSE),
            m_bRequire(IMS_FALSE)
    {
    }
    inline virtual ~ServiceIdentifierPrivate() {}

    ServiceIdentifierPrivate(IN const ServiceIdentifierPrivate&) = delete;
    ServiceIdentifierPrivate& operator=(IN const ServiceIdentifierPrivate&) = delete;

private:
    friend class ServiceIdentifier;

    AString m_strName;
    IMS_BOOL m_bExplicit;
    IMS_BOOL m_bRequire;
};

PUBLIC
ServiceIdentifier::ServiceIdentifier() :
        m_pServiceIdPrivate(new ServiceIdentifierPrivate())
{
    if (m_pServiceIdPrivate != IMS_NULL)
    {
        m_pServiceIdPrivate->AddReference();
    }
}

PUBLIC
ServiceIdentifier::ServiceIdentifier(IN const ServiceIdentifier& other) :
        m_pServiceIdPrivate(other.m_pServiceIdPrivate)
{
    if (m_pServiceIdPrivate != IMS_NULL)
    {
        m_pServiceIdPrivate->AddReference();
    }
}

PUBLIC VIRTUAL ServiceIdentifier::~ServiceIdentifier()
{
    if (m_pServiceIdPrivate != IMS_NULL)
    {
        m_pServiceIdPrivate->RemoveReference();
    }
}

PRIVATE
ServiceIdentifier::ServiceIdentifier(
        IN const AString& strName, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire) :
        m_pServiceIdPrivate(new ServiceIdentifierPrivate())
{
    if (m_pServiceIdPrivate != IMS_NULL)
    {
        m_pServiceIdPrivate->AddReference();

        m_pServiceIdPrivate->m_strName = strName.Trim();
        m_pServiceIdPrivate->m_bExplicit = bExplicit;
        m_pServiceIdPrivate->m_bRequire = bRequire;
    }
}

PUBLIC
ServiceIdentifier& ServiceIdentifier::operator=(IN const ServiceIdentifier& other)
{
    if (this != &other)
    {
        ServiceIdentifierPrivate* pOldServiceIdPrivate = m_pServiceIdPrivate;

        m_pServiceIdPrivate = other.m_pServiceIdPrivate;

        if (m_pServiceIdPrivate != IMS_NULL)
        {
            m_pServiceIdPrivate->AddReference();
        }

        if (pOldServiceIdPrivate != IMS_NULL)
        {
            pOldServiceIdPrivate->RemoveReference();
        }
    }

    return (*this);
}

PUBLIC
const AString& ServiceIdentifier::GetName() const
{
    return m_pServiceIdPrivate->m_strName;
}

PUBLIC
IMS_BOOL ServiceIdentifier::IsExplicitPresent() const
{
    return m_pServiceIdPrivate->m_bExplicit;
}

PUBLIC
IMS_BOOL ServiceIdentifier::IsRequirePresent() const
{
    return m_pServiceIdPrivate->m_bRequire;
}

PUBLIC
AString ServiceIdentifier::ToString() const
{
    AString strServiceIdentifier(m_pServiceIdPrivate->m_strName);

    if (m_pServiceIdPrivate->m_bRequire)
    {
        strServiceIdentifier.Append(TextParser::CHAR_SEMICOLON);
        strServiceIdentifier.Append(Feature::FLAG_REQUIRE);
    }

    if (m_pServiceIdPrivate->m_bExplicit)
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

        if (strToken.Equals(Feature::FLAG_EXPLICIT) || strToken.Equals(Feature::FLAG_REQUIRE))
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
