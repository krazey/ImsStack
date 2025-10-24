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

#ifndef INTERFACE_MESSAGE_SENDER_H_
#define INTERFACE_MESSAGE_SENDER_H_

#include "ImsTypeDef.h"

struct CallReasonInfo;
enum class CallType;
enum class UpdateType;

class IMessageSender
{
public:
    virtual ~IMessageSender() {}

    /**
     * @brief Starts
     *
     * @param eCallType
     * @return
     */
    virtual IMS_RESULT Start(IN CallType eCallType) = 0;

    /**
     * @brief Sends
     *
     * @param eStatusCode
     * @param bReliable
     * @param bIncludeSdp
     * @param bIncludeAlertInfo
     * @return
     */
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
            IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo) = 0;

    /**
     * @brief Sends
     *
     * @return
     */
    virtual IMS_RESULT SendPrack() = 0;

    /**
     * @brief Responds
     *
     * @param eStatusCode
     * @return
     */
    virtual IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Sends
     *
     * @param eUpdateType
     * @return
     */
    virtual IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) = 0;

    /**
     * @brief Responds
     *
     * @param eStatusCode
     * @return
     */
    virtual IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Accepts
     *
     * @return
     */
    virtual IMS_RESULT Accept() = 0;

    /**
     * @brief Rejects
     *
     * @param objReason
     * @return
     */
    virtual IMS_RESULT Reject(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends
     *
     * @return
     */
    virtual IMS_RESULT SendAck() = 0;

    /**
     * @brief Updates
     *
     * @param eUpdateType
     * @param bIncludeAlertInfo
     * @param eMethod
     * @param bSessionRefresh
     * @return
     */
    virtual IMS_RESULT Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
            IN IMS_SINT32 eMethod, IN IMS_BOOL bSessionRefresh) = 0;

    /**
     * @brief Accepts
     *
     * @return
     */
    virtual IMS_RESULT AcceptUpdate() = 0;

    /**
     * @brief Cancels
     *
     * @param objReason
     * @return
     */
    virtual IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates
     *
     * @param bUseBye
     * @param objReason
     * @return
     */
    virtual IMS_RESULT Terminate(IN IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) = 0;
};

#endif
