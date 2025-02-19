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
#include "IImsAosInfo.h"
#include "ISipHeader.h"
#include "ImsTypeDef.h"
#include "MockICoreService.h"
#include "MockIMessage.h"
#include "MockIMtcService.h"
#include "MockISession.h"
#include "SipAddress.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/ParticipantInfo.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "dialingplan/MockIMtcDialingPlan.h"
#include "helper/MockIMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class ParticipantInfoTest : public ::testing::Test
{
public:
    MockIMtcCallContext objContext;
    MockIMessageUtils objMessageUtils;
    MockMtcConfigurationProxy objConfigurationProxy;
    ParticipantInfo* pParticipantInfo;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        pParticipantInfo = new ParticipantInfo(objContext);
    }

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

TEST_F(ParticipantInfoTest, GetLocalUriReturnsNullIfEmergency)
{
    MockIMtcAosConnector objAosConnector;
    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objService, GetAosConnector).WillByDefault(Return(&objAosConnector));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NORMAL));
    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_ADMIN));
    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_INTERNAL));
    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());

    ON_CALL(objAosConnector, GetRegistrationMode)
            .WillByDefault(Return(IImsAosInfo::REG_MODE_NOUICC));
    EXPECT_TRUE(pParticipantInfo->GetLocalUri().IsNull());
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

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    objSupplementaryService.Add(SuppType::TARGET_URI, strUri);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(strUri, pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, GetRemoteUriReturnsInitialRemoteUri)
{
    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(AString::ConstNull(), pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, GetRemoteUriReturnsLocalUriIfCallPullIsEnabled)
{
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
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

TEST_F(ParticipantInfoTest, GetRemoteUriReturnsNullIfCallPullIsEnabledAndCoreServiceIsNull)
{
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));
    objSupplementaryService.Add(SuppType::CALL_PULL, IMS_FALSE);

    MockIMtcService objService;
    ON_CALL(objService, GetICoreService).WillByDefault(Return(nullptr));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    EXPECT_EQ(pParticipantInfo->GetRemoteUri(), AString::ConstNull());
}

TEST_F(ParticipantInfoTest, GetRemoteDisplayNameReturnsFromSupplementaryService)
{
    const AString strCnap("some_cnap");

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    objSupplementaryService.Add(SuppType::CNAP, strCnap);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(strCnap, pParticipantInfo->GetRemoteDisplayName());
}

TEST_F(ParticipantInfoTest, GetRemoteDisplayNameReturnsInitialRemoteNumber)
{
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(AString::ConstNull(), pParticipantInfo->GetRemoteDisplayName());
}

TEST_F(ParticipantInfoTest, GetOipTypeReturnsFromSupplementaryService)
{
    const OipType eOipType = OipType::IDENTITY;

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    objSupplementaryService.Add(SuppType::CALLER_ID, static_cast<IMS_SINT32>(eOipType));
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(eOipType, pParticipantInfo->GetOipType());
}

TEST_F(ParticipantInfoTest, GetOipTypeReturnsNone)
{
    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    EXPECT_EQ(OipType::NONE, pParticipantInfo->GetOipType());
}

TEST_F(ParticipantInfoTest, UpdateFromRemoteNumberUpdatesRemoteNumber)
{
    const AString strNumber = "some_number";
    const AString strToUri = "some_uri";

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strToUri));
    ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    pParticipantInfo->UpdateFromRemoteNumber(strNumber);

    EXPECT_EQ(strNumber, pParticipantInfo->GetRemoteNumber());
}

TEST_F(ParticipantInfoTest, UpdateFromRemoteNumberUpdatesRemoteUri)
{
    const AString strNumber = "some_number";
    const AString strToUri = "some_uri";

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strToUri));
    ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    pParticipantInfo->UpdateFromRemoteNumber(strNumber);

    EXPECT_EQ(strToUri, pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, UpdateFromRemoteNumberUpdatesRemoteUriIfIsEmergency)
{
    const AString strNumber = "some_number";
    const AString strToUri = "sip_uri";

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    MockIMtcDialingPlan objDialingPlan;
    ON_CALL(objDialingPlan, GetToUri(_, _, _)).WillByDefault(Return(strToUri));
    ON_CALL(objContext, GetDialingPlan).WillByDefault(ReturnRef(objDialingPlan));

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    pParticipantInfo->UpdateFromRemoteNumber(strNumber);

    EXPECT_EQ(strToUri, pParticipantInfo->GetRemoteUri());
}

