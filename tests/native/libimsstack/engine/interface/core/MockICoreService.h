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
#ifndef MOCK_I_CORE_SERVICE_H_
#define MOCK_I_CORE_SERVICE_H_

#include <gmock/gmock.h>

#include "IService.h"
#include "ICoreService.h"
class ICapabilities;
class ICoreServiceListener;
class IDirectCoreServiceListener;
class IPageMessage;
class IPublication;
class IReference;
class ISession;
class ISipConnectionFactory;
class ISubscription;

class MockICoreService : public ICoreService
{
public:
    ~MockICoreService() override = default;

    // IConnection
    MOCK_METHOD(void, Close, (), (override));

    // IService
    MOCK_METHOD(const AString&, GetAppId, (), (const, override));
    MOCK_METHOD(const AString&, GetScheme, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetAuthorizedUserId, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetContactAddress, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetContactAddressForOutgoingMessage, (), (const, override));
    MOCK_METHOD(ISipHeader*, GetContactHeader,
            (IN IMS_BOOL bPrivacy, IN IMS_BOOL bRequest, IN IMS_SINT32 nSipMethod),
            (const, override));
    MOCK_METHOD(IFeatureCaps*, GetFeatureCaps, (), (const, override));
    MOCK_METHOD(IServiceFilterCriteria*, GetFilterCriteria, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetPathHeaders, (), (const, override));
    MOCK_METHOD(const IRegInfo*, GetRegInfo, (), (const, override));
    MOCK_METHOD(const IpAddress&, GetIpAddress, (), (const, override));
    MOCK_METHOD(SipProfile*, GetSipProfile, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetUserIdentities, (), (const, override));
    MOCK_METHOD(const AString&, GetUserIdentity, (IN IMS_SINT32 nScheme), (const, override));
    MOCK_METHOD(const SipParameter*, GetInstanceParameter, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetPublicGruu, (), (const, override));
    MOCK_METHOD(const SipAddress*, GetTemporaryGruu, (), (const, override));
    MOCK_METHOD(const ImsList<SipAddress*>&, GetTemporaryGruus, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsBehindNat, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsConnected, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWithinTrustDomain, (), (const, override));
    MOCK_METHOD(IMS_BOOL, AddFeatureTags,
            (IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired), (override));
    MOCK_METHOD(IMS_BOOL, RemoveFeatureTags,
            (IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired), (override));
    MOCK_METHOD(void, SetSipProfile, (IN SipProfile * pProfile), (override));

    // ICoreService
    MOCK_METHOD(ICapabilities*, CreateCapabilities,
            (IN const AString& strFrom, IN const AString& strTo), (override));
    MOCK_METHOD(IPageMessage*, CreatePageMessage,
            (IN const AString& strFrom, IN const AString& strTo), (override));
    MOCK_METHOD(IPublication*, CreatePublication,
            (IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent),
            (override));
    MOCK_METHOD(IReference*, CreateReference,
            (IN const AString& strFrom, IN const AString& strTo, IN const AString& strReferTo,
                    IN const AString& strReferMethod),
            (override));
    MOCK_METHOD(ISession*, CreateSession, (IN const AString& strFrom, IN const AString& strTo),
            (override));
    MOCK_METHOD(ISubscription*, CreateSubscription,
            (IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent),
            (override));
    MOCK_METHOD(AString, GetLocalUserId, (), (const, override));
    MOCK_METHOD(void, SetListener, (IN ICoreServiceListener * piListener), (override));
    MOCK_METHOD(ISipConnectionFactory*, CreateSipConnectionFactory, (), (override));
    MOCK_METHOD(void, SetDirectListener, (IN IDirectCoreServiceListener * piListener), (override));
};

#endif
