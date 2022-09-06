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

#include <gtest/gtest.h>
#include "utility/MessageUtil.h"
#include "CallReasonInfo.h"
#include "ImsList.h"
#include "core/IReference.h"
#include "core/MockIMessage.h"
#include "core/MockISession.h"
#include "sipcore/MockISipMessage.h"
#include "sipcore/ISipHeader.h"
#include "sipcore/ISipMessageBodyPart.h"
#include "sipcore/SipHeaderName.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL IMS_SINT32 ANY_METHOD = IMessage::SESSION_START;
LOCAL IMS_SINT32 ANY_HEADER = ISipHeader::SUPPORTED;

namespace android
{

class MessageUtilTest : public ::testing::Test
{
public:
    MockIMessage* piMessage;
    MockISipMessage* piSipMessage;
    MockISession* piSession;
    ImsList<IMessage*> objMessages;

protected:
    virtual void SetUp() override
    {
        piMessage = new MockIMessage();
        piSipMessage = new MockISipMessage();
        piSession = new MockISession();

        ON_CALL(*piSession, GetNextRequest)
                .WillByDefault(Return(piMessage));
        ON_CALL(*piSession, GetNextResponse)
                .WillByDefault(Return(piMessage));

        ON_CALL(*piMessage, GetMessage)
                .WillByDefault(Return(piSipMessage));
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
        ON_CALL(*piSession, GetPreviousRequest(eServiceMethod))
                .WillByDefault(Return(piMessage));
    }

    void SetUpPreviousResponse(IN IMS_SINT32 eServiceMethod = IMessage::SESSION_START)
    {
        ON_CALL(*piSession, GetPreviousResponse(eServiceMethod))
                .WillByDefault(Return(piMessage));
    }

    void SetUpPreviousResponses(IN IMS_SINT32 eServiceMethod = IMessage::SESSION_START)
    {
        objMessages.Append(piMessage);
        ON_CALL(*piSession, GetPreviousResponses(eServiceMethod))
                .WillByDefault(Return(objMessages));
    }
};

TEST_F(MessageUtilTest, GetPreviousResponse)
{
    EXPECT_TRUE(MessageUtil::GetPreviousResponse(IMS_NULL, ANY_METHOD) == IMS_NULL);

    SetUpPreviousResponses();
    EXPECT_EQ(MessageUtil::GetPreviousResponse(piSession, ANY_METHOD), piMessage);

    EXPECT_TRUE(MessageUtil::GetPreviousResponse(piSession, ANY_METHOD, 10) == IMS_NULL);
}

TEST_F(MessageUtilTest, GetRemotePreviousMessage)
{
    EXPECT_TRUE(MessageUtil::GetRemotePreviousMessage(IMS_NULL, ANY_METHOD, IMS_FALSE) == IMS_NULL);

    SetUpPreviousResponses();
    EXPECT_EQ(MessageUtil::GetRemotePreviousMessage(piSession, ANY_METHOD, IMS_TRUE), piMessage);

    SetUpPreviousRequest();
    EXPECT_EQ(MessageUtil::GetRemotePreviousMessage(piSession, ANY_METHOD, IMS_FALSE), piMessage);
}

TEST_F(MessageUtilTest, GetResponseStatusCode)
{
    EXPECT_EQ(MessageUtil::GetResponseStatusCode(IMS_NULL, ANY_METHOD), SipStatusCode::SC_INVALID);

    IMS_SINT32 nAnyCode = SipStatusCode::SC_200;
    SetUpPreviousResponses();
    ON_CALL(*piMessage, GetStatusCode)
            .WillByDefault(Return(nAnyCode));
    EXPECT_EQ(MessageUtil::GetResponseStatusCode(piSession, ANY_METHOD), nAnyCode);
}

TEST_F(MessageUtilTest, GetRemoteUris)
{
    ImsList<AString> objUris;
    EXPECT_EQ(MessageUtil::GetRemoteUris(IMS_NULL, PeerType::MO, objUris), IMS_FAILURE);

    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId)
            .WillByDefault(Return(objAddresses));

