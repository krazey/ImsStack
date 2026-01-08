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
#include <gtest/gtest.h>

#include "MockIConfigUpdateListener.h"
#include "private/ConfigBase.h"

using ::testing::_;

namespace android
{
static const IMS_SINT32 KEY_1 = 1;
static const IMS_SINT32 KEY_2 = 2;
static const IMS_SINT32 KEY_3 = 3;

class TestConfigBase : public ConfigBase
{
public:
    inline TestConfigBase() :
            ConfigBase(IMS_SLOT_0),
            m_pConfigurable(IMS_NULL)
    {
        m_pConfigurable = new Configurable(this);
    }

    ~TestConfigBase() override { delete m_pConfigurable; }

    inline IConfigurable* GetConfigurable() const { return m_pConfigurable; }

protected:
    inline IMS_BOOL Update(
            IN IMS_SINT32 nCpi, IN const AString& /*strValue = AString::ConstNull()*/) override
    {
        return NotifyUpdate(nCpi);
    }

private:
    Configurable* m_pConfigurable;
};

class ConfigBaseTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}

public:
    MockIConfigUpdateListener m_objMockIConfigUpdateListener;
};

TEST_F(ConfigBaseTest, AddAndRemoveListener)
{
    TestConfigBase objTestConfigBase;

    IConfigurable* piConfigurable = objTestConfigBase.GetConfigurable();

    // Listener is null, fail
    EXPECT_FALSE(piConfigurable->AddListener(KEY_1, IMS_NULL));

    // simply returns as listener is null
    piConfigurable->RemoveListener(KEY_1, IMS_NULL);

    // simply returns as listener is not yet added
    piConfigurable->RemoveListener(KEY_1, &m_objMockIConfigUpdateListener);

    // Listener passed with KEY_1, success
    EXPECT_TRUE(piConfigurable->AddListener(KEY_1, &m_objMockIConfigUpdateListener));

    // Listener already added with KEY_1, fail
    EXPECT_TRUE(piConfigurable->AddListener(KEY_1, &m_objMockIConfigUpdateListener));

    // Listener passed with KEY_2, success
    EXPECT_TRUE(piConfigurable->AddListener(KEY_2, &m_objMockIConfigUpdateListener));

    piConfigurable->RemoveListener(KEY_1, &m_objMockIConfigUpdateListener);
    piConfigurable->RemoveListener(KEY_2, &m_objMockIConfigUpdateListener);
}

TEST_F(ConfigBaseTest, NotifyUpdate)
{
    TestConfigBase objTestConfigBase;

    IConfigurable* piConfigurable = objTestConfigBase.GetConfigurable();

    // Listener passed with KEY_1, success
    EXPECT_TRUE(piConfigurable->AddListener(KEY_1, &m_objMockIConfigUpdateListener));

    // Listener passed with KEY_2, success
    EXPECT_TRUE(piConfigurable->AddListener(KEY_2, &m_objMockIConfigUpdateListener));

    EXPECT_CALL(m_objMockIConfigUpdateListener, ConfigUpdate_NotifyUpdate(KEY_1, _, _)).Times(1);
    EXPECT_CALL(m_objMockIConfigUpdateListener, ConfigUpdate_NotifyUpdate(KEY_2, _, _)).Times(1);
    EXPECT_CALL(m_objMockIConfigUpdateListener, ConfigUpdate_NotifyUpdate(KEY_3, _, _)).Times(0);

    // Update KEY_1
    EXPECT_TRUE(piConfigurable->Update(KEY_1, AString::ConstNull()));

    // Update KEY_2
    EXPECT_TRUE(piConfigurable->Update(KEY_2, AString::ConstNull()));

    // Update KEY_3, no listener added for this key
    EXPECT_FALSE(piConfigurable->Update(KEY_3, AString::ConstNull()));
}

TEST_F(ConfigBaseTest, DefaultBaseEmptyMethods)
{
    TestConfigBase objTestConfigBase;

    EXPECT_FALSE(objTestConfigBase.Load());
    EXPECT_FALSE(objTestConfigBase.Load(AString("Test")));

    EXPECT_FALSE(objTestConfigBase.Store());
    EXPECT_FALSE(objTestConfigBase.Store(AString("Test")));

    IConfigurable* piConfigurable = objTestConfigBase.GetConfigurable();

    EXPECT_FALSE(piConfigurable->Update(KEY_1));
}

}  // namespace android