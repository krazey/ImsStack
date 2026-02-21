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

#include "ISipHeader.h"
#include "ImsList.h"
#include "MockIMessage.h"
#include "MockISession.h"
#include "MockISipClientConnection.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "MockISipServerConnection.h"
#include "SipHeaderName.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MockIMtcUiNotifier.h"
#include "ussi/MockUssiData.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiController.h"
#include "ussi/UssiDef.h"
#include "utility/MockIMessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

MATCHER_P(IsEqualSipMethod, method, "")
{
    return arg.Equals(method);
}

LOCAL const AString HEADER_APPLICATION_USSDXML(UssiConstants::HEADER_APPLICATION_USSDXML);
LOCAL const AString HEADER_USSD_PACKAGE(UssiConstants::HEADER_USSD_PACKAGE);
LOCAL const AString HEADER_MULTIPART_MIXED(UssiConstants::HEADER_MULTIPART_MIXED);
LOCAL const AString HEADER_RENDER_HANDLING(UssiConstants::HEADER_RENDER_HANDLING);
LOCAL const AString HEADER_APPLICATION_SDP(UssiConstants::HEADER_APPLICATION_SDP);
LOCAL const AString HEADER_APPLICATION_IMSXML(UssiConstants::HEADER_APPLICATION_IMSXML);
LOCAL const AString HEADER_INFO_PACKAGE(UssiConstants::HEADER_INFO_PACKAGE);

namespace android
{

MATCHER_P(IsByteArrayContains, str, "")
{
    return arg.ToString().Contains(str);
}

class UssiControllerTest : public ::testing::Test
{
public:
    inline UssiControllerTest() :
            objContext(),
            pUssiDataParser(new MockUssiDataParser()),  // deleted internally
            pUssiData(IMS_NULL),
            objAnyExtension(),
            objUiNotifier(),
            objMtcSession(),
            objSession(),
            objMessage(),
            objSipMessage(),
            objSipClientConnection(),
            objSipServerConnection(),
            objMessageUtils(),
            objMessageBodyPart(),
            objMessageBodyPartList(),
            objUssiBody("any ussi body"),
            objUssiController(objContext, pUssiDataParser)
    {
    }
    inline ~UssiControllerTest() {}

    MockIMtcCallContext objContext;
    MockUssiDataParser* pUssiDataParser;
    MockUssiData* pUssiData;
    MockAnyExtension objAnyExtension;
    MockIMtcUiNotifier objUiNotifier;
    MockIMtcSession objMtcSession;
    MockISession objSession;
    MockIMessage objMessage;
    MockISipMessage objSipMessage;
    MockISipClientConnection objSipClientConnection;
    MockISipServerConnection objSipServerConnection;
    MockIMessageUtils objMessageUtils;
    MockISipMessageBodyPart objMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objMessageBodyPartList;
    ByteArray objUssiBody;
    CallInfo objCallinfo;

    UssiController objUssiController;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetMessageUtils).WillByDefault(ReturnRef(objMessageUtils));
        ON_CALL(objContext, GetUiNotifier).WillByDefault(ReturnRef(objUiNotifier));
        ON_CALL(objContext, GetCallInfo).WillByDefault(ReturnRef(objCallinfo));
        ON_CALL(objContext, GetSession()).WillByDefault(Return(&objMtcSession));
        ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(objSession));
        ON_CALL(objSession, GetNextRequest).WillByDefault(Return(&objMessage));
        ON_CALL(objMessage, GetMessage).WillByDefault(Return(&objSipMessage));
        ON_CALL(objSipClientConnection, GetMessage).WillByDefault(Return(&objSipMessage));
        ON_CALL(objSipServerConnection, GetMessage).WillByDefault(Return(&objSipMessage));
        objMessageBodyPartList.Append(&objMessageBodyPart);
        ON_CALL(objSipMessage, GetBodyParts).WillByDefault(Return(objMessageBodyPartList));
        ON_CALL(objSipMessage, CreateBodyPart).WillByDefault(Return(&objMessageBodyPart));
        ON_CALL(objMessageBodyPart, GetContent).WillByDefault(ReturnRef(objUssiBody));
        ON_CALL(*pUssiDataParser, Parse(_)).WillByDefault(Return(nullptr));
    }

    void SetUpUssiData()
    {
        pUssiData = new MockUssiData();  // deleted internally
        ON_CALL(*pUssiDataParser, Parse(_)).WillByDefault(Return(pUssiData));
        ON_CALL(*pUssiData, GetAnyExtension).WillByDefault(ReturnRef(objAnyExtension));
    }

    void SetUpUssiData(IN AString& strUssdString, IN UssiError eError)
    {
        SetUpUssiData();
        ON_CALL(*pUssiData, GetUssdString).WillByDefault(ReturnRef(strUssdString));
        ON_CALL(*pUssiData, GetErrorCode).WillByDefault(Return(eError));
    }
};

