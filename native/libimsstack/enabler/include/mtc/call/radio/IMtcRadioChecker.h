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

#ifndef INTERFACE_MTC_RADIO_CHECKER_H_
#define INTERFACE_MTC_RADIO_CHECKER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IMtcRadioCheckerListener;
enum class CheckResult;

class IMtcRadioChecker
{
public:
    virtual ~IMtcRadioChecker() = default;

    /**
     * @brief Sets
     *
     * @param pListener
     */
    virtual void SetTrafficCheckerListener(IN IMtcRadioCheckerListener* pListener) = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param bEmergency
     * @param ePeerType
     * @param bWifi
     * @param bUssi
     */
    virtual CheckResult Check(IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
            IN IMS_BOOL bWifi, IN IMS_BOOL bUssi, IN CallKey nCallKey) = 0;
};

class IMtcRadioCheckerListener
{
public:
    virtual ~IMtcRadioCheckerListener() = default;

    /**
     * @brief Notifies
     *
     */
    virtual void OnConnectionSetupPrepared() = 0;

    /**
     * @brief Notifies
     *
     * @param nFailureReason See ConnectionFailureReason of IImsRadio.
     * @param nWaitTimeMillis
     */
    virtual void OnConnectionFailed(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) = 0;
};

enum class CheckResult
{
    UNBLOCKED,
    BLOCKED,
    PENDING,
};

#endif
