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
#include "IReference.h"
#include "ISipHeader.h"
#include "ISipMessageBodyPart.h"
#include "ImsList.h"
#include "MockIMessage.h"
#include "MockIMessageBodyPart.h"
#include "MockIMtcContext.h"
#include "MockISession.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "SdpMedia.h"
#include "SipHeaderName.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "conferencecall/ConferenceDef.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "media/IMedia.h"
#include "media/MockIMedia.h"
#include "media/MockIMediaDescriptor.h"
#include "utility/MessageUtil.h"
#include "utility/MessageUtils.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

LOCAL IMS_SINT32 ANY_METHOD = IMessage::SESSION_START;
LOCAL IMS_SINT32 ANY_HEADER = ISipHeader::SUPPORTED;

// ResourceList
MATCHER_P(IsEqualResourceList, entryList, "")
{
    for (IMS_UINT32 i = 0; i < entryList.GetSize(); ++i)
    {
        if (arg.ToString().Contains(entryList.GetAt(i)) == IMS_FALSE)
        {
            return IMS_FALSE;
        }
    }
    return IMS_TRUE;
}

namespace android
{
// clang-format off
static const IMS_CHAR IMS_3GPP_XML[] = {
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
"<ims-3gpp version=\"1\">\n"
    "<alternative-service>\n"
        "<type>_TYPE_</type>\n"
        "<reason>_REASON_</reason>\n"
        "<action>_ACTION_</action>\n"
    "</alternative-service>\n"
"</ims-3gpp>\n"
};
// clang-format on

class MessageUtilsTest : public ::testing::Test
{
public:
    inline MessageUtilsTest() :
            objMessageUtils(objContext)
    {
    }

    MockIMessage* piMessage;
    MockISipMessage* piSipMessage;
    MockISession* piSession;
    ImsList<IMessage*> objMessages;
    MockIMtcContext objContext;
    MockMtcConfigurationProxy objConfigurationProxy;
    MessageUtils objMessageUtils;

protected:
    virtual void SetUp() override
    {
        piMessage = new MockIMessage();
        piSipMessage = new MockISipMessage();
        piSession = new MockISession();

        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));

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

    void SetUpPreviousResponses(IN IMS_SINT32 eServiceMethod = IMessage::SESSION_START,
            IN IMS_BOOL bIsEmpty = IMS_FALSE)
    {
        if (!bIsEmpty)
        {
            objMessages.Append(piMessage);
        }
        ON_CALL(*piSession, GetPreviousResponses(eServiceMethod))
                .WillByDefault(Return(objMessages));
    }

    const AString GetIms3gppXml(
            IN const AString& strType, IN const AString& strReason, IN const AString& strAction)
    {
        AString strIms3gpp(IMS_3GPP_XML);
        strIms3gpp = strIms3gpp.Replace("_TYPE_", strType);
        strIms3gpp = strIms3gpp.Replace("_REASON_", strReason);
        strIms3gpp = strIms3gpp.Replace("_ACTION_", strAction);

        return strIms3gpp;
    }
};

TEST_F(MessageUtilsTest, GetPreviousResponse)
{
    EXPECT_TRUE(objMessageUtils.GetPreviousResponse(IMS_NULL, ANY_METHOD) == IMS_NULL);

    SetUpPreviousResponses(ANY_METHOD, IMS_TRUE);
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
    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));

    AString strAnyUri = "sip:anyHeader";
    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append(strAnyUri);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));

    SetUpPreviousRequest(IMessage::SESSION_START);

    ImsList<AString> objUris = objMessageUtils.GetRemoteUris(piSession, PeerType::MO);
    EXPECT_EQ(objUris.GetSize(), 1);
    EXPECT_STREQ(objUris.GetAt(0).GetStr(), strAnyUri.GetStr());
}

TEST_F(MessageUtilsTest, GetRemoteUrisReturnsEmptyIfNoInformation)
{
    EXPECT_EQ(objMessageUtils.GetRemoteUris(IMS_NULL, PeerType::MO).GetSize(), 0);

    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));
    ON_CALL(*piSession, GetPreviousRequest).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetRemoteUris(piSession, PeerType::MO).GetSize(), 0);

    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));
    SetUpPreviousRequest(IMessage::SESSION_START);
    EXPECT_EQ(objMessageUtils.GetRemoteUris(piSession, PeerType::MO).GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetRemoteUri)
{
    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));

    AString strAnyUri1 = "sip:anyHeader1";
    AString strAnyUri2 = "sip:anyHeader2";
    ImsList<AString> objHeaders;
    objHeaders.Append(strAnyUri1);
    objHeaders.Append(strAnyUri2);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));

    SetUpPreviousRequest(IMessage::SESSION_START);

    EXPECT_STREQ(
            objMessageUtils.GetRemoteUri(piSession, PeerType::MO).GetStr(), strAnyUri1.GetStr());
}

TEST_F(MessageUtilsTest, GetRemoteUriReturnsNullIfNoInformation)
{
    EXPECT_STREQ(objMessageUtils.GetRemoteUri(IMS_NULL, PeerType::MO).GetStr(), "");

    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));

    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));
    SetUpPreviousRequest(IMessage::SESSION_START);
    EXPECT_STREQ(objMessageUtils.GetRemoteUri(piSession, PeerType::MO).GetStr(), "");
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
    objHeaders.Append("");
    objHeaders.Append("sip:anyHeader");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    objOutHeaders = objMessageUtils.GetHeaders(piMessage, ANY_HEADER);
    EXPECT_EQ(objOutHeaders.GetSize(), 2);
    EXPECT_STREQ(objOutHeaders.GetAt(0).GetStr(), objHeaders.GetAt(0).GetStr());
    EXPECT_STREQ(objOutHeaders.GetAt(1).GetStr(), objHeaders.GetAt(1).GetStr());

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objOutHeaders = objMessageUtils.GetHeaders(piMessage, ANY_HEADER);
    EXPECT_EQ(objOutHeaders.GetSize(), 0);
}

TEST_F(MessageUtilsTest, GetHeader)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("sip:anyHeader");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strHeader = objMessageUtils.GetHeader(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHeader.GetStr(), objHeaders.GetAt(1).GetStr());
}

