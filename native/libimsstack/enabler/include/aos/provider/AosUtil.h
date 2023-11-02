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
#ifndef AOS_UTIL_H_
#define AOS_UTIL_H_

#include "AStringArray.h"

class ITimer;
class ITimerListener;
class ISipConfigV;
class ISipMessage;
class IRegistration;

/**
 * @brief This class provides useful services to be used commonly to Aos classes
 */

class AosUtil
{
public:
    AosUtil();
    virtual ~AosUtil();

    static AosUtil* GetInstance();

    // SIP Message
    IMS_SINT32 GetResponseCode(IN const ISipMessage* piSipMsg);
    IMS_UINT32 GetRetryAfterValue(IN const IRegistration* piRegistration);
    IMS_SINT32 GetRetryAfterValue(IN const ISipMessage* piSipMsg);
    IMS_SINT32 GetMinExpiresValue(IN const ISipMessage* piSipMsg);

    IMS_BOOL IsInitialRegistrationRequired(IN ISipMessage* piSipMsg);
    // Check whether some extension is unsupported (included in "Unsupported" header)
    IMS_BOOL IsParameterIncluded(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nHeaderType,
            IN const AString& strParameter);
    // Check whether parameter is included in specific header
    IMS_BOOL IsParameterIncluded(IN const ISipMessage* piSipMsg, IN IMS_SINT32 nHeaderType,
            IN const AString& strName, IN const AString& strParameter);

    // Configuration
    IMS_SINT32 GetLocalPort(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    // Feature (Bit Mask)
    void AddFeature(IN IMS_UINT32 nAdd, IN_OUT IMS_UINT32& nFeatures);
    void RemoveFeature(IN IMS_UINT32 nRemove, IN_OUT IMS_UINT32& nFeatures);
    IMS_BOOL IsFeatureOn(IN IMS_UINT32 nFeature, IN IMS_UINT32 nFeatures);
    void ClearFeature(IN_OUT IMS_UINT32& nFeatures);

    // Timer
    ITimer* StartTimer(IN IMS_UINT32 nDuration, IN ITimerListener* piListener,
            IN AString strLog = AString(""));
    void StopTimer(IN ITimer*& piTimer, IN AString strLog = AString(""));

    // List
    void AddElementToList(IN IMS_UINT32 nElement, IN ImsList<IMS_UINT32>& objTarget);

    IMS_BOOL IsListEqual(IN const AStringArray& objLeft, IN const AStringArray& objRight,
            IN IMS_BOOL bIsIpAddress = IMS_FALSE);
    IMS_BOOL IsStrExistInList(IN const AString& strValue, IN const AStringArray& objList,
            IN IMS_BOOL bIsIpAddress = IMS_FALSE);
    IMS_BOOL IsListEqual(IN const ImsList<IMS_UINT32>& objLeft,
            IN const ImsList<IMS_UINT32>& objRight, IN IMS_BOOL bOrderChecked);
    IMS_BOOL IsElementExistInList(
            IN const ImsList<IMS_UINT32>& objElements, IN const ImsList<IMS_UINT32>& objTarget);

    // Misc
    IMS_UINT32 Pow(IN IMS_UINT32 nArg1, IN IMS_UINT32 nArg2);
    IMS_UINT32 CalculateUpperBoundTime(
            IN IMS_UINT32 nBaseTime, IN IMS_UINT32 nMaxTime, IN IMS_UINT32 nConsecutiveFailCount);
    IMS_UINT32 WaitTimeForFlowRecovery(
            IN IMS_UINT32 nBaseTime, IN IMS_UINT32 nMaxTime, IN IMS_UINT32 nConsecutiveFailCount);

    void SetSocketOption(
            IN IMS_UINT32 nOption, IN IMS_UINT32 nValue, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetSocketOptionLinger(IN IMS_UINT32 nOption, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    void SetSocketOptionShutDown(IN IMS_UINT32 nOption, IN IMS_SINT32 nSlotId = IMS_SLOT_0);
    IMS_BOOL UpdateFeatureTagOptions(IN IMS_UINT32 nUpdatedFeatureTags, IN IMS_BOOL bIsSupported,
            IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    IMS_BOOL IsSupportedNetworkType(IN IMS_UINT32 nType) const;
    IMS_BOOL IsSupportedNetworkTypeForCellular(IN IMS_UINT32 nType) const;

    IMS_BOOL IsWifiTest() const;

    IMS_BOOL IsDifferentCountry(IN AString strSimCountry, IN IMS_SINT32 nSlotId) const;

    // Test
    void SetISipConfigV(IN ISipConfigV* piSipConfigV);
    void SetWifiTest(IN IMS_BOOL bEnabled);

private:
    // ( 2^24 * BaseTime ) MUST be bigger than MaxTime
    static const IMS_UINT32 REASONABLE_MAX_FAILURE_COUNT = 24;

    ISipConfigV* m_piSipConfigV;

    IMS_BOOL m_bIsWifiTest;

private:
    friend class AosUtilTest;
};
#endif  // AOS_UTIL_H_
