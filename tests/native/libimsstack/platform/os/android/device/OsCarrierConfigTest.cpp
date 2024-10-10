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

#include "ImsMessageDef.h"
#include "MockICarrierConfigListener.h"
#include "MockISystem.h"
#include "MockIThread.h"
#include "OsParcel.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "device/OsCarrierConfig.h"
#include "system-intf/SystemConstants.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Unused;

using namespace android::os;
using android::String16;

namespace android
{

class OsCarrierConfigTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pOldSystem = PlatformContext::GetInstance()->SetSystem(&m_objSystem);

        EXPECT_CALL(m_objSystem, AddListener(SystemConstants::CATEGORY_CONFIG, _, _))
                .Times(AnyNumber());
        EXPECT_CALL(m_objSystem, RemoveListener(SystemConstants::CATEGORY_CONFIG, _, _))
                .Times(AnyNumber());

        m_pThreadService = new TestThreadService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, m_pThreadService);
        m_pThreadService->SetThread(&m_objMockThread);
    }

    virtual void TearDown() override
    {
        PlatformContext::GetInstance()->SetSystem(m_pOldSystem);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete m_pThreadService;
    }

protected:
    ISystem* m_pOldSystem;
    MockICarrierConfigListener m_objMockICarrierConfigListener;
    MockISystem m_objSystem;
    MockIThread m_objMockThread;
    TestThreadService* m_pThreadService;
};

TEST_F(OsCarrierConfigTest, System_NotifyEvent)
{
    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);
    IMS_UINTP nWParam = 20;
    IMS_UINTP nLParam = 30;

    EXPECT_CALL(m_objMockThread, PostMessageI(IMS_MSG_CONFIGURATION, _, _))
            .Times(1)
            .WillRepeatedly(Return(IMS_TRUE));

    // valid event, should post message to thread
    objOsCarrierConfig.System_NotifyEvent(IMS_SYSTEM_CONFIGURATION_CHANGED, nWParam, nLParam);

    EXPECT_CALL(m_objMockThread, PostMessageI(_, _, _)).Times(0);

    // Invalid event, should not post message
    objOsCarrierConfig.System_NotifyEvent(
            IMS_SYSTEM_VOICE_RADIOTECH_STATE_CHANGED, nWParam, nLParam);
}

TEST_F(OsCarrierConfigTest, DispatchServiceMessage)
{
    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;
                        const IMS_CHAR* KEY_VALUE_BOOL = "value_available_bool";
                        const IMS_BOOL bBooleanValue = IMS_TRUE;

                        objPersistableBundle.putBoolean(String16(KEY_VALUE_BOOL), bBooleanValue);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    objOsCarrierConfig.AddListener(&m_objMockICarrierConfigListener);

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(2);
    EXPECT_CALL(m_objMockICarrierConfigListener, CarrierConfig_NotifyConfigChanged(_)).Times(1);

    objOsCarrierConfig.DispatchServiceMessage(0, 0);
    objOsCarrierConfig.DispatchServiceMessage(0, 0);

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1).WillRepeatedly(Return(0));

    objOsCarrierConfig.DispatchServiceMessage(0, 0);
}

TEST_F(OsCarrierConfigTest, LoadConfig)
{
    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;
                        const IMS_CHAR* KEY_VALUE_BOOL = "value_available_bool";
                        const IMS_BOOL bBooleanValue = IMS_TRUE;

                        objPersistableBundle.putBoolean(String16(KEY_VALUE_BOOL), bBooleanValue);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1).WillRepeatedly(Return(0));

    objOsCarrierConfig.LoadConfig();
}

TEST_F(OsCarrierConfigTest, GetBoolean)
{
    const IMS_CHAR* KEY_VALUE_BOOL = "value_available_bool";
    const IMS_BOOL bBooleanValue = IMS_TRUE;

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_BOOL](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;

                        objPersistableBundle.putBoolean(String16(KEY_VALUE_BOOL), bBooleanValue);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(IMS_FALSE, objOsCarrierConfig.GetBoolean(KEY_VALUE_BOOL));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_EQ(bBooleanValue, objOsCarrierConfig.GetBoolean(KEY_VALUE_BOOL));
}