TEST_F(MessageUtilsTest, GetHeaderReturnsNullIfNoInformation)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strHeader = objMessageUtils.GetHeader(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHeader.GetStr(), "");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strHeader = objMessageUtils.GetHeader(piMessage, ANY_HEADER);
    EXPECT_STREQ(strHeader.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetHeaderValue)
{
    AString strValue;
    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("anyValue");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strValue = objMessageUtils.GetHeaderValue(piMessage, ANY_HEADER);
    EXPECT_STREQ(strValue.GetStr(), objHeaders.GetAt(1).GetStr());
}

TEST_F(MessageUtilsTest, GetHeaderValueReturnsNullIfNoInformation)
{
    AString strValue;
    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));
    strValue = objMessageUtils.GetHeaderValue(piMessage, ANY_HEADER);
    EXPECT_STREQ(strValue.GetStr(), "");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strValue = objMessageUtils.GetHeaderValue(piMessage, ANY_HEADER);
    EXPECT_STREQ(strValue.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetHeaderValueInt)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("3600");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::EXPIRES_SEC, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), 3600);
}

TEST_F(MessageUtilsTest, GetHeaderValueIntReturnsNegativeIfNoInformation)
{
    EXPECT_LT(objMessageUtils.GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), 0);

    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::EXPIRES_SEC, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_LT(objMessageUtils.GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), 0);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_LT(objMessageUtils.GetHeaderValueInt(piMessage, ISipHeader::EXPIRES_SEC), 0);
}

TEST_F(MessageUtilsTest, GetParameterValue)
{
    AString strParameterValue;
    AString strAnyParamName = "expires";
    ImsList<AString> objHeaders;
    objHeaders.Append("");
    objHeaders.Append("<sip:12345>;expires=3600");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strParameterValue = objMessageUtils.GetParameterValue(piMessage, strAnyParamName, ANY_HEADER);
    EXPECT_STREQ(strParameterValue.GetStr(), "3600");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strParameterValue = objMessageUtils.GetParameterValue(piMessage, strAnyParamName, ANY_HEADER);
    EXPECT_STREQ(strParameterValue.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetParameterValueFromUnknownHeaderBody)
{
    AString strParameterValue;
    AString strAnyParamName = "namewithvalue";
    ImsList<AString> objHeaders;
    objHeaders.Append("anyReaonHeader;nameonly;namewithvalue=value");
    AString strUnknownHeaderName("unknown");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::UNKNOWN, strUnknownHeaderName))
            .WillByDefault(Return(objHeaders));
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::UNKNOWN, AString::ConstNull()))
            .WillByDefault(Return(ImsList<AString>()));

    strParameterValue =
            objMessageUtils.GetParameterValue(piMessage, strAnyParamName, ISipHeader::UNKNOWN);
    EXPECT_EQ(strParameterValue, AString::ConstNull());

    strParameterValue = objMessageUtils.GetParameterValue(
            piMessage, strAnyParamName, ISipHeader::UNKNOWN, strUnknownHeaderName);
    EXPECT_STREQ(strParameterValue.GetStr(), "value");
}

