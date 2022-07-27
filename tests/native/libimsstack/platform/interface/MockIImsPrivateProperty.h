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
#ifndef MOCK_I_IMS_PRIVATE_PROPERTY_H_
#define MOCK_I_IMS_PRIVATE_PROPERTY_H_

#include <gmock/gmock.h>

#include "IImsPrivateProperty.h"

class MockIImsPrivateProperty : public IImsPrivateProperty
{
public:
    inline MockIImsPrivateProperty() {}
    inline virtual ~MockIImsPrivateProperty() {}

    MOCK_METHOD(AString, Get, (IN const AString& strKey, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(
            IMS_BOOL, GetBoolean, (IN const AString& strKey, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_SINT32, GetInt, (IN const AString& strKey, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, Set,
            (IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, SetBoolean,
            (IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, SetInt,
            (IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(
            AString, GetPersistent, (IN const AString& strKey, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(IMS_BOOL, GetPersistentBoolean, (IN const AString& strKey, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(IMS_SINT32, GetPersistentInt, (IN const AString& strKey, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, SetPersistent,
            (IN const AString& strKey, IN const AString& strValue, IN IMS_SINT32 nSlotId),
            (override));
    MOCK_METHOD(void, SetPersistentBoolean,
            (IN const AString& strKey, IN IMS_BOOL bValue, IN IMS_SINT32 nSlotId), (override));
    MOCK_METHOD(void, SetPersistentInt,
            (IN const AString& strKey, IN IMS_SINT32 nValue, IN IMS_SINT32 nSlotId), (override));
};

#endif
