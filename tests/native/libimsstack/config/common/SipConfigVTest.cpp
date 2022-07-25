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

#include "private/SipConfigV.h"

namespace android
{

class SipConfigVTest : public ::testing::Test
{
public:
    SipConfigVTest();

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}

protected:
    AStringArray m_objAllowMethods;
};

SipConfigVTest::SipConfigVTest()
{
    m_objAllowMethods.AddElement("INVITE");
    m_objAllowMethods.AddElement("BYE");
    m_objAllowMethods.AddElement("CANCEL");
    m_objAllowMethods.AddElement("ACK");
    m_objAllowMethods.AddElement("NOTIFY");
    m_objAllowMethods.AddElement("UPDATE");
    m_objAllowMethods.AddElement("REFER");
    m_objAllowMethods.AddElement("PRACK");
    m_objAllowMethods.AddElement("INFO");
    m_objAllowMethods.AddElement("MESSAGE");
    m_objAllowMethods.AddElement("OPTIONS");
}

TEST_F(SipConfigVTest, Refresh)
{
    SipConfigV objSipConfigV(IMS_SLOT_0);

    objSipConfigV.Init();

    const AStringArray& objAllowMethods = objSipConfigV.GetAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethods.GetElements());

    ICarrierConfigListener& objListener = DYNAMIC_CAST(ICarrierConfigListener&, objSipConfigV);
    objListener.CarrierConfig_NotifyConfigChanged(IMS_SLOT_0);
    const AStringArray& objAllowMethodsForRefresh = objSipConfigV.GetAllowMethods();
    EXPECT_EQ(m_objAllowMethods.GetElements(), objAllowMethodsForRefresh.GetElements());
}

}  // namespace android
