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
#ifndef REFRESH_HELPER_H_
#define REFRESH_HELPER_H_

#include "ITimer.h"

#include "ISipClientConnection.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "base/IMessageMediator.h"

class IRefreshable;

class RefreshHelper :
        public ITimerListener,
        public ISipClientConnectionListener,
        public ISipErrorListener
{
public:
    RefreshHelper(IN IRefreshable* piRefreshable, IN IMS_BOOL bRepeatable);
    ~RefreshHelper() override;

public:
    void AbortConnection();
    inline ISipClientConnection* GetConnection() const { return m_piRefreshSc; }
    inline IMS_SINT32 GetDuration() const { return m_nDuration; }
    inline IMS_BOOL IsRequestPending() const { return (m_piRefreshSc != IMS_NULL); }
    inline IMS_BOOL IsTimerActive() const { return (m_piTimer != IMS_NULL); }
    inline void SetMessageMediator(IN IMessageMediator* piMediator)
    {
        m_piMessageMediator = piMediator;
    }
    void SetPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt);

protected:
    virtual IMS_BOOL AddSpecificHeader(IN ISipConnection* piSc) = 0;
    virtual IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piScc);
    virtual IMS_RESULT UpdateOnMessageReceived(IN const ISipConnection* piSc) = 0;
    virtual IMS_RESULT UpdateOnMessageSent(IN const ISipConnection* piSc) = 0;

    virtual IMS_SINT32 GetTimerInterval() const;
    virtual void RefreshCompleted(IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) = 0;
    virtual void RefreshStarted() = 0;
    virtual void RefreshTerminated() = 0;

    inline virtual IMS_BOOL IsSessionTimerUpdateRequiredByReInvite() const { return IMS_TRUE; }

    void Refreshable_RefreshCompleted(IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0);
    IMS_BOOL Refreshable_RefreshStarted();
    void Refreshable_RefreshTerminated();

    IMS_BOOL ConsumeRemainedTime();
    void SetConnection(IN ISipClientConnection* piScc);
    inline IMS_SINT32 GetPolicy() const { return m_nPolicy; }
    inline void SetDuration(IN IMS_SINT32 nDuration) { m_nDuration = nDuration; }
    IMS_BOOL StartRefresh();
    void StopRefresh();

private:
    // ITimerListener interface
    void Timer_TimerExpired(IN ITimer* piTimer) override;
    // ISipClientConnectionListener interface
    void ClientConnection_NotifyResponse(IN ISipClientConnection* piScc,
            IN ISipClientConnection* piForkedScc = IMS_NULL) override;
    // ISipErrorListener interface
    void Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;
    IMS_BOOL SetTimer(IN IMS_SINT32 nTimerDuration);

public:
    /// Refresh Policy for refresh helper
    enum
    {
        /// No refresh by engine
        POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.
        ///     nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        POLICY_SPEC = 0,

        /// Set the remain time before it is expired
        ///    nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired
        ///    nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)
        ///    nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)
        /// Ex) Expires: 3600, Ratio: 10
        ///        -> Refresh timer is expired after 3240s
        POLICY_RATIO
    };

    enum
    {
        TRANSACTION_TIMEOUT = (-1)
    };

private:
    enum
    {
        CRITERIA_INTERVAL = 1200
    };
    enum
    {
        MINIMUM_REMAIN_INTERVAL = 600
    };

    IRefreshable* m_piRefreshable;

    // Refresh policy
    IMS_SINT32 m_nPolicy;
    // Criteria value for the refresh duration
    IMS_SINT32 m_nCriteriaInterval;
    // It depends on the type; Value when the refresh duration is less than the base interval
    IMS_SINT32 m_nValueEorLt;
    // It depends on the type; Value when the refresh duration is equal or greater
    // than the base interval
    IMS_SINT32 m_nValueGt;
    IMS_BOOL m_bRepeatable;
    IMS_SINT32 m_nDuration;
    IMS_SINT32 m_nRemainDuration;
    ITimer* m_piTimer;
    ISipClientConnection* m_piRefreshSc;
    IMessageMediator* m_piMessageMediator;
};

#endif
