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

#include "CallReasonInfo.h"
#include "ImsList.h"
#include "core/IReference.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/ISipMessageBodyPart.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/SipHeaderName.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL IMS_SINT32 ANY_METHOD = IMessage::SESSION_START;
LOCAL IMS_SINT32 ANY_HEADER = ISipHeader::SUPPORTED;

namespace android
{

class MessageUtilsTest : public ::testing::Test
{
public:
    MockIMessage* piMessage;
    MockISipMessage* piSipMessage;
    MockISession* piSession;
    ImsList<IMessage*> objMessages;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        piMessage = new MockIMessage();
        piSipMessage = new MockISipMessage();
        piSession = new MockISession();

        ON_CALL(*piSession, GetNextRequest).WillByDefault(Return(piMessage));
        ON_CALL(*piSession, GetNextResponse).WillByDefault(Return(piMessage));

        ON_CALL(*piMessage, GetMessage).WillByDefault(Return(piSipMessage));
    }

    virtual void TearDown() override
    {
        delete piMessage;
        delete piSipMessage;
        delete piSession;

        objMessages.Clear();
    }

public:
    void SetUpPreviousRequest(IN IMS_SINT32 eServiceMethod = IMessage::SESSION_START)
    {
        ON_CALL(*piSession, GetPreviousRequest(eServiceMethod)).WillByDefault(Return(piMessage));
    }

    void SetUpPreviousResponse(IN IMS_SINT32 eServiceMethod = IMessage::SESSION_START)
    {
        ON_CALL(*piSession, GetPreviousResponse(eServiceMethod)).WillByDefault(Return(piMessage));
    }

    void SetUpPreviousResponses(IN IMS_SINT32 eServiceMethod = IMessage::SESSION_START)
    {
        objMessages.Append(piMessage);
        ON_CALL(*piSession, GetPreviousResponses(eServiceMethod))
                .WillByDefault(Return(objMessages));
    }
};

TEST_F(MessageUtilsTest, GetPreviousResponse)
{
    EXPECT_TRUE(objMessageUtils.GetPreviousResponse(IMS_NULL, ANY_METHOD) == IMS_NULL);

    SetUpPreviousResponses();
    EXPECT_EQ(objMessageUtils.GetPreviousResponse(piSession, ANY_METHOD), piMessage);

    EXPECT_TRUE(objMessageUtils.GetPreviousResponse(piSession, ANY_METHOD, 10) == IMS_NULL);
}

TEST_F(MessageUtilsTest, GetRemotePreviousMessage)
{
    EXPECT_TRUE(
            objMessageUtils.GetRemotePreviousMessage(IMS_NULL, ANY_METHOD, IMS_FALSE) == IMS_NULL);

    SetUpPreviousResponses();
    EXPECT_EQ(objMessageUtils.GetRemotePreviousMessage(piSession, ANY_METHOD, IMS_TRUE), piMessage);

    SetUpPreviousRequest();
    EXPECT_EQ(
            objMessageUtils.GetRemotePreviousMessage(piSession, ANY_METHOD, IMS_FALSE), piMessage);
}

TEST_F(MessageUtilsTest, GetResponseStatusCode)
{
    EXPECT_EQ(
            objMessageUtils.GetResponseStatusCode(IMS_NULL, ANY_METHOD), SipStatusCode::SC_INVALID);

    IMS_SINT32 nAnyCode = SipStatusCode::SC_200;
    SetUpPreviousResponses();
    ON_CALL(*piMessage, GetStatusCode).WillByDefault(Return(nAnyCode));
    EXPECT_EQ(objMessageUtils.GetResponseStatusCode(piSession, ANY_METHOD), nAnyCode);
}

TEST_F(MessageUtilsTest, GetRemoteUris)
{
    ImsList<AString> objUris = objMessageUtils.GetRemoteUris(IMS_NULL, PeerType::MO);
    EXPECT_EQ(objUris.GetSize(), 0);

    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));

    AString strAnyUri = "sip:anyHeader";
    ImsList<AString> objHeaders;
    objHeaders.Append(strAnyUri);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));

    SetUpPreviousRequest(IMessage::SESSION_START);

    objUris = objMessageUtils.GetRemoteUris(piSession, PeerType::MO);
    EXPECT_EQ(objUris.GetSize(), 1);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), strAnyUri.GetStr());
}