TEST_F(UssiControllerTest, DestructorInvokesConnectionCloseIfNotNull)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    EXPECT_CALL(objSipClientConnection, Close);
}

TEST_F(UssiControllerTest, ClientConnectionNotifyResponseClosesConnectionIfReceiveFails)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    ON_CALL(objSipClientConnection, Receive).WillByDefault(Return(IMS_FAILURE));
    EXPECT_CALL(objSipClientConnection, Close);

    objUssiController.ClientConnection_NotifyResponse(
            &objSipClientConnection, &objSipClientConnection);
}

TEST_F(UssiControllerTest, ClientConnectionNotifyResponseDoesNothingProvisionalResponse)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    ON_CALL(objSipClientConnection, Receive).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objSipClientConnection, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_180));

    EXPECT_CALL(objSipClientConnection, Close).Times(0);
    objUssiController.ClientConnection_NotifyResponse(
            &objSipClientConnection, &objSipClientConnection);

    testing::Mock::VerifyAndClearExpectations(&objSipClientConnection);
}

TEST_F(UssiControllerTest, ClientConnectionNotifyResponseClosesConnectionIfFinalResponse)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    ON_CALL(objSipClientConnection, Receive).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objSipClientConnection, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objSipClientConnection, Close);
    objUssiController.ClientConnection_NotifyResponse(
            &objSipClientConnection, &objSipClientConnection);

    testing::Mock::VerifyAndClearExpectations(&objSipClientConnection);
}

TEST_F(UssiControllerTest, ClientConnectionNotifyResponseTerminatesCallIfLastActionTerminate)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    ON_CALL(objSipClientConnection, Receive).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objSipClientConnection, GetStatusCode).WillByDefault(Return(SipStatusCode::SC_200));

    EXPECT_CALL(objSipClientConnection, Close);
    MockIMtcCall objCall;
    ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objCall));
    EXPECT_CALL(objCall, Terminate(_));

    objUssiController.SetNextActionByTerminateUssi();
    objUssiController.ClientConnection_NotifyResponse(
            &objSipClientConnection, &objSipClientConnection);

    testing::Mock::VerifyAndClearExpectations(&objSipClientConnection);
}

TEST_F(UssiControllerTest, ErrorNotifyErrorClosesConnection)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    EXPECT_CALL(objSipClientConnection, Close);

    objUssiController.Error_NotifyError(&objSipClientConnection, SipStatusCode::SC_INVALID, "");

    testing::Mock::VerifyAndClearExpectations(&objSipClientConnection);
}

TEST_F(UssiControllerTest, ErrorNotifyErrorTerminatesCallIfLastActionTerminate)
{
    // Sets up objSipClientConnection.
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));
    objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE);

    EXPECT_CALL(objSipClientConnection, Close);
    MockIMtcCall objCall;
    ON_CALL(objContext, GetCall).WillByDefault(ReturnRef(objCall));
    EXPECT_CALL(objCall, Terminate(_));

    objUssiController.SetNextActionByTerminateUssi();
    objUssiController.Error_NotifyError(&objSipClientConnection, SipStatusCode::SC_INVALID, "");

    testing::Mock::VerifyAndClearExpectations(&objSipClientConnection);
}

TEST_F(UssiControllerTest, HasValidXmlReturnsFalseIfMessageIsNull)
{
    EXPECT_FALSE(objUssiController.HasValidXmlBodyForNetworkInitiatedUssi(IMS_NULL));
}

