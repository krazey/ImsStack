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

#include "ImsMessage.h"
#include "ImsTypeDef.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "helper/ObjectAsyncDestroyer.h"
#include <gtest/gtest.h>

// to use ObjectAsyncDestroyer::MSG_DESTROY_OBJECT = 0
#define MSG_DESTROY_OBJECT 0

namespace android
{

MATCHER_P(IsSameMessage, message, "")
{
    return arg.nMSG == message.nMSG && arg.nWparam == message.nWparam &&
            arg.nLparam == message.nLparam;
}

class TestObject
{
public:
    inline TestObject() {}
    inline ~TestObject() {}
};

class ObjectAsyncDestroyerTest : public ::testing::Test
{
public:
    inline explicit ObjectAsyncDestroyerTest() :
            pDestroyer(IMS_NULL),
            objMockThread(),
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);

        // this must be created after SetService(SERVICE_THREAD)
        pDestroyer = new ObjectAsyncDestroyer<TestObject>();
    }

    inline virtual ~ObjectAsyncDestroyerTest()
    {
        delete pDestroyer;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

protected:
    ObjectAsyncDestroyer<TestObject>* pDestroyer;
    MockIThread objMockThread;
    TestThreadService* pThreadService;
};

TEST_F(ObjectAsyncDestroyerTest, GetControllerReturnsNull)
{
    ASSERT_EQ(pDestroyer->GetController(), nullptr);
}

TEST_F(ObjectAsyncDestroyerTest, DispatchUnknownMessageReturnsFalse)
{
    IMS_SINT32 nAnyMessageOtherThanDestroyObject = 1;
    ImsMessage objMessage(nAnyMessageOtherThanDestroyObject, 0, 0);
    EXPECT_FALSE(pDestroyer->DispatchMessage(objMessage));
}

TEST_F(ObjectAsyncDestroyerTest, DispatchDestroyMessageReturnsTrue)
{
    TestObject* pTestObject = new TestObject();  // will be deleted inside ObjectAsyncDestroyer
    ImsMessage objMessage(MSG_DESTROY_OBJECT, 0, reinterpret_cast<IMS_UINTP>(pTestObject));
    EXPECT_TRUE(pDestroyer->DispatchMessage(objMessage));
}

TEST_F(ObjectAsyncDestroyerTest, DestroyInvokesPostMessage)
{
    TestObject* pTestObject = new TestObject();
    ImsMessage objMessage(MSG_DESTROY_OBJECT, 0, reinterpret_cast<IMS_UINTP>(pTestObject));
    EXPECT_CALL(objMockThread, PostMessageI(IsSameMessage(objMessage)));

    pDestroyer->Destroy(pTestObject);

    delete pTestObject;
}

}  // namespace android