TEST_F(MessageUtilsTest, GetUserPart)
{
    AString strUserPart;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strUserPart = objMessageUtils.GetUserPart(piMessage, ANY_HEADER);
    EXPECT_STREQ(strUserPart.GetStr(), "anyname1");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strUserPart = objMessageUtils.GetUserPart(piMessage, ANY_HEADER);
    EXPECT_STREQ(strUserPart.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetUserPartWithUri)
{
    AString strUri = "<sip:anyname1;userparam@ims.google.com>;anyheaderparam";
    AString strUserPart = objMessageUtils.GetUserPart(strUri);
    EXPECT_STREQ(strUserPart.GetStr(), "anyname1");

    strUri = "<tel:12345;anyuriparam>;anyparam";
    strUserPart = objMessageUtils.GetUserPart(strUri);
    EXPECT_STREQ(strUserPart.GetStr(), "12345");

    strUri = "invalid";
    strUserPart = objMessageUtils.GetUserPart(strUri);
    EXPECT_STREQ(strUserPart.GetStr(), "");

    strUri = "";
    strUserPart = objMessageUtils.GetUserPart(strUri);
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

TEST_F(MessageUtilsTest, GetDisplayName)
{
    AString strDisplayName;
    ImsList<AString> objHeaders;
    objHeaders.Append("\"any display name\" <sip:anyname1@ims.google.com>");
    objHeaders.Append("\"any display name2\" <tel:12345>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strDisplayName = objMessageUtils.GetDisplayName(piMessage, ANY_HEADER);
    EXPECT_STREQ(strDisplayName.GetStr(), "any display name");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    strDisplayName = objMessageUtils.GetDisplayName(piMessage, ANY_HEADER);
    EXPECT_STREQ(strDisplayName.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetDisplayNameWithPercentEncodedValueReturnsDecodedValue)
{
    AString strDisplayName;
    ImsList<AString> objHeaders;
    objHeaders.Append("\"Alphanumeric%2001\" <sip:anyname1@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    strDisplayName = objMessageUtils.GetDisplayName(piMessage, ANY_HEADER);
    EXPECT_STREQ(strDisplayName.GetStr(), "Alphanumeric 01");
}

TEST_F(MessageUtilsTest, GetDisplayNameReturnsNullIfNoHeader)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(_, _)).WillByDefault(Return(objHeaders));

    EXPECT_STREQ("", objMessageUtils.GetDisplayName(piMessage, ANY_HEADER).GetStr());
    EXPECT_STREQ("",
            objMessageUtils.GetDisplayName(piMessage, ISipHeader::P_ASSERTED_IDENTITY).GetStr());
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
    EXPECT_STREQ(objHosts.GetAt(1).GetStr(), "12345");

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

TEST_F(MessageUtilsTest, GetSosTypeFromServiceUrn)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);

    objHeaders.Append("<urn:service:sos>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);

    objHeaders.SetAt("<urn:service:sos.police>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_POLICE);

    objHeaders.SetAt("<urn:service:sos.ambulance>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_AMBULANCE);

    objHeaders.SetAt("<urn:service:sos.fire>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_FIRE);

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

    objHeaders.SetAt("<urn:service:sos.ecall.manual>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_MIEC);

    objHeaders.SetAt("<urn:service:sos.ecall.automatic>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_AIEC);

    objHeaders.SetAt("<urn:service:sos.country-specific>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_INVALID);

    objHeaders.SetAt("<urn:service:sos.country-specific.xy.567>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_COUNTRY_SPECIFIC);

    objHeaders.SetAt("<urn:service:sos.gas>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_UNSPECIFIED);

    objHeaders.SetAt("<urn:nonserviceurn>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);

    objHeaders.SetAt("<urn:service:>", 0);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);

    objHeaders.Clear();
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetSosTypeFromServiceUrn(piMessage, ISipHeader::CONTACT_NORMAL),
            EXTRA_CODE_EMERGENCYSERVICE_GENERIC);
}

TEST_F(MessageUtilsTest, GetCauseFromReasonHeader)
{
    ImsList<AString> objHeaders;
    objHeaders.Append("SIP;cause=603;text=\"any resason\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::REASON, _)).WillByDefault(Return(objHeaders));

    EXPECT_EQ(objMessageUtils.GetCauseFromReasonHeader(piMessage), 603);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetCauseFromReasonHeader(piMessage), -1);
}

TEST_F(MessageUtilsTest, GetCauseAndTextFromReasonHeader)
{
    ReasonHeaderValue objValue;
    ImsList<AString> objHeaders;
    objHeaders.Append("SIP;cause=603;text=\"any resason\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::REASON, _)).WillByDefault(Return(objHeaders));

    objValue = objMessageUtils.GetCauseAndTextFromReasonHeader(piMessage);
    EXPECT_EQ(objValue.nCause, 603);
    EXPECT_STREQ(objValue.strText.GetStr(), "\"any resason\"");
    EXPECT_STREQ(objValue.strProtocol.GetStr(), "SIP");

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    objValue = objMessageUtils.GetCauseAndTextFromReasonHeader(piMessage);
    EXPECT_EQ(objValue.nCause, -1);
    EXPECT_STREQ(objValue.strText.GetStr(), "");
    EXPECT_STREQ(objValue.strProtocol.GetStr(), "");
}

TEST_F(MessageUtilsTest, GetPrioritizedReasonHeader)
{
    ReasonHeaderValue objResult;
    ImsList<AString> objHeaders;
    objHeaders.Append("SIP;cause=603;text=\"any resason\"");
    objHeaders.Append("Q.850;cause=19;text=\"no answer\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::REASON, _)).WillByDefault(Return(objHeaders));
    objResult = objMessageUtils.GetPrioritizedReasonHeader(
            piMessage, {"SIP", "Q.850", AString::ConstNull()});
    EXPECT_EQ(objResult.nCause, 603);
    EXPECT_STREQ(objResult.strText.GetStr(), "\"any resason\"");
    EXPECT_STREQ(objResult.strProtocol.GetStr(), "SIP");
}

TEST_F(MessageUtilsTest, GetIms3gppFromBody)
{
    MockISipMessageBodyPart objISipMessageBodyPartOtherContent1;
    MockISipMessageBodyPart objISipMessageBodyPartOtherContent2;
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objISipMessageBodyPartOtherContent1);
    objBodyParts.Append(&objISipMessageBodyPartOtherContent2);
    objBodyParts.Append(&objISipMessageBodyPart);
    ON_CALL(*piSipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    // invalid type case
    AString strOtherContextType1("other_application/3gpp-ims+xml");
    ON_CALL(objISipMessageBodyPartOtherContent1, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strOtherContextType1));

    // invalid sub type case
    AString strOtherContextType2("application/other+xml");
    ON_CALL(objISipMessageBodyPartOtherContent2, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strOtherContextType2));

    AString strContextType("application/3gpp-ims+xml");
    ON_CALL(objISipMessageBodyPart, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strContextType));

    AString strIms3gppType("unknown type");
    AString strIms3gppReason("unknown reason");
    AString strIms3gppAction("unknown action");
    ByteArray objContent(GetIms3gppXml(strIms3gppType, strIms3gppReason, strIms3gppAction));
    ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    Ims3gpp objIms3gpp;
    objMessageUtils.GetIms3gppFromBody(piMessage, objIms3gpp);
    EXPECT_EQ(objIms3gpp.GetAlternativeService().GetType(),
            Ims3gpp::AlternativeService::TYPE_UNKNOWN);
    EXPECT_EQ(objIms3gpp.GetAlternativeService().GetUnknownType(), strIms3gppType);
    EXPECT_EQ(objIms3gpp.GetAlternativeService().GetReason(), strIms3gppReason);
    EXPECT_EQ(objIms3gpp.GetAlternativeService().GetAction(),
            Ims3gpp::AlternativeService::ACTION_UNKNOWN);
    EXPECT_EQ(objIms3gpp.GetAlternativeService().GetUnknownAction(), strIms3gppAction);
}

TEST_F(MessageUtilsTest, GetIms3gpp)
{
    MockISipMessageBodyPart objISipMessageBodyPartOtherContent1;
    MockISipMessageBodyPart objISipMessageBodyPartOtherContent2;
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objISipMessageBodyPartOtherContent1);
    objBodyParts.Append(&objISipMessageBodyPartOtherContent2);
    objBodyParts.Append(&objISipMessageBodyPart);
    ON_CALL(*piSipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    // invalid type case
    AString strOtherContextType1("other_application/3gpp-ims+xml");
    ON_CALL(objISipMessageBodyPartOtherContent1, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strOtherContextType1));

    // invalid sub type case
    AString strOtherContextType2("application/other+xml");
    ON_CALL(objISipMessageBodyPartOtherContent2, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strOtherContextType2));

    AString strContextType("application/3gpp-ims+xml");
    ON_CALL(objISipMessageBodyPart, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strContextType));

    AString strIms3gppType("unknown type");
    AString strIms3gppReason("unknown reason");
    AString strIms3gppAction("unknown action");
    ByteArray objContent(GetIms3gppXml(strIms3gppType, strIms3gppReason, strIms3gppAction));
    ON_CALL(objISipMessageBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    Ims3gppData objIms3gppData;
    objIms3gppData = objMessageUtils.GetIms3gppData(piMessage);
    EXPECT_EQ(objIms3gppData.eType, Ims3gpp::TYPE_ALTERNATIVE_SERVICE);
    EXPECT_EQ(objIms3gppData.eAlternativeServiceType, Ims3gpp::AlternativeService::TYPE_UNKNOWN);
    EXPECT_EQ(
            objIms3gppData.eAlternativeServiceAction, Ims3gpp::AlternativeService::ACTION_UNKNOWN);
}

TEST_F(MessageUtilsTest, IsInitialRegistrationRequired)
{
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objISipMessageBodyPart);
    ON_CALL(*piSipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    AString strContextType("application/3gpp-ims+xml");
    ON_CALL(objISipMessageBodyPart, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strContextType));

    ByteArray objContentRegirationRestorationContent(
            GetIms3gppXml("restoration", "any reason", "initial-registration"));
    ON_CALL(objISipMessageBodyPart, GetContent)
            .WillByDefault(ReturnRef(objContentRegirationRestorationContent));
    EXPECT_TRUE(objMessageUtils.IsInitialRegistrationRequired(piMessage));

    ByteArray objEmergencyRegistrationRestorationContent(
            GetIms3gppXml("emergency", "any reason", "emergency-registration"));
    ON_CALL(objISipMessageBodyPart, GetContent)
            .WillByDefault(ReturnRef(objEmergencyRegistrationRestorationContent));
    EXPECT_FALSE(objMessageUtils.IsInitialRegistrationRequired(piMessage));
}

TEST_F(MessageUtilsTest, IsInitialEmergencyRegistrationRequired)
{
    MockISipMessageBodyPart objISipMessageBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(&objISipMessageBodyPart);
    ON_CALL(*piSipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    AString strContextType("application/3gpp-ims+xml");
    ON_CALL(objISipMessageBodyPart, GetHeader(ISipMessageBodyPart::CONTENT_TYPE, _))
            .WillByDefault(Return(strContextType));

    ByteArray objContentRegirationRestorationContent(
            GetIms3gppXml("restoration", "any reason", "initial-registration"));
    ON_CALL(objISipMessageBodyPart, GetContent)
            .WillByDefault(ReturnRef(objContentRegirationRestorationContent));
    EXPECT_FALSE(objMessageUtils.IsInitialEmergencyRegistrationRequired(piMessage));

    ByteArray objEmergencyRegistrationRestorationContent(
            GetIms3gppXml("emergency", "any reason", "emergency-registration"));
    ON_CALL(objISipMessageBodyPart, GetContent)
            .WillByDefault(ReturnRef(objEmergencyRegistrationRestorationContent));
    EXPECT_TRUE(objMessageUtils.IsInitialEmergencyRegistrationRequired(piMessage));
}

TEST_F(MessageUtilsTest, GetStatusCodeInNotifyReturnsInvalid)
{
    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_INVALID);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(piSipMessage));
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTENT_TYPE, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_INVALID);

    AString strValue(MessageUtil::STR_CONTENT_TYPE_SIP_FRAG);
    objHeaders.Append(strValue);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTENT_TYPE, _))
            .WillByDefault(Return(objHeaders));

    MockISipMessageBodyPart objBodyPart;
    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objBodyPart);
    ByteArray objContent("");
    ON_CALL(objBodyPart, GetContent).WillByDefault(ReturnRef(objContent));

    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_INVALID);
}

