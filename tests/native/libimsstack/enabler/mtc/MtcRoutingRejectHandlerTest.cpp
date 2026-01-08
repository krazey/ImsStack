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

#include "ImsEventDef.h"
#include "MockIMtcContext.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockINetworkWatcher.h"
#include "MockISipMessage.h"
#include "MockISipServerConnection.h"
#include "MtcRoutingRejectHandler.h"
#include "SipMethod.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::Return;
using ::testing::ReturnRef;

class MtcRoutingRejectHandlerTest : public ::testing::Test
{
public:
    MtcRoutingRejectHandler* pRoutingRejectHandler;
    MockIMtcContext objContext;
    MockINetworkWatcher objNetworkWatcher;
    MockIMtcImsEventReceiver objImsEventReceiver;
    MockMtcConfigurationProxy objConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

        ON_CALL(objContext, GetImsEventReceiver).WillByDefault(ReturnRef(objImsEventReceiver));
        pRoutingRejectHandler = new MtcRoutingRejectHandler(objContext, objNetworkWatcher);
    }

    virtual void TearDown() override { delete pRoutingRejectHandler; }
};

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestDoesNothingForNotInvite)
{
    std::vector<SipMethod> objNotInviteMethods{
            SipMethod::INVALID,
            SipMethod::ACK,
            SipMethod::BYE,
            SipMethod::CANCEL,
            SipMethod::OPTIONS,
            SipMethod::REGISTER,
            SipMethod::PRACK,
            SipMethod::SUBSCRIBE,
            SipMethod::NOTIFY,
            SipMethod::UPDATE,
            SipMethod::MESSAGE,
            SipMethod::REFER,
            SipMethod::PUBLISH,
            SipMethod::INFO,
            SipMethod::UNKNOWN,
            SipMethod::MAX,
    };

    for (SipMethod eMethod : objNotInviteMethods)
    {
        MockISipMessage objMessage;
        MockISipServerConnection objSipServerConnection;
        SipStatusCode objStatusCode;

        ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
        ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

        EXPECT_FALSE(
                pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
        EXPECT_FALSE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
                &objSipServerConnection, objStatusCode));
    }
}

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestSets488ForInviteInEhrpd)
{
    const SipMethod eMethod = SipMethod::INVITE;
    MockISipMessage objMessage;
    MockISipServerConnection objSipServerConnection;
    SipStatusCode objStatusCode;

    ON_CALL(objNetworkWatcher, GetNetRadioTechType()).WillByDefault(Return(NW_REPORT_RADIO_EHRPD));
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
    ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
    EXPECT_EQ(SipStatusCode::SC_488, objStatusCode.ToInt());
    EXPECT_STREQ("On eHRPD", objStatusCode.GetReasonPhrase().GetStr());

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
            &objSipServerConnection, objStatusCode));
    EXPECT_EQ(SipStatusCode::SC_488, objStatusCode.ToInt());
    EXPECT_STREQ("On eHRPD", objStatusCode.GetReasonPhrase().GetStr());
}

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestSetsDefaultCodeForInviteInWlanWfcOn)
{
    const SipMethod eMethod = SipMethod::INVITE;
    MockISipMessage objMessage;
    MockISipServerConnection objSipServerConnection;
    SipStatusCode objStatusCode;
    const SipStatusCode objInitialStatusCode(objStatusCode);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
    ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType()).WillByDefault(Return(NW_REPORT_RADIO_WLAN));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_ON));

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
    EXPECT_EQ(objInitialStatusCode.ToInt(), objStatusCode.ToInt());
    EXPECT_EQ(objInitialStatusCode.GetReasonPhrase(), objStatusCode.GetReasonPhrase());

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
            &objSipServerConnection, objStatusCode));
    EXPECT_EQ(objInitialStatusCode.ToInt(), objStatusCode.ToInt());
    EXPECT_EQ(objInitialStatusCode.GetReasonPhrase(), objStatusCode.GetReasonPhrase());
}

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestSets486ForInviteInWlanWfcOff)
{
    const SipMethod eMethod = SipMethod::INVITE;
    MockISipMessage objMessage;
    MockISipServerConnection objSipServerConnection;
    SipStatusCode objStatusCode;
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
    ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType()).WillByDefault(Return(NW_REPORT_RADIO_WLAN));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_WFC_SETTING_CHANGED))
            .WillByDefault(Return(IMS_WFC_OFF));
    ON_CALL(objConfigurationProxy,
            GetString(ConfigVoice::KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING))
            .WillByDefault(Return(AString("VoWiFi OFF")));

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
    EXPECT_EQ(SipStatusCode::SC_486, objStatusCode.ToInt());
    EXPECT_STREQ("VoWiFi OFF", objStatusCode.GetReasonPhrase().GetStr());

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
            &objSipServerConnection, objStatusCode));
    EXPECT_EQ(SipStatusCode::SC_486, objStatusCode.ToInt());
    EXPECT_STREQ("VoWiFi OFF", objStatusCode.GetReasonPhrase().GetStr());
}

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestSets488ForInviteInLteVolteOff)
{
    const SipMethod eMethod = SipMethod::INVITE;
    MockISipMessage objMessage;
    MockISipServerConnection objSipServerConnection;
    SipStatusCode objStatusCode;
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
    ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType()).WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_VOLTE_SETTING))
            .WillByDefault(Return(IMS_VOLTE_SETTING_OFF));

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
    EXPECT_EQ(SipStatusCode::SC_488, objStatusCode.ToInt());
    EXPECT_STREQ("VoLTE setting OFF", objStatusCode.GetReasonPhrase().GetStr());

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
            &objSipServerConnection, objStatusCode));
    EXPECT_EQ(SipStatusCode::SC_488, objStatusCode.ToInt());
    EXPECT_STREQ("VoLTE setting OFF", objStatusCode.GetReasonPhrase().GetStr());
}

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestSetsDefaultCodeForInviteInLte)
{
    const SipMethod eMethod = SipMethod::INVITE;
    MockISipMessage objMessage;
    MockISipServerConnection objSipServerConnection;
    SipStatusCode objStatusCode;
    const SipStatusCode objInitialStatusCode(objStatusCode);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
    ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType()).WillByDefault(Return(NW_REPORT_RADIO_LTE));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_VOLTE_SETTING))
            .WillByDefault(Return(IMS_VOLTE_SETTING_ON));
    ON_CALL(objImsEventReceiver, GetWParam(IMS_EVENT_IMS_VOICE_OVER_PS_STATE))
            .WillByDefault(Return(IMS_VOICE_OVER_PS_SUPPORTED));

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
    EXPECT_EQ(objInitialStatusCode.ToInt(), objStatusCode.ToInt());
    EXPECT_EQ(objInitialStatusCode.GetReasonPhrase(), objStatusCode.GetReasonPhrase());

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
            &objSipServerConnection, objStatusCode));
    EXPECT_EQ(objInitialStatusCode.ToInt(), objStatusCode.ToInt());
    EXPECT_EQ(objInitialStatusCode.GetReasonPhrase(), objStatusCode.GetReasonPhrase());
}

