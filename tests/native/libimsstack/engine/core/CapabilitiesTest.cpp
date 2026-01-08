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

#include "Capabilities.h"
#include "TestCoreBase.h"

using ::testing::_;
using ::testing::Return;

namespace android
{

class CapabilitiesTest : public TestCoreBase
{
public:
    inline CapabilitiesTest() :
            m_pCapabilities(IMS_NULL)
    {
    }
    inline ~CapabilitiesTest() override
    {
        if (m_pCapabilities != IMS_NULL)
        {
            delete m_pCapabilities;
        }
    }

protected:
    virtual void SetUp() override
    {
        TestCoreBase::SetUp();

        m_pCapabilities = new Capabilities(GetCoreService());
        InitMethod(m_pCapabilities);
    }

    virtual void TearDown() override
    {
        if (m_pCapabilities != IMS_NULL)
        {
            delete m_pCapabilities;
            m_pCapabilities = IMS_NULL;
        }

        TestCoreBase::TearDown();
    }

    void SetUpServerConnection() override
    {
        TestCoreBase::SetUpServerConnection();
        m_pCapabilities->ServerConnection_NotifyRequest(&GetSsc());
    }

protected:
    Capabilities* m_pCapabilities;
};

TEST_F(CapabilitiesTest, QueryCapabilitiesOnServiceNotConnected)
{
    EXPECT_EQ(m_pCapabilities->QueryCapabilities(), IMS_FAILURE);
}

TEST_F(CapabilitiesTest, QueryCapabilitiesOnStatePending)
{
    SetUpClientConnection();

    EXPECT_EQ(m_pCapabilities->QueryCapabilities(Capabilities::FLAG_NONE), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_PENDING);
    EXPECT_EQ(m_pCapabilities->QueryCapabilities(Capabilities::FLAG_NONE), IMS_FAILURE);
}

TEST_F(CapabilitiesTest, QueryCapabilitiesWithNoFlags)
{
    SetUpClientConnection();

    EXPECT_CALL(GetSipMsg(), SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);
    EXPECT_CALL(GetSipMsg(), CreateSdpBodyPart()).Times(0);

    EXPECT_EQ(m_pCapabilities->QueryCapabilities(Capabilities::FLAG_NONE), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_PENDING);
}

TEST_F(CapabilitiesTest, QueryCapabilitiesWithContactHeader)
{
    SetUpClientConnection();

    EXPECT_CALL(GetSipMsg(), SetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(
            m_pCapabilities->QueryCapabilities(Capabilities::FLAG_ADD_CONTACT_HEADER), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_PENDING);
}

TEST_F(CapabilitiesTest, QueryCapabilitiesWithSdpBodyPart)
{
    const AString strSdpContentType(Capabilities::DEFAULT_MEDIA_TYPE);
    SetUpClientConnection();

    EXPECT_CALL(GetSipMsg(), CreateSdpBodyPart()).WillOnce(Return(&GetSipMsgBodyPart()));
    EXPECT_CALL(
            GetSipMsgBodyPart(), SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strSdpContentType, _))
            .WillOnce(Return());
    EXPECT_CALL(GetSipMsgBodyPart(), SetContent(_)).WillOnce(Return());

    EXPECT_EQ(
            m_pCapabilities->QueryCapabilities(Capabilities::FLAG_ADD_SDP_BODY_PART), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_PENDING);
}

TEST_F(CapabilitiesTest, QueryCapabilitiesWithSdpBodyPartAndCheckMediaCapabilities)
{
    const AString strSdpContentType(Capabilities::DEFAULT_MEDIA_TYPE);
    SetUpClientConnection();

    EXPECT_CALL(GetSipMsg(), CreateSdpBodyPart()).WillOnce(Return(&GetSipMsgBodyPart()));
    EXPECT_CALL(
            GetSipMsgBodyPart(), SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strSdpContentType, _))
            .WillOnce(Return());
    EXPECT_CALL(GetSipMsgBodyPart(), SetContent(_)).WillOnce(Return());