TEST_F(MessageUtilsTest, GetRemoteUri)
{
    AString strRemoteUri = objMessageUtils.GetRemoteUri(IMS_NULL, PeerType::MO);
    EXPECT_STREQ(strRemoteUri.GetStr(), "");

    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));

    AString strAnyUri = "sip:anyHeader";
    ImsList<AString> objHeaders;
    objHeaders.Append(strAnyUri);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));

    SetUpPreviousRequest(IMessage::SESSION_START);

    strRemoteUri = objMessageUtils.GetRemoteUri(piSession, PeerType::MO);
    EXPECT_STREQ(strRemoteUri.GetStr(), strAnyUri.GetStr());
}

TEST_F(MessageUtilsTest, GetSessionId)
{
    AString strSessionId = objMessageUtils.GetSessionId(IMS_NULL);
    EXPECT_STREQ(strSessionId.GetStr(), "");

    AString strAnySessionId;
    ON_CALL(*piSession, GetSessionId).WillByDefault(ReturnRef(strAnySessionId));

    strSessionId = objMessageUtils.GetSessionId(piSession);
    EXPECT_STREQ(strSessionId.GetStr(), "");

    strAnySessionId = "123456789";
    strSessionId = objMessageUtils.GetSessionId(piSession);
    EXPECT_STREQ(strSessionId.GetStr(), strAnySessionId.GetStr());
}

TEST_F(MessageUtilsTest, GetHeaders)
{
    ImsList<AString> objOutHeaders;
    ImsList<AString> objHeaders;
    objHeaders.Append("sip:anyHeader");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    objOutHeaders = objMessageUtils.GetHeaders(piMessage, ANY_HEADER);
    EXPECT_EQ(objOutHeaders.GetSize(), 1);
    EXPECT_STREQ(objOutHeaders.GetAt(0).GetStr(), objHeaders.GetAt(0).GetStr());

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objOutHeaders = objMessageUtils.GetHeaders(piMessage, ANY_HEADER);
    EXPECT_EQ(objOutHeaders.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetHeader)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("sip:anyHeader");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strHeader = objMessageUtils.GetHeader(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHeader.GetStr(), objHeaders.GetAt(0).GetStr());

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strHeader = objMessageUtils.GetHeader(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHeader.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetHeaderValue)
{
    AString strValue;
    ImsList<AString> objHeaders;
    objHeaders.Append("anyValue");  // TODO: check
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strValue = objMessageUtils.GetHeaderValue(piMessage, ANY_HEADER);
    EXPECT_STREQ(strValue.GetStr(), objHeaders.GetAt(0).GetStr());

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strValue = objMessageUtils.GetHeaderValue(piMessage, ANY_HEADER);
    EXPECT_STREQ(strValue.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetHeaderValueInt)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("3600");  // TODO: check
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::EXPIRES_SEC, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), 3600);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), -1);
}

