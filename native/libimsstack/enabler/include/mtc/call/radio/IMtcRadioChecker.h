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

/**
 * The {@code IMtcRadioChecker} interface defines a listener for checking the radio traffic.
 */
class IMtcRadioChecker
{
public:
    virtual ~IMtcRadioChecker() = default;

    /**
     * Adds a traffic checker listener.
     *
     * @param objListener the listener to add
     */
    virtual void AddTrafficCheckerListener(IN IMtcRadioCheckerListener& objListener) = 0;

    /**
     * Removes a traffic checker listener.
     *
     * @param pListener the listener to remove
     */
    virtual void RemoveTrafficCheckerListener(IN IMtcRadioCheckerListener& objListener) = 0;

    /**
     * This will be called when the Call is terminated before creating a session.
     *
     * @param nCallKey the call key
     */
    virtual void OnTerminatedBeforeCreatingSession(IN CallKey nCallKey) = 0;

    /**
     * Checks the radio traffic.
     *
     * @param eCallType the call type
     * @param bEmergency {@code IMS_TRUE} if the call is an emergency call,
     *                   {@code IMS_FALSE} otherwise
     * @param ePeerType the peer type
     * @param eRatType the RAT type for this call
     * @param bUssi {@code IMS_TRUE} if the call is a USSI call, {@code IMS_FALSE} otherwise
     * @param nCallKey the call key
     * @return the check result. Refer {@code CheckResult}
     */
    virtual CheckResult Check(IN CallType eCallType, IN IMS_BOOL bEmergency, IN PeerType ePeerType,
            IN IMS_SINT32 eRatType, IN IMS_BOOL bUssi, IN CallKey nCallKey) = 0;
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