TEST_F(OsCarrierConfigTest, GetInt)
{
    const IMS_CHAR* KEY_VALUE_INT = "value_available_int";
    const IMS_BOOL nIntegerValue = 35;

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_INT](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;

                        objPersistableBundle.putInt(String16(KEY_VALUE_INT), nIntegerValue);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(-1, objOsCarrierConfig.GetInt(KEY_VALUE_INT));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_EQ(nIntegerValue, objOsCarrierConfig.GetInt(KEY_VALUE_INT));
}

TEST_F(OsCarrierConfigTest, GetLong)
{
    const IMS_CHAR* KEY_VALUE_LONG = "value_available_long";
    const IMS_BOOL lLongValue = 123456789L;

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_LONG](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;

                        objPersistableBundle.putLong(String16(KEY_VALUE_LONG), lLongValue);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(-1L, objOsCarrierConfig.GetLong(KEY_VALUE_LONG));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_EQ(lLongValue, objOsCarrierConfig.GetLong(KEY_VALUE_LONG));
}

TEST_F(OsCarrierConfigTest, GetString)
{
    const IMS_CHAR* KEY_VALUE_STRING = "value_available_string";
    const IMS_CHAR* strStringValue = "TestString";

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_STRING, strStringValue](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;

                        objPersistableBundle.putString(
                                String16(KEY_VALUE_STRING), String16(strStringValue));

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(AString::ConstNull(), objOsCarrierConfig.GetString(KEY_VALUE_STRING));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_EQ(AString(strStringValue), objOsCarrierConfig.GetString(KEY_VALUE_STRING));
}

TEST_F(OsCarrierConfigTest, GetBooleanArray)
{
    const IMS_CHAR* KEY_VALUE_BOOL_ARRAY = "value_available_bool_array";
    const std::vector<IMS_BOOL> arrBoolArray = {IMS_TRUE, IMS_FALSE, IMS_TRUE, IMS_FALSE};

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_BOOL_ARRAY, arrBoolArray](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;

                        objPersistableBundle.putBooleanVector(
                                String16(KEY_VALUE_BOOL_ARRAY), arrBoolArray);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_TRUE(objOsCarrierConfig.GetBooleanArray(KEY_VALUE_BOOL_ARRAY).IsEmpty());
    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_FALSE(objOsCarrierConfig.GetBooleanArray(KEY_VALUE_BOOL_ARRAY).IsEmpty());
    EXPECT_TRUE(
            arrBoolArray == objOsCarrierConfig.GetBooleanArray(KEY_VALUE_BOOL_ARRAY).GetVector());
}

TEST_F(OsCarrierConfigTest, GetIntArray)
{
    const IMS_CHAR* KEY_VALUE_INT_ARRAY = "value_available_int_array";
    const std::vector<IMS_SINT32> arrIntArray = {1, 2, 3, 4, 5};

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_INT_ARRAY, arrIntArray](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;

                        objPersistableBundle.putIntVector(
                                String16(KEY_VALUE_INT_ARRAY), arrIntArray);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(ImsVector<IMS_SINT32>(), objOsCarrierConfig.GetIntArray(KEY_VALUE_INT_ARRAY));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_FALSE(objOsCarrierConfig.GetIntArray(KEY_VALUE_INT_ARRAY).IsEmpty());
    EXPECT_TRUE(arrIntArray == objOsCarrierConfig.GetIntArray(KEY_VALUE_INT_ARRAY).GetVector());
}