TEST_F(MessageUtilsTest, GetStatusCodeInNotifyReturnsStatusCode)
{
    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_INVALID);

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(piSipMessage));
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTENT_TYPE, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_INVALID);

    AString strValue(MessageUtil::STR_CONTENT_TYPE_SIP_FRAG);
    objHeaders.Append(strValue);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTENT_TYPE, _))
            .WillByDefault(Return(objHeaders));

    MockISipMessageBodyPart objBodyPart;
    ByteArray objContentWithCrlf("SIP/2.0 200 OK\r\n");
    ON_CALL(objBodyPart, GetContent).WillByDefault(ReturnRef(objContentWithCrlf));

    ImsList<ISipMessageBodyPart*> objBodyParts;
    objBodyParts.Append(IMS_NULL);
    objBodyParts.Append(&objBodyPart);
    ON_CALL(*piSipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_200);

    ByteArray objContentWithoutCrlf("SIP/2.0 200 OK");
    ON_CALL(objBodyPart, GetContent).WillByDefault(ReturnRef(objContentWithoutCrlf));
    objBodyParts.Clear();
    objBodyParts.Append(&objBodyPart);
    ON_CALL(*piSipMessage, GetBodyParts).WillByDefault(Return(objBodyParts));

    EXPECT_EQ(objMessageUtils.GetStatusCodeInNotify(piMessage), SipStatusCode::SC_200);
}

TEST_F(MessageUtilsTest, HasSdp)
{
    ISipMessageBodyPart* piBodyPart = reinterpret_cast<ISipMessageBodyPart*>(0x1);
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(piBodyPart));
    EXPECT_TRUE(objMessageUtils.HasSdp(piMessage));

    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objMessageUtils.HasSdp(piMessage));

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objMessageUtils.HasSdp(piMessage));
}

TEST_F(MessageUtilsTest, IsFocusConf)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_FALSE(objMessageUtils.IsFocusConf(piMessage));

    objHeaders.Append(AString::ConstEmpty());
    objHeaders.Append("<sip:anycontact>");
    objHeaders.Append("<sip:anycontact>;anyparameter");
    objHeaders.Append("<sip:anycontact>;isfocus=false");
    objHeaders.Append("<sip:anycontact>;isfocus");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_TRUE(objMessageUtils.IsFocusConf(piMessage));

    objHeaders.Clear();
    objHeaders.Append("<sip:anycontact>;isfocus=true");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));
    EXPECT_TRUE(objMessageUtils.IsFocusConf(piMessage));
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

TEST_F(MessageUtilsTest, ContainsValueIgnoreCase)
{
    AString strValue = "anyValue";
    AString strUppercasedValue = strValue.MakeUpper();
    ImsList<AString> objHeaders;
    objHeaders.Append(strUppercasedValue);
    objHeaders.Append("anotherValue");
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.ContainsValueIgnoreCase(piMessage, strValue, ANY_HEADER));

    objHeaders.Clear();
    ON_CALL(*piSipMessage, GetHeaders(ANY_HEADER, _)).WillByDefault(Return(objHeaders));

    EXPECT_FALSE(objMessageUtils.ContainsValueIgnoreCase(piMessage, strValue, ANY_HEADER));
}

