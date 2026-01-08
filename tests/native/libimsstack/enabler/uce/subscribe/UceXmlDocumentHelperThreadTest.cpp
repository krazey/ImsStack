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
#include <gmock/gmock.h>
#include "subscribe/UceXmlDocumentHelperThread.h"
#include "MockIThread.h"
#include "MockIXmlTransaction.h"
#include "MockIXmlResponse.h"
#include "TestThreadService.h"
#include "PlatformContext.h"
#include "IXmlResponse.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnNull;

__IMS_TRACE_TAG_UCE__;

class TestUceXmlDocumentHelperThread : public UceXmlDocumentHelperThread
{
public:
    TestUceXmlDocumentHelperThread() :
            UceXmlDocumentHelperThread(AString("UceXmlDocumentHelperThread"), 0)
    {
    }
    virtual ~TestUceXmlDocumentHelperThread() override {}

    void setThread(IThread* piThread) { m_piThread = piThread; }
    IThread* getThread() { return GetThread(); }
    void push(IXmlTransaction* xmlTxn) { m_objTransactionQueue.Push(xmlTxn); }
};

class UceXmlDocumentHelperThreadTest : public ::testing::Test
{
public:
    inline UceXmlDocumentHelperThreadTest() :
            m_pThreadService(new TestThreadService()),
            objMockIThread(m_pThreadService->GetMockThread())
    {
        pUceXmlDocumentHelperThread = IMS_NULL;
    }
    TestThreadService* m_pThreadService;
    MockIThread& objMockIThread;
    TestUceXmlDocumentHelperThread* pUceXmlDocumentHelperThread;

protected:
    virtual void SetUp() override
    {
        pUceXmlDocumentHelperThread = new TestUceXmlDocumentHelperThread();
        ASSERT_TRUE(pUceXmlDocumentHelperThread != nullptr);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
    }

    virtual void TearDown() override
    {
        if (pUceXmlDocumentHelperThread)
        {
            delete pUceXmlDocumentHelperThread;
        }
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
    }
};

TEST_F(UceXmlDocumentHelperThreadTest, XmlTransaction_NotifyParsingCompletedWithNoMatched)
{
    IMS_TRACE_D("XmlTransaction_NotifyParsingCompletedWithNoMatched", 0, 0, 0);
    EXPECT_EQ(IMS_FAILURE,
            pUceXmlDocumentHelperThread->XmlTransaction_NotifyParsingCompleted(IMS_NULL));

    MockIXmlTransaction objMockIXmlTransaction;
    pUceXmlDocumentHelperThread->push(&objMockIXmlTransaction);
    EXPECT_EQ(IMS_FAILURE,
            pUceXmlDocumentHelperThread->XmlTransaction_NotifyParsingCompleted(IMS_NULL));
}

TEST_F(UceXmlDocumentHelperThreadTest, XmlTransaction_NotifyParsingCompleted)
{
    IMS_TRACE_D("XmlTransaction_NotifyParsingCompleted", 0, 0, 0);

    MockIXmlTransaction objMockIXmlTransaction;
    pUceXmlDocumentHelperThread->push(&objMockIXmlTransaction);

    ON_CALL(objMockIXmlTransaction, GetResponse).WillByDefault(ReturnNull());
    EXPECT_EQ(IMS_FAILURE,
            pUceXmlDocumentHelperThread->XmlTransaction_NotifyParsingCompleted(
                    &objMockIXmlTransaction));

    MockIXmlResponse objMockIXmlResponse;
    ON_CALL(objMockIXmlTransaction, GetResponse).WillByDefault(Return(&objMockIXmlResponse));
    ON_CALL(objMockIXmlResponse, GetResponseCode)
            .WillByDefault(Return(IXmlResponse::RESPONSE_CODE_SUCCESS));
    ON_CALL(objMockIXmlResponse, GetDocument).WillByDefault(ReturnNull());
    EXPECT_EQ(IMS_FAILURE,
            pUceXmlDocumentHelperThread->XmlTransaction_NotifyParsingCompleted(
                    &objMockIXmlTransaction));
}

TEST_F(UceXmlDocumentHelperThreadTest, StartWithActive)
{
    IMS_TRACE_D("StartWithActive", 0, 0, 0);
    EXPECT_CALL(objMockIThread, SetRunnable).Times(1);
    EXPECT_CALL(objMockIThread, Activate).Times(1).WillRepeatedly(Return(IMS_FALSE));
    EXPECT_FALSE(pUceXmlDocumentHelperThread->Start("Test"));
}

TEST_F(UceXmlDocumentHelperThreadTest, StartWithActiveFailed)
{
    IMS_TRACE_D("StartWithActiveFailed", 0, 0, 0);
    EXPECT_CALL(objMockIThread, SetRunnable).Times(1);
    EXPECT_CALL(objMockIThread, Activate).Times(1).WillRepeatedly(Return(IMS_TRUE));
    EXPECT_TRUE(pUceXmlDocumentHelperThread->Start("Test"));
}

TEST_F(UceXmlDocumentHelperThreadTest, Terminate)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);
    EXPECT_CALL(objMockIThread, Deactivate).Times(0);
    pUceXmlDocumentHelperThread->Terminate();

    EXPECT_EQ(IMS_NULL, pUceXmlDocumentHelperThread->getThread());
}

TEST_F(UceXmlDocumentHelperThreadTest, SendMsg)
{
    IMS_TRACE_D("SendMsg", 0, 0, 0);
    EXPECT_CALL(objMockIThread, PostMessageI(_, _, _)).Times(1);
    pUceXmlDocumentHelperThread->SendMsg(0, 0, 0);

    pUceXmlDocumentHelperThread->setThread(&objMockIThread);
    pUceXmlDocumentHelperThread->SendMsg(0, 0, 0);
}

TEST_F(UceXmlDocumentHelperThreadTest, XmlState_NotifyStateChanged)
{
    IMS_TRACE_D("SendMsg", 0, 0, 0);
    pUceXmlDocumentHelperThread->XmlState_NotifyStateChanged();
    EXPECT_EQ(IMS_NULL, pUceXmlDocumentHelperThread->getThread());
}
