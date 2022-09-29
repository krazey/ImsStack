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
            objRegErrCodeWithRaTimeForUpdate(IMSVector<IMS_SINT32>())
    {
    }

    AosRegErrCodeWithRaTimeBundle(IN const AosRegErrCodeWithRaTimeBundle&) = delete;
    AosRegErrCodeWithRaTimeBundle& operator=(IN const AosRegErrCodeWithRaTimeBundle&) = delete;

public:
    IMS_BOOL bRegErrCodeWithRaTimeOnlyDefined;
    IMSVector<IMS_SINT32> objRegErrCodeWithRaTime;
    IMSVector<IMS_SINT32> objRegErrCodeWithRaTimeForUpdate;
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

struct AosSpecificRegistrationErrorBundle
{
public:
    AosSpecificRegistrationErrorBundle() :
            nSpecificRegErrFinalType(0),
            nSpecificRegErrPolicy(0),
            nSpecificRegErrMaxCount(0),
            nSpecificRegErrMinCount(0),
            bSpecificRegErrRetryCountSharedForRegAndRegEvent(IMS_FALSE),
            objSpecificRegErrNumMultipliedByPcscfNum(IMSVector<IMS_SINT32>()),
            objSpecificRegErrCode(IMSVector<IMS_SINT32>()),
            objSpecificReregErrCode(IMSVector<IMS_SINT32>()),
            objSpecificRegErrWaitTimeSec(IMSVector<IMS_SINT32>()),
            bSpecificReregFailureWithErrCodeInRoaming(IMS_FALSE)
    {
    }

    AosSpecificRegistrationErrorBundle(IN const AosSpecificRegistrationErrorBundle&) = delete;
    AosSpecificRegistrationErrorBundle& operator=(
            IN const AosSpecificRegistrationErrorBundle&) = delete;

public:
    IMS_SINT32 nSpecificRegErrFinalType;
    IMS_SINT32 nSpecificRegErrPolicy;
    IMS_SINT32 nSpecificRegErrMaxCount;
    IMS_SINT32 nSpecificRegErrMinCount;
    IMS_BOOL bSpecificRegErrRetryCountSharedForRegAndRegEvent;
    IMSVector<IMS_SINT32> objSpecificRegErrNumMultipliedByPcscfNum;
    IMSVector<IMS_SINT32> objSpecificRegErrCode;
    IMSVector<IMS_SINT32> objSpecificReregErrCode;
    IMSVector<IMS_SINT32> objSpecificRegErrWaitTimeSec;
    IMS_BOOL bSpecificReregFailureWithErrCodeInRoaming;
};

struct AosRegistrationRetryBundle
{
public:
    AosRegistrationRetryBundle() :
            nRegistrationRetryMinCount(0),
            nRegistrationRetrySip305CodePolicy(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP),
            objRegistrationRetryErrorCodeWithoutIpsec(IMSVector<IMS_SINT32>()),
            nRegistrationRetryTimerFPolicy(CarrierConfig::Assets::TIMER_F_POLICY_NONE),
            objRegistrationRetryErrorCodeWithDifferentPcscf(IMSVector<IMS_SINT32>()),
            bRegistrationRetryWithIpVersionFallback(IMS_FALSE),
            nRegistrationRetryDefaultPolicy(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC),
            nRegistrationRetrySip503CodePolicy(CarrierConfig::Assets::SIP_305_CODE_POLICY_3GPP)
    {
    }

    AosRegistrationRetryBundle(IN const AosRegistrationRetryBundle&) = delete;
    AosRegistrationRetryBundle& operator=(IN const AosRegistrationRetryBundle&) = delete;

public:
    IMS_SINT32 nRegistrationRetryMinCount;
    IMS_SINT32 nRegistrationRetrySip305CodePolicy;
    IMSVector<IMS_SINT32> objRegistrationRetryErrorCodeWithoutIpsec;
    IMS_SINT32 nRegistrationRetryTimerFPolicy;
    IMSVector<IMS_SINT32> objRegistrationRetryErrorCodeWithDifferentPcscf;
    IMS_BOOL bRegistrationRetryWithIpVersionFallback;
    IMS_SINT32 nRegistrationRetryDefaultPolicy;
    IMS_SINT32 nRegistrationRetrySip503CodePolicy;
};

struct AosReregistrationRetryBundle
{
public:
    AosReregistrationRetryBundle() :
            objReregistrationRetryErrorCodeWithInitialRegistration(IMSVector<IMS_SINT32>()),
            bReregistrationRetryExpireTimeChecked(IMS_FALSE),
            nReregistrationRetryMaxCountKeptRegistration(0),
            objReregistrationRetryErrorCodeWithInitialRegistrationWithSamePcscf(
                    IMSVector<IMS_SINT32>()),
            nReregistrationRetrySip305CodePolicy(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT)
    {
    }

    AosReregistrationRetryBundle(IN const AosReregistrationRetryBundle&) = delete;
    AosReregistrationRetryBundle& operator=(IN const AosReregistrationRetryBundle&) = delete;

public:
    IMSVector<IMS_SINT32> objReregistrationRetryErrorCodeWithInitialRegistration;
    IMS_BOOL bReregistrationRetryExpireTimeChecked;
    IMS_SINT32 nReregistrationRetryMaxCountKeptRegistration;
    IMSVector<IMS_SINT32> objReregistrationRetryErrorCodeWithInitialRegistrationWithSamePcscf;
    IMS_SINT32 nReregistrationRetrySip305CodePolicy;
};

struct AosReregistrationErrorPolicyDuringCallBundle
{
public:
    AosReregistrationErrorPolicyDuringCallBundle() :
            objReregistrationErrorCodeWithCallEnd(IMSVector<IMS_SINT32>()),
            objReregistrationErrorCauseWithPdnReactivationAfterCallEnd(IMSVector<IMS_SINT32>())
    {
    }

    AosReregistrationErrorPolicyDuringCallBundle(
            IN const AosReregistrationErrorPolicyDuringCallBundle&) = delete;
    AosReregistrationErrorPolicyDuringCallBundle& operator=(
            IN const AosReregistrationErrorPolicyDuringCallBundle&) = delete;

public:
    IMSVector<IMS_SINT32> objReregistrationErrorCodeWithCallEnd;
    IMSVector<IMS_SINT32> objReregistrationErrorCauseWithPdnReactivationAfterCallEnd;
};

#endif  // AOS_ASSET_BUNDLE_H_
