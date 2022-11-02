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

#ifndef INTERFACE_MTC_CALL_TRAFFIC_CHECKER_H_
#define INTERFACE_MTC_CALL_TRAFFIC_CHECKER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

using TrafficType = IMS_UINT32;
using CallDirection = IMS_UINT32;

class IMtcCallTrafficCheckerListener;
enum class CheckResult;

class IMtcCallTrafficChecker
{
public:
    ~IMtcCallTrafficChecker() = default;

    /**
     * @brief Sets
     *
     * @param pListener
     */
    virtual void SetTrafficCheckerListener(IN IMtcCallTrafficCheckerListener* pListener) = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param bEmergency
     * @param ePeerType
     * @param bWifi
     */
    virtual CheckResult Check(IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
            IN IMS_BOOL bWifi) = 0;
};

class IMtcCallTrafficCheckerListener
{
public:
    ~IMtcCallTrafficCheckerListener() = default;

    /**
     * @brief Notifies
     *
     */
    virtual void OnConnectionFailed() = 0;

    /**
     * @brief Notifies
     *
     */
    virtual void OnConnectionSetupPrepared() = 0;
};

class IMtcRadioConnectionFailureListener
{
public:
    ~IMtcRadioConnectionFailureListener() = default;

    /**
     * @brief Notifies
     *
     * @param nCallKey
     */
    virtual void OnConnectionFailed(IN CallKey nCallKey) = 0;
};

enum class CheckResult
{
    UNBLOCKED,
    BLOCKED,
    PENDING,
};

#endif
