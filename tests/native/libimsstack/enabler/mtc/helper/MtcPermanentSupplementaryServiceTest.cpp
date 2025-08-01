/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "ImsList.h"
#include "MtcDef.h"
#include "helper/MtcPermanentSupplementaryService.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace android
{

class MtcPermanentSupplementaryServiceTest : public ::testing::Test
{
public:
    MtcPermanentSupplementaryService objMtcPermanentSupplementaryService;
};

TEST_F(MtcPermanentSupplementaryServiceTest, UpdateServicesAndIsEnabledReturnsTrue)
{
    SuppService* pTestSupp = new SuppService();
    pTestSupp->nType = static_cast<IMS_SINT32>(PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VOICE);
    pTestSupp->bValue = IMS_TRUE;

    ImsList<SuppService*> objInSuppService;
    objInSuppService.Append(pTestSupp);

    objMtcPermanentSupplementaryService.UpdateServices(objInSuppService);

    EXPECT_TRUE(objMtcPermanentSupplementaryService.IsEnabled(
            PermanentSuppType::TB_CB_INCOMING_ANONYMOUS_VOICE));
    EXPECT_FALSE(objMtcPermanentSupplementaryService.IsEnabled(
            PermanentSuppType::TB_CB_INCOMING_ROAMING_VOICE));
}

TEST_F(MtcPermanentSupplementaryServiceTest, IsEnabledForCallWaitingReturnsTrueEvenIfNotProvisioned)
{
    EXPECT_TRUE(objMtcPermanentSupplementaryService.IsEnabled(PermanentSuppType::TB_CW));
}

}  // namespace android