TEST_F(MessageUtilsTest, GetParameterValue)
{
    AString strParameterValue;
    AString strAnyParamName = "expires";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:12345>;expires=3600");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strParameterValue = objMessageUtils.GetParameterValue(piMessage, strAnyParamName, ANY_HEADER);
    EXPECT_STREQ(strParameterValue.GetStr(), "3600");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strParameterValue = objMessageUtils.GetParameterValue(piMessage, strAnyParamName, ANY_HEADER);
    EXPECT_STREQ(strParameterValue.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetUserParts)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    ImsList<AString> objUserParts = objMessageUtils.GetUserParts(piMessage, ANY_HEADER);
    EXPECT_EQ(objUserParts.GetSize(), 2);
    EXPECT_STREQ(objUserParts.GetAt(0).GetStr(), "anyname1");
    EXPECT_STREQ(objUserParts.GetAt(1).GetStr(), "12345");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objUserParts = objMessageUtils.GetUserParts(piMessage, ANY_HEADER);
    EXPECT_EQ(objUserParts.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetUserPart)
{
    AString strUserPart;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strUserPart = objMessageUtils.GetUserPart(piMessage, ANY_HEADER);
    EXPECT_STREQ(strUserPart.GetStr(), "anyname1");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strUserPart = objMessageUtils.GetUserPart(piMessage, ANY_HEADER);
    EXPECT_STREQ(strUserPart.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetUserIds)
{
    ImsList<AString> objUserIds;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    objUserIds = objMessageUtils.GetUserIds(piMessage, ANY_HEADER);
    EXPECT_EQ(objUserIds.GetSize(), 2);
    EXPECT_STREQ(objUserIds.GetAt(0).GetStr(), "anyname1");
    EXPECT_STREQ(objUserIds.GetAt(1).GetStr(), "12345");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objUserIds = objMessageUtils.GetUserIds(piMessage, ANY_HEADER);
    EXPECT_EQ(objUserIds.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetUserId)
{
    AString strUserId;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strUserId = objMessageUtils.GetUserId(piMessage, ANY_HEADER);
    EXPECT_STREQ(strUserId.GetStr(), "anyname1");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strUserId = objMessageUtils.GetUserId(piMessage, ANY_HEADER);
    EXPECT_STREQ(strUserId.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetDisplayNames)
{
    ImsList<AString> objDisplayNames;
    ImsList<AString> objHeaders;
    objHeaders.Append("\"any display name1\" <sip:anyname1@ims.google.com>");
    objHeaders.Append("\"any display name2\" <tel:12345>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    objDisplayNames = objMessageUtils.GetDisplayNames(piMessage, ANY_HEADER);
    EXPECT_EQ(objDisplayNames.GetSize(), 2);
    EXPECT_STREQ(objDisplayNames.GetAt(0).GetStr(), "any display name1");
    EXPECT_STREQ(objDisplayNames.GetAt(1).GetStr(), "any display name2");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objDisplayNames = objMessageUtils.GetDisplayNames(piMessage, ANY_HEADER);
    EXPECT_EQ(objDisplayNames.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetDisplayName)
{
    AString strDisplayName;
    ImsList<AString> objHeaders;
    objHeaders.Append("\"any display name\" <sip:anyname1@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strDisplayName = objMessageUtils.GetDisplayName(piMessage, ANY_HEADER);
    EXPECT_STREQ(strDisplayName.GetStr(), "any display name");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strDisplayName = objMessageUtils.GetDisplayName(piMessage, ANY_HEADER);
    EXPECT_STREQ(strDisplayName.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetHosts)
{
    ImsList<AString> objHosts;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    objHosts = objMessageUtils.GetHosts(piMessage, ANY_HEADER);
    EXPECT_EQ(objHosts.GetSize(), 2);
    EXPECT_STREQ(objHosts.GetAt(0).GetStr(), "ims.google.com");
    EXPECT_STREQ(objHosts.GetAt(1).GetStr(), "12345");  // TODO: check

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objHosts = objMessageUtils.GetHosts(piMessage, ANY_HEADER);
    EXPECT_EQ(objHosts.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetHost)
{
    AString strHost;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strHost = objMessageUtils.GetHost(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHost.GetStr(), "ims.google.com");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strHost = objMessageUtils.GetHost(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHost.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetParameterValueFromUri)
{
    AString strParamValue;
    AString strAnyParamName = "userparam";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname;userparam=userparamvalue@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strParamValue =
            objMessageUtils.GetParameterValueFromUri(piMessage, strAnyParamName, ANY_HEADER);
    EXPECT_STREQ(strParamValue.GetStr(), "userparamvalue");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strParamValue =
            objMessageUtils.GetParameterValueFromUri(piMessage, strAnyParamName, ANY_HEADER);
    EXPECT_STREQ(strParamValue.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetUris)
{
    ImsList<AString> objUris;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1@ims.google.com;anyuriparam>");
    objHeaders.Append("<tel:12345;anyuriparam>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    objUris = objMessageUtils.GetUris(piMessage, IMS_TRUE, ANY_HEADER);
    EXPECT_EQ(objUris.GetSize(), 2);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), "<sip:anyname1@ims.google.com;anyuriparam>");
    EXPECT_STREQ(objUris.GetAt(1).GetStr(), "<tel:12345;anyuriparam>");

    objUris = objMessageUtils.GetUris(piMessage, IMS_FALSE, ANY_HEADER);
    EXPECT_EQ(objUris.GetSize(), 2);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), "sip:anyname1@ims.google.com");
    EXPECT_STREQ(objUris.GetAt(1).GetStr(), "tel:12345");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objUris = objMessageUtils.GetUris(piMessage, IMS_FALSE, ANY_HEADER);
    EXPECT_EQ(objUris.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetUri)
{
    AString strUri;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1@ims.google.com;anyuriparam>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strUri = objMessageUtils.GetUri(piMessage, IMS_TRUE, ANY_HEADER);
    EXPECT_STREQ(strUri.GetStr(), "<sip:anyname1@ims.google.com;anyuriparam>");

    strUri = objMessageUtils.GetUri(piMessage, IMS_FALSE, ANY_HEADER);
    EXPECT_STREQ(strUri.GetStr(), "sip:anyname1@ims.google.com");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strUri = objMessageUtils.GetUri(piMessage, IMS_FALSE, ANY_HEADER);
    EXPECT_STREQ(strUri.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetFeatures)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("timer");
    objHeaders.Append("100rel");
    objHeaders.Append("precondition");
    IMS_SINT32 nFeature = FEATURE_TIMER | FEATURE_100REL | FEATURE_PRECONDITION;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::SUPPORTED, _)).WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetFeatures(piMessage, ISipHeader::SUPPORTED), nFeature);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetFeatures(piMessage, ISipHeader::SUPPORTED), FEATURE_NONE);
}

TEST_F(MessageUtilsTest, GetSosTypeFromServiceUrn)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_INVALID);

    objHeaders.Append("<urn:service:sos>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);

    objHeaders.SetAt("<urn:service:sos.ambulance>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE);

    objHeaders.SetAt("<urn:service:sos.animal-control>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_ANIMAL_CONTROL);

    objHeaders.SetAt("<urn:service:sos.fire>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_FIRE);

    objHeaders.SetAt("<urn:service:sos.gas>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GAS);

    objHeaders.SetAt("<urn:service:sos.marine>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_MARINE);

    objHeaders.SetAt("<urn:service:sos.mountain>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN);

    objHeaders.SetAt("<urn:service:sos.physician>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_PHYSICIAN);

    objHeaders.SetAt("<urn:service:sos.poison>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_POISON);

    objHeaders.SetAt("<urn:service:sos.police>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_POLICE);

    objHeaders.SetAt("<urn:service:sos.country-specific>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC);
}

TEST_F(MessageUtilsTest, GetCauseFromReasonHeader)
{
    ImsList<AString> objHeaders;
    AString strReasonHeaderName = "Reason";
    objHeaders.Append("Reason: SIP;cause=603;text=\"any resason\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetCauseFromReasonHeader(piMessage), 603);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetCauseFromReasonHeader(piMessage), -1);
}

TEST_F(MessageUtilsTest, GetCauseAndTextFromReasonHeader)
{
    ReasonHeaderValue objValue;
    ImsList<AString> objHeaders;
    AString strReasonHeaderName = "Reason";
    objHeaders.Append("Reason: SIP;cause=603;text=\"any resason\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(objHeaders));

    objValue = objMessageUtils.GetCauseAndTextFromReasonHeader(piMessage);
    EXPECT_EQ(objValue.nCause, 603);
    EXPECT_STREQ(objValue.strText.GetStr(), "\"any resason\"");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objValue = objMessageUtils.GetCauseAndTextFromReasonHeader(piMessage);
    EXPECT_EQ(objValue.nCause, -1);
    EXPECT_STREQ(objValue.strText.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetSupportedFeatures)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("timer");
    objHeaders.Append("100rel");
    objHeaders.Append("precondition");
    IMS_SINT32 nFeature = FEATURE_TIMER | FEATURE_100REL | FEATURE_PRECONDITION;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::SUPPORTED, _)).WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetSupportedFeatures(piMessage), nFeature);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetSupportedFeatures(piMessage), FEATURE_NONE);
}

TEST_F(MessageUtilsTest, GetRequireFeatures)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("timer");
    objHeaders.Append("100rel");
    objHeaders.Append("precondition");
    IMS_SINT32 nFeature = FEATURE_TIMER | FEATURE_100REL | FEATURE_PRECONDITION;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::REQUIRE, _)).WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetRequireFeatures(piMessage), nFeature);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetRequireFeatures(piMessage), FEATURE_NONE);
}

TEST_F(MessageUtilsTest, GetIms3gppFromBody)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, GetStatusCodeInNotify)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, HasSdp)
{
    ISipMessageBodyPart* piBodyPart = reinterpret_cast<ISipMessageBodyPart*>(0x1);
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(piBodyPart));

    EXPECT_TRUE(objMessageUtils.HasSdp(piMessage));

    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(nullptr));

    EXPECT_FALSE(objMessageUtils.HasSdp(piMessage));
}

TEST_F(MessageUtilsTest, IsFocusConf)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anycontact>;isfocus");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.IsFocusConf(piMessage));

    objHeaders.SetAt("<sip:anycontact>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(objMessageUtils.IsFocusConf(piMessage));
}

TEST_F(MessageUtilsTest, IsInitialRegistrationRequired)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, ContainsValue)
{
    AString strValue = "anyValue";
    ImsList<AString> objHeaders;
    objHeaders.Append(strValue);
    objHeaders.Append("anotherValue");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.ContainsValue(piMessage, strValue, ANY_HEADER));

    objHeaders.Clear();
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    EXPECT_FALSE(objMessageUtils.ContainsValue(piMessage, strValue, ANY_HEADER));
}

TEST_F(MessageUtilsTest, HasValue)
{
    AString strValue = "anyValue";
    ImsList<AString> objHeaders;
    objHeaders.Append(strValue);
    objHeaders.Append("anotherValue");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.HasValue(piMessage, strValue, ANY_HEADER));

    objHeaders.Clear();
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    EXPECT_FALSE(objMessageUtils.HasValue(piMessage, strValue, ANY_HEADER));
}

