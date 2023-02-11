/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MockISystem.h"
#include "PlatformContext.h"
#include "ImsPrivateProperty.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Unused;

namespace android
{

class ImsPrivatePropertyTest : public ::testing::Test
{
public:
    inline ImsPrivatePropertyTest() :
            strTestKey(AString("TestKey")),
            m_pOldSystem(IMS_NULL),
            m_pImsPrivateProperty(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pImsPrivateProperty = ImsPrivateProperty::GetInstance();
        m_pOldSystem = PlatformContext::GetInstance()->SetSystem(&m_objSystem);
    }

    virtual void TearDown() override { PlatformContext::GetInstance()->SetSystem(m_pOldSystem); }

protected:
    AString strTestKey;
    ISystem* m_pOldSystem;
    MockISystem m_objSystem;
    ImsPrivateProperty* m_pImsPrivateProperty;
};

TEST_F(ImsPrivatePropertyTest, Get)
{
    AString strValue("GetMethod");

    EXPECT_CALL(m_objSystem, GetPrivateProperty(IMS_FALSE, _, _))
            .Times(1)
            .WillOnce(Return(strValue));
    EXPECT_EQ(strValue, m_pImsPrivateProperty->Get(strTestKey, IMS_SLOT_0));
}

TEST_F(ImsPrivatePropertyTest, GetBoolean)
{
    AString strValue("true");

    EXPECT_CALL(m_objSystem, GetPrivateProperty(IMS_FALSE, _, _))
            .Times(1)
            .WillOnce(Return(strValue));
    EXPECT_EQ(IMS_TRUE, m_pImsPrivateProperty->GetBoolean(strTestKey, IMS_SLOT_0));
}

TEST_F(ImsPrivatePropertyTest, GetInt)
{
    AString strValue("1234");

    EXPECT_CALL(m_objSystem, GetPrivateProperty(IMS_FALSE, _, _))
            .Times(2)
            .WillOnce(Return(AString("")))
            .WillOnce(Return(strValue));

    EXPECT_EQ(-1, m_pImsPrivateProperty->GetInt(strTestKey, IMS_SLOT_0));
    EXPECT_EQ(strValue.ToInt32(), m_pImsPrivateProperty->GetInt(strTestKey, IMS_SLOT_0));
}

TEST_F(ImsPrivatePropertyTest, Set)
{
    AString strValue("SetMethod");

    EXPECT_CALL(m_objSystem, SetPrivateProperty(IMS_FALSE, _, strValue, _)).Times(1);
    m_pImsPrivateProperty->Set(strTestKey, strValue, IMS_SLOT_0);
}

TEST_F(ImsPrivatePropertyTest, SetBoolean)
{
    AString strValue("true");

    EXPECT_CALL(m_objSystem, SetPrivateProperty(IMS_FALSE, _, strValue, _)).Times(1);
    m_pImsPrivateProperty->SetBoolean(strTestKey, IMS_TRUE, IMS_SLOT_0);
}

TEST_F(ImsPrivatePropertyTest, SetInt)
{
    AString strValue("1234");

    EXPECT_CALL(m_objSystem, SetPrivateProperty(IMS_FALSE, _, strValue, _)).Times(1);

    m_pImsPrivateProperty->SetInt(strTestKey, strValue.ToInt32(), IMS_SLOT_0);
}

TEST_F(ImsPrivatePropertyTest, GetPersistent)
{
    AString strValue("GetMethod");

    EXPECT_CALL(m_objSystem, GetPrivateProperty(IMS_TRUE, _, _))
            .Times(1)
            .WillOnce(Return(strValue));
    EXPECT_EQ(strValue, m_pImsPrivateProperty->GetPersistent(strTestKey, IMS_SLOT_0));
}

TEST_F(ImsPrivatePropertyTest, GetPersistentBoolean)
{
    AString strValue("true");

    EXPECT_CALL(m_objSystem, GetPrivateProperty(IMS_TRUE, _, _))
            .Times(1)
            .WillOnce(Return(strValue));
    EXPECT_EQ(IMS_TRUE, m_pImsPrivateProperty->GetPersistentBoolean(strTestKey, IMS_SLOT_0));
}

TEST_F(ImsPrivatePropertyTest, GetPersistentInt)
{
    AString strValue("1234");

    EXPECT_CALL(m_objSystem, GetPrivateProperty(IMS_TRUE, _, _))
            .Times(2)
            .WillOnce(Return(AString("")))
            .WillOnce(Return(strValue));

    EXPECT_EQ(-1, m_pImsPrivateProperty->GetPersistentInt(strTestKey, IMS_SLOT_0));
    EXPECT_EQ(strValue.ToInt32(), m_pImsPrivateProperty->GetPersistentInt(strTestKey, IMS_SLOT_0));
}

TEST_F(ImsPrivatePropertyTest, SetPersistent)
{
    AString strValue("SetMethod");

    EXPECT_CALL(m_objSystem, SetPrivateProperty(IMS_TRUE, _, strValue, _)).Times(1);
    m_pImsPrivateProperty->SetPersistent(strTestKey, strValue, IMS_SLOT_0);
}

TEST_F(ImsPrivatePropertyTest, SetPersistentBoolean)
{
    AString strValue("true");

    EXPECT_CALL(m_objSystem, SetPrivateProperty(IMS_TRUE, _, strValue, _)).Times(1);
    m_pImsPrivateProperty->SetPersistentBoolean(strTestKey, IMS_TRUE, IMS_SLOT_0);
}

TEST_F(ImsPrivatePropertyTest, SetPersistentInt)
{
    AString strValue("1234");

    EXPECT_CALL(m_objSystem, SetPrivateProperty(IMS_TRUE, _, strValue, _)).Times(1);

    m_pImsPrivateProperty->SetPersistentInt(strTestKey, strValue.ToInt32(), IMS_SLOT_0);
}

}  // namespace android