    AString strAnyUri = "sip:anyHeader";
    ImsList<AString> objHeaders;
    objHeaders.Append(strAnyUri);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _))
            .WillByDefault(Return(objHeaders));

    SetUpPreviousRequest(IMessage::SESSION_START);

    EXPECT_EQ(MessageUtil::GetRemoteUris(piSession, PeerType::MO, objUris), IMS_SUCCESS);
    EXPECT_EQ(objUris.GetSize(), 1);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), strAnyUri.GetStr());
}

TEST_F(MessageUtilTest, GetRemoteUri)
{
    AString strRemoteUri;
    EXPECT_EQ(MessageUtil::GetRemoteUri(IMS_NULL, PeerType::MO, strRemoteUri), IMS_FAILURE);

    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId)
            .WillByDefault(Return(objAddresses));

    AString strAnyUri = "sip:anyHeader";
    ImsList<AString> objHeaders;
    objHeaders.Append(strAnyUri);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _))
            .WillByDefault(Return(objHeaders));

    SetUpPreviousRequest(IMessage::SESSION_START);

    EXPECT_EQ(MessageUtil::GetRemoteUri(piSession, PeerType::MO, strRemoteUri), IMS_SUCCESS);
    EXPECT_STREQ(strRemoteUri.GetStr(), strAnyUri.GetStr());
}

TEST_F(MessageUtilTest, GetSessionId)
{
    AString strSessionId;
    EXPECT_EQ(MessageUtil::GetSessionId(IMS_NULL, strSessionId), IMS_FAILURE);

    AString strAnySessionId;
    ON_CALL(*piSession, GetSessionId)
            .WillByDefault(ReturnRef(strAnySessionId));

    EXPECT_EQ(MessageUtil::GetSessionId(piSession, strSessionId), IMS_FAILURE);

    strAnySessionId = "123456789";
    EXPECT_EQ(MessageUtil::GetSessionId(piSession, strSessionId), IMS_SUCCESS);
    EXPECT_STREQ(strSessionId.GetStr(), strAnySessionId.GetStr());
}

TEST_F(MessageUtilTest, GetHeaders)
{
    ImsList<AString> objOutHeaders;
    ImsList<AString> objHeaders;
    objHeaders.Append("sip:anyHeader");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetHeaders(piMessage, ANY_HEADER, objOutHeaders), IMS_SUCCESS);
    EXPECT_EQ(objOutHeaders.GetSize(), 1);
    EXPECT_STREQ(objOutHeaders.GetAt(0).GetStr(), objHeaders.GetAt(0).GetStr());

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetHeaders(piMessage, ANY_HEADER, objOutHeaders), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetHeader)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("sip:anyHeader");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetHeader(piMessage, ANY_HEADER, strHeader), IMS_SUCCESS);
    EXPECT_STREQ(strHeader.GetStr(), objHeaders.GetAt(0).GetStr());

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetHeader(piMessage, ANY_HEADER, strHeader), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetHeaderValue)
{
    AString strValue;
    ImsList<AString> objHeaders;
    objHeaders.Append("anyValue"); // TODO: check
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetHeaderValue(piMessage, ANY_HEADER, strValue), IMS_SUCCESS);
    EXPECT_STREQ(strValue.GetStr(), objHeaders.GetAt(0).GetStr());

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetHeaderValue(piMessage, ANY_HEADER, strValue), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetHeaderValueInt)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("3600"); // TODO: check
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::EXPIRES_SEC, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), 3600);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), -1);
}

