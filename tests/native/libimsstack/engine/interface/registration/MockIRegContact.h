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
#ifndef MOCK_I_REG_CONTACT_H_
#define MOCK_I_REG_CONTACT_H_

#include <gmock/gmock.h>

#include "IpAddress.h"
#include "SipAddress.h"
class IRegContactListener;

#include "IRegContact.h"

class MockIRegContact : public IRegContact
{
public:
    MOCK_METHOD(IMS_BOOL, AddHeaderParameter,
            (IN const AString& strName, IN const AString& strValue), (override));
    MOCK_METHOD(IMS_BOOL, AddUriParameter, (IN const AString& strName, IN const AString& strValue),
            (override));
    MOCK_METHOD(const SipAddress&, GetContactAddress, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetExpires, (), (const, override));
    MOCK_METHOD(const IpAddress&, GetIpAddress, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPort, (), (const, override));
    MOCK_METHOD(const ImsList<SipParameter*>&, GetHeaderParameters, (), (const, override));
    MOCK_METHOD(const SipParameter*, GetInstanceParameter, (), (const, override));
    MOCK_METHOD(const SipParameter*, GetRegIdParameter, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetPublicGruu, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetTemporaryGruu, (), (const, override));
    MOCK_METHOD(const ImsList<SipAddress*>&, GetTemporaryGruus, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsActiveBinding, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmpty, (), (const, override));
    MOCK_METHOD(void, RemoveAllHeaderParameters, (), (override));
    MOCK_METHOD(void, RemoveHeaderParameter,
            (IN const AString& strName, IN const AString& strValue), (override));
    MOCK_METHOD(void, RemoveUriParameter, (IN const AString& strName, IN const AString& strValue),
            (override));
    MOCK_METHOD(void, SetDisplayName, (IN const AString& strDisplayName), (override));
    MOCK_METHOD(void, SetListener, (IN IRegContactListener * piListener), (override));
    MOCK_METHOD(void, SetPolicyForCallerCapability, (IN IMS_BOOL bCapsByApp), (override));
    MOCK_METHOD(void, SetPort, (IN IMS_SINT32 nPort), (override));
    MOCK_METHOD(void, SetUserInfo, (IN IMS_SINT32 nUserInfoPart), (override));
    MOCK_METHOD(IMS_BOOL, AddExtraCapability,
            (IN const AString& strName, IN const AString& strValue), (override));
    MOCK_METHOD(void, RemoveExtraCapability,
            (IN const AString& strName, IN const AString& strValue), (override));
    MOCK_METHOD(IMS_BOOL, AddService, (IN const AString& strAppId, IN const AString& strServiceId),
            (override));
    MOCK_METHOD(void, RemoveService, (IN const AString& strAppId, IN const AString& strServiceId),
            (override));
    MOCK_METHOD(IMS_BOOL, IsServiceRegistered,
            (IN const AString& strAppId, IN const AString& strServiceId), (const, override));
    MOCK_METHOD(IMS_BOOL, IsFeatureRegistered,
            (IN const AString& strFtName, IN const AString& strFtValue), (const, override));
    MOCK_METHOD(void, RecalculateCallerCapabilities, (), (override));
};

#endif  // MOCK_I_REG_CONTACT_H_
