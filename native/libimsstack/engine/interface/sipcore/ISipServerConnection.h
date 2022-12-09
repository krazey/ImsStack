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
#ifndef INTERFACE_SIP_SERVER_CONNECTION_H_
#define INTERFACE_SIP_SERVER_CONNECTION_H_

#include "ISipConnection.h"

/**
 * @brief This class provides an interface to handle SIP server transaction.
 *
 * ISipServerConnection is created by the ISipConnectionNotifier when a new request is received.
 *
 * @see ISipConnection
 */
class ISipServerConnection : public ISipConnection
{
protected:
    virtual ~ISipServerConnection() = default;

public:
    /**
     * @brief Initializes ISipServerConnection with a specific SIP response
     *        to the received request.
     *
     * The default headers and reason phrase will be initialized automatically.\n
     * After this, the ISipServerConnection is in INITIALIZED state and the response can be sent.
     *
     * The following headers will be set by the method:
     *     - From
     *     - Call-ID
     *     - CSeq
     *     - To
     *     - Contact
     *
     * If the system has automatically sent the "100 Trying" response, the 100 response initialized
     * and sent by the user is just ignored.\n
     * If the system has automatically sent a response to a MESSAGE request, then this method will
     * throw ALREADY_RESPONDED.
     *
     * @param nStatusCode SIP response status code between 1xx and 6xx
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT InitResponse(IN IMS_SINT32 nStatusCode) = 0;

    /**
     * @brief Changes the default reason phrase.
     *
     * Empty string or null means that an empty (zero-length) reason phrase will be set.
     *
     * @param strReasonPhrase SIP reason phrase to be set
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetReasonPhrase(IN const AString& strReasonPhrase) = 0;

    /**
     * @brief Checks if the specified SIP server connection is the same transaction.
     *
     * This function can be called by CANCEL transaction only.\n
     * If the caller of this function is not a CANCEL SIP connection (transaction),
     * the function returns IMS_FALSE without any comparison.
     *
     * @param piOngoingSsc Pointer to SIP server connection to be compared\n
     *                     In this moment, INVITE server connection only supports.
     * @return If the specified server connection equals to this, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsSameTransaction(IN const ISipServerConnection* piOngoingSsc) const = 0;
};

#endif
