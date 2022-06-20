/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "MtcApp.h"
#include "IMtcService.h"

LOCAL IMS_SINT32 SLOT_ID = 0;

namespace android
{

class MtcAppTest : public ::testing::Test
{
public:
    MtcApp* pMtcApp;

protected:
    virtual void SetUp() override { pMtcApp = new MtcApp(SLOT_ID); }

    virtual void TearDown() override { delete pMtcApp; }
};

}  // namespace android
