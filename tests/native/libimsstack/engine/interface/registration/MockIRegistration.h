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
#ifndef MOCK_I_REGISTRATION_H_
#define MOCK_I_REGISTRATION_H_

#include <gmock/gmock.h>

#include "SipAddress.h"
#include "IRegBase.h"
#include "IRegContact.h"
#include "IRegParameter.h"
#include "IRegistration.h"

class Credential;
class SipProfile;
class IRegistrationListener;
class IRegBindingStateListener;
class IRegUserIdentityNotifier;
class IRegSubscription;

class MockIRegistration : public IRegistration
{
public:
    MOCK_METHOD(ISipMessage*, GetNextRequest, (), (override));
    MOCK_METHOD(ISipMessage*, GetPreviousRequest, (), (const, override));
    MOCK_METHOD(ISipMessage*, GetPreviousResponse, (), (const, override));
    MOCK_METHOD(void, SetSipMessageMediator, (IN IMessageMediator * piMediator), (override));

    MOCK_METHOD(IMS_BOOL, CreateBinding,
            (IN const AString& strAppId, IN const AString& strServiceId), (override));
    MOCK_METHOD(void, DestroyBinding, (IN const AString& strAppId, IN const AString& strServiceId),
            (override));
    MOCK_METHOD(IRegContact*, CreateContact,
            (IN const IpAddress& objIPA, IN IMS_SINT32 nPort, IN IMS_SINT32 nExpiresPolicy,
                    IN IMS_UINT32 nExpiresValue),
            (override));
    MOCK_METHOD(void, DestroyContact, (IN IRegContact * piContact), (override));
    MOCK_METHOD(
            void, DestroyContact, (IN const IpAddress& objIPA, IN IMS_SINT32 nPort), (override));
    MOCK_METHOD(IMS_BOOL, Equals, (IN const IRegistration* piReg), (const, override));
    MOCK_METHOD(const Credential*, GetCredential, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetAor, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetAssociatedUris, (), (const, override));
    MOCK_METHOD(const SipAddress&, GetAuthorizedAor, (), (const, override));
    MOCK_METHOD(ImsList<IRegContact*>, GetAllContacts, (), (const, override));
    MOCK_METHOD(IRegContact*, GetContact, (IN const IpAddress& objIPA, IN IMS_SINT32 nPort),
            (const, override));
    MOCK_METHOD(IRegContact*, GetPreferredContact, (), (const, override));
    MOCK_METHOD(IRegParameter*, GetParameter, (), (const, override));
    MOCK_METHOD(const IpAddress&, GetPublicIpAddress, (), (const, override));
    MOCK_METHOD(const AStringArray&, GetServiceRoutes, (), (const, override));
    MOCK_METHOD(SipProfile*, GetSipProfile, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsBehindNat, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsBindingsUpdated, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsBindingsUpdating, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsNetworkInterworkingRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWithinTrustDomain, (), (const, override));
    MOCK_METHOD(IMS_RESULT, Register, (IN IMS_SINT32 nExpires), (override));
    MOCK_METHOD(IMS_RESULT, Deregister, (), (override));
    MOCK_METHOD(void, RemoveActiveBindingsForcingly, (), (override));
    MOCK_METHOD(void, Restore, (), (override));
    MOCK_METHOD(IMS_RESULT, RestoreActiveBindings, (), (override));
    MOCK_METHOD(void, SetActiveBindingsRestorationUsage, (IN IMS_BOOL bEnabled), (override));
    MOCK_METHOD(
            void, SetAor, (IN const SipAddress& objAOR, IN const AString& strSubsId), (override));
    MOCK_METHOD(void, SetListener, (IN IRegistrationListener * piListener), (override));
    MOCK_METHOD(void, SetRefreshPolicy,
            (IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval, IN IMS_SINT32 nValueEorLT,
                    IN IMS_SINT32 nValueGT),
            (override));
    MOCK_METHOD(void, SetSipProfile, (IN SipProfile * pProfile), (override));
    MOCK_METHOD(
            void, SetBindingStateListener, (IN IRegBindingStateListener * piListener), (override));
    MOCK_METHOD(void, SetFlagForWithinTrustDomain, (IN IMS_BOOL bWithinTrustDomain), (override));
    MOCK_METHOD(void, SetUserIdentityNotifier, (IN IRegUserIdentityNotifier * piUserIdNotifier),
            (override));
    MOCK_METHOD(void, SetUserInfoForContactHeader, (IN const AString& strUserInfo), (override));
    MOCK_METHOD(IRegSubscription*, CreateSubscription, (IN SipAddress * pResourceUri), (override));
};

#endif  // MOCK_I_REGISTRATION_H_