TEST_F(MtcRoutingRejectHandlerTest, NotifyRequestDoesntSetsCodeForInviteInInvalidRadio)
{
    const SipMethod eMethod = SipMethod::INVITE;
    MockISipMessage objMessage;
    MockISipServerConnection objSipServerConnection;
    SipStatusCode objStatusCode;
    const SipStatusCode objInitialStatusCode(objStatusCode);
    ON_CALL(objMessage, GetMethod).WillByDefault(ReturnRef(eMethod));
    ON_CALL(objSipServerConnection, GetMethod).WillByDefault(ReturnRef(eMethod));

    ON_CALL(objNetworkWatcher, GetNetRadioTechType())
            .WillByDefault(Return(NW_REPORT_RADIO_INVALID));

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(&objMessage, objStatusCode));
    EXPECT_EQ(objInitialStatusCode.ToInt(), objStatusCode.ToInt());
    EXPECT_EQ(objInitialStatusCode.GetReasonPhrase(), objStatusCode.GetReasonPhrase());

    EXPECT_TRUE(pRoutingRejectHandler->RoutingReject_NotifyRequest(
            &objSipServerConnection, objStatusCode));
    EXPECT_EQ(objInitialStatusCode.ToInt(), objStatusCode.ToInt());
    EXPECT_EQ(objInitialStatusCode.GetReasonPhrase(), objStatusCode.GetReasonPhrase());
}
