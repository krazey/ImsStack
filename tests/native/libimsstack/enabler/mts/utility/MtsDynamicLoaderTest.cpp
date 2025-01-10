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

#include "MockIMtsContext.h"
#include "utility/MtsDynamicLoader.h"
#include <gtest/gtest.h>

using ::testing::Return;

namespace android
{

const IMS_SINT32 SLOT_ID = 0;

class MtsDynamicLoaderTest : public ::testing::Test
{
public:
    MockIMtsContext objContext;
    MtsDynamicLoader* pMtsDynamicLoader;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objContext, GetSlotId).WillByDefault(Return(SLOT_ID));

        pMtsDynamicLoader = new MtsDynamicLoader(objContext);
    }

    virtual void TearDown() override { delete pMtsDynamicLoader; }
};

TEST_F(MtsDynamicLoaderTest, Constructor)
{
    ASSERT_NE(pMtsDynamicLoader, nullptr);
    ASSERT_NE(pMtsDynamicLoader->GetMtsSipFormUtils(), nullptr);
    ASSERT_NE(pMtsDynamicLoader->GetMtsSmUtils(), nullptr);
}

}  // namespace android