TEST_F(UssiControllerTest, HasValidXmlReturnsFalseIfSipMessageIsNull)
{
    MockIMessage objEmptyMessage;
    ON_CALL(objEmptyMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objUssiController.HasValidXmlBodyForNetworkInitiatedUssi(&objEmptyMessage));
}

TEST_F(UssiControllerTest, HasValidXmlReturnsFalseIfUssiDataIsNull)
{
    EXPECT_FALSE(objUssiController.HasValidXmlBodyForNetworkInitiatedUssi(&objMessage));
}

TEST_F(UssiControllerTest, HasValidXmlReturnsFalseIfUssiModeIsNone)
{
    SetUpUssiData();
    ON_CALL(objAnyExtension, GetUssiModeType).WillByDefault(Return(UssiModeType::NONE));
    EXPECT_FALSE(objUssiController.HasValidXmlBodyForNetworkInitiatedUssi(&objMessage));
}

TEST_F(UssiControllerTest, HasValidXmlReturnsFalseIfBodyPartIsEmpty)
{
    objMessageBodyPartList.Clear();
    ON_CALL(objSipMessage, GetBodyParts).WillByDefault(Return(objMessageBodyPartList));
    ON_CALL(objAnyExtension, GetUssiModeType).WillByDefault(Return(UssiModeType::REQUEST));
    EXPECT_FALSE(objUssiController.HasValidXmlBodyForNetworkInitiatedUssi(&objMessage));
}

TEST_F(UssiControllerTest, HasValidXmlReturnsTrueIfUssiModeIsNotNone)
{
    SetUpUssiData();
    ON_CALL(objAnyExtension, GetUssiModeType).WillByDefault(Return(UssiModeType::REQUEST));
    EXPECT_TRUE(objUssiController.HasValidXmlBodyForNetworkInitiatedUssi(&objMessage));
}

TEST_F(UssiControllerTest, IsUssiInfoReceivedReturnsFalseIfConnectionIsNull)
{
    EXPECT_FALSE(objUssiController.IsUssiInfoReceived(IMS_NULL));
}

TEST_F(UssiControllerTest, IsUssiInfoReceivedReturnsFalseIfMessageIsNull)
{
    ON_CALL(objSipServerConnection, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objUssiController.IsUssiInfoReceived(&objSipServerConnection));
}

TEST_F(UssiControllerTest, IsUssiInfoReceivedReturnsFalseIfNoPackageHeaderExist)
{
    ImsList<AString> lstHeaders;
    AString strNoPackageHeader("no package header");
    lstHeaders.Append(strNoPackageHeader);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::INFO_PACKAGE, _))
            .WillByDefault(Return(lstHeaders));
    EXPECT_FALSE(objUssiController.IsUssiInfoReceived(&objSipServerConnection));
}

TEST_F(UssiControllerTest, IsUssiInfoReceivedReturnsTrueIfPackageHeaderExists)
{
    ImsList<AString> lstHeaders;
    lstHeaders.Append(HEADER_USSD_PACKAGE);
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::INFO_PACKAGE, _))
            .WillByDefault(Return(lstHeaders));
    EXPECT_TRUE(objUssiController.IsUssiInfoReceived(&objSipServerConnection));
}

TEST_F(UssiControllerTest, HasXmlBodyInInfoReturnsFalseIfConnectionIsNull)
{
    EXPECT_FALSE(objUssiController.HasXmlBodyInInfo(IMS_NULL));
}

TEST_F(UssiControllerTest, HasXmlBodyInInfoReturnsFalseIfParsingUssiDataFails)
{
    EXPECT_FALSE(objUssiController.HasXmlBodyInInfo(&objSipServerConnection));
}

TEST_F(UssiControllerTest, HasXmlBodyInInfoReturnsTrueIfParsingUssiDataSucceeds)
{
    SetUpUssiData();
    EXPECT_TRUE(objUssiController.HasXmlBodyInInfo(&objSipServerConnection));
}

