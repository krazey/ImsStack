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

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

class TestUceXmlDocumentHelperThread : public UceXmlDocumentHelperThread
{
public:
    TestUceXmlDocumentHelperThread() :
            UceXmlDocumentHelperThread(AString("UceXmlDocumentHelperThread"), 0)
    {
    }
    virtual ~TestUceXmlDocumentHelperThread() {}

    IThread* getThread() { return GetThread(); }
};

class UceXmlDocumentHelperThreadTest : public ::testing::Test
{
public:
    TestUceXmlDocumentHelperThread* pUceXmlDocumentHelperThread;

protected:
    virtual void SetUp() override
    {
        pUceXmlDocumentHelperThread = new TestUceXmlDocumentHelperThread();
        ASSERT_TRUE(pUceXmlDocumentHelperThread != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceXmlDocumentHelperThread)
        {
            delete pUceXmlDocumentHelperThread;
        }
    }
};

TEST_F(UceXmlDocumentHelperThreadTest, Start)
{
    IMS_TRACE_D("Start", 0, 0, 0);
    EXPECT_TRUE(pUceXmlDocumentHelperThread->Start("Test"));
}

TEST_F(UceXmlDocumentHelperThreadTest, Terminate)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);
    pUceXmlDocumentHelperThread->Terminate();

    EXPECT_EQ(IMS_NULL, pUceXmlDocumentHelperThread->getThread());
}