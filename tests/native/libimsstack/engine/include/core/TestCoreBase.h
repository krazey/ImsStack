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

#include "IpAddress.h"
#include "TestThreadService.h"

#include "ISipHeader.h"
#include "ISipMessageBodyPart.h"
#include "MockISipDialog.h"
#include "MockISipMessage.h"
#include "MockISipMessageBodyPart.h"
#include "MockISipServerConnection.h"
#include "ServiceMethod.h"
#include "SipAddress.h"
#include "TestCoreService.h"

namespace android
{

class TestCoreBase : public ::testing::Test
{
public:
    TestCoreBase();
    ~TestCoreBase() override;

public:
    virtual void SetUpClientConnection(IN IMS_BOOL bMidDialog = IMS_FALSE);
    virtual void TearDownClientConnection();
    virtual void SetUpServerConnection();
    virtual void TearDownServerConnection();
    virtual void VerifyAndClear();

    void InitMethod(IN_OUT ServiceMethod* pMethod, IN IMS_BOOL bOriginated = IMS_TRUE);
    inline MockISipClientConnection& GetScc() { return m_objScc; }
    inline MockISipServerConnection& GetSsc() { return m_objSsc; }
    inline MockISipDialog& GetDialog() { return m_objDialog; }
    inline MockISipMessage& GetSipMsg() { return m_objSipMsg; }
    inline MockISipMessageBodyPart& GetSipMsgBodyPart() { return m_objSipMsgBodyPart; }
    inline TestCoreService* GetCoreService() { return m_pCoreService; }
    inline virtual const SipAddress& GetContactAddress() const { return m_objContactAddress; }
    inline virtual const IpAddress& GetIpAddress() const { return m_objIpAddress; }
    inline virtual const AString& GetLocalUserId() const { return m_strLocalUserId; }
    inline virtual const AString& GetRemoteUserId() const { return m_strRemoteUserId; }
    inline virtual const SipAddress& GetDefaultUserId() const { return m_objDefaultUserId; }
    inline virtual const ImsList<AString>& GetAssertedIds() const { return m_objAssertedIds; }
    inline virtual const ImsList<ISipMessageBodyPart*>& GetSipMsgBodyParts() const
    {
        return m_objBodyParts;
    }
    inline virtual const SipMethod& GetMethodForSipConnection() const
    {
        return m_objMethodForSipConnection;
    }
    inline void SetMethodForSipConnection(IN const SipMethod& objMethod)
    {
        m_objMethodForSipConnection = objMethod;
    }

protected:
    void SetUp() override;
    void TearDown() override;

protected:
    TestThreadService* m_pThreadService;
    TestCoreService* m_pCoreService;
    MockISipClientConnection m_objScc;
    MockISipServerConnection m_objSsc;
    MockISipDialog m_objDialog;
    MockISipMessage m_objSipMsg;
    MockISipMessageBodyPart m_objSipMsgBodyPart;

    SipAddress m_objContactAddress;
    IpAddress m_objIpAddress;
    AString m_strLocalUserId;
    AString m_strRemoteUserId;
    SipAddress m_objDefaultUserId;
    ImsList<AString> m_objAssertedIds;
    SipMethod m_objMethodForSipConnection;
    ImsList<ISipMessageBodyPart*> m_objBodyParts;
};

}  // namespace android