TEST_F(MessageUtilsTest, IsHeaderPresent)
{
    ON_CALL(*piSipMessage, IsHeaderPresent(ANY_HEADER, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(objMessageUtils.IsHeaderPresent(piMessage, ANY_HEADER));

    ON_CALL(*piSipMessage, IsHeaderPresent(ANY_HEADER, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(objMessageUtils.IsHeaderPresent(piMessage, ANY_HEADER));

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objMessageUtils.IsHeaderPresent(piMessage, ANY_HEADER));
}

TEST_F(MessageUtilsTest, ContainsTag)
{
    EXPECT_TRUE(objMessageUtils.ContainsTag("headerContainsAnyTag", "AnyTag"));

    EXPECT_FALSE(objMessageUtils.ContainsTag("headerContainsAnyTag", "DifferentTag"));

    EXPECT_FALSE(objMessageUtils.ContainsTag("", "DifferentTag"));

    EXPECT_FALSE(objMessageUtils.ContainsTag("headerContainsEmptyTag", ""));
}

TEST_F(MessageUtilsTest, ContainsAddressInPaid)
{
    AString strEmptyAddress;
    EXPECT_FALSE(objMessageUtils.ContainsAddressInPaid(piMessage, strEmptyAddress));

    AString strAddress = "sip:anyAddress@ims.google.com";
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyAddress@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_TRUE(objMessageUtils.ContainsAddressInPaid(piMessage, strAddress));

    objHeaders.SetAt("<sip:differentAddress1@ims.google.com>", 0);
    objHeaders.Append("<sip:differentAddress2@ims.google.com>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_FALSE(objMessageUtils.ContainsAddressInPaid(piMessage, strAddress));

    ON_CALL(*piMessage, GetMessage).WillByDefault(Return(nullptr));
    EXPECT_FALSE(objMessageUtils.ContainsAddressInPaid(piMessage, strAddress));
}

TEST_F(MessageUtilsTest, GetPaiReturnsNullIfNoHeader)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_STREQ("", objMessageUtils.GetPai(*piMessage).GetStr());
}

TEST_F(MessageUtilsTest, GetPaiWithTopmostPreferredConfiguration)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_MULTIPLE_P_ASSERTED_IDENTITY_HEADERS_INT))
            .WillByDefault(Return(ConfigVoice::PAI_POLICY_PREFER_TOPMOST));

    ImsList<AString> objHeaders;
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_STREQ("<tel:12345;anyuriparam>;anyparam", objMessageUtils.GetPai(*piMessage).GetStr());
}

TEST_F(MessageUtilsTest, GetPaiWithSipPreferredConfigurationReturnsSipUri)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_MULTIPLE_P_ASSERTED_IDENTITY_HEADERS_INT))
            .WillByDefault(Return(ConfigVoice::PAI_POLICY_PREFER_SIP_URI));

    ImsList<AString> objHeaders;
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    objHeaders.Append("<sip:anyname1;userparam@ims.google.com>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_STREQ("<sip:anyname1;userparam@ims.google.com>;anyheaderparam",
            objMessageUtils.GetPai(*piMessage).GetStr());
}

TEST_F(MessageUtilsTest, GetPaiWithSipPreferredConfigurationReturnsSipsUri)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_MULTIPLE_P_ASSERTED_IDENTITY_HEADERS_INT))
            .WillByDefault(Return(ConfigVoice::PAI_POLICY_PREFER_SIP_URI));

    ImsList<AString> objHeaders;
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    objHeaders.Append("<sips:anyname1;userparam@ims.google.com>;anyheaderparam");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_STREQ("<sips:anyname1;userparam@ims.google.com>;anyheaderparam",
            objMessageUtils.GetPai(*piMessage).GetStr());
}

TEST_F(MessageUtilsTest, GetPaiWithSipPreferredConfigurationReturnsNullIfNotFound)
{
    ON_CALL(objConfigurationProxy,
            GetInt(ConfigVoice::KEY_POLICY_FOR_MULTIPLE_P_ASSERTED_IDENTITY_HEADERS_INT))
            .WillByDefault(Return(ConfigVoice::PAI_POLICY_PREFER_SIP_URI));

    ImsList<AString> objHeaders;
    objHeaders.Append("<tel:12345;anyuriparam>;anyparam");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_STREQ("", objMessageUtils.GetPai(*piMessage).GetStr());
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
    AString strHost("host");
    AString strContentId = objMessageUtils.GenerateContentId(strHost);
    EXPECT_TRUE(strContentId.Contains(strHost));

    AString strEmptyHost;
    strContentId = objMessageUtils.GenerateContentId(strEmptyHost);
    EXPECT_FALSE(strContentId.Contains(strHost));
}

TEST_F(MessageUtilsTest, SetResourceListWithoutDialogId)
{
    ImsList<ConfUser*> lstConfUser;
    ConfUser objUser1;
    objUser1.strTarget = "sip:user1Target";
    objUser1.eCcType = COPYCONTROLTYPE_TO;
    ConfUser objUser2;
    objUser2.strTarget = "sip:user2Target";
    objUser2.eCcType = COPYCONTROLTYPE_CC;
    ConfUser objUser3;
    objUser3.strTarget = "sip:user3Target";
    objUser3.eCcType = COPYCONTROLTYPE_BCC;
    objUser3.bAnonymize = IMS_TRUE;
    lstConfUser.Append(&objUser1);
    lstConfUser.Append(&objUser2);
    lstConfUser.Append(&objUser3);

    EXPECT_EQ(objMessageUtils.SetResourceList(IMS_NULL, lstConfUser, IMS_FALSE, IMS_TRUE),
            IMS_FAILURE);

    ON_CALL(*piMessage, CreateBodyPart).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.SetResourceList(piMessage, lstConfUser, IMS_FALSE, IMS_TRUE),
            IMS_FAILURE);

    MockIMessageBodyPart objMessageBodyPart;
    ON_CALL(*piMessage, CreateBodyPart).WillByDefault(Return(&objMessageBodyPart));

    const AString strContentType(SipHeaderName::CONTENT_TYPE);
    const AString strResourceList(MessageUtil::STR_CONTENT_TYPE_RESOURCE_LISTS_XML);
    const AString strDisposition(SipHeaderName::CONTENT_DISPOSITION);
    const AString strRecipientList(MessageUtil::STR_CONTENT_DISPOSITION_RECIPIENT_LIST);
    const AString strLength(SipHeaderName::CONTENT_LENGTH);
    const AString strContentId(MessageUtil::STR_CONTENT_ID);

    EXPECT_CALL(objMessageBodyPart, SetHeader(strContentType, strResourceList));
    EXPECT_CALL(objMessageBodyPart, SetHeader(strDisposition, strRecipientList));
    EXPECT_CALL(objMessageBodyPart, SetHeader(strLength, _));

    ImsList<AString> objResourceList;
    objResourceList.Append("entry uri=\"sip:user1Target\" cp:copyControl=\"to\"");
    objResourceList.Append("entry uri=\"sip:user2Target\" cp:copyControl=\"cc\"");
    objResourceList.Append(
            "entry uri=\"sip:user3Target\" cp:copyControl=\"bcc\" cp:anonymize=\"true\"");
    EXPECT_CALL(objMessageBodyPart, SetContent(IsEqualResourceList(objResourceList)));

    EXPECT_EQ(objMessageUtils.SetResourceList(piMessage, lstConfUser, IMS_FALSE, IMS_TRUE),
            IMS_SUCCESS);
}

