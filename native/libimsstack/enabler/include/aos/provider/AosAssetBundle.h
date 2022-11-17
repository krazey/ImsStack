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
#ifndef AOS_ASSET_BUNDLE_H_
#define AOS_ASSET_BUNDLE_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "ImsVector.h"
#include "CarrierConfig.h"

struct AosExtraRegErrBundle
{
public:
    AosExtraRegErrBundle() :
            bExtraReregFailureWithErrCodeInRoaming(IMS_FALSE),
            bExtraRegErrRetryCntSharedForRegAndSub(IMS_FALSE),
            nExtraRegErrFinalType(CarrierConfig::Assets::ERROR_TYPE_NOT_SPECIFIED),
            nExtraRegErrMaxCnt(0),
            nExtraRegErrMinCnt(0),
            nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached(0),
            nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached(0),
            nExtraRegErrPolicy(CarrierConfig::Assets::ERROR_POLICY_NOT_SPECIFIED),
            objExtraRegErrCode(IMSVector<IMS_SINT32>()),
            objExtraReregErrCode(IMSVector<IMS_SINT32>()),
            objExtraRegErrWaitTimeSec(IMSVector<IMS_SINT32>())
    {
    }

    AosExtraRegErrBundle(IN const AosExtraRegErrBundle&) = delete;
    AosExtraRegErrBundle& operator=(IN const AosExtraRegErrBundle&) = delete;

public:
    IMS_BOOL bExtraReregFailureWithErrCodeInRoaming;
    IMS_BOOL bExtraRegErrRetryCntSharedForRegAndSub;
    IMS_SINT32 nExtraRegErrFinalType;
    IMS_SINT32 nExtraRegErrMaxCnt;
    IMS_SINT32 nExtraRegErrMinCnt;
    IMS_SINT32 nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached;
    IMS_SINT32 nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached;
    IMS_SINT32 nExtraRegErrPolicy;
    IMSVector<IMS_SINT32> objExtraRegErrCode;
    IMSVector<IMS_SINT32> objExtraReregErrCode;
    IMSVector<IMS_SINT32> objExtraRegErrWaitTimeSec;
};

struct AosNotifyTerminatedForInitRegBundle
{
public:
    AosNotifyTerminatedForInitRegBundle() :
            nWaitTimeForInitRegOnTerminatedState(0),
            objEventForInitRegOnTerminatedState(IMSVector<IMS_SINT32>()),
            objEventWithWtForInitRegOnTerminatedState(IMSVector<IMS_SINT32>())
    {
    }

    AosNotifyTerminatedForInitRegBundle(IN const AosNotifyTerminatedForInitRegBundle&) = delete;
    AosNotifyTerminatedForInitRegBundle& operator=(
            IN const AosNotifyTerminatedForInitRegBundle&) = delete;

public:
    IMS_SINT32 nWaitTimeForInitRegOnTerminatedState;
    IMSVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    IMSVector<IMS_SINT32> objEventWithWtForInitRegOnTerminatedState;
};

struct AosRegErrCodeWithRaTimeBundle
{
public:
    AosRegErrCodeWithRaTimeBundle() :
            bRegErrCodeWithRaTimeOnlyDefined(IMS_FALSE),
            objRegErrCodeWithRaTime(IMSVector<IMS_SINT32>()),
            objReregErrCodeWithRaTime(IMSVector<IMS_SINT32>())
    {
    }

    AosRegErrCodeWithRaTimeBundle(IN const AosRegErrCodeWithRaTimeBundle&) = delete;
    AosRegErrCodeWithRaTimeBundle& operator=(IN const AosRegErrCodeWithRaTimeBundle&) = delete;

public:
    IMS_BOOL bRegErrCodeWithRaTimeOnlyDefined;
    IMSVector<IMS_SINT32> objRegErrCodeWithRaTime;
    IMSVector<IMS_SINT32> objReregErrCodeWithRaTime;
};

struct AosRegRetryIntervalBundle
{
public:
    AosRegRetryIntervalBundle() :
            bUseRegRetryIntervalForSub(IMS_TRUE),
            objRegRetryRandomUpperValueSec(IMSVector<IMS_SINT32>()),
            objRegRetryIntervalSec(IMSVector<IMS_SINT32>())
    {
    }

    AosRegRetryIntervalBundle(IN const AosRegRetryIntervalBundle&) = delete;
    AosRegRetryIntervalBundle& operator=(IN const AosRegRetryIntervalBundle&) = delete;

public:
    IMS_BOOL bUseRegRetryIntervalForSub;
    IMSVector<IMS_SINT32> objRegRetryRandomUpperValueSec;
    IMSVector<IMS_SINT32> objRegRetryIntervalSec;
};

struct AosSubErrCodeForInitRegBundle
{
public:
    AosSubErrCodeForInitRegBundle() :
            nSubErrCodeForInitRegWithRetryMaxCnt(0),
            objSubErrCodeForInitReg(IMSVector<IMS_SINT32>())
    {
    }

    AosSubErrCodeForInitRegBundle(IN const AosSubErrCodeForInitRegBundle&) = delete;
    AosSubErrCodeForInitRegBundle& operator=(IN const AosSubErrCodeForInitRegBundle&) = delete;

public:
    IMS_SINT32 nSubErrCodeForInitRegWithRetryMaxCnt;
    IMSVector<IMS_SINT32> objSubErrCodeForInitReg;
};

struct AosSubErrCodeForTerminatedBundle
{
public:
    AosSubErrCodeForTerminatedBundle() :
            nSubErrCodeForTerminatedRetryMaxCnt(0),
            objSubErrCodeForTerminated(IMSVector<IMS_SINT32>())
    {
    }

    AosSubErrCodeForTerminatedBundle(IN const AosSubErrCodeForTerminatedBundle&) = delete;
    AosSubErrCodeForTerminatedBundle& operator=(
            IN const AosSubErrCodeForTerminatedBundle&) = delete;

public:
    IMS_SINT32 nSubErrCodeForTerminatedRetryMaxCnt;
    IMSVector<IMS_SINT32> objSubErrCodeForTerminated;
};

#endif  // AOS_ASSET_BUNDLE_H_
