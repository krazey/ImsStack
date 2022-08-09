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
#ifndef MOCK_I_REG_INFO_REGISTRATION_H_
#define MOCK_I_REG_INFO_REGISTRATION_H_

#include <gmock/gmock.h>

#include "IRegInfoContact.h"
#include "IRegInfoRegistration.h"

class MockIRegInfoRegistration : public IRegInfoRegistration
{
public:
    MOCK_METHOD(const SipAddress&, GetAor, (), (const, override));
    MOCK_METHOD(
            IRegInfoContact*, GetContact, (IN const SipAddress& objContactUri), (const, override));
    MOCK_METHOD(IMSList<IRegInfoContact*>, GetContacts, (), (const, override));
    MOCK_METHOD(IRegInfoContact*, GetPriorContact, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetState, (), (const, override));
};

#endif  // MOCK_I_REG_INFO_REGISTRATION_H_
