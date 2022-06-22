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
#ifndef SERVICE_IDENTIFIER_H_
#define SERVICE_IDENTIFIER_H_

#include "AString.h"

class ServiceIdentifierPrivate;

class ServiceIdentifier
{
public:
    ServiceIdentifier();
    ServiceIdentifier(IN const ServiceIdentifier& other);
    virtual ~ServiceIdentifier();

private:
    ServiceIdentifier(IN const AString& strName, IN IMS_BOOL bExplicit, IN IMS_BOOL bRequire);

public:
    ServiceIdentifier& operator=(IN const ServiceIdentifier& other);

public:
    const AString& GetName() const;
    IMS_BOOL IsExplicitPresent() const;
    IMS_BOOL IsRequirePresent() const;
    AString ToString() const;
    static ServiceIdentifier Create(IN const AString& strValue);
    static IMS_BOOL CheckFeatureFlags(
            IN const AString& strValue, IN IMS_BOOL bAllowExplicitRequire);

private:
    ServiceIdentifierPrivate* m_pServiceIdPrivate;
};

#endif
