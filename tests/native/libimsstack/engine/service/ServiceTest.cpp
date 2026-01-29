/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "AString.h"
#include "AStringArray.h"
#include "IpAddress.h"
#include "PlatformContext.h"

#include "TestCoreService.h"
#include "TestThreadService.h"

#include "Service.h"
#include "Sip.h"
#include "SipAddress.h"

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class ServiceTest : public ::testing::Test
{
public:
    inline ServiceTest() :
            m_pThreadService(new TestThreadService()),
            m_pCoreService(IMS_NULL)
    {
        m_strUserInfo = "1234";
        m_nPortS = 39001;
        AString strIpAddr("192.168.0.1");
        m_objIpAddress.Parse(strIpAddr);
        AString strContactAddress;
        strContactAddress.Sprintf(
                "sip:contact-%s@%s:%d", m_strUserInfo.GetStr(), strIpAddr.GetStr(), m_nPortS);
        m_objContactAddress.Create(strContactAddress);

        m_strImpu1.Sprintf("sip:%s@test.ims.com", m_strUserInfo.GetStr());
        m_strImpu2.Sprintf("tel:%s", m_strUserInfo.GetStr());
        m_objAssociatedUris.AddElement(m_strImpu1);
        m_objAssociatedUris.AddElement(m_strImpu2);
    }

protected:
    virtual void SetUp() override
    {
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);

        m_pCoreService = new TestCoreService();
    }

    virtual void TearDown() override
    {
        delete m_pCoreService;

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }

    void SetUpRegBinding()
    {
        MockIRegBinding& objRegBinding = m_pCoreService->GetMockRegBinding();
        ON_CALL(objRegBinding, GetPortUc).WillByDefault(Return(38001));
        ON_CALL(objRegBinding, GetPortUs).WillByDefault(Return(39001));
        ON_CALL(objRegBinding, GetPortFlowControl).WillByDefault(Return(Sip::PORT_UNSPECIFIED));
        ON_CALL(objRegBinding, GetTransportExt).WillByDefault(Return(Sip::TRANSPORT_EXT_TCP));
        ON_CALL(objRegBinding, GetIpAddress).WillByDefault(ReturnRef(m_objIpAddress));
        ON_CALL(objRegBinding, GetContactAddress).WillByDefault(ReturnRef(m_objContactAddress));
        ON_CALL(objRegBinding, GetContactAddressForOutgoingMessage).WillByDefault(Return(IMS_NULL));
        ON_CALL(objRegBinding, GetSecurityClients)
                .WillByDefault(ReturnRef(AStringArray::ConstNull()));
        ON_CALL(objRegBinding, GetSecurityVerifys)
                .WillByDefault(ReturnRef(AStringArray::ConstNull()));
        ON_CALL(objRegBinding, GetPublicGruu).WillByDefault(Return(IMS_NULL));
        ON_CALL(objRegBinding, GetTemporaryGruu).WillByDefault(Return(IMS_NULL));
        ON_CALL(objRegBinding, GetAssociatedUris).WillByDefault(ReturnRef(m_objAssociatedUris));
        ON_CALL(objRegBinding, GetSipProfile).WillByDefault(Return(IMS_NULL));
        ON_CALL(objRegBinding, IsEmergencyRegistration).WillByDefault(Return(IMS_FALSE));
    }

protected:
    TestThreadService* m_pThreadService;
    TestCoreService* m_pCoreService;

    IMS_SINT32 m_nPortS;
    AString m_strUserInfo;
    AString m_strImpu1;
    AString m_strImpu2;
    AStringArray m_objAssociatedUris;
    IpAddress m_objIpAddress;
    SipAddress m_objContactAddress;
};

TEST_F(ServiceTest, ValidateRequestUri)
{
    SetUpRegBinding();

    SipAddress objAor(m_strImpu1);
    m_pCoreService->MarkAsRegistrationActive(objAor);

    // Contact address
    SipAddress objRequestUri(m_objContactAddress);
    IMS_BOOL bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_TRUE(bResult);

    // IMPU1
    objRequestUri.Create(m_strImpu1);
    bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_TRUE(bResult);

    // IMPU2
    objRequestUri.Create(m_strImpu2);
    bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_TRUE(bResult);

    // "user-info" of IMPU1 & IP/port
    AString strTemp;
    strTemp.Sprintf(
            "sip:%s@%s:%d", m_strUserInfo.GetStr(), m_objIpAddress.ToString().GetStr(), m_nPortS);
    objRequestUri.Create(strTemp);
    bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_TRUE(bResult);

    // IP/port
    strTemp.Sprintf("sip:%s:%d", m_objIpAddress.ToString().GetStr(), m_nPortS);
    objRequestUri.Create(strTemp);
    bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_TRUE(bResult);

    // Unknown "user-info"
    strTemp.Sprintf("sip:5678@%s:%d", m_objIpAddress.ToString().GetStr(), m_nPortS);
    objRequestUri.Create(strTemp);
    bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_FALSE(bResult);

    // Unknown IP/port
    objRequestUri.Create("sip:192.168.0.5:5060");
    bResult = m_pCoreService->ValidateRequestUri(objRequestUri);
    EXPECT_FALSE(bResult);
}

}  // namespace android