TEST_F(MessageUtilsTest, IsHeaderPresent)
{
    ON_CALL(*piSipMessage, IsHeaderPresent(ANY_HEADER, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(objMessageUtils.IsHeaderPresent(piMessage, ANY_HEADER));

    ON_CALL(*piSipMessage, IsHeaderPresent(ANY_HEADER, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(objMessageUtils.IsHeaderPresent(piMessage, ANY_HEADER));
}

TEST_F(MessageUtilsTest, ContainsTag)
{
    EXPECT_TRUE(objMessageUtils.ContainsTag("headerContainsAnyTag", "AnyTag"));

    EXPECT_FALSE(objMessageUtils.ContainsTag("headerContainsAnyTag", "DifferentTag"));
}

TEST_F(MessageUtilsTest, ContainsAddressInPaid)
{
    AString strAddress = "sip:anyAddress@ims.google.com";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyAddress@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.ContainsAddressInPaid(piMessage, strAddress));

    objHeaders.SetAt("<sip:differentAddress@ims.google.com>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(objMessageUtils.ContainsAddressInPaid(piMessage, strAddress));

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objMessageUtils.ContainsAddressInPaid(piMessage, strAddress));
}

TEST_F(MessageUtilsTest, SetHeader)
{
    AString strValue = "anyValue";
    AString strHeaderName;
    EXPECT_CALL(*piSipMessage, SetHeader(ANY_HEADER, strValue, strHeaderName)).Times(1);

    EXPECT_EQ(objMessageUtils.SetHeader(piMessage, strValue, ANY_HEADER), IMS_SUCCESS);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.SetHeader(piMessage, strValue, ANY_HEADER), IMS_FAILURE);
}

TEST_F(MessageUtilsTest, AddValueIfNotExists)
{
    AString strValue = "anyValue";
    AString strHeaderName;
    EXPECT_CALL(*piSipMessage, AddHeader(ANY_HEADER, strValue, strHeaderName)).Times(1);

    EXPECT_EQ(objMessageUtils.AddValueIfNotExists(piMessage, strValue, ANY_HEADER), IMS_SUCCESS);

    ImsList<AString> objHeaders;
    objHeaders.Append(strValue);
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.AddValueIfNotExists(piMessage, strValue, ANY_HEADER), IMS_SUCCESS);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.AddValueIfNotExists(piMessage, strValue, ANY_HEADER), IMS_FAILURE);
}