TEST_F(MessageUtilsTest, SetResourceListWithDialogId)
{
    // Sets up to get a ISession using a ConfUser#nCallConnectionId
    MockICallStateProxy objStateProxy;
    ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objStateProxy));
    MockCallConnectionIdManager objIdManager(objContext);
    ON_CALL(objContext, GetCallConnectionIdManager).WillByDefault(ReturnRef(objIdManager));
    ON_CALL(objIdManager, GetCallKey(1)).WillByDefault(Return(1));
    MockIMtcCallManager objCallManager;
    ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
    MockIMtcCall objCall;
    ON_CALL(objCallManager, GetCallByCallKey(1)).WillByDefault(Return(&objCall));
    MockIMtcCallContext objCallContext;
    ON_CALL(objCall, GetCallContext).WillByDefault(ReturnRef(objCallContext));
    MockIMtcSession objMtcSession;
    ON_CALL(objCallContext, GetSession()).WillByDefault(Return(&objMtcSession));
    ON_CALL(objMtcSession, GetISession).WillByDefault(ReturnRef(*piSession));

    MockIMessage objConfirmedMessage;
    ON_CALL(*piSession, GetPreviousResponse(IMessage::SESSION_START))
            .WillByDefault(Return(&objConfirmedMessage));

    CallInfo objCallInfo;
    objCallInfo.ePeerType = PeerType::MO;
    ON_CALL(objCallContext, GetCallInfo).WillByDefault(ReturnRef(objCallInfo));

    ImsList<ConfUser*> lstConfUser;
    ConfUser objUser1;
    objUser1.nConnectionId = 1;
    lstConfUser.Append(&objUser1);

    // GetRemoteUri
    ImsList<AString> objAddresses;
    ON_CALL(*piSession, GetRemoteUserId).WillByDefault(Return(objAddresses));
    AString strSomeUri = "sip:someUri";
    ImsList<AString> objHeaders;
    objHeaders.Append(strSomeUri);
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objHeaders));
    ON_CALL(*piSession, GetPreviousRequest(IMessage::SESSION_START))
            .WillByDefault(Return(piMessage));

    MockISipMessage objSipMessage;
    ON_CALL(objConfirmedMessage, GetMessage).WillByDefault(Return(&objSipMessage));

    // GetHeader for CALL_ID
    ImsList<AString> objCallIdHeaders;
    objCallIdHeaders.Append("someCallId");
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::CALL_ID, _))
            .WillByDefault(Return(objCallIdHeaders));

    // GetHeader for FROM
    ImsList<AString> objFromHeaders;
    objFromHeaders.Append("sip:someFrom;tag=fromtag");
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::FROM, _)).WillByDefault(Return(objFromHeaders));

    // GetHeader for TO
    ImsList<AString> objToHeaders;
    objToHeaders.Append("sip:someTo;tag=totag");
    ON_CALL(objSipMessage, GetHeaders(ISipHeader::TO, _)).WillByDefault(Return(objToHeaders));

    // GetHeader for Session-ID
    ImsList<AString> objSessionIdHeaders;
    objSessionIdHeaders.Append("abcdef123456");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::SESSION_ID, _))
            .WillByDefault(Return(objSessionIdHeaders));

    EXPECT_EQ(objMessageUtils.SetResourceList(IMS_NULL, lstConfUser, IMS_TRUE, IMS_TRUE),
            IMS_FAILURE);

    ON_CALL(*piMessage, CreateBodyPart).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.SetResourceList(piMessage, lstConfUser, IMS_TRUE, IMS_TRUE),
            IMS_FAILURE);

    MockIMessageBodyPart objMessageBodyPart;
    ON_CALL(*piMessage, CreateBodyPart).WillByDefault(Return(&objMessageBodyPart));

    ImsList<AString> objResourceList;
    AString strEntry("entry uri=\"<sip:someUri?Call-ID=someCallId>&amp;");
    strEntry += "From=%3Csip%3AsomeFrom%3E%3Btag%3Dfromtag&amp;";
    strEntry += "To=%3Csip%3AsomeTo%3E%3Btag%3Dtotag&amp;";
    strEntry += "Session-ID=abcdef123456\"";
    strEntry += " cp:copyControl=\"to\"";
    objResourceList.Append(strEntry);

    EXPECT_CALL(objMessageBodyPart, SetContent(IsEqualResourceList(objResourceList)));

    EXPECT_EQ(objMessageUtils.SetResourceList(piMessage, lstConfUser, IMS_TRUE, IMS_TRUE),
            IMS_SUCCESS);
}

TEST_F(MessageUtilsTest, IsMediaFeaturesIncludedReturnsNulloptIfNoContactHeader)
{
    ImsList<AString> objHeaders;
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(std::nullopt, objMessageUtils.IsMmtelFeatureIncluded(piMessage));
    EXPECT_EQ(std::nullopt, objMessageUtils.IsVideoFeatureIncluded(piMessage));
    EXPECT_EQ(std::nullopt, objMessageUtils.IsTextFeatureIncluded(piMessage));
}

TEST_F(MessageUtilsTest, IsMmtelFeatureIncludedReturnsFalseIfNoMmtelTag)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyUri>");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(IMS_FALSE, objMessageUtils.IsMmtelFeatureIncluded(piMessage));
}

TEST_F(MessageUtilsTest, IsVideoTextFeatureIncludedReturnsNulloptIfNoMmtelTag)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyUri>;video;text");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(std::nullopt, objMessageUtils.IsVideoFeatureIncluded(piMessage));
    EXPECT_EQ(std::nullopt, objMessageUtils.IsTextFeatureIncluded(piMessage));
}