    EXPECT_EQ(m_pCapabilities->QueryCapabilities(Capabilities::FLAG_ADD_SDP_BODY_PART |
                      Capabilities::FLAG_CHECK_MEDIA_CAPABILITIES),
            IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_PENDING);
}

TEST_F(CapabilitiesTest, AcceptOnServiceNotConnected)
{
    EXPECT_EQ(m_pCapabilities->Accept(), IMS_FAILURE);
}

TEST_F(CapabilitiesTest, AcceptOnSipServerConnectionNotFound)
{
    GetCoreService()->MarkAsImsConnected(IMS_TRUE);
    EXPECT_EQ(m_pCapabilities->Accept(), IMS_FAILURE);
}

TEST_F(CapabilitiesTest, AcceptOnStateActive)
{
    SetUpServerConnection();

    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_NONE), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_ACTIVE);
    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_NONE), IMS_FAILURE);
}

TEST_F(CapabilitiesTest, AcceptWithNoFlags)
{
    SetUpServerConnection();

    EXPECT_CALL(GetSipMsg(), SetHeader(ISipHeader::CONTACT_NORMAL, _, _)).Times(0);
    EXPECT_CALL(GetSipMsg(), CreateSdpBodyPart()).Times(0);

    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_NONE), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_ACTIVE);
}

TEST_F(CapabilitiesTest, AcceptWithContactHeader)
{
    SetUpServerConnection();

    EXPECT_CALL(GetSipMsg(), SetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_ADD_CONTACT_HEADER), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_ACTIVE);
}

TEST_F(CapabilitiesTest, AcceptWithContactHeaderAndAllRegisteredFeatures)
{
    SetUpServerConnection();

    EXPECT_CALL(GetSipMsg(), SetHeader(ISipHeader::CONTACT_NORMAL, _, _))
            .WillOnce(Return(IMS_SUCCESS));

    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_ADD_CONTACT_HEADER |
                      Capabilities::FLAG_ADD_ALL_REGISTERED_FEATURES_IN_CONTACT_HEADER),
            IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_ACTIVE);
}

TEST_F(CapabilitiesTest, AcceptWithSdpBodyPart)
{
    const AString strSdpContentType(Capabilities::DEFAULT_MEDIA_TYPE);
    SetUpServerConnection();

    EXPECT_CALL(GetSipMsg(), PrependHeader(ISipHeader::ACCEPT, strSdpContentType, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(GetSipMsg(), CreateSdpBodyPart()).WillOnce(Return(&GetSipMsgBodyPart()));
    EXPECT_CALL(
            GetSipMsgBodyPart(), SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strSdpContentType, _))
            .WillOnce(Return());
    EXPECT_CALL(GetSipMsgBodyPart(), SetContent(_)).WillOnce(Return());

    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_ADD_SDP_BODY_PART), IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_ACTIVE);
}

TEST_F(CapabilitiesTest, AcceptWithSdpBodyPartAndCheckMediaCapabilities)
{
    const AString strSdpContentType(Capabilities::DEFAULT_MEDIA_TYPE);
    SetUpServerConnection();

    EXPECT_CALL(GetSipMsg(), PrependHeader(ISipHeader::ACCEPT, strSdpContentType, _))
            .WillOnce(Return(IMS_SUCCESS));
    EXPECT_CALL(GetSipMsg(), CreateSdpBodyPart()).WillOnce(Return(&GetSipMsgBodyPart()));
    EXPECT_CALL(
            GetSipMsgBodyPart(), SetHeader(ISipMessageBodyPart::CONTENT_TYPE, strSdpContentType, _))
            .WillOnce(Return());
    EXPECT_CALL(GetSipMsgBodyPart(), SetContent(_)).WillOnce(Return());

    EXPECT_EQ(m_pCapabilities->Accept(Capabilities::FLAG_ADD_SDP_BODY_PART |
                      Capabilities::FLAG_CHECK_MEDIA_CAPABILITIES),
            IMS_SUCCESS);
    EXPECT_EQ(m_pCapabilities->GetState(), Capabilities::STATE_ACTIVE);
}

}  // namespace android
