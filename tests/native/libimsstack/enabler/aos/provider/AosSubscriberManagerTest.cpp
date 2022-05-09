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

#include "interface/IAosAppContext.h"
#include "interface/IAosSubscriberManagerListener.h"
#include "provider/AosSubscriberManager.h"

#include "interface/MockIAosSubscriberManagerListener.h"

const IMS_SINT32 SLOT_ID = 0;

class AosSubscriberManagerTest : public ::testing::Test {
public:
    AosSubscriberManager* pAosSubscriberManager;

protected:
    virtual void SetUp() override {
        pAosSubscriberManager = new AosSubscriberManager(SLOT_ID);
        ASSERT_TRUE(pAosSubscriberManager != nullptr);
    }

    virtual void TearDown() override {
        if (pAosSubscriberManager) {
            delete pAosSubscriberManager;
        }
    }

    void SetProvisioned(IN IMS_BOOL bIsFake, IN IMS_BOOL bIsProvisioned) {
        if (bIsFake) {
            pAosSubscriberManager->m_bIsProvisionedForFake = bIsProvisioned;
        } else {
            pAosSubscriberManager->m_bIsProvisioned = bIsProvisioned;
        }
    }

    IMSList<IAosSubscriberManagerListener*> GetSubscriberManagerListeners() {
        return pAosSubscriberManager->m_objListeners;
    }

    IMSList<IAosSubscriberManagerListener*> GetSubscriberManagerMonitorListeners() {
        return pAosSubscriberManager->m_objMonitorListeners;
    }

    void SetPuids(IN IMS_BOOL bIsFake, IN AStringArray& objPuids) {
        if (bIsFake) {
            pAosSubscriberManager->m_objPuidsForFake = objPuids;
        } else {
            pAosSubscriberManager->m_objPuids = objPuids;
        }
    }
};

TEST_F(AosSubscriberManagerTest, IsReady_True) {
    SetProvisioned(IMS_FALSE, IMS_TRUE);
    EXPECT_TRUE(pAosSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, IsReady_False) {
    SetProvisioned(IMS_FALSE, IMS_FALSE);
    EXPECT_FALSE(pAosSubscriberManager->IsReady(IMS_FALSE));
}

TEST_F(AosSubscriberManagerTest, IsReady_FakeTrue) {
    SetProvisioned(IMS_TRUE, IMS_TRUE);
    EXPECT_TRUE(pAosSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, IsReady_FakeFalse) {
    SetProvisioned(IMS_TRUE, IMS_FALSE);
    EXPECT_FALSE(pAosSubscriberManager->IsReady(IMS_TRUE));
}

TEST_F(AosSubscriberManagerTest, AddListener_Null) {
    pAosSubscriberManager->AddListener(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListener_NotNull) {
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    pAosSubscriberManager->AddListener(piListener1);
    pAosSubscriberManager->AddListener(piListener2);
    pAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener_Null) {
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    pAosSubscriberManager->AddListener(piListener1);
    pAosSubscriberManager->AddListener(piListener2);
    pAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);

    pAosSubscriberManager->RemoveListener(IMS_NULL);
    pAosSubscriberManager->RemoveListener(IMS_NULL);
    pAosSubscriberManager->RemoveListener(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListener_NotNull) {
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    pAosSubscriberManager->AddListener(piListener1);
    pAosSubscriberManager->AddListener(piListener2);
    pAosSubscriberManager->AddListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 3);

    pAosSubscriberManager->RemoveListener(piListener1);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 2);
    pAosSubscriberManager->RemoveListener(piListener2);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 1);
    pAosSubscriberManager->RemoveListener(piListener3);
    EXPECT_EQ(GetSubscriberManagerListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_Null) {
    pAosSubscriberManager->AddListenerForMonitor(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, AddListenerForMonitor_NotNull) {
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    pAosSubscriberManager->AddListenerForMonitor(piListener1);
    pAosSubscriberManager->AddListenerForMonitor(piListener2);
    pAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor_Null) {
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    pAosSubscriberManager->AddListenerForMonitor(piListener1);
    pAosSubscriberManager->AddListenerForMonitor(piListener2);
    pAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);

    pAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    pAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    pAosSubscriberManager->RemoveListenerForMonitor(IMS_NULL);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);
}

TEST_F(AosSubscriberManagerTest, RemoveListenerForMonitor_NotNull) {
    IAosSubscriberManagerListener* piListener1 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener2 = new MockIAosSubscriberManagerListener();
    IAosSubscriberManagerListener* piListener3 = new MockIAosSubscriberManagerListener();

    pAosSubscriberManager->AddListenerForMonitor(piListener1);
    pAosSubscriberManager->AddListenerForMonitor(piListener2);
    pAosSubscriberManager->AddListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 3);

    pAosSubscriberManager->RemoveListenerForMonitor(piListener1);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 2);
    pAosSubscriberManager->RemoveListenerForMonitor(piListener2);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 1);
    pAosSubscriberManager->RemoveListenerForMonitor(piListener3);
    EXPECT_EQ(GetSubscriberManagerMonitorListeners().GetSize(), 0);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpus_Fake) {
    AStringArray objPuids;
    objPuids.AddElement(AString("PUID1"));
    objPuids.AddElement(AString("PUID2"));
    objPuids.AddElement(AString("PUID3"));

    SetPuids(IMS_TRUE, objPuids);
    EXPECT_EQ(pAosSubscriberManager->GetConfiguredImpus(IMS_TRUE).GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetConfiguredImpus_NotFake) {
    AStringArray objPuids;
    objPuids.AddElement(AString("PUID1"));
    objPuids.AddElement(AString("PUID2"));
    objPuids.AddElement(AString("PUID3"));

    SetPuids(IMS_FALSE, objPuids);
    EXPECT_EQ(pAosSubscriberManager->GetConfiguredImpus(IMS_FALSE).GetCount(), 3);
}

TEST_F(AosSubscriberManagerTest, GetFakeImpus_SubscriberConfigNull) {
    EXPECT_EQ(pAosSubscriberManager->GetFakeImpus().GetCount(), 0);
}