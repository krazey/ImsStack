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

#include "PlatformContext.h"

#include "private/ConfigurationManager.h"

#include "TestCoreBase.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

TestCoreBase::TestCoreBase() :
        m_pThreadService(new TestThreadService()),
        m_pCoreService(IMS_NULL)
{
    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, m_pThreadService);
    ConfigurationManager::GetInstance()->Initialize();

    m_pCoreService = new TestCoreService();

    m_objContactAddress.Create("sip:test@192.168.0.1:5060");
    m_objIpAddress.Parse("192.168.0.1");
    m_strLocalUserId = "sip:1234@test.ims.com";
    m_strRemoteUserId = "sip:5678@test.ims.com";
    m_objDefaultUserId.Create(m_strLocalUserId);
    m_objAssertedIds.Append("sip:1234@test.ims.com");
    m_objBodyParts.Append(&m_objSipMsgBodyPart);
}

TestCoreBase::~TestCoreBase()
{
    if (m_pCoreService != IMS_NULL)
    {
        delete m_pCoreService;
    }

    PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);

    if (m_pThreadService != IMS_NULL)
    {
        m_pThreadService->Destroy();
    }
}

void TestCoreBase::SetUpClientConnection(IN IMS_BOOL bMidDialog /*= IMS_FALSE*/)
{
    if (bMidDialog)
    {
        m_pCoreService->SetSccForMidDialog(&m_objScc);
    }
    else
    {
        m_pCoreService->SetScc(&m_objScc);
    }
    m_pCoreService->MarkAsImsConnected(IMS_TRUE);

    ON_CALL(m_objScc, Close()).WillByDefault(Return());
    ON_CALL(m_objScc, SetErrorListener(_)).WillByDefault(Return());
    ON_CALL(m_objScc, SetListener(_)).WillByDefault(Return());
    ON_CALL(m_objScc, GetDialog()).WillByDefault(Return(&m_objDialog));
    ON_CALL(m_objScc, GetMessage()).WillByDefault(Return(&m_objSipMsg));
    ON_CALL(m_objScc, GetMethod()).WillByDefault(ReturnRef(GetMethodForSipConnection()));
    ON_CALL(m_objScc, Send()).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(m_objSipMsg, GetMethod()).WillByDefault(ReturnRef(GetMethodForSipConnection()));
}

void TestCoreBase::TearDownClientConnection()
{
    m_pCoreService->SetScc(IMS_NULL);
}

void TestCoreBase::SetUpServerConnection()
{
    m_pCoreService->MarkAsImsConnected(IMS_TRUE);

    ON_CALL(m_objSsc, Close()).WillByDefault(Return());
    ON_CALL(m_objSsc, SetErrorListener(_)).WillByDefault(Return());
    ON_CALL(m_objSsc, GetMethod()).WillByDefault(ReturnRef(GetMethodForSipConnection()));
    ON_CALL(m_objSsc, GetMessage()).WillByDefault(Return(&m_objSipMsg));
    ON_CALL(m_objSsc, Send()).WillByDefault(Return(IMS_SUCCESS));

    ON_CALL(m_objSipMsg, GetMethod()).WillByDefault(ReturnRef(GetMethodForSipConnection()));
}

void TestCoreBase::TearDownServerConnection() {}

void TestCoreBase::VerifyAndClear()
{
    Mock::VerifyAndClear(&m_pCoreService->GetMockRegBinding());
    Mock::VerifyAndClear(&m_objScc);
    Mock::VerifyAndClear(&m_objSsc);
    Mock::VerifyAndClear(&m_objSipMsg);
    Mock::VerifyAndClear(&m_objSipMsgBodyPart);
}

void TestCoreBase::InitMethod(IN_OUT ServiceMethod* pMethod, IN IMS_BOOL bOriginated /*= IMS_TRUE*/)
{
    pMethod->InitMethod(m_strLocalUserId, m_strRemoteUserId, m_objDefaultUserId, bOriginated);
}

void TestCoreBase::SetUp()
{
    VerifyAndClear();

    ON_CALL(m_pCoreService->GetMockRegBinding(), GetContactAddress())
            .WillByDefault(ReturnRef(m_objContactAddress));
    ON_CALL(m_pCoreService->GetMockRegBinding(), GetContactAddressForOutgoingMessage())
            .WillByDefault(Return(&m_objContactAddress));
    ON_CALL(m_pCoreService->GetMockRegBinding(), GetIpAddress())
            .WillByDefault(ReturnRef(m_objIpAddress));
    ON_CALL(m_pCoreService->GetMockRegBinding(), GetServiceRoutes())
            .WillByDefault(ReturnRef(AStringArray::ConstNull()));

    ON_CALL(m_objSipMsg, Destroy()).WillByDefault(Return());
    ON_CALL(m_objSipMsg, Clone()).WillByDefault(Return(&m_objSipMsg));
    ON_CALL(m_objSipMsg, GetHeaders(ISipHeader::P_ASSERTED_IDENTITY, _))
            .WillByDefault(Return(m_objAssertedIds));
    ON_CALL(m_objSipMsg, GetBodyParts()).WillByDefault(Return(m_objBodyParts));
    ON_CALL(m_objSipMsgBodyPart, Clone()).WillByDefault(Return(&m_objSipMsgBodyPart));
}

void TestCoreBase::TearDown()
{
    TearDownClientConnection();
    TearDownServerConnection();

    m_pCoreService->MarkAsImsConnected(IMS_FALSE);
}

}  // namespace android
