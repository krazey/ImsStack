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

#include "CarrierConfig.h"
#include "MtcContextRepository.h"
#include "call/MockIMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "conferencecall/ConferenceDef.h"
#include "conferencecall/UriFormatter.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/MtcSupplementaryService.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL AString ANY_NUMBER = "12345";
LOCAL AString ANY_TEL_URI = "tel:12345";
LOCAL AString ANY_SIP_URI = "sip:12345@google.ims.com";
LOCAL AString ANY_SIP_URI_WITH_USER = "sip:12345@google.ims.com;user=phone";
LOCAL AString ANONYMOUS_URI = "sip:anonymous@anonymous.invalid";

namespace android
{

class UriFormatterTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMtcDialingPlan objDialingPlan;
    CallInfo objCallInfo;
    MockIMtcConfigurationManager* pConfigurationManager;
    MtcConfigurationProxy* pConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        // For ConferenceConfigurationWrapper
        MtcContextRepository::GetInstance()->AddContext(0, &objContext);

        ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        pConfigurationManager = new MockIMtcConfigurationManager();
        pConfigurationProxy = new MtcConfigurationProxy(pConfigurationManager);
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
    }

    virtual void TearDown() override { delete pConfigurationProxy; }
};

TEST_F(UriFormatterTest, GetReferToForInvite)
{
    // TODO: test for PAID should be added
    MtcSupplementaryService objSupplementaryService(objContext, *pConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    ParticipantInfo objParticipantInfo(objContext);
    ON_CALL(objContext, GetParticipantInfo).WillByDefault(ReturnRef(objParticipantInfo));
    ON_CALL(*pConfigurationManager, IsConferenceReferToUriSourcePaid)
            .WillByDefault(Return(IMS_FALSE));

    ON_CALL(objDialingPlan, GetToUri(_, _, Scheme::SIP)).WillByDefault(Return(ANY_SIP_URI));
    ON_CALL(objDialingPlan, GetToUri(_, _, Scheme::UNKNOWN)).WillByDefault(Return(ANY_SIP_URI));

    objParticipantInfo.UpdateFromRemoteNumber(ANY_NUMBER);

    AString strUri;
    UriFormatter::GetReferToForInvite(strUri, objContext, IMS_FALSE);
    EXPECT_STREQ(strUri.GetStr(), ANY_SIP_URI_WITH_USER.GetStr());

    ON_CALL(objDialingPlan, GetToUri(_, _, Scheme::TEL)).WillByDefault(Return(ANY_TEL_URI));
    UriFormatter::GetReferToForInvite(strUri, objContext, IMS_FALSE);
    EXPECT_STREQ(strUri.GetStr(), ANY_SIP_URI_WITH_USER.GetStr());
}

TEST_F(UriFormatterTest, GetReferToForInviteWithConfUser)
{
    ConfUser* pConfUser = IMS_NULL;
    AString strUri;
    UriFormatter::GetReferToForInvite(strUri, objContext, pConfUser);
    EXPECT_STREQ("", strUri.GetStr());

    pConfUser = new ConfUser();
    UriFormatter::GetReferToForInvite(strUri, objContext, pConfUser);
    EXPECT_STREQ("", strUri.GetStr());

    pConfUser->strTarget = ANY_SIP_URI;
    UriFormatter::GetReferToForInvite(strUri, objContext, pConfUser);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());

    ON_CALL(objDialingPlan, GetToUri(ANY_NUMBER, _, Scheme::SIP))
            .WillByDefault(Return(ANY_SIP_URI));
    pConfUser->strTarget = ANY_TEL_URI;
    UriFormatter::GetReferToForInvite(strUri, objContext, pConfUser);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());

    pConfUser->strTarget = ANY_NUMBER;
    UriFormatter::GetReferToForInvite(strUri, objContext, pConfUser);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());

    pConfUser->strTarget = ANONYMOUS_URI;
    UriFormatter::GetReferToForInvite(strUri, objContext, pConfUser);
    EXPECT_STREQ(ANONYMOUS_URI.GetStr(), strUri.GetStr());

    delete pConfUser;
}

TEST_F(UriFormatterTest, GetReferToForByeUserNull)
{
    // ConfUser null -> Invited URI
    ConfUser* pConfUser = IMS_NULL;
    AString strUri;
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANY_SIP_URI_WITH_USER);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());
    delete pConfUser;
}

TEST_F(UriFormatterTest, GetReferToForByeEmptyUserEntity)
{
    // emnpty User Entity -> Invited URI
    AString strUri;
    ConfUser* pConfUser = new ConfUser();
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANY_SIP_URI_WITH_USER);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());
}

TEST_F(UriFormatterTest, GetReferToForByeAnonymous)
{
    // real anonymous -> User Entity
    AString strAnonymousUserEntiry("sip:anonymous1@anonymous.invalid");
    AString strUri;
    ConfUser* pConfUser = new ConfUser();
    pConfUser->strUserEntity = strAnonymousUserEntiry;
    pConfUser->strTarget = ANY_SIP_URI_WITH_USER;
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANONYMOUS_URI);
    EXPECT_STREQ(strAnonymousUserEntiry.GetStr(), strUri.GetStr());
}

TEST_F(UriFormatterTest, GetReferToForByeTelUri)
{
    // TEL URI -> SIP URI
    AString strUri;
    ConfUser* pConfUser = new ConfUser();
    pConfUser->strUserEntity = "tel:12345";
    pConfUser->strTarget = ANY_TEL_URI;
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANY_SIP_URI_WITH_USER);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());
}

TEST_F(UriFormatterTest, GetReferToForByeReuseUri)
{
    // Invited URI by configuration.
    AString strUri;
    ON_CALL(*pConfigurationManager, GetConferenceDropReferToUriSourceType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::
                            CONFERENCE_DROP_REFER_TO_URI_SOURCE_REFER_TO_URI_FOR_INVITE));

    ConfUser* pConfUser = new ConfUser();
    pConfUser->strUserEntity = "sip:anyUri";
    pConfUser->strTarget = "sip:anotherUri";
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANY_SIP_URI_WITH_USER);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());
}

TEST_F(UriFormatterTest, GetReferToForByeInvalidAnonymous)
{
    // invalid anonymous -> Invited URI
    AString strUri;
    ON_CALL(*pConfigurationManager, GetConferenceDropReferToUriSourceType)
            .WillByDefault(Return(CarrierConfig::ImsVoice::
                            CONFERENCE_DROP_REFER_TO_URI_SOURCE_USER_ENTITY_IN_CONFERENCE_EVENT_PACKAGE));

    ConfUser* pConfUser = new ConfUser();
    pConfUser->strUserEntity = "sip:anonymous@anonymous.invalid";
    pConfUser->strTarget = ANONYMOUS_URI;
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANY_SIP_URI_WITH_USER);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());
}

TEST_F(UriFormatterTest, GetReferToForBye)
{
    // User Entity with user=phone
    AString strUri;
    ConfUser* pConfUser = new ConfUser();
    pConfUser->strUserEntity = ANY_SIP_URI;
    pConfUser->strTarget = ANY_SIP_URI_WITH_USER;
    UriFormatter::GetReferToForBye(strUri, pConfUser, ANY_SIP_URI_WITH_USER);
    EXPECT_STREQ(ANY_SIP_URI_WITH_USER.GetStr(), strUri.GetStr());

    delete pConfUser;
}

}  // namespace android