TEST_F(MessageUtilTest, GetParameterValue)
{
    AString strParameterValue;
    AString strAnyParamName = "expires";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:12345>;expires=3600");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetParameterValue(
            piMessage, strAnyParamName, ANY_HEADER, strParameterValue), IMS_SUCCESS);
    EXPECT_STREQ(strParameterValue.GetStr(), "3600");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetParameterValue(
            piMessage, strAnyParamName, ANY_HEADER, strParameterValue), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetUserParts)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    ImsList<AString> objUserParts = MessageUtil::GetUserParts(piMessage, ANY_HEADER);
    EXPECT_EQ(objUserParts.GetSize(), 2);
    EXPECT_STREQ(objUserParts.GetAt(0).GetStr(), "anyname1");
    EXPECT_STREQ(objUserParts.GetAt(1).GetStr(), "12345");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    objUserParts = MessageUtil::GetUserParts(piMessage, ANY_HEADER);
    EXPECT_EQ(objUserParts.GetSize(), 0);
}

TEST_F(MessageUtilTest, GetUserPart)
{
    AString strUserPart;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetUserPart(piMessage, ANY_HEADER, strUserPart), IMS_SUCCESS);
    EXPECT_STREQ(strUserPart.GetStr(), "anyname1");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetUserPart(piMessage, ANY_HEADER, strUserPart), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetUserIds)
{
    ImsList<AString> objUserIds;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetUserIds(piMessage, ANY_HEADER, objUserIds), IMS_SUCCESS);
    EXPECT_EQ(objUserIds.GetSize(), 2);
    EXPECT_STREQ(objUserIds.GetAt(0).GetStr(), "anyname1");
    EXPECT_STREQ(objUserIds.GetAt(1).GetStr(), "12345");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetUserIds(piMessage, ANY_HEADER, objUserIds), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetUserId)
{
    AString strUserId;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetUserId(piMessage, ANY_HEADER, strUserId), IMS_SUCCESS);
    EXPECT_STREQ(strUserId.GetStr(), "anyname1");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetUserId(piMessage, ANY_HEADER, strUserId), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetDisplayNames)
{
    ImsList<AString> objDisplayNames;
    ImsList<AString> objHeaders;
    objHeaders.Append("\"any display name1\" <sip:anyname1@ims.google.com>");
    objHeaders.Append("\"any display name2\" <tel:12345>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetDisplayNames(piMessage, ANY_HEADER, objDisplayNames), IMS_SUCCESS);
    EXPECT_EQ(objDisplayNames.GetSize(), 2);
    EXPECT_STREQ(objDisplayNames.GetAt(0).GetStr(), "any display name1");
    EXPECT_STREQ(objDisplayNames.GetAt(1).GetStr(), "any display name2");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetDisplayNames(piMessage, ANY_HEADER, objDisplayNames), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetDisplayName)
{
    AString strDisplayName;
    ImsList<AString> objHeaders;
    objHeaders.Append("\"any display name\" <sip:anyname1@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetDisplayName(piMessage, ANY_HEADER, strDisplayName), IMS_SUCCESS);
    EXPECT_STREQ(strDisplayName.GetStr(), "any display name");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetDisplayName(piMessage, ANY_HEADER, strDisplayName), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetHosts)
{
    ImsList<AString> objHosts;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetHosts(piMessage, ANY_HEADER, objHosts), IMS_SUCCESS);
    EXPECT_EQ(objHosts.GetSize(), 2);
    EXPECT_STREQ(objHosts.GetAt(0).GetStr(), "ims.google.com");
    EXPECT_STREQ(objHosts.GetAt(1).GetStr(), "12345"); // TODO: check

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetHosts(piMessage, ANY_HEADER, objHosts), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetHost)
{
    AString strHost;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname;userparam@ims.google.com;anyuriparam>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetHost(piMessage, ANY_HEADER, strHost), IMS_SUCCESS);
    EXPECT_STREQ(strHost.GetStr(), "ims.google.com");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetHost(piMessage, ANY_HEADER, strHost), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetParameterValueFromUri)
{
    AString strParamValue;
    AString strAnyParamName = "userparam";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname;userparam=userparamvalue@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetParameterValueFromUri(
            piMessage, strAnyParamName, ANY_HEADER, strParamValue), IMS_SUCCESS);
    EXPECT_STREQ(strParamValue.GetStr(), "userparamvalue");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetParameterValueFromUri(
            piMessage, strAnyParamName, ANY_HEADER, strParamValue), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetUris)
{
    ImsList<AString> objUris;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1@ims.google.com;anyuriparam>");
    objHeaders.Append("<tel:12345;anyuriparam>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetUris(piMessage, IMS_TRUE, ANY_HEADER, objUris), IMS_SUCCESS);
    EXPECT_EQ(objUris.GetSize(), 2);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), "<sip:anyname1@ims.google.com;anyuriparam>");
    EXPECT_STREQ(objUris.GetAt(1).GetStr(), "<tel:12345;anyuriparam>");

    EXPECT_EQ(MessageUtil::GetUris(piMessage, IMS_FALSE, ANY_HEADER, objUris), IMS_SUCCESS);
    EXPECT_EQ(objUris.GetSize(), 2);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), "sip:anyname1@ims.google.com");
    EXPECT_STREQ(objUris.GetAt(1).GetStr(), "tel:12345");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetUris(piMessage, IMS_FALSE, ANY_HEADER, objUris), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetUri)
{
    AString strUri;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1@ims.google.com;anyuriparam>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetUri(piMessage, IMS_TRUE, ANY_HEADER, strUri), IMS_SUCCESS);
    EXPECT_STREQ(strUri.GetStr(), "<sip:anyname1@ims.google.com;anyuriparam>");

    EXPECT_EQ(MessageUtil::GetUri(piMessage, IMS_FALSE, ANY_HEADER, strUri), IMS_SUCCESS);
    EXPECT_STREQ(strUri.GetStr(), "sip:anyname1@ims.google.com");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetUri(piMessage, IMS_FALSE, ANY_HEADER, strUri), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetFeatures)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("timer");
    objHeaders.Append("100rel");
    objHeaders.Append("precondition");
    IMS_SINT32 nFeature = FEATURE_TIMER | FEATURE_100REL | FEATURE_PRECONDITION;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetFeatures(piMessage, ISipHeader::SUPPORTED), nFeature);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetFeatures(piMessage, ISipHeader::SUPPORTED), FEATURE_NONE);
}

