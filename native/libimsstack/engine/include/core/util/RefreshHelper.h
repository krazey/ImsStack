/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090825  toastops@                 Created
    </table>

    Description

*/

#ifndef _REFRESH_HELPER_H_
#define _REFRESH_HELPER_H_

#include "ITimer.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "ISipClientConnection.h"
// SIP_MESSAGE_MEDIATOR
#include "base/IMessageMediator.h"

class IRefreshable;

class RefreshHelper :
        public ITimerListener,
        public ISipClientConnectionListener,
        public ISipErrorListener
{
public:
    RefreshHelper(IN IRefreshable* piRefreshable_, IN IMS_BOOL bRepeatable_);
    virtual ~RefreshHelper();

public:
    void AbortConnection();
    ISipClientConnection* GetConnection() const;
    IMS_SINT32 GetDuration() const;
    IMS_BOOL IsRequestPending() const;
    IMS_BOOL IsTimerActive() const;
    // SIP_MESSAGE_MEDIATOR
    void SetMessageMediator(IN IMessageMediator* piMediator);
    void SetPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLT, IN IMS_SINT32 nValueGT);

protected:
    virtual IMS_BOOL AddSpecificHeader(IN ISipConnection* piSC) = 0;
    virtual IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piSCC);
    virtual IMS_RESULT UpdateOnMessageReceived(IN CONST ISipConnection* piSC) = 0;
    virtual IMS_RESULT UpdateOnMessageSent(IN CONST ISipConnection* piSC) = 0;

    virtual IMS_SINT32 GetTimerInterval() const;
    virtual void RefreshCompleted(IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0) = 0;
    virtual void RefreshStarted() = 0;
    virtual void RefreshTerminated() = 0;

    inline virtual IMS_BOOL IsSessionTimerUpdateRequiredByReInvite() const { return IMS_TRUE; }

    void Refreshable_RefreshCompleted(IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    IMS_BOOL Refreshable_RefreshStarted();
    void Refreshable_RefreshTerminated();

    IMS_BOOL ConsumeRemainedTime();
    IMS_SINT32 GetPolicy() const;
    void SetDuration(IN IMS_SINT32 nDuration);
    IMS_BOOL StartRefresh();
    void StopRefresh();

private:
    // ITimerListener interface
    virtual void Timer_TimerExpired(IN ITimer* piTimer);
    // ISipClientConnectionListener interface
    virtual void ClientConnection_NotifyResponse(
            IN ISipClientConnection* piSCC, IN ISipClientConnection* piForkedSCC = IMS_NULL);
    // ISipErrorListener interface
    virtual void Error_NotifyError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);
    IMS_BOOL SetTimer(IN IMS_SINT32 nTimerDuration);

public:
    // Refresh Policy for refresh helper
    enum
    {
        // No refresh by engine
        POLICY_NO_REFRESH = (-1),

        // Default policy; Select the refresh time according to 3GPP spec.
        //     nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Ratio when the refresh duration is equal or less
        //              than the criteria interval (1 ~ 100; default 50)
        //    nValueGT : Interval value when the refresh duration is greater
        //              than the criteria interval
        POLICY_SPEC = 0,

        // Set the remain time before it is expired
        //    nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Interval value when the refresh duration is equal or less
        //              than the criteria interval
        //    nValueGT : Interval value when the refresh duration is greater
        //              than the criteria interval
        POLICY_REMAIN_TIME,

        // Set the ratio before it is expired
        //    nCriteriaInterval : Criteria value for the refresh duration
        //    nValueEorLT : Ratio when the refresh duration is equal or less
        //              than the criteria interval (1 ~ 100)
        //    nValueGT : Ratio when the refresh duration is greater
        //              than the criteria interval (1 ~ 100)
        // Ex) Expires: 3600, Ratio: 10
        //        -> Refresh timer is expired after 3240s
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

    IRefreshable* piRefreshable;

    // Refresh policy
    IMS_SINT32 nPolicy;
    // Criteria value for the refresh duration
    IMS_SINT32 nCriteriaInterval;
    // It depends on the type; Value when the refresh duration is less than the base interval
    IMS_SINT32 nValueEorLT;
    // It depends on the type; Value when the refresh duration is equal or greater
    // than the base interval
    IMS_SINT32 nValueGT;

    IMS_BOOL bRepeatable;
    IMS_SINT32 nDuration;
    IMS_SINT32 nRemainDuration;
    ITimer* piTimer;

    ISipClientConnection* piRefreshSC;

    // SIP_MESSAGE_MEDIATOR
    IMessageMediator* piMessageMediator;
};

#endif  // _REFRESH_HELPER_H_