TEST_F(MessageUtilsTest, IsMediaFeaturesIncludedReturnsTrueIfHasTag)
{
    AString strHeader;
    ImsList<AString> objHeaders;
    objHeaders.Append("<sip:anyUri>;video;text;+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims."
                      "icsi.mmtel\"");
    ON_CALL(*piSipMessage, GetHeaders(ISipHeader::CONTACT_NORMAL, _))
            .WillByDefault(Return(objHeaders));

    EXPECT_EQ(IMS_TRUE, objMessageUtils.IsMmtelFeatureIncluded(piMessage));
    EXPECT_EQ(IMS_TRUE, objMessageUtils.IsVideoFeatureIncluded(piMessage));
    EXPECT_EQ(IMS_TRUE, objMessageUtils.IsTextFeatureIncluded(piMessage));
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsUnknownIfNoSdp)
{
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::UNKNOWN);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsUnknownIfNoDescriptor)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;
    MockIMedia objMedia;
    lstIMedia.Append(&objMedia);
    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    ON_CALL(objMedia, GetProposal).WillByDefault(Return(&objMedia));
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_TRUE), CallType::UNKNOWN);

    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_MODIFIED));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_TRUE), CallType::UNKNOWN);

    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::UNKNOWN);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsUnknownIfNoSdpMedia)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;
    MockIMedia objMedia;
    lstIMedia.Append(&objMedia);
    MockIMediaDescriptor objMediaDescriptor;
    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_REMOVED));

    ON_CALL(objMediaDescriptor, GetMediaDescriptionEx).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_TRUE), CallType::UNKNOWN);

    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal).WillByDefault(Return(nullptr));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::UNKNOWN);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsUnknownIfAudioPortIsInvalid)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;
    MockIMedia objMedia;
    lstIMedia.Append(&objMedia);
    MockIMediaDescriptor objMediaDescriptor;
    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    SdpMedia objSdpMedia;
    objSdpMedia.SetPort(0);
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal).WillByDefault(Return(&objSdpMedia));

    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::UNKNOWN);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsVoip)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;
    MockIMedia objMedia;
    lstIMedia.Append(&objMedia);
    MockIMediaDescriptor objMediaDescriptor;
    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objMediaDescriptor));
    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objSdpMedia.SetPort(12345);
    ON_CALL(objMediaDescriptor, GetMediaDescriptionExAsLocal).WillByDefault(Return(&objSdpMedia));

    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::VOIP);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsVt)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;

    MockIMedia objAudioMedia;
    lstIMedia.Append(&objAudioMedia);
    MockIMediaDescriptor objAudioMediaDescriptor;
    ON_CALL(objAudioMedia, GetMediaDescriptor).WillByDefault(Return(&objAudioMediaDescriptor));
    SdpMedia objAudioSdpMedia;
    objAudioSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objAudioSdpMedia.SetPort(12345);
    ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objAudioSdpMedia));

    MockIMedia objVideoMedia;
    lstIMedia.Append(&objVideoMedia);
    MockIMediaDescriptor objVideoMediaDescriptor;
    ON_CALL(objVideoMedia, GetMediaDescriptor).WillByDefault(Return(&objVideoMediaDescriptor));
    SdpMedia objVideoSdpMedia;
    objVideoSdpMedia.SetType(SdpMedia::TYPE_VIDEO);
    objVideoSdpMedia.SetPort(12345);
    ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objVideoSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::VT);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsRtt)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;

    MockIMedia objAudioMedia;
    lstIMedia.Append(&objAudioMedia);
    MockIMediaDescriptor objAudioMediaDescriptor;
    ON_CALL(objAudioMedia, GetMediaDescriptor).WillByDefault(Return(&objAudioMediaDescriptor));
    SdpMedia objAudioSdpMedia;
    objAudioSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objAudioSdpMedia.SetPort(12345);
    ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objAudioSdpMedia));

    MockIMedia objTextMedia;
    lstIMedia.Append(&objTextMedia);
    MockIMediaDescriptor objTextMediaDescriptor;
    ON_CALL(objTextMedia, GetMediaDescriptor).WillByDefault(Return(&objTextMediaDescriptor));
    SdpMedia objTextSdpMedia;
    objTextSdpMedia.SetType(SdpMedia::TYPE_TEXT);
    objTextSdpMedia.SetPort(12345);
    ON_CALL(objTextMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objTextSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::RTT);
}

TEST_F(MessageUtilsTest, GetCallTypeReturnsVideoRtt)
{
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;

    MockIMedia objAudioMedia;
    lstIMedia.Append(&objAudioMedia);
    MockIMediaDescriptor objAudioMediaDescriptor;
    ON_CALL(objAudioMedia, GetMediaDescriptor).WillByDefault(Return(&objAudioMediaDescriptor));
    SdpMedia objAudioSdpMedia;
    objAudioSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objAudioSdpMedia.SetPort(12345);
    ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objAudioSdpMedia));

    MockIMedia objVideoMedia;
    lstIMedia.Append(&objVideoMedia);
    MockIMediaDescriptor objVideoMediaDescriptor;
    ON_CALL(objVideoMedia, GetMediaDescriptor).WillByDefault(Return(&objVideoMediaDescriptor));
    SdpMedia objVideoSdpMedia;
    objVideoSdpMedia.SetType(SdpMedia::TYPE_VIDEO);
    objVideoSdpMedia.SetPort(12345);
    ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objVideoSdpMedia));

    MockIMedia objTextMedia;
    lstIMedia.Append(&objTextMedia);
    MockIMediaDescriptor objTextMediaDescriptor;
    ON_CALL(objTextMedia, GetMediaDescriptor).WillByDefault(Return(&objTextMediaDescriptor));
    SdpMedia objTextSdpMedia;
    objTextSdpMedia.SetType(SdpMedia::TYPE_TEXT);
    objTextSdpMedia.SetPort(12345);
    ON_CALL(objTextMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objTextSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));
    EXPECT_EQ(objMessageUtils.GetCallType(piMessage, piSession, IMS_FALSE), CallType::VIDEO_RTT);
}

