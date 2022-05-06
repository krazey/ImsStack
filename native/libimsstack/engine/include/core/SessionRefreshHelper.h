/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090826  toastops@                 Created
    </table>

    Description

*/

#ifndef _SESSION_REFRESH_HELPER_H_
#define _SESSION_REFRESH_HELPER_H_

#include "util/RefreshHelper.h"

class ISipConnection;
class Service;

class SessionRefreshHelper : public RefreshHelper
{
public:
    SessionRefreshHelper(IN Service* pService_, IN IRefreshable* piRefreshable_);
    virtual ~SessionRefreshHelper();

public:
    virtual IMS_BOOL AddSpecificHeader(IN ISipConnection* piSC);
    virtual IMS_BOOL AddSpecificHeaderWithoutParameterChange(IN ISipConnection* piSC);
    virtual IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piSCC);
    virtual IMS_RESULT UpdateOnMessageReceived(IN CONST ISipConnection* piSC);
    virtual IMS_RESULT UpdateOnMessageSent(IN CONST ISipConnection* piSC);

    IMS_BOOL AddSpecificHeaderOnEarlyUPDATE(
            IN ISipConnection* piSC, IN IMS_BOOL bTimerOptionSupported);
    IMS_SINT32 GetRefreshMethod() const;
    IMS_BOOL IsSessionTimerSupported(
            IN CONST ISipConnection* piSC, IN IMS_BOOL bCheckSEPresentity = IMS_TRUE);
    IMS_BOOL IsSessionTimerSupportedBySessionExpires() const;
    void StopSessionTimer(IN CONST ISipConnection* piSC);
    void UpdateTimerOptionOnRequestReceived(IN CONST ISipConnection* piSC);

protected:
    virtual IMS_SINT32 GetTimerInterval() const;
    virtual void RefreshCompleted(IN ISipClientConnection* piSCC, IN IMS_SINT32 nCode = 0);
    virtual void RefreshStarted();
    virtual void RefreshTerminated();

    IMS_BOOL IsSessionTimerUpdateRequiredByReInvite() const override;

private:
    IMS_BOOL IsLocalSessionTimerRequired() const;
    IMS_BOOL IsMinSEHeaderRequired() const;
    IMS_BOOL IsRefresherParameterControlledOnEarlyUPDATE() const;
    IMS_BOOL IsRequireHeaderRequired() const;
    IMS_BOOL IsSessionExpiresHeaderRequired() const;
    IMS_BOOL IsSessionRefreshRequired(IN CONST ISipConnection* piSC) const;
    IMS_BOOL IsSessionTimerSupportedOnRemoteUA() const;
    IMS_BOOL IsSessionTimerSupportedOnUAC() const;
    void NegotiateRefresher(IN IMS_BOOL bTimerOptionSupported);
    void UpdateProperties(IN CONST ISipConnection* piSC, IN IMS_BOOL bTimerOptionSupported,
            IN IMS_BOOL bSent = IMS_FALSE);

    inline IMS_BOOL IsTimerSupportedOnRemoteEnd(IN IMS_SINT32 nFlag) const
    {
        return (nTimerSupportedOnRemoteEnd & nFlag) != 0;
    }
    inline void ClearTimerSupportedOnRemoteEnd(IN IMS_SINT32 nFlag)
    {
        nTimerSupportedOnRemoteEnd &= (~nFlag);
    }
    inline void SetTimerSupportedOnRemoteEnd(IN IMS_SINT32 nFlag)
    {
        nTimerSupportedOnRemoteEnd |= nFlag;
    }
    inline void SetOrClearTimerSupportedOnRemoteEnd(IN IMS_BOOL bSet, IN IMS_SINT32 nFlag)
    {
        if (bSet)
        {
            nTimerSupportedOnRemoteEnd |= nFlag;
        }
        else
        {
            nTimerSupportedOnRemoteEnd &= (~nFlag);
        }
    }

public:
    enum
    {
        RESULT_ERROR = (-1),
        RESULT_SUCCESS = 0,
        RESULT_REJECT_422,
        RESULT_REJECT_500
    };

private:
    enum
    {
        REFRESHER_NONE = 0,
        REFRESHER_LOCAL,
        REFRESHER_REMOTE
    };

    static const IMS_CHAR STR_REFRESHER[];
    static const IMS_CHAR STR_TIMER[];
    static const IMS_CHAR STR_UAC[];
    static const IMS_CHAR STR_UAS[];

    IMS_SINT32 nMinSE;
    IMS_SINT32 nSessionInterval;
    IMS_SINT32 nRefresher;
    IMS_SINT32 nRefreshRequest;
    IMS_BOOL bUPDATEAllowed;

    enum
    {
        // UAS: Incoming initial INVITE request
        TIMER_SUPPORTED_ON_INITIAL_INVITE = 0x0001,
        // UAS: Incoming re-INVITE or UPDATE request (session update)
        TIMER_SUPPORTED_ON_SESSION_UPDATE = 0x0002,
        // UAC/UAS: Most recent 200 OK response
        TIMER_SUPPORTED_ON_REMOTE_UA = 0x0004,
        // UAS: Temporary condition
        TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_INVITE = 0x4000,
        TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_UPDATE = 0x8000
    };

    IMS_SINT32 nTimerSupportedOnRemoteEnd;

    // SipConfigV :: SESSION_HEADER_XXX
    IMS_SINT32 nSIPHeaders;

    Service* pService;
};

#endif  // _SESSION_REFRESH_HELPER_H_