TEST_F(ParticipantInfoTest, UpdateFromRemoteNumberDoesNotThingIfIsEmergencyAndTargetUriIsSet)
{
    const AString strNumber = "some_number";
    const AString strUri = "some_uri";

    MtcSupplementaryService objSupplementaryService(objContext, objConfigurationProxy);
    objSupplementaryService.Add(SuppType::TARGET_URI, strUri);
    ON_CALL(objContext, GetSupplementaryService).WillByDefault(ReturnRef(objSupplementaryService));

    MockIMtcService objService;
    ON_CALL(objService, IsEmergency).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objContext, GetService).WillByDefault(ReturnRef(objService));

    CallInfo objCallInfo;
    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    pParticipantInfo->UpdateFromRemoteNumber(strNumber);

    EXPECT_EQ(strNumber, pParticipantInfo->GetRemoteNumber());
}

TEST_F(ParticipantInfoTest, HandleRequestDoesNothingIfNotStartMethod)
{
    const AString strInitialRemoteNumber = pParticipantInfo->GetRemoteNumber();

    MockIMessage objMessage;
    pParticipantInfo->HandleRequest(RequestType::PRACK, objMessage);

    EXPECT_EQ(strInitialRemoteNumber, pParticipantInfo->GetRemoteNumber());
}

// TODO: HandleRequestUpdatesRemoteUri

TEST_F(ParticipantInfoTest, HandleRequestUpdatesRemoteNumber)
{
    const AString strInitialRemoteNumber = pParticipantInfo->GetRemoteNumber();
    MockIMessage objMessage;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    const AString strRemoteUri("some uri");
    ON_CALL(objMessageUtils, GetRemoteUri(_, _)).WillByDefault(Return(strRemoteUri));
    ON_CALL(objConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    const AString strRemoteNumber("some number");
    ON_CALL(objMessageUtils, GetUserPart(&objMessage, ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(strRemoteNumber));
    const AString strEmptySet;
    ON_CALL(objConfigurationProxy, GetString(ConfigVoice::KEY_LOCAL_NUMBER_PRESENTATION_SET_STRING))
            .WillByDefault(Return(strEmptySet));

    pParticipantInfo->HandleRequest(RequestType::START, objMessage);

    EXPECT_EQ(strRemoteNumber, pParticipantInfo->GetRemoteNumber());
}

TEST_F(ParticipantInfoTest, HandleRequestUpdatesRemoteNumberAndConvertPrefixIfConfigured)
{
    const AString strInitialRemoteNumber = pParticipantInfo->GetRemoteNumber();
    MockIMessage objMessage;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));

    const AString strRemoteUri("some uri");
    ON_CALL(objMessageUtils, GetRemoteUri(_, _)).WillByDefault(Return(strRemoteUri));
    ON_CALL(objConfigurationProxy, GetBoolean(ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    const AString strRemoteNumberAsGlobal("globalPrefixAndNumber");
    const AString strRemoteNumberAsLocal("localPrefixAndNumber");
    ON_CALL(objMessageUtils, GetUserPart(&objMessage, ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(strRemoteNumberAsGlobal));
    const AString strTestSet("globalPrefix:localPrefix");
    ON_CALL(objConfigurationProxy, GetString(ConfigVoice::KEY_LOCAL_NUMBER_PRESENTATION_SET_STRING))
            .WillByDefault(Return(strTestSet));

    pParticipantInfo->HandleRequest(RequestType::START, objMessage);

    EXPECT_EQ(strRemoteNumberAsLocal, pParticipantInfo->GetRemoteNumber());
}

TEST_F(ParticipantInfoTest, HandleResponseDoesNothing)
{
    const AString strInitialRemoteNumber = pParticipantInfo->GetRemoteNumber();

    ResponseType eAnyRequest = ResponseType::PROVISIONAL_RESPONSE;
    MockIMessage objMessage;
    pParticipantInfo->HandleResponse(eAnyRequest, objMessage);

    EXPECT_EQ(strInitialRemoteNumber, pParticipantInfo->GetRemoteNumber());
}
