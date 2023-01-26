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

#ifndef MOCK_I_AOS_PCSCF_H_
#define MOCK_I_AOS_PCSCF_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "ImsList.h"
#include "IpAddress.h"
#include "AStringArray.h"
#include "interface/IAosPcscf.h"

class MockIAosPcscf : public IAosPcscf
{
public:
    MOCK_METHOD(void, Configure, (IN IMS_UINT32 nIpVersion), (override));
    MOCK_METHOD(IMS_BOOL, IsConfigured, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsAsyncDnsDiscovery, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSinglePcoScheme, (), (override));
    MOCK_METHOD(const AStringArray&, GetPcscfs, (), (override));
    MOCK_METHOD(const IMSList<IMS_SINT32>&, GetPcscfsPorts, (), (override));
    MOCK_METHOD(void, UpdatePcscfs,
            (IN const AStringArray& objPcscfs, IN IMSList<IMS_SINT32> objPorts), (override));
    MOCK_METHOD(IMS_BOOL, HasPcscf, (IN IMS_SINT32 nIndex), (override));
    MOCK_METHOD(IMS_UINT32, GetPcscfCount, (), (override));
    MOCK_METHOD(void, SetCurrentPcscfInvalid, (IN IMS_BOOL bIsTimer, IN IMS_UINT32 nSeconds),
            (override));
    MOCK_METHOD(void, RemoveCurrentPcscf, (), (override));
    MOCK_METHOD(void, SetAllPcscfValid, (), (override));
    MOCK_METHOD(IMS_BOOL, IsAllPcscfTried, (), (override));
    MOCK_METHOD(void, SetCurrentPcscfTried, (), (override));
    MOCK_METHOD(void, ResetAllPcscfTried, (), (override));
    MOCK_METHOD(IMS_UINT32, GetCurrentPcscfTriedCount, (), (override));
    MOCK_METHOD(void, IncreaseCurrentPcscfTriedCount, (), (override));
    MOCK_METHOD(void, ResetCurrentPcscfTriedCount, (), (override));
    MOCK_METHOD(void, ResetAllPcscfTriedCount, (), (override));
    MOCK_METHOD(
            IMS_BOOL, GetCurrentPcscf, (OUT AString & objPcscf, OUT IMS_UINT32& nPort), (override));
    MOCK_METHOD(IMS_UINT32, GetCurrentIndex, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsFirstPcscf, (), (override));
    MOCK_METHOD(
            IMS_BOOL, GetFirstPcscf, (OUT AString & objPcscf, OUT IMS_UINT32& nPort), (override));
    MOCK_METHOD(IMS_BOOL, HasNextPcscf, (), (override));
    MOCK_METHOD(IMS_SINT32, GetNextPcscfIndex, (), (override));
    MOCK_METHOD(
            IMS_BOOL, GetNextPcscf, (OUT AString & objPcscf, OUT IMS_UINT32& nPort), (override));
    MOCK_METHOD(void, SetFirstPcscfIndex, (), (override));
    MOCK_METHOD(IMS_BOOL, CheckAndProcessChangeFromPco, (), (override));
    MOCK_METHOD(IMS_UINT32, GetChangedType, (), (override));
    MOCK_METHOD(void, SetListener, (IN IAosPcscfListener * piListener), (override));

    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

class MockIAosPcscfListener : public IAosPcscfListener
{
public:
    MOCK_METHOD(void, Pcscf_NotifyResult, (IN IMS_BOOL bResult), (override));
};
#endif  // MOCK_I_AOS_PCSCF_H_
