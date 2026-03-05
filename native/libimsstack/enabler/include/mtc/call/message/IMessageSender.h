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

/**
 * @brief Interface for sending SIP messages related to a call session.
 *
 * This interface defines methods to trigger the transmission of various SIP requests and responses
 * required for establishing, maintaining, updating, and terminating a multimedia call session.
 */
class IMessageSender
{
public:
    virtual ~IMessageSender() {}

    /**
     * @brief Starts the call session setup.
     *
     * @param eCallType The type of the call (e.g. Voice, Video).
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Start(IN CallType eCallType) = 0;

    /**
     * @brief Sends a provisional response (1xx) to the remote party.
     *
     * @param eStatusCode The SIP status code to send.
     * @param bReliable IMS_TRUE to send reliably (require PRACK), IMS_FALSE otherwise.
     * @param bIncludeSdp IMS_TRUE to include SDP in the body, IMS_FALSE otherwise.
     * @param bIncludeAlertInfo IMS_TRUE to include Alert-Info header, IMS_FALSE otherwise.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 eStatusCode, IN IMS_BOOL bReliable,
            IN IMS_BOOL bIncludeSdp, IN IMS_BOOL bIncludeAlertInfo) = 0;

    /**
     * @brief Sends a PRACK request to acknowledge a reliable provisional response.
     *
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT SendPrack() = 0;

    /**
     * @brief Sends a response to a received PRACK request.
     *
     * @param eStatusCode The SIP status code to send.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Sends an UPDATE request during the early dialog state.
     *
     * @param eUpdateType The type of update.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) = 0;

    /**
     * @brief Sends a response to a received UPDATE request during the early dialog state.
     *
     * @param eStatusCode The SIP status code to send.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Accepts the incoming call session (sends 200 OK).
     *
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Accept() = 0;

    /**
     * @brief Rejects the incoming call session.
     *
     * @param objReason The reason for rejection.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Reject(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends an ACK request to acknowledge the final response.
     *
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT SendAck() = 0;

    /**
     * @brief Sends a session update request (re-INVITE or UPDATE).
     *
     * @param eUpdateType The type of update (e.g. HOLD, RESUME).
     * @param bIncludeAlertInfo IMS_TRUE to include Alert-Info header, IMS_FALSE otherwise.
     * @param eMethod The SIP method to use (e.g. SipMethod::INVITE, SipMethod::UPDATE).
     * @param bSessionRefresh IMS_TRUE if this is a session timer refresh, IMS_FALSE otherwise.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Update(IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo,
            IN IMS_SINT32 eMethod, IN IMS_BOOL bSessionRefresh) = 0;

    /**
     * @brief Accepts the session update request.
     *
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT AcceptUpdate() = 0;

    /**
     * @brief Cancels the ongoing session update.
     *
     * @param objReason The reason for cancellation.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates the call session.
     *
     * @param bUseBye IMS_TRUE to use BYE method, IMS_FALSE to use CANCEL.
     * @param objReason The reason for termination.
     * @return IMS_SUCCESS if successful, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Terminate(IN IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) = 0;
};

#endif