TEST_F(UssiControllerTest, HandleUssiBodyReturnsDefaultResultIfParsingUssiDataFailsAndNotBye)
{
    ON_CALL(*pUssiDataParser, Parse(_)).WillByDefault(Return(nullptr));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::INFO),
            UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest,
        HandleUssiBodyReturnsDefaultResultIfParsingUssiDataFailsAndNetworkInitiated)
{
    objCallinfo.ePeerType = PeerType::MT;
    ON_CALL(*pUssiDataParser, Parse(_)).WillByDefault(Return(nullptr));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::BYE),
            UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest,
        HandleUssiBodyNotifiesNotifyWithErrorCode1IfParsingUssiDataFailsAndUeInitiated)
{
    objCallinfo.ePeerType = PeerType::MO;
    ON_CALL(*pUssiDataParser, Parse(_)).WillByDefault(Return(nullptr));
    const IMS_UINT32 INFO_TYPE_USSI = 11;
    const AString strEmptyUssdString(AString::ConstEmpty());
    EXPECT_CALL(
            objUiNotifier, SendNotifyInfo(INFO_TYPE_USSI, _, (IMS_SINT32)UssiModeType::ERROR, _));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::BYE),
            UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, HandleUssiBodyInByeNotifiesResultWithUssiNotifyType)
{
    objCallinfo.ePeerType = PeerType::MO;
    const IMS_UINT32 INFO_TYPE_USSI = 11;
    AString strUssdString("any ussd string");
    SetUpUssiData(strUssdString, UssiError::CODE_NONE);

    EXPECT_CALL(objUiNotifier,
            SendNotifyInfo(INFO_TYPE_USSI, strUssdString, (IMS_SINT32)UssiModeType::NOTIFY, _));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::BYE),
            UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, HandleUssiBodySetsResultWithErrorCode1IfNoUssdString)
{
    objCallinfo.ePeerType = PeerType::MT;
    AString strEmptyUssdString;
    SetUpUssiData(strEmptyUssdString, UssiError::CODE_NONE);

    EXPECT_CALL(objUiNotifier, SendNotifyInfo(_, _, _, _)).Times(0);

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::INVITE),
            UssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE, UssiError::CODE_1));
}

TEST_F(UssiControllerTest, HandleUssiBodyInAckSetsResultWithErrorCode4IfAnotherCallExists)
{
    objCallinfo.ePeerType = PeerType::MO;
    AString strUssdString("any ussd string");
    SetUpUssiData(strUssdString, UssiError::CODE_NONE);

    MockIMtcCallManager objCallManager;
    ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
    ImsList<IMtcCall*> objCalls;
    objCalls.Append(IMS_NULL);
    objCalls.Append(IMS_NULL);  // to make call size 2
    ON_CALL(objCallManager, GetCalls).WillByDefault(Return(objCalls));

    EXPECT_CALL(objUiNotifier, SendNotifyInfo(_, _, _, _)).Times(0);

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::ACK),
            UssiResult(UssiNextAction::SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE, UssiError::CODE_4));
}

