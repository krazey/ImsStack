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
#ifndef INTERFACE_ON_SESSION_LISTENER_H_
#define INTERFACE_ON_SESSION_LISTENER_H_

#include "Session.h"

class ISipServerConnection;
class Reference;

class IOnSessionListener
{
protected:
    virtual ~IOnSessionListener() = default;

public:
    /**
     * @brief Notifies the application that the remote part's terminal is alerting the user
     *        of this session invitation.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_Alerting(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that a reference request has been received
     *        from a remote endpoint.
     *
     * Only references that are created in a session are notified in this method.
     *
     * @param pSession The Session object
     * @param pReference The Reference object
     */
    virtual void OnSession_ReferenceReceived(IN Session* pSession, IN Reference* pReference) = 0;

    /**
     * @brief Notifies the application that the session has been established.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_Started(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the session could not be established.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_StartFailed(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the session has been terminated or that the session
     *        could no longer stay established.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_Terminated(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the session has been updated.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_Updated(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the session update has been rejected.
     *
     * This callback is invoked at both involved endpoints.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_UpdateFailed(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the remote endpoint adds more media components to
     *        an established session and the local endpoint is now expected to accept or
     *        reject the update.
     *
     * If the remote removed a media component, changed media directions, or updated
     * application-specific SDP attributes, then this method is not called.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_UpdateReceived(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the CANCEL operation is successfully done
     *        during an active call.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_CancelDelivered(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the CANCEL operation is failed during an active call.
     *
     * @param pSession The Session object
     */
    virtual void OnSession_CancelDeliveryFailed(IN Session* pSession) = 0;

    /**
     * @brief Notifies the application that the session has been received the forked response
     *        except for 100.
     *
     * @param pSession The Session object
     * @param pForkedSession The Session object that is forked
     * @return true if the transaction is successfully processed, false otherwise.
     */
    virtual IMS_BOOL OnSession_ForkedResponseReceived(
            IN Session* pSession, IN Session* pForkedSession) = 0;

    /**
     * @brief Notifies the application that the session has been received the provisional response
     *        except for 100.
     *
     * @param pSession The Session object
     * @param nIndex The index of the current response message\n
     *               #Session#INDEX_MOST_RECENT_MESSAGE
     */
    virtual void OnSession_ProvisionalResponseReceived(
            IN Session* pSession, IN IMS_UINT32 nIndex = Session::INDEX_MOST_RECENT_MESSAGE) = 0;

    /**
     * @brief Notifies the application that the remote endpoint starts a new SIP transaction
     *        within the session.
     *
     * @param pSession The Session object
     * @param piSsc The ISipServerConnection object
     * @return true if the transaction is successfully processed, false otherwise.
     */
    virtual IMS_BOOL OnSession_TransactionReceived(
            IN Session* pSession, IN ISipServerConnection* piSsc) = 0;
};

#endif
