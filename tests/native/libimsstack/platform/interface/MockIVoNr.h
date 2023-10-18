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

#ifndef MOCK_I_VO_NR_H_
#define MOCK_I_VO_NR_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "IVoNr.h"

class MockIVoNr : public IVoNr
{
public:
    MOCK_METHOD(IMS_BOOL, IsVoNrSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUacCheckRequired, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, IsUeCapabilityVoNrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, NotifyCallState,
            (IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState,
                    IN IMS_UINT32 nSysMode, IN IMS_UINT32 nDirection),
            (override));
    MOCK_METHOD(IMS_SINT32, RequestCallPreference, (IN IMS_UINT32 nRat, IN IMS_UINT32 nType),
            (override));
    MOCK_METHOD(IMS_BOOL, SetImsSession, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(IMS_BOOL, SetImsVoice, (IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode), (override));
    MOCK_METHOD(IMS_BOOL, SetImsSignaling, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, SetUacCheck, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(IMS_BOOL, SetVoice, (IN IMS_UINT32 nState, IN IMS_BOOL bIsEmergency), (override));
    MOCK_METHOD(void, AddListenerForUac, (IN IVoNrUacListener * piListener), (override));
    MOCK_METHOD(void, RemoveListenerForUac, (IN IVoNrUacListener * piListener), (override));
    MOCK_METHOD(void, AddListenerForCallPreference, (IN IVoNrCallPreferenceListener * piListener),
            (override));
    MOCK_METHOD(void, RemoveListenerForCallPreference,
            (IN IVoNrCallPreferenceListener * piListener), (override));
    MOCK_METHOD(void, AddListenerForHandoff, (IN IVoNrHandoffListener * piListener), (override));
    MOCK_METHOD(void, RemoveListenerForHandoff, (IN IVoNrHandoffListener * piListener), (override));
};

#endif  // MOCK_I_VO_NR_H_