TEST_F(UssiControllerTest, HandleUssiBodyInInfoNotifiesResultWithUssiNotifyTypeIfMo)
{
    objCallinfo.ePeerType = PeerType::MO;
    const IMS_UINT32 INFO_TYPE_USSI = 11;
    AString strUssdString("any ussd string");
    SetUpUssiData(strUssdString, UssiError::CODE_NONE);

    EXPECT_CALL(objUiNotifier,
            SendNotifyInfo(INFO_TYPE_USSI, strUssdString, (IMS_SINT32)UssiModeType::REQUEST, _));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::INFO),
            UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, HandleUssiBodyNotifiesResultWithUssiNotifyTypeIfMtAndUssdDataIsNotify)
{
    objCallinfo.ePeerType = PeerType::MT;
    const IMS_UINT32 INFO_TYPE_USSI = 11;
    AString strUssdString("any ussd string");
    SetUpUssiData(strUssdString, UssiError::CODE_NONE);
    ON_CALL(objAnyExtension, GetUssiModeType).WillByDefault(Return(UssiModeType::NOTIFY));

    EXPECT_CALL(objUiNotifier,
            SendNotifyInfo(INFO_TYPE_USSI, strUssdString, (IMS_SINT32)UssiModeType::NOTIFY, _));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::INVITE),
            UssiResult(UssiNextAction::SEND_INFO_WITH_NOTIFY_ELEMENT, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, HandleUssiBodyNotifiesUssiErrorIfUssiDataIsError)
{
    objCallinfo.ePeerType = PeerType::MT;
    const IMS_UINT32 INFO_TYPE_USSI = 11;
    AString strUssdString("any ussd string");
    SetUpUssiData(strUssdString, UssiError::CODE_1);
    ON_CALL(objAnyExtension, GetUssiModeType).WillByDefault(Return(UssiModeType::NOTIFY));

    EXPECT_CALL(objUiNotifier,
            SendNotifyInfo(INFO_TYPE_USSI, strUssdString, (IMS_SINT32)UssiModeType::ERROR, _));

    EXPECT_EQ(objUssiController.HandleUssiBody(&objSipMessage, SipMethod::BYE),
            UssiResult(UssiNextAction::NOTHING, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsRecvInfo)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsAcceptHeaderAndApplicationSdpHeaderFails)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_SDP, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsAcceptHeaderAndApplicationImsxmlHeaderFails)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_SDP, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_IMSXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsAcceptHeaderAndApplicationUssdxmlHeaderFails)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_SDP, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_IMSXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_USSDXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsAcceptHeaderAndMultipartMixedHeaderFails)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_SDP, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_IMSXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_USSDXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_MULTIPART_MIXED, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsContentType)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_SDP, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_IMSXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_USSDXML, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_MULTIPART_MIXED, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_MULTIPART_MIXED, ISipHeader::CONTENT_TYPE, _))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsBodyPartAndFailsIfSipMessageIsNull)
{
    // to pass FormHeadersForStartUssi()
    ON_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(objMessage, GetMessage).WillByDefault(Return(nullptr));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsBodyPartAndFailsIfCreateBodyPartFails)
{
    // to pass FormHeadersForStartUssi()
    ON_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(objSipMessage, CreateBodyPart).WillByDefault(Return(nullptr));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_EQ(IMS_FAILURE, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormStartUssiRequestSetsBodyPart)
{
    // to pass FormHeadersForStartUssi()
    ON_CALL(objMessageUtils, AddValueIfNotExists(_, _, _, _)).WillByDefault(Return(IMS_SUCCESS));

    EXPECT_CALL(objMessageBodyPart,
            SetHeader(ISipMessageBodyPart::CONTENT_TYPE, HEADER_APPLICATION_USSDXML, _));
    EXPECT_CALL(objMessageBodyPart,
            SetHeader(ISipMessageBodyPart::CONTENT_DISPOSITION, HEADER_RENDER_HANDLING, _));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_CALL(objMessageBodyPart, SetContent(IsByteArrayContains(strAnyUssdString)));

    EXPECT_EQ(IMS_SUCCESS, objUssiController.FormStartUssiRequest(strAnyUssdString));
}

TEST_F(UssiControllerTest, FormAcceptUssiSetsRecvInfo)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_EQ(IMS_FAILURE, objUssiController.FormAcceptUssi());
}

TEST_F(UssiControllerTest, FormAcceptUssiSetsAcceptHeaderAndFails)
{
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_APPLICATION_SDP, ISipHeader::ACCEPT, _))
            .WillOnce(Return(IMS_FAILURE));

    EXPECT_EQ(IMS_FAILURE, objUssiController.FormAcceptUssi());
}

