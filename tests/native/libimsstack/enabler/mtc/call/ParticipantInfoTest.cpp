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

#include "AString.h"
#include "ImsTypeDef.h"
#include "MockIMtcService.h"
#include "aos/IImsAosInfo.h"
#include "call/MockIMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/MockICoreService.h"
#include "core/MockIMessage.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "sipcore/SipAddress.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class ParticipantInfoTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    ParticipantInfo* pParticipantInfo;

protected:
    virtual void SetUp() override { pParticipantInfo = new ParticipantInfo(objContext); }

    virtual void TearDown() override { delete pParticipantInfo; }
};

TEST_F(ParticipantInfoTest, GetLocalNumberReturnsNullIfCoreServiceIsNull)
{
    MockIMtcService objService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(nullptr));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_TRUE(pParticipantInfo->GetLocalNumber().IsNull());
}

TEST_F(ParticipantInfoTest, GetLocalNumberReturnsNumber)
{
    AString strUserPart("some_number");
    SipAddress objSipAddress;
    objSipAddress.SetUser(strUserPart);

    MockICoreService objCoreService;
    ON_CALL(objCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    MockIMtcService objService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_EQ(strUserPart, pParticipantInfo->GetLocalNumber());
}

TEST_F(ParticipantInfoTest, GetLocalUriReturnsNullIfNotEmergency)
{
    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());
}

TEST_F(ParticipantInfoTest, GetLocalUriReturnsNullIfEmergencyAndAosConnectorIsNull)
{
    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(nullptr));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());
}

TEST_F(ParticipantInfoTest, GetLocalUriReturnsNullIfEmergencyNormalRegistered)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());
}

TEST_F(ParticipantInfoTest, GetLocalUriReturnsAnonymousIfEmergencyNoUiccRegistered)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    AString objAnonymousUri("Anonymous <sip:anonymous@anonymous.invalid>");
    EXPECT_EQ(objAnonymousUri, pParticipantInfo->GetLocalUri());
}

TEST_F(ParticipantInfoTest, GetLocalUriReturnsAnonymousIfEmergencyAdminRegistered)
{
    MockIMtcAosConnector objAosConnector;
    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    AString objAnonymousUri("Anonymous <sip:anonymous@anonymous.invalid>");
    EXPECT_EQ(objAnonymousUri, pParticipantInfo->GetLocalUri());
}

TEST_F(ParticipantInfoTest, GetRemoteNumberReturnsNullInitially)
{
    EXPECT_TRUE(pParticipantInfo->GetRemoteNumber().IsNull());
}

TEST_F(ParticipantInfoTest, GetRemoteUriReturnsFromSupplementaryServiceIfTargetUriIsSet)
{
    const AString strUri("some_uri");

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    objSupplementaryService.Add(SuppType::TARGET_URI, strUri);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(strUri, pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, GetRemoteUriReturnsInitialRemoteUri)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(AString::ConstNull(), pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, GetRemoteUriReturnsLocalUriIfCallPullIsEnabled)
{
    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));
    objSupplementaryService.Add(SuppType::CALL_PULL, IMS_FALSE);

    SipAddress objSipAddress("localAddress");
    MockICoreService objCoreService;
    ON_CALL(objCoreService, GetAuthorizedUserId).WillByDefault(ReturnRef(objSipAddress));

    MockIMtcService objService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(&objCoreService));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_STREQ("localAddress", pParticipantInfo->GetRemoteUri().GetStr());
}

TEST_F(ParticipantInfoTest, GetRemoteDisplayNameReturnsFromSupplementaryService)
{
    const AString strCnap("some_cnap");

    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    objSupplementaryService.Add(SuppType::CNAP, strCnap);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(strCnap, pParticipantInfo->GetRemoteDisplayName());
}

TEST_F(ParticipantInfoTest, GetRemoteDisplayNameReturnsInitialRemoteNumber)
{
    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(AString::ConstNull(), pParticipantInfo->GetRemoteDisplayName());
}

TEST_F(ParticipantInfoTest, GetOipTypeReturnsFromSupplementaryService)
{
    const OipType eOipType = OipType::IDENTITY;

    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    objSupplementaryService.Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(eOipType));
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(eOipType, pParticipantInfo->GetOipType());
}

TEST_F(ParticipantInfoTest, GetOipTypeReturnsNone)
{
    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(OipType::NONE, pParticipantInfo->GetOipType());
}

TEST_F(ParticipantInfoTest, UpdateFromRemoteNumberUpdatesRemoteNumber)
{
    const AString strNumber = "some_number";
    const AString strToUri = "some_uri";

    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strToUri));
    ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    pParticipantInfo->UpdateFromRemoteNumber(strNumber);

    EXPECT_EQ(strNumber, pParticipantInfo->GetRemoteNumber());
}

TEST_F(ParticipantInfoTest, UpdateFromRemoteNumberUpdatesRemoteUri)
{
    const AString strNumber = "some_number";
    const AString strToUri = "some_uri";

    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strToUri));
    ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcConfigurationProxy objConfigurationProxy(new MockIMtcConfigurationManager());
    ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

    MtcSupplementaryService objSupplementaryService(objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    pParticipantInfo->UpdateFromRemoteNumber(strNumber);

    EXPECT_EQ(strToUri, pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, HandleRequestDoesNothingIfNotStartMethod)
{
    const AString strInitialRemoteNumber = pParticipantInfo->GetRemoteNumber();

    MockIMessage objMessage;
    pParticipantInfo->HandleRequest(RequestType::PRACK, objMessage);

    EXPECT_EQ(strInitialRemoteNumber, pParticipantInfo->GetRemoteNumber());
}

// TODO: HandleRequestUpdatesRemoteUri
// TODO: HandleRequestUpdatesRemoteNumber

TEST_F(ParticipantInfoTest, HandleResponseDoesNothing)
{
    const AString strInitialRemoteNumber = pParticipantInfo->GetRemoteNumber();

    ResponseType eAnyRequest = ResponseType::PROVISIONAL_RESPONSE;
    MockIMessage objMessage;
    pParticipantInfo->HandleResponse(eAnyRequest, objMessage);

    EXPECT_EQ(strInitialRemoteNumber, pParticipantInfo->GetRemoteNumber());
}
