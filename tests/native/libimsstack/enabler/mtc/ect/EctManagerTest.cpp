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

#include "MockIMtcContext.h"
#include "ect/EctManager.h"
#include "ect/IEctManager.h"
#include "ect/MockBlindTransferController.h"
#include "ect/MockConsultativeTransferController.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{
LOCAL CallKey ANY_CALL_KEY = 100;

class TestEctManager : public EctManager
{
public:
    inline explicit TestEctManager(IN IMtcContext& objContext) :
            EctManager(objContext)
    {
    }
    inline void SetBlindController(IN std::unique_ptr<BlindTransferController> pController)
    {
        m_objEctFactory.SetBlindController(std::move(pController));
    }
    inline void SetConsultativeController(
            IN std::unique_ptr<ConsultativeTransferController> pController)
    {
        m_objEctFactory.SetConsultativeController(std::move(pController));
    }
    inline EctFactory& GetFactory() { return m_objEctFactory; }
};

class EctManagerTest : public ::testing::Test
{
public:
    TestEctManager* pManager;

    MockIMtcContext objContext;
    MockICallStateProxy objCallStateProxy;
    std::unique_ptr<MockBlindTransferController> pBlindController;
    std::unique_ptr<MockConsultativeTransferController> pConsultativeController;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));

        pManager = new TestEctManager(objContext);
    }

    virtual void TearDown() override { delete pManager; }

    MockBlindTransferController* GetMockBlindController()
    {
        pBlindController = std::make_unique<MockBlindTransferController>(
                objContext, ANY_CALL_KEY, *pManager, pManager->GetFactory());
        MockBlindTransferController* pRawPtr = pBlindController.get();
        pManager->SetBlindController(std::move(pBlindController));
        return pRawPtr;
    }

    MockConsultativeTransferController* GetMockConsultativeController()
    {
        pConsultativeController = std::make_unique<MockConsultativeTransferController>(
                objContext, ANY_CALL_KEY, *pManager, pManager->GetFactory());
        MockConsultativeTransferController* pRawPtr = pConsultativeController.get();
        pManager->SetConsultativeController(std::move(pConsultativeController));
        return pRawPtr;
    }
};

TEST_F(EctManagerTest, InitialStateReturnsIdle)
{
    EXPECT_EQ(IEctManager::State::IDLE, pManager->GetState());
}

TEST_F(EctManagerTest, TransferWithTargetNumberInvokesBlindTransfer)
{
    AString strAnyTargetNumber("12345");
    EXPECT_CALL(*GetMockBlindController(), Transfer(strAnyTargetNumber));

    pManager->Transfer(ANY_CALL_KEY, strAnyTargetNumber);
    EXPECT_EQ(IEctManager::State::BLIND_TRANSFERRING, pManager->GetState());

    pManager->OnEctCompleted();
    EXPECT_EQ(IEctManager::State::IDLE, pManager->GetState());
}

TEST_F(EctManagerTest, TransferWithoutTargetNumberInvokesConsultativeTransfer)
{
    AString strEmptyTargetNumber("");
    EXPECT_CALL(*GetMockConsultativeController(), Transfer());

    pManager->Transfer(ANY_CALL_KEY, strEmptyTargetNumber);
    EXPECT_EQ(IEctManager::State::CONSULTATIVE_TRANSFERRING, pManager->GetState());

    pManager->OnEctCompleted();
    EXPECT_EQ(IEctManager::State::IDLE, pManager->GetState());
}

TEST_F(EctManagerTest, MultipleTransferIsBlocked)
{
    AString strEmptyNumber("");
    ON_CALL(*GetMockConsultativeController(), Transfer).WillByDefault(Return());

    EXPECT_EQ(IMS_SUCCESS, pManager->Transfer(ANY_CALL_KEY, strEmptyNumber));
    EXPECT_EQ(IMS_FAILURE, pManager->Transfer(ANY_CALL_KEY, strEmptyNumber));
}

}  // namespace android