TEST_F(OsCarrierConfigTest, GetLongArray)
{
    const IMS_CHAR* KEY_VALUE_LONG_ARRAY = "value_available_long_array";

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_LONG_ARRAY](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;
                        const std::vector<int64_t> arrLongArray = {123456789L, 987654321L};

                        objPersistableBundle.putLongVector(
                                String16(KEY_VALUE_LONG_ARRAY), arrLongArray);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));
    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(ImsVector<IMS_SLONG>(), objOsCarrierConfig.GetLongArray(KEY_VALUE_LONG_ARRAY));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_FALSE(objOsCarrierConfig.GetLongArray(KEY_VALUE_LONG_ARRAY).IsEmpty());

    const std::vector<IMS_SLONG> arrLongArray = {123456789L, 987654321L};
    EXPECT_TRUE(arrLongArray == objOsCarrierConfig.GetLongArray(KEY_VALUE_LONG_ARRAY).GetVector());
}

TEST_F(OsCarrierConfigTest, GetStringArray)
{
    const IMS_CHAR* KEY_VALUE_STRING_ARRAY = "value_available_string_array";

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_STRING_ARRAY](Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;
                        const std::vector<String16> arrStringArray = {
                                String16("String1"), String16("String2"), String16("String3")};

                        objPersistableBundle.putStringVector(
                                String16(KEY_VALUE_STRING_ARRAY), arrStringArray);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(ImsVector<AString>(), objOsCarrierConfig.GetStringArray(KEY_VALUE_STRING_ARRAY));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    EXPECT_FALSE(objOsCarrierConfig.GetStringArray(KEY_VALUE_STRING_ARRAY).IsEmpty());

    const std::vector<AString> arrStringArray = {
            AString("String1"), AString("String2"), AString("String3")};
    EXPECT_TRUE(arrStringArray ==
            objOsCarrierConfig.GetStringArray(KEY_VALUE_STRING_ARRAY).GetVector());
}

TEST_F(OsCarrierConfigTest, GetBundle)
{
    const IMS_CHAR* KEY_VALUE_BUNDLE = "value_available_bundle";
    const IMS_CHAR* KEY_VALUE_STRING = "value_available_string";
    const IMS_CHAR* strBundleStringValue = "TestBundleString";

    ON_CALL(m_objSystem, GetCarrierConfig(_, _))
            .WillByDefault(Invoke(
                    [KEY_VALUE_BUNDLE, KEY_VALUE_STRING, strBundleStringValue](
                            Unused, ImsParcel& objConfig)
                    {
                        PersistableBundle objPersistableBundle;
                        PersistableBundle objPersistableBundleValue;

                        objPersistableBundleValue.putString(
                                String16(KEY_VALUE_STRING), String16(strBundleStringValue));
                        objPersistableBundle.putPersistableBundle(
                                String16(KEY_VALUE_BUNDLE), objPersistableBundleValue);

                        OsParcel& objOsConfig = (OsParcel&)objConfig;
                        objPersistableBundle.writeToParcel(&objOsConfig.GetParcel());
                        objOsConfig.GetParcel().setDataPosition(0);

                        return IMS_TRUE;
                    }));

    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    EXPECT_EQ(IMS_NULL, objOsCarrierConfig.GetBundle(KEY_VALUE_BUNDLE));

    EXPECT_CALL(m_objSystem, GetCarrierConfig(_, _)).Times(1);

    objOsCarrierConfig.LoadConfig();

    ICarrierConfig* pObjOsCarrierConfig = objOsCarrierConfig.GetBundle(KEY_VALUE_BUNDLE);

    ASSERT_TRUE(pObjOsCarrierConfig != nullptr);

    EXPECT_EQ(AString(strBundleStringValue), pObjOsCarrierConfig->GetString(KEY_VALUE_STRING));

    pObjOsCarrierConfig->ReleaseBundle();
}

TEST_F(OsCarrierConfigTest, AddAndRemoveListener)
{
    OsCarrierConfig objOsCarrierConfig(IMS_SLOT_0);

    objOsCarrierConfig.AddListener(nullptr);

    objOsCarrierConfig.AddListener(&m_objMockICarrierConfigListener);

    objOsCarrierConfig.RemoveListener(&m_objMockICarrierConfigListener);
}

}  // namespace android
