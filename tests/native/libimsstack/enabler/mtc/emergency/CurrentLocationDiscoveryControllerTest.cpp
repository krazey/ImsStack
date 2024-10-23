/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "ByteArray.h"
#include "CarrierConfig.h"
#include "ImsList.h"
#include "MockIMtcContext.h"
#include "MockIPublication.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "MockISipServerConnection.h"
#include "RetryTaskHelper.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcSession.h"
#include "call/state/MockIMtcCallState.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/CurrentLocationDiscoveryController.h"
#include "utility/MessageUtil.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

LOCAL const AString STR_REQUEST_FOR_LOCATION_INFORMATION("requestForLocationInformation");

class CurrentLocationDiscoveryControllerTest : public ::testing::Test
{
protected:
    MockMtcConfigurationProxy* pConfigurationProxy;
    MockIMtcCallContext objContext;
    MockIMtcSession objMtcSession;
    MockISession objISession;
    MockIPublication objIPublication;
    MockISipServerConnection objSipServerConnection;
    MockISipMessage objISipMessage;
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    ByteArray objContent;
    CallInfo objCallInfo;
    ImsList<AString> lstHeaders;

    virtual void SetUp() override
    {
        pConfigurationProxy = new MockMtcConfigurationProxy();

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(*pConfigurationProxy));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objISession));
        ON_CALL(objISession, CreatePublication).WillByDefault(Return(&objIPublication));
        ON_CALL(objSipServerConnection, GetMessage).WillByDefault(Return(&objISipMessage));
        ON_CALL(*pConfigurationProxy,
                GetBoolean(ConfigEmergency::
                                KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL))
                .WillByDefault(Return(IMS_TRUE));

        objBodyParts.Append(&objISipMessageBodyPart);
        ON_CALL(objISipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

        objContent = ByteArray(STR_REQUEST_FOR_LOCATION_INFORMATION);
        ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

        objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

        lstHeaders.Append(MessageUtil::STR_PACKAGE_CURRENT_LOCATION_DISCOVERY);
        ON_CALL(objISipMessage, GetHeaders).WillByDefault(Return(lstHeaders));
    }

    virtual void TearDown() override
    {
        delete pConfigurationProxy;
    }
};

TEST_F(CurrentLocationDiscoveryControllerTest, ISipMessageIsNull)
{
    ON_CALL(objSipServerConnection, GetMessage).WillByDefault(ReturnNull());

    EXPECT_EQ(IMS_FALSE, CurrentLocationDiscoveryController::IsCurrentLocationDiscoveryInfoReceived(
            objSipServerConnection));
}

TEST_F(CurrentLocationDiscoveryControllerTest, OtherInfoReceived)
{
    ImsList<AString> lstEmptyHeaders;
    ON_CALL(objISipMessage, GetHeaders).WillByDefault(Return(lstEmptyHeaders));

    EXPECT_EQ(IMS_FALSE, CurrentLocationDiscoveryController::IsCurrentLocationDiscoveryInfoReceived(
            objSipServerConnection));
}

TEST_F(CurrentLocationDiscoveryControllerTest, CurrentLocationDiscoveryInfoReceived)
{
    EXPECT_EQ(IMS_TRUE, CurrentLocationDiscoveryController::IsCurrentLocationDiscoveryInfoReceived(
            objSipServerConnection));
}

TEST_F(CurrentLocationDiscoveryControllerTest, Send469WhenFeatureNotSupported)
{
    CurrentLocationDiscoveryController objController(objContext);

    ON_CALL(*pConfigurationProxy,
            GetBoolean(
                    ConfigEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL))
            .WillByDefault(Return(IMS_FALSE));
    EXPECT_CALL(objSipServerConnection, InitResponse(SipStatusCode::SC_469)).Times(1);

    objController.OnCurrentLocationDiscoveryInfoReceived(objSipServerConnection);
}

TEST_F(CurrentLocationDiscoveryControllerTest, Send469ForNonEmergencyCall)
{
    CurrentLocationDiscoveryController objController(objContext);

    objCallInfo.eEmergencyType = EmergencyType::NONE;
    EXPECT_CALL(objSipServerConnection, InitResponse(SipStatusCode::SC_469)).Times(1);

    objController.OnCurrentLocationDiscoveryInfoReceived(objSipServerConnection);

    ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));
}

TEST_F(CurrentLocationDiscoveryControllerTest, Send200)
{
    CurrentLocationDiscoveryController objController(objContext);

    EXPECT_CALL(objSipServerConnection, InitResponse(SipStatusCode::SC_200)).Times(1);

    objController.OnCurrentLocationDiscoveryInfoReceived(objSipServerConnection);
}

TEST_F(CurrentLocationDiscoveryControllerTest, DoesNotSendPublish)
{
    CurrentLocationDiscoveryController objController(objContext);

    ImsList<ISipMessageBodyPart*> objEmptyBodyParts;
    ON_CALL(objISipMessage, GetBodyParts).WillByDefault(Return(objEmptyBodyParts));

    EXPECT_CALL(objIPublication, Publish(_, _)).Times(0);

    objController.OnCurrentLocationDiscoveryInfoReceived(objSipServerConnection);
}

TEST_F(CurrentLocationDiscoveryControllerTest, SendPublish)
{
    CurrentLocationDiscoveryController objController(objContext);

    EXPECT_CALL(objIPublication, Publish(_, _)).Times(1);

    objController.OnCurrentLocationDiscoveryInfoReceived(objSipServerConnection);
}

TEST_F(CurrentLocationDiscoveryControllerTest, StartPeriodicLocationDiscoveryStartsRetryTaskHelper)
{
    CurrentLocationDiscoveryController objController(objContext);
    EXPECT_CALL(*pConfigurationProxy,
            GetInt(ConfigEmergency::KEY_CALL_PERIODIC_LOCATION_DISCOVERY_TIMER_MILLIS_INT))
            .Times(1)
            .WillRepeatedly(Return(30));

    objController.StartPeriodicLocationDiscovery();
    objController.StartPeriodicLocationDiscovery();
}

TEST_F(CurrentLocationDiscoveryControllerTest, ExecuteCmdSendsLocationUpdate)
{
    CurrentLocationDiscoveryController objController(objContext);

    EXPECT_CALL(objMtcSession, Update(UpdateType::LOCATION, IMS_FALSE, SipMethod::UPDATE)).Times(1);

    objController.ExecuteCmd();
}
