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

#include "ImsTypeDef.h"
#include "AString.h"
#include "ImsVector.h"
#include "CarrierConfig.h"

struct AosExtraRegErrBundle
{
public:
    AosExtraRegErrBundle() :
            bExtraReregFailureWithErrCodeInRoaming(IMS_FALSE),
            bExtraRegErrRetryCntSharedForRegAndSub(IMS_FALSE),
            nExtraRegErrFinalType(CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED),
            nExtraRegErrMaxCnt(0),
            nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached(0),
            nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached(0),
            nExtraRegErrPolicy(CarrierConfig::Ims::ERROR_POLICY_NOT_SPECIFIED),
            objExtraRegErrCode(ImsVector<IMS_SINT32>()),
            objExtraReregErrCode(ImsVector<IMS_SINT32>()),
            objExtraRegErrWaitTimeSec(ImsVector<IMS_SINT32>())
    {
    }

    AosExtraRegErrBundle(IN const AosExtraRegErrBundle&) = delete;
    AosExtraRegErrBundle& operator=(IN const AosExtraRegErrBundle&) = delete;

    void InitializeContainers()
    {
        objExtraRegErrCode.Clear();
        objExtraReregErrCode.Clear();
        objExtraRegErrWaitTimeSec.Clear();
    }

public:
    IMS_BOOL bExtraReregFailureWithErrCodeInRoaming;
    IMS_BOOL bExtraRegErrRetryCntSharedForRegAndSub;
    IMS_SINT32 nExtraRegErrFinalType;
    IMS_SINT32 nExtraRegErrMaxCnt;
    IMS_SINT32 nExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached;
    IMS_SINT32 nExtraRegErrPcscfsRepeatedCntForLteCombinedAttached;
    IMS_SINT32 nExtraRegErrPolicy;
    ImsVector<IMS_SINT32> objExtraRegErrCode;
    ImsVector<IMS_SINT32> objExtraReregErrCode;
    ImsVector<IMS_SINT32> objExtraRegErrWaitTimeSec;
};

struct AosNotifyTerminatedForInitRegBundle
{
public:
    AosNotifyTerminatedForInitRegBundle() :
            nWaitTimeForInitRegOnTerminatedState(0),
            objEventForInitRegOnTerminatedState(ImsVector<IMS_SINT32>()),
            objEventWithWtForInitRegOnTerminatedState(ImsVector<IMS_SINT32>())
    {
        objEventForInitRegOnTerminatedState.Add(1);
        objEventForInitRegOnTerminatedState.Add(2);
        objEventForInitRegOnTerminatedState.Add(3);
    }

    AosNotifyTerminatedForInitRegBundle(IN const AosNotifyTerminatedForInitRegBundle&) = delete;
    AosNotifyTerminatedForInitRegBundle& operator=(
            IN const AosNotifyTerminatedForInitRegBundle&) = delete;

    void InitializeContainers()
    {
        objEventForInitRegOnTerminatedState.Clear();
        objEventForInitRegOnTerminatedState.Add(1);
        objEventForInitRegOnTerminatedState.Add(2);
        objEventForInitRegOnTerminatedState.Add(3);
        objEventWithWtForInitRegOnTerminatedState.Clear();
    }

public:
    IMS_SINT32 nWaitTimeForInitRegOnTerminatedState;
    ImsVector<IMS_SINT32> objEventForInitRegOnTerminatedState;
    ImsVector<IMS_SINT32> objEventWithWtForInitRegOnTerminatedState;
};

struct AosPcscfRecoveryConditionsBundle
{
public:
    AosPcscfRecoveryConditionsBundle() :
            nMaxRetryCnt(3),
            nWaitTime(20),
            nBaseTime(20),
            nMaxTime(1800)
    {
    }

    AosPcscfRecoveryConditionsBundle(IN const AosPcscfRecoveryConditionsBundle&) = delete;
    AosPcscfRecoveryConditionsBundle& operator=(
            IN const AosPcscfRecoveryConditionsBundle&) = delete;

public:
    IMS_SINT32 nMaxRetryCnt;
    IMS_SINT32 nWaitTime;
    IMS_SINT32 nBaseTime;
    IMS_SINT32 nMaxTime;
};

