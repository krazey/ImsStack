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

#include "AString.h"
#include "ImsTypeDef.h"
#include "MockIMtcContext.h"
#include "dialogevent/IDialogInfoManager.h"
#include "dialogevent/IDialogSubscription.h"
#include "dialogevent/MockIDialogSubscription.h"
#include "dialogevent/MultiEndpointFactory.h"
#include "helper/sipinterfaceholder/MockIInterfaceHolderListener.h"
#include "helper/sipinterfaceholder/MockIMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/MockSubscriptionInterfaceHolder.h"
#include <gtest/gtest.h>
#include <memory>

using ::testing::Return;
using ::testing::ReturnRef;

TEST(MultiEndpointFactoryTest, CreateDialogInfoManagerReturnsNonNullPointer)
{
    std::unique_ptr<IDialogInfoManager> pManager = MultiEndpointFactory().CreateDialogInfoManager();
    ASSERT_NE(pManager, nullptr);
}

TEST(MultiEndpointFactoryTest, CreateSubscriptionReturnsNonNullPointer)
{
    MockIMtcContext objContext;
    MockIMtcSipInterfaceFactory objSipFactory;
    MockIInterfaceHolderListener objHolderListener;
    MockSubscriptionInterfaceHolder objSubscriptionHolder(objHolderListener);
    ON_CALL(objContext, GetSipInterfaceFactory).WillByDefault(ReturnRef(objSipFactory));
    ON_CALL(objSipFactory, GetISubscriptionHolder).WillByDefault(Return(&objSubscriptionHolder));
    MockIDialogSubscriptionListener objListener;

    std::unique_ptr<IDialogSubscription> pSubscription =
            MultiEndpointFactory().CreateDialogSubscription(
                    objContext, objListener, AString::ConstNull());
    ASSERT_NE(pSubscription, nullptr);
}
