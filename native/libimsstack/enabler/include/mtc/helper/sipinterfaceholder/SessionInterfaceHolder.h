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

#ifndef SESSION_INTERFACE_HOLDER_H_
#define SESSION_INTERFACE_HOLDER_H_

#include "ISessionListener.h"
#include "ITimer.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include <unordered_map>

class ISession;
class ICoreService;
class IInterfaceHolderListener;

/**
 * @brief Manages the lifecycle of ISession objects.
 *
 * This class is responsible for creating, tracking, and releasing ISession instances.
 * It uses a guard timer to ensure that sessions that are no longer needed are properly
 * destroyed, preventing resource leaks. It also notifies listeners when all sessions for a
 * specific call key have been released.
 */
class SessionInterfaceHolder : public ISessionListener, public ITimerListener
{
public:
    SessionInterfaceHolder();
    virtual ~SessionInterfaceHolder() override;
    SessionInterfaceHolder(IN const SessionInterfaceHolder&) = delete;
    SessionInterfaceHolder& operator=(IN const SessionInterfaceHolder&) = delete;

public:
    // ISessionListener interface implementation
    inline void SessionAlerting(IN ISession*) override {}
    inline void SessionReferenceReceived(IN ISession*, IN IReference*) override {}

    /**
     * @brief Handles the session started event.
     *
     * In a Multiple Early Session scenario, after `SessionStarted` is called for one session,
     * this handles subsequent `SessionStarted` (200 OK) calls for remaining forked sessions
     * by sending ACK and then BYE, to comply with GCF 7.24b.
     *
     * @param piSession The session that has started.
     */
    void SessionStarted(IN ISession* piSession) override;

    /**
     * @brief Handles the session start failed event. Releases the {@link ISession}.
     *
     * @param piSession The session that failed to start.
     */
    void SessionStartFailed(IN ISession* piSession) override;

    /**
     * @brief Handles the session terminated event. Releases the {@link ISession}.
     *
     * @param piSession The session that has been terminated.
     */
    void SessionTerminated(IN ISession* piSession) override;
    inline void SessionUpdated(IN ISession*) override {}
    inline void SessionUpdateFailed(IN ISession*) override {}
    inline void SessionUpdateReceived(IN ISession*) override {}
    inline void SessionCanceledOnAccepted(IN ISession*) override {}
    inline void SessionCancelDelivered(IN ISession*) override {}
    inline void SessionCancelDeliveryFailed(IN ISession*) override {}
    inline void SessionEarlyMediaUpdated(IN ISession*) override {}
    inline void SessionEarlyMediaUpdateFailed(IN ISession*) override {}
    inline void SessionEarlyMediaUpdateReceived(IN ISession*) override {}
    inline void SessionForkedResponseReceived(IN ISession*, IN ISession*) override {}
    inline void SessionPrackDelivered(IN ISession*) override {}
    inline void SessionPrackDeliveryFailed(IN ISession*) override {}
    inline void SessionPrackReceived(IN ISession*) override {}
    inline void SessionProvisionalResponseReceived(IN ISession*, IN IMS_UINT32) override {}
    inline void SessionRprDeliveryFailed(IN ISession*) override {}
    inline void SessionRprReceived(IN ISession*, IN IMS_UINT32) override {}
    inline void SessionTransactionReceived(IN ISession*, IN ISipServerConnection*) override {}

    void Timer_TimerExpired(IN ITimer* piTimer) override;

    /**
     * @brief Adds a listener for interface holder events.
     *
     * @param piListener The listener to add.
     */
    virtual void AddListener(IN IInterfaceHolderListener* piListener);

    /**
     * @brief Removes a listener for interface holder events.
     *
     * @param piListener The listener to remove.
     */
    virtual void RemoveListener(IN IInterfaceHolderListener* piListener);

    /**
     * @brief Creates and retrieves a new ISession.
     *
     * @param nKey The call key to associate with the session.
     * @param piCoreService A pointer to the {@link ICoreService} used to create the session.
     * @param strFrom The 'From' address for the session.
     * @param strTo The 'To' address for the session.
     * @return A pointer to the newly created ISession, or IMS_NULL on failure.
     */
    virtual ISession* GetISession(IN CallKey nKey, IN ICoreService* piCoreService,
            IN const AString& strFrom, IN const AString& strTo);

    /**
     * @brief Adds an existing ISession to be managed by the holder.
     *
     * @param nKey The call key to associate with the session.
     * @param piSession The session to add.
     */
    virtual void AddISession(IN CallKey nKey, IN ISession* piSession);

    /**
     * @brief Releases an ISession.
     *
     * This may start a guard timer before destroying the session to handle
     * any final SIP transactions gracefully.
     *
     * This is a convenience method that calls {@link ReleaseISession()} with default values.
     *
     * @param piSession The session to release.
     */
    virtual void ReleaseISession(IN ISession* piSession);

    /**
     * @brief Releases an ISession with more control over its destruction.
     *
     * @param piSession The session to release.
     * @param bEnforceDestroy If true, the session is destroyed immediately.
     * @param bSessionTerminatedOrStartFailed Indicates if the session has already terminated or
     *                                        failed to start.
     */
    virtual void ReleaseISession(IN ISession* piSession, IN IMS_BOOL bEnforceDestroy,
            IN IMS_BOOL bSessionTerminatedOrStartFailed);

    // For Unit Test.
    inline virtual IMS_UINT32 GetSessionCount() const { return m_objSessionRecords.size(); }

private:
    /**
     * @brief A private helper class to store a session and its associated guard timer.
     */
    class SessionRecord
    {
    public:
        explicit SessionRecord(IN ISession* piSession)
        {
            this->piSession = piSession;
            piTimer = IMS_NULL;
        }

        SessionRecord(IN const SessionRecord&) = delete;
        SessionRecord& operator=(IN const SessionRecord&) = delete;

        ISession* piSession;
        ITimer* piTimer;
    };

    /**
     * @brief Checks if a session is in a state where it can be safely destroyed.
     *
     * Ref. go/mtc-1p-sessionstate.
     *
     * @param piSession The session to check.
     * @param bSessionTerminatedOrStartFailed True if the session has already terminated or failed.
     * @return IMS_TRUE if the session is ready to be destroyed, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsReadyToDestroy(
            IN const ISession* piSession, IN IMS_BOOL bSessionTerminatedOrStartFailed);

    /**
     * @brief Starts a guard timer for the given session.
     *
     * @param piSession The session to start the timer for.
     */
    void StartTimer(IN const ISession* piSession);

    /**
     * @brief Stops and destroys the given timer.
     *
     * @param piTimer The timer to stop.
     */
    void StopTimer(IN ITimer* piTimer);

    /** Retrieves the timer associated with a given session. */
    ITimer* GetTimer(IN const ISession* piSession) const;

    /**
     * @brief The duration in milliseconds for the transaction guard timer.
     *
     * It's set by Timer_F with TERMINATED_TRANSACTION_MARGIN_MS.
     */
    IMS_SINT32 m_nTransactionGuardTimeMillis;

    /** A list of listeners to be notified of interface holder events. */
    ImsList<IInterfaceHolderListener*> m_objListeners;

    /**
     * @brief A map that stores SessionRecord objects, keyed by CallKey.
     *
     * This allows for tracking multiple sessions associated with a single call,
     * for example, in call forking scenarios.
     */
    std::unordered_multimap<CallKey, SessionRecord*> m_objSessionRecords;

    /** The margin time for transaction termination. */
    static const IMS_UINT32 TERMINATED_TRANSACTION_MARGIN_MS = 2000;
};

#endif