TEST_F(MessageUtilsTest, GetCallTypeFromSdpWithActiveMediaOnly)
{
    // bNegoSdp=false case is done by GetCallType
    MockISipMessageBodyPart objBodyPart;
    ON_CALL(*piSipMessage, GetSdpBodyPart()).WillByDefault(Return(&objBodyPart));

    ImsList<IMedia*> lstIMedia;

    MockIMedia objAudioMedia;
    lstIMedia.Append(&objAudioMedia);
    MockIMediaDescriptor objAudioMediaDescriptor;
    ON_CALL(objAudioMedia, GetMediaDescriptor).WillByDefault(Return(&objAudioMediaDescriptor));
    SdpMedia objAudioSdpMedia;
    objAudioSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objAudioSdpMedia));

    MockIMedia objVideoMedia;
    lstIMedia.Append(&objVideoMedia);
    MockIMediaDescriptor objVideoMediaDescriptor;
    ON_CALL(objVideoMedia, GetState).WillByDefault(Return(IMedia::STATE_DELETED));
    ON_CALL(objVideoMedia, GetMediaDescriptor).WillByDefault(Return(&objVideoMediaDescriptor));
    SdpMedia objVideoSdpMedia;
    objVideoSdpMedia.SetType(SdpMedia::TYPE_VIDEO);
    ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objVideoSdpMedia));

    MockIMedia objTextMedia;
    lstIMedia.Append(&objTextMedia);
    MockIMediaDescriptor objTextMediaDescriptor;
    ON_CALL(objTextMedia, GetState).WillByDefault(Return(IMedia::STATE_DELETED));
    ON_CALL(objTextMedia, GetMediaDescriptor).WillByDefault(Return(&objTextMediaDescriptor));
    SdpMedia objTextSdpMedia;
    objTextSdpMedia.SetType(SdpMedia::TYPE_TEXT);
    ON_CALL(objTextMediaDescriptor, GetMediaDescriptionExAsLocal)
            .WillByDefault(Return(&objTextSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));
    EXPECT_EQ(objMessageUtils.GetCallTypeFromSdp(piSession, IMS_TRUE, IMS_FALSE, IMS_FALSE),
            CallType::VOIP);
}

TEST_F(MessageUtilsTest, GetRemotePortFromSdpReturnsCorrectPort)
{
    ImsList<IMedia*> lstIMedia;

    // Setup Audio Media
    MockIMedia objAudioMedia;
    lstIMedia.Append(&objAudioMedia);
    MockIMediaDescriptor objAudioMediaDescriptor;
    ON_CALL(objAudioMedia, GetMediaDescriptor).WillByDefault(Return(&objAudioMediaDescriptor));

    SdpMedia objAudioSdpMedia;
    objAudioSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objAudioSdpMedia.SetPort(12345);
    // Use GetMediaDescriptionEx to simulate Remote SDP
    ON_CALL(objAudioMediaDescriptor, GetMediaDescriptionEx)
            .WillByDefault(Return(&objAudioSdpMedia));

    // Setup Video Media
    MockIMedia objVideoMedia;
    lstIMedia.Append(&objVideoMedia);
    MockIMediaDescriptor objVideoMediaDescriptor;
    ON_CALL(objVideoMedia, GetMediaDescriptor).WillByDefault(Return(&objVideoMediaDescriptor));

    SdpMedia objVideoSdpMedia;
    objVideoSdpMedia.SetType(SdpMedia::TYPE_VIDEO);
    objVideoSdpMedia.SetPort(54321);
    ON_CALL(objVideoMediaDescriptor, GetMediaDescriptionEx)
            .WillByDefault(Return(&objVideoSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    EXPECT_EQ(objMessageUtils.GetRemotePortFromSdp(piSession, SdpMedia::TYPE_AUDIO), 12345);

    EXPECT_EQ(objMessageUtils.GetRemotePortFromSdp(piSession, SdpMedia::TYPE_VIDEO), 54321);
}

TEST_F(MessageUtilsTest, GetRemotePortFromSdpHandlesModifiedState)
{
    ImsList<IMedia*> lstIMedia;
    MockIMedia objMedia;
    lstIMedia.Append(&objMedia);

    // UPDATE_MODIFIED state
    ON_CALL(objMedia, GetUpdateState).WillByDefault(Return(IMedia::UPDATE_MODIFIED));

    // Setup Proposal Media
    MockIMedia objProposalMedia;
    ON_CALL(objMedia, GetProposal).WillByDefault(Return(&objProposalMedia));

    MockIMediaDescriptor objProposalDescriptor;
    ON_CALL(objProposalMedia, GetMediaDescriptor).WillByDefault(Return(&objProposalDescriptor));

    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objSdpMedia.SetPort(12345);
    ON_CALL(objProposalDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    EXPECT_EQ(objMessageUtils.GetRemotePortFromSdp(piSession, SdpMedia::TYPE_AUDIO), 12345);
}

TEST_F(MessageUtilsTest, GetRemotePortFromSdpReturnsZeroIfNotFound)
{
    ImsList<IMedia*> lstIMedia;

    // Setup Media with Port 0
    MockIMedia objMedia;
    lstIMedia.Append(&objMedia);
    MockIMediaDescriptor objDescriptor;
    ON_CALL(objMedia, GetMediaDescriptor).WillByDefault(Return(&objDescriptor));

    SdpMedia objSdpMedia;
    objSdpMedia.SetType(SdpMedia::TYPE_AUDIO);
    objSdpMedia.SetPort(0);
    ON_CALL(objDescriptor, GetMediaDescriptionEx).WillByDefault(Return(&objSdpMedia));

    ON_CALL(*piSession, GetMedia).WillByDefault(Return(lstIMedia));

    EXPECT_EQ(objMessageUtils.GetRemotePortFromSdp(piSession, SdpMedia::TYPE_AUDIO), 0);

    // Verify returns -1 for non-existent media type
    EXPECT_EQ(objMessageUtils.GetRemotePortFromSdp(piSession, SdpMedia::TYPE_VIDEO), -1);
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

TEST_F(MessageUtilsTest, GetNumberOfPreviousResponses)
{
    EXPECT_EQ(0, objMessageUtils.GetNumberOfPreviousResponses(IMS_NULL, ANY_METHOD));

    SetUpPreviousResponses();
    EXPECT_EQ(objMessages.GetSize(),
            objMessageUtils.GetNumberOfPreviousResponses(piSession, ANY_METHOD));
}

}  // namespace android