TEST_F(UssiControllerTest, FormAcceptUssiSetsAcceptHeader)
{
    ON_CALL(objMessageUtils,
            AddValueIfNotExists(&objMessage, HEADER_USSD_PACKAGE, ISipHeader::RECV_INFO, _))
            .WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objMessageUtils, AddValueIfNotExists(&objMessage, _, ISipHeader::ACCEPT, _))
            .WillByDefault(Return(IMS_SUCCESS));

    EXPECT_EQ(IMS_SUCCESS, objUssiController.FormAcceptUssi());
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfConnectionIsNull)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(IMS_NULL));

    EXPECT_EQ(IMS_FAILURE, objUssiController.SendInfo(objSession, "", UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfSettingInfoPackageFails)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    EXPECT_CALL(objSipClientConnection, SetListener(&objUssiController));
    EXPECT_CALL(objSipClientConnection, SetErrorListener(&objUssiController));

    EXPECT_CALL(objSipClientConnection, SetHeader(HEADER_INFO_PACKAGE, HEADER_USSD_PACKAGE))
            .WillOnce(Return(IMS_FAILURE));
    const AString strAnyUssdString("anyUssdString");

    EXPECT_EQ(IMS_FAILURE,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfSettingApplicationUssdxmlFails)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    EXPECT_CALL(objSipClientConnection, SetHeader(HEADER_INFO_PACKAGE, HEADER_USSD_PACKAGE))
            .WillOnce(Return(IMS_SUCCESS));
    const AString strContentType(SipHeaderName::CONTENT_TYPE);
    EXPECT_CALL(objSipClientConnection, SetHeader(strContentType, HEADER_APPLICATION_USSDXML))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");

    EXPECT_EQ(IMS_FAILURE,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfSettingContentDispositionFails)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    EXPECT_CALL(objSipClientConnection, SetHeader(HEADER_INFO_PACKAGE, HEADER_USSD_PACKAGE))
            .WillOnce(Return(IMS_SUCCESS));
    const AString strContentType(SipHeaderName::CONTENT_TYPE);
    EXPECT_CALL(objSipClientConnection, SetHeader(strContentType, HEADER_APPLICATION_USSDXML))
            .WillOnce(Return(IMS_FAILURE));

    const AString strAnyUssdString("anyUssdString");
    EXPECT_CALL(objSipClientConnection, Close);

    EXPECT_EQ(IMS_FAILURE,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfSipMessageIsNull)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    ON_CALL(objSipClientConnection, SetHeader(_, _)).WillByDefault(Return(IMS_SUCCESS));
    ON_CALL(objSipClientConnection, GetMessage).WillByDefault(Return(nullptr));

    const AString strAnyUssdString("anyUssdString");

    EXPECT_CALL(objSipClientConnection, Close);
    EXPECT_EQ(IMS_FAILURE,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfCreateBodyPartFails)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    ON_CALL(objSipMessage, CreateBodyPart).WillByDefault(Return(nullptr));

    const AString strAnyUssdString("anyUssdString");

    EXPECT_CALL(objSipClientConnection, Close);
    EXPECT_EQ(IMS_FAILURE,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsFailureIfSendFails)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    EXPECT_CALL(objSipClientConnection, SetListener(&objUssiController));
    EXPECT_CALL(objSipClientConnection, SetErrorListener(&objUssiController));

    const AString strAnyUssdString("anyUssdString");

    EXPECT_CALL(objSipClientConnection, Send).WillOnce(Return(IMS_FAILURE));
    EXPECT_CALL(objSipClientConnection, Close);

    EXPECT_EQ(IMS_FAILURE,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

TEST_F(UssiControllerTest, SendInfoReturnsSuccess)
{
    MockISession objSession;
    SipMethod objMethod(SipMethod::INFO);
    ON_CALL(objSession, CreateTransaction(IsEqualSipMethod(objMethod)))
            .WillByDefault(Return(&objSipClientConnection));

    EXPECT_CALL(objSipClientConnection, SetListener(&objUssiController));
    EXPECT_CALL(objSipClientConnection, SetErrorListener(&objUssiController));

    const AString strAnyUssdString("anyUssdString");

    // to cover for statement in GetParsedUssiData
    MockISipMessageBodyPart objEmptyBodyPart;
    objMessageBodyPartList.Append(&objEmptyBodyPart);
    ON_CALL(objSipMessage, GetBodyParts).WillByDefault(Return(objMessageBodyPartList));

    EXPECT_CALL(objMessageBodyPart, SetContent(IsByteArrayContains(strAnyUssdString)));
    EXPECT_CALL(objSipClientConnection, Send).WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(IMS_SUCCESS,
            objUssiController.SendInfo(objSession, strAnyUssdString, UssiError::CODE_NONE));
}

}  // namespace android