struct AosRegErrCodeWithRaTimeBundle
{
public:
    AosRegErrCodeWithRaTimeBundle() :
            bRegErrCodeWithRaTimeOnlyDefined(IMS_FALSE),
            objRegErrCodeWithRaTime(ImsVector<IMS_SINT32>()),
            objReregErrCodeWithRaTime(ImsVector<IMS_SINT32>())
    {
    }

    AosRegErrCodeWithRaTimeBundle(IN const AosRegErrCodeWithRaTimeBundle&) = delete;
    AosRegErrCodeWithRaTimeBundle& operator=(IN const AosRegErrCodeWithRaTimeBundle&) = delete;

    void InitializeContainers()
    {
        objRegErrCodeWithRaTime.Clear();
        objReregErrCodeWithRaTime.Clear();
    }

public:
    IMS_BOOL bRegErrCodeWithRaTimeOnlyDefined;
    ImsVector<IMS_SINT32> objRegErrCodeWithRaTime;
    ImsVector<IMS_SINT32> objReregErrCodeWithRaTime;
};

struct AosRegRetryIntervalBundle
{
public:
    AosRegRetryIntervalBundle() :
            bUseRegRetryIntervalForSub(IMS_TRUE),
            objRegRetryRandomUpperValueSec(ImsVector<IMS_SINT32>()),
            objRegRetryIntervalSec(ImsVector<IMS_SINT32>())
    {
    }

    AosRegRetryIntervalBundle(IN const AosRegRetryIntervalBundle&) = delete;
    AosRegRetryIntervalBundle& operator=(IN const AosRegRetryIntervalBundle&) = delete;

    void InitializeContainers()
    {
        objRegRetryRandomUpperValueSec.Clear();
        objRegRetryIntervalSec.Clear();
    }

public:
    IMS_BOOL bUseRegRetryIntervalForSub;
    ImsVector<IMS_SINT32> objRegRetryRandomUpperValueSec;
    ImsVector<IMS_SINT32> objRegRetryIntervalSec;
};

struct AosSubErrCodeForInitRegBundle
{
public:
    AosSubErrCodeForInitRegBundle() :
            nSubErrCodeForInitRegWithRetryMaxCnt(0),
            objSubErrCodeForInitReg(ImsVector<IMS_SINT32>())
    {
    }

    AosSubErrCodeForInitRegBundle(IN const AosSubErrCodeForInitRegBundle&) = delete;
    AosSubErrCodeForInitRegBundle& operator=(IN const AosSubErrCodeForInitRegBundle&) = delete;

    void InitializeContainers() { objSubErrCodeForInitReg.Clear(); }

public:
    IMS_SINT32 nSubErrCodeForInitRegWithRetryMaxCnt;
    ImsVector<IMS_SINT32> objSubErrCodeForInitReg;
};

struct AosSubErrCodeForTerminatedBundle
{
public:
    AosSubErrCodeForTerminatedBundle() :
            nSubErrCodeForTerminatedRetryMaxCnt(0),
            objSubErrCodeForTerminated(ImsVector<IMS_SINT32>())
    {
    }

    AosSubErrCodeForTerminatedBundle(IN const AosSubErrCodeForTerminatedBundle&) = delete;
    AosSubErrCodeForTerminatedBundle& operator=(
            IN const AosSubErrCodeForTerminatedBundle&) = delete;

    void InitializeContainers() { objSubErrCodeForTerminated.Clear(); }

public:
    IMS_SINT32 nSubErrCodeForTerminatedRetryMaxCnt;
    ImsVector<IMS_SINT32> objSubErrCodeForTerminated;
};

struct AosWfcErrMessageBundle
{
public:
    AosWfcErrMessageBundle() {}

    AosWfcErrMessageBundle(IN const AosWfcErrMessageBundle&) = delete;
    AosWfcErrMessageBundle& operator=(IN const AosWfcErrMessageBundle&) = delete;

public:
    AString strWfcErrorReg403;
    AString strWfcErrorReg500;
    AString strWfcErrorNotSupportedCountry;
    AString strWfcErrorSub403;
    AString strWfcErrorNotifyTerminated;
    AString strWfcErrorOtherFailures;
};
#endif  // AOS_ASSET_BUNDLE_H_
