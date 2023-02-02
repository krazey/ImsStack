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
#include <dlfcn.h>

#include "OsDll.h"

namespace android
{

class OsDllTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(OsDllTest, LoadLibrary)
{
    ASSERT_TRUE(OsDll::LoadLibrary(nullptr) == nullptr);
    ASSERT_TRUE(OsDll::LoadLibrary("InvalidDllName") == nullptr);

    void* pvHandle = OsDll::LoadLibrary("libc.so");
    ASSERT_TRUE(pvHandle != nullptr);

    OsDll::FreeLibrary(pvHandle);
}

TEST_F(OsDllTest, GetProcAddress)
{
    void* pvHandle = OsDll::LoadLibrary("libc.so");
    ASSERT_TRUE(pvHandle != NULL);

    ASSERT_TRUE(OsDll::GetProcAddress(pvHandle, nullptr) == nullptr);
    ASSERT_TRUE(OsDll::GetProcAddress(pvHandle, "InvalidSymbol") == nullptr);

    void* symbol = OsDll::GetProcAddress(pvHandle, "strlen");
    ASSERT_TRUE(symbol != NULL);

    char* pString = (char*)"abcdef";

    int (*function)(char*) = reinterpret_cast<int (*)(char*)>(symbol);

    EXPECT_EQ(function(pString), 6);

    OsDll::FreeLibrary(nullptr);
    OsDll::FreeLibrary(pvHandle);
}

}  // namespace android