TEST_F(MessageUtilsTest, GenerateContentId)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, SetResourceListByConfUser)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, SetResourceListByEntryUri)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, IsVideoFeatureIncluded)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append(
            "<sip:anyUri>;video;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.IsVideoFeatureIncluded(piMessage));

    objHeaders.SetAt("<sip:anyUri>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_FALSE(objMessageUtils.IsVideoFeatureIncluded(piMessage));
}

TEST_F(MessageUtilsTest, IsTextFeatureIncluded)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append(
            "<sip:anyUri>;text;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.IsTextFeatureIncluded(piMessage));

    objHeaders.SetAt("<sip:anyUri>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_FALSE(objMessageUtils.IsTextFeatureIncluded(piMessage));
}

TEST_F(MessageUtilsTest, GetCallType)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, GetCallTypeFromSdp)
{
    // TODO: add
}

TEST_F(MessageUtilsTest, IsResponseExist)
{
    IMS_SINT32 nAnyCode = SipStatusCode::SC_200;
    SetUpPreviousResponses(IMessage::SESSION_START);
    ON_CALL(*piMessage, GetStatusCode).WillByDefault(Return(nAnyCode));

    EXPECT_TRUE(objMessageUtils.IsResponseExist(piSession, nAnyCode));

    IMS_SINT32 nAnotherCode = SipStatusCode::SC_180;
    EXPECT_FALSE(objMessageUtils.IsResponseExist(piSession, nAnotherCode));
}

}  // namespace android
