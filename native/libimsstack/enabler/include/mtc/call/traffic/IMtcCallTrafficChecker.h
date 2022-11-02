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

class IMtcCallTrafficCheckerListener;

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
     * @brief Checks
     *
     * @param eCallType
     * @param bEmergency
     * @return
     */
    virtual IMS_BOOL IsTrafficPrepared(IN CallType eCallType, IN IMS_BOOL bEmergency) const = 0;

    /**
     * @brief Checks
     *
     * @param eCallType
     * @param bEmergency
     * @return
     */
    virtual IMS_BOOL IsTrafficAllowed(IN CallType eCallType, IN IMS_BOOL bEmergency) const = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param bEmergency
     * @param bWifi
     */
    virtual void StartTrafficChecking(
            IN CallType eCallType, IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi) = 0;

    /**
     * @brief Stops
     *
     * @param eTrafficType
     */
    virtual void StopTrafficChecking(IN TrafficType eTrafficType) = 0;
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

class IMtcRadioConnectionListener
{
public:
    ~IMtcRadioConnectionListener() = default;

    /**
     * @brief Notifies
     *
     * @param eTrafficType
     * @param nFailureReason
     * @param nCauseCode
     * @param nWaitTimeMillis
     */
    virtual void OnConnectionFailed(IN TrafficType eTrafficType, IN IMS_UINT32 nFailureReason,
            IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis) = 0;

    /**
     * @brief Notifies
     *
     * @param eTrafficType
     */
    virtual void OnConnectionSetupPrepared(IN TrafficType eTrafficType) = 0;
};

#endif
