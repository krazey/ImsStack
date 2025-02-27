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
#ifndef INTERFACE_SESSION_LISTENER_H_
#define INTERFACE_SESSION_LISTENER_H_

#include "ISession.h"

class IReference;
class ISipServerConnection;

/**
 * @brief This class provides a listener interface to receive notification on session events.
 *
 * @see ISession
 */
class ISessionListener
{
protected:
    virtual ~ISessionListener() = default;

public:
    /**
     * @brief Notifies the application that the remote part's terminal is alerting the user
     *        of this session invitation.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionAlerting(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that a reference request has been received
     *        from a remote endpoint.
     *
     * Only references that are created in a session are notified in this method.
     *
     * @param piSession The concerned ISession
     * @param piReference The IReference representing the request
     */
    virtual void SessionReferenceReceived(IN ISession* piSession, IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application that the session has been established.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionStarted(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the session could not be established.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionStartFailed(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the session has been terminated or that the session
     *        could no longer stay established.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionTerminated(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the session has been updated.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionUpdated(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the session update has been rejected.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionUpdateFailed(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the remote endpoint adds more media components to
     *        an established session and the local endpoint is now expected to accept or
     *        reject the update.
     *
     * If the remote removed a media component, changed media directions,
     * or updated application-specific SDP attributes, then this method is not called.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionUpdateReceived(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the CANCEL request is received after 200 OK for INVITE
     *        request is already sent.
     *
     * This is only called when the session is in STATE_ESTABLISHING or STATE_ESTABLISHED.
     * If the CANCEL request is received during the session initiation or session update is
     * in progress (1xx sent), the SessionTerminated or SessionUpdateFailed will be called.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionCanceledOnAccepted(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the CANCEL operation is successfully done
     *        during an active call.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionCancelDelivered(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the CANCEL operation is failed during an active call.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionCancelDeliveryFailed(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the UPDATE was successfully delivered
     *        on the early state.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionEarlyMediaUpdated(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the UPDATE was not successfully delivered
     *        on the early state.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionEarlyMediaUpdateFailed(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        is received.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionEarlyMediaUpdateReceived(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the forked response is received.
     *
     * @param piSession The concerned ISession
     * @param piForkedSession The new ISession which contains a forked response
     */
    virtual void SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        was successfully delivered.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionPrackDelivered(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        was not successfully delivered.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionPrackDeliveryFailed(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the acknowledgement of the provisional response
     *        is received.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionPrackReceived(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the provisional response is received.
     *
     * @param piSession The concerned ISession
     * @param nIndex Index of the current response message\n
     *               #ISession#INDEX_MOST_RECENT_MESSAGE
     */
    virtual void SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex = ISession::INDEX_MOST_RECENT_MESSAGE) = 0;

    /**
     * @brief Notifies the application that the reliable provisional response delivery is failed.
     *
     * @param piSession The concerned ISession
     */
    virtual void SessionRprDeliveryFailed(IN ISession* piSession) = 0;

    /**
     * @brief Notifies the application that the reliable provisional response is received.
     *
     * @param piSession The concerned ISession
     * @param nIndex Index of the current response message\n
     *               #ISession#INDEX_MOST_RECENT_MESSAGE
     */
    virtual void SessionRprReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex = ISession::INDEX_MOST_RECENT_MESSAGE) = 0;

    /**
     * @brief Notifies the application that the new SIP transaction
     *        within the session dialog is received.
     *
     * @param piSession The concerned ISession
     * @param piSsc New ISipServerConnection which contains an incoming SIP request
     */
    virtual void SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSsc) = 0;
};

#endif