TEST_F(MessageUtilTest, GetSosTypeFromServiceUrn)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_INVALID);

    objHeaders.Append("<urn:service:sos>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);

    objHeaders.SetAt("<urn:service:sos.ambulance>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE);

    objHeaders.SetAt("<urn:service:sos.animal-control>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_ANIMAL_CONTROL);

    objHeaders.SetAt("<urn:service:sos.fire>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_FIRE);

    objHeaders.SetAt("<urn:service:sos.gas>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GAS);

    objHeaders.SetAt("<urn:service:sos.marine>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_MARINE);

    objHeaders.SetAt("<urn:service:sos.mountain>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_MOUNTAIN);

    objHeaders.SetAt("<urn:service:sos.physician>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_PHYSICIAN);

    objHeaders.SetAt("<urn:service:sos.poison>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_POISON);

    objHeaders.SetAt("<urn:service:sos.police>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_POLICE);

    objHeaders.SetAt("<urn:service:sos.country-specific>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC);
}

TEST_F(MessageUtilTest, GetCauseFromReasonHeader)
{
    ImsList<AString> objHeaders;
    AString strReasonHeaderName = "Reason";
    objHeaders.Append("Reason: SIP;cause=603;text=\"any resason\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetCauseFromReasonHeader(piMessage), 603);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetCauseFromReasonHeader(piMessage), -1);
}

TEST_F(MessageUtilTest, GetCauseAndTextFromReasonHeader)
{
    IMS_SINT32 nCause;
    AString strText;
    ImsList<AString> objHeaders;
    AString strReasonHeaderName = "Reason";
    objHeaders.Append("Reason: SIP;cause=603;text=\"any resason\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::UNKNOWN, strReasonHeaderName))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetCauseAndTextFromReasonHeader(piMessage, nCause, strText),
            IMS_SUCCESS);
    EXPECT_EQ(nCause, 603);
    EXPECT_STREQ(strText.GetStr(), "\"any resason\"");

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetCauseAndTextFromReasonHeader(piMessage, nCause, strText),
            IMS_FAILURE);
}

TEST_F(MessageUtilTest, GetSupportedFeatures)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("timer");
    objHeaders.Append("100rel");
    objHeaders.Append("precondition");
    IMS_SINT32 nFeature = FEATURE_TIMER | FEATURE_100REL | FEATURE_PRECONDITION;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::SUPPORTED, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetSupportedFeatures(piMessage), nFeature);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetSupportedFeatures(piMessage), FEATURE_NONE);
}

TEST_F(MessageUtilTest, GetRequireFeatures)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("timer");
    objHeaders.Append("100rel");
    objHeaders.Append("precondition");
    IMS_SINT32 nFeature = FEATURE_TIMER | FEATURE_100REL | FEATURE_PRECONDITION;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::REQUIRE, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(MessageUtil::GetRequireFeatures(piMessage), nFeature);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::GetRequireFeatures(piMessage), FEATURE_NONE);
}

TEST_F(MessageUtilTest, GetIms3gppFromBody)
{
    // TODO: add
}

TEST_F(MessageUtilTest, GetStatusCodeInNotify)
{
    // TODO: add
}

TEST_F(MessageUtilTest, HasSdp)
{
    ISipMessageBodyPart* piBodyPart = reinterpret_cast<ISipMessageBodyPart*>(0x1);
    ON_CALL(*piSipMessage, GetSdpBodyPart())
            .WillByDefault(Return(piBodyPart));

    EXPECT_TRUE(MessageUtil::HasSdp(piMessage));

    ON_CALL(*piSipMessage, GetSdpBodyPart())
            .WillByDefault(Return(nullptr));

    EXPECT_FALSE(MessageUtil::HasSdp(piMessage));
}

TEST_F(MessageUtilTest, IsFocusConf)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anycontact>;isfocus");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(MessageUtil::IsFocusConf(piMessage));

    objHeaders.SetAt("<sip:anycontact>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(MessageUtil::IsFocusConf(piMessage));
}

TEST_F(MessageUtilTest, IsInitialRegistrationRequired)
{
    // TODO: add
}

TEST_F(MessageUtilTest, ContainsValue)
{
    AString strValue = "anyValue";
    ImsList<AString> objHeaders;
    objHeaders.Append(strValue);
    objHeaders.Append("anotherValue");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(MessageUtil::ContainsValue(piMessage, strValue, ANY_HEADER));

    objHeaders.Clear();
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(MessageUtil::ContainsValue(piMessage, strValue, ANY_HEADER));
}

TEST_F(MessageUtilTest, HasValue)
{
    AString strValue = "anyValue";
    ImsList<AString> objHeaders;
    objHeaders.Append(strValue);
    objHeaders.Append("anotherValue");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(MessageUtil::HasValue(piMessage, strValue, ANY_HEADER));

    objHeaders.Clear();
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(MessageUtil::HasValue(piMessage, strValue, ANY_HEADER));
}

TEST_F(MessageUtilTest, IsHeaderPresent)
{
    ON_CALL(*piSipMessage, IsHeaderPresent(ANY_HEADER, _))
            .WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(MessageUtil::IsHeaderPresent(piMessage, ANY_HEADER));

    ON_CALL(*piSipMessage, IsHeaderPresent(ANY_HEADER, _))
            .WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(MessageUtil::IsHeaderPresent(piMessage, ANY_HEADER));
}

TEST_F(MessageUtilTest, ContainsTag)
{
    EXPECT_TRUE(MessageUtil::ContainsTag("headerContainsAnyTag", "AnyTag"));

    EXPECT_FALSE(MessageUtil::ContainsTag("headerContainsAnyTag", "DifferentTag"));
}

TEST_F(MessageUtilTest, ContainsAddressInPaid)
{
    AString strAddress = "sip:anyAddress@ims.google.com";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyAddress@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(MessageUtil::ContainsAddressInPaid(piMessage, strAddress));

    objHeaders.SetAt("<sip:differentAddress@ims.google.com>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(MessageUtil::ContainsAddressInPaid(piMessage, strAddress));

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_FALSE(MessageUtil::ContainsAddressInPaid(piMessage, strAddress));
}

TEST_F(MessageUtilTest, SetHeader)
{
    AString strValue = "anyValue";
    AString strHeaderName;
    EXPECT_CALL(*piSipMessage, SetHeader(ANY_HEADER, strValue, strHeaderName))
            .Times(1);

    EXPECT_EQ(MessageUtil::SetHeader(piMessage, strValue, ANY_HEADER), IMS_SUCCESS);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::SetHeader(piMessage, strValue, ANY_HEADER), IMS_FAILURE);
}

TEST_F(MessageUtilTest, AddValueIfNotExists)
{
    AString strValue = "anyValue";
    AString strHeaderName;
    EXPECT_CALL(*piSipMessage, AddHeader(ANY_HEADER, strValue, strHeaderName))
            .Times(1);

    EXPECT_EQ(MessageUtil::AddValueIfNotExists(piMessage, strValue, ANY_HEADER), IMS_SUCCESS);

    ImsList<AString> objHeaders;
    objHeaders.Append(strValue);
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(MessageUtil::AddValueIfNotExists(piMessage, strValue, ANY_HEADER), IMS_SUCCESS);

    ON_CALL(*piMessage, GetMessage)
            .WillByDefault(Return(nullptr));
    EXPECT_EQ(MessageUtil::AddValueIfNotExists(piMessage, strValue, ANY_HEADER), IMS_FAILURE);
}

TEST_F(MessageUtilTest, GenerateContentId)
{
    // TODO: add
}

TEST_F(MessageUtilTest, SetResourceListByConfUser)
{
    // TODO: add
}

TEST_F(MessageUtilTest, SetResourceListByEntryUri)
{
    // TODO: add
}

TEST_F(MessageUtilTest, IsVideoFeatureIncluded)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append(
            "<sip:anyUri>;video;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(MessageUtil::IsVideoFeatureIncluded(piMessage));

    objHeaders.SetAt("<sip:anyUri>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_FALSE(MessageUtil::IsVideoFeatureIncluded(piMessage));
}

TEST_F(MessageUtilTest, IsTextFeatureIncluded)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append(
            "<sip:anyUri>;text;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(MessageUtil::IsTextFeatureIncluded(piMessage));

    objHeaders.SetAt("<sip:anyUri>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_FALSE(MessageUtil::IsTextFeatureIncluded(piMessage));
}

TEST_F(MessageUtilTest, CheckServiceType)
{
    // TODO: remove api
}

TEST_F(MessageUtilTest, GetCallType)
{
    // TODO: add
}

TEST_F(MessageUtilTest, GetCallTypeFromSdp)
{
    // TODO: add
}

TEST_F(MessageUtilTest, GetCallTypeFromAcceptContact)
{
    // TODO: remove api
}

TEST_F(MessageUtilTest, CheckRttUpdateRequest)
{
    // TODO: remove api
}

TEST_F(MessageUtilTest, IsSessionRefresh)
{
    // TODO: remove api
}

TEST_F(MessageUtilTest, IsTextSession)
{
    // TODO: remove api
}

TEST_F(MessageUtilTest, IsResponseExist)
{
    IMS_SINT32 nAnyCode = SipStatusCode::SC_200;
    SetUpPreviousResponses(IMessage::SESSION_START);
    ON_CALL(*piMessage, GetStatusCode)
            .WillByDefault(Return(nAnyCode));

    EXPECT_TRUE(MessageUtil::IsResponseExist(piSession, nAnyCode));

    IMS_SINT32 nAnotherCode = SipStatusCode::SC_180;
    EXPECT_FALSE(MessageUtil::IsResponseExist(piSession, nAnotherCode));
}

}  // namespace android
