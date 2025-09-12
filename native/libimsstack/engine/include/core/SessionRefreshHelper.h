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
#ifndef SESSION_REFRESH_HELPER_H_
#define SESSION_REFRESH_HELPER_H_

#include "private/SipConfigV.h"

#include "util/RefreshHelper.h"

class ISipConnection;
class Service;

class SessionRefreshHelper : public RefreshHelper
{
public:
    SessionRefreshHelper(IN Service* pService, IN IRefreshable* piRefreshable);
    ~SessionRefreshHelper() override;

public:
    IMS_BOOL AddSpecificHeader(IN ISipConnection* piSc) override;
    IMS_RESULT SendRefreshRequest(IN ISipClientConnection* piScc) override;
    IMS_RESULT UpdateOnMessageReceived(IN const ISipConnection* piSc) override;
    IMS_RESULT UpdateOnMessageSent(IN const ISipConnection* piSc) override;

    IMS_BOOL AddSpecificHeaderWithoutParameterChange(IN ISipConnection* piSc);
    IMS_BOOL AddSpecificHeaderOnEarlyUpdate(
            IN ISipConnection* piSc, IN IMS_BOOL bTimerOptionSupported);
    IMS_SINT32 GetRefreshMethod() const;
    IMS_BOOL IsSessionTimerSupported(IN const ISipConnection* piSc, IN IMS_BOOL bSent);
    void StopSessionTimer(IN const ISipConnection* piSc);
    void UpdateTimerOptionOnRequestReceived(IN const ISipConnection* piSc);

protected:
    IMS_SINT32 GetTimerInterval() const override;
    void RefreshCompleted(IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    void RefreshStarted() override;
    inline void RefreshTerminated() override { Refreshable_RefreshTerminated(); }
    IMS_BOOL IsSessionTimerUpdateRequiredByReInvite() const override;

private:
    inline IMS_SINT32 GetSessionTimerDuration() const
    {
        return (m_nSessionTimerDuration > 0) ? m_nSessionTimerDuration
                                             : m_nLocalSessionTimerDuration;
    }
    inline IMS_BOOL HasSessionTimerDuration() const { return GetSessionTimerDuration() > 0; }
    inline IMS_BOOL IsLocalSessionTimerRequired() const
    {
        return ((m_nSipHeaders & SipConfigV::SESSION_HEADER_LOCAL_TIMER_REQUIRED) != 0);
    }
    inline IMS_BOOL IsMinSeHeaderRequired() const
    {
        return ((m_nSipHeaders & SipConfigV::SESSION_HEADER_MIN_SE) != 0);
    }
    inline IMS_BOOL IsRefresherParameterControlledOnEarlyUpdate() const
    {
        return ((m_nSipHeaders &
                        SipConfigV::SESSION_HEADER_NO_REFRESHER_CONTROLLED_ON_EARLY_UPDATE) == 0);
    }
    inline IMS_BOOL IsRequireHeaderRequired() const
    {
        return ((m_nSipHeaders & SipConfigV::SESSION_HEADER_REQUIRE_TIMER_OPTION) != 0);
    }
    inline IMS_BOOL IsSessionExpiresHeaderRequired() const
    {
        return ((m_nSipHeaders & SipConfigV::SESSION_HEADER_SESSION_EXPIRES) != 0);
    }
    IMS_BOOL IsSessionRefreshRequired(IN const ISipConnection* piSc) const;
    inline IMS_BOOL IsSessionTimerSupportedOnRemoteUa() const
    {
        return IsTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_REMOTE_UA);
    }
    inline IMS_BOOL IsSessionTimerSupportedOnUac() const
    {
        return IsTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_INITIAL_INVITE) ||
                IsTimerSupportedOnRemoteEnd(TIMER_SUPPORTED_ON_SESSION_UPDATE);
    }
    inline IMS_BOOL IsSessionTimerTurnOffAllowed() const
    {
        return ((m_nSipHeaders & SipConfigV::SESSION_HEADER_SESSION_TIMER_TURN_OFF_ALLOWED) != 0);
    }
    void NegotiateRefresher(IN IMS_BOOL bTimerOptionSupported);
    void UpdateProperties(IN const ISipConnection* piSc, IN IMS_BOOL bTimerOptionSupported,
            IN IMS_BOOL bSent = IMS_FALSE);

    inline IMS_BOOL IsTimerSupportedOnRemoteEnd(IN IMS_SINT32 nFlag) const
    {
        return (m_nTimerSupportedOnRemoteEnd & nFlag) != 0;
    }
    inline void ClearTimerSupportedOnRemoteEnd(IN IMS_SINT32 nFlag)
    {
        m_nTimerSupportedOnRemoteEnd &= (~nFlag);
    }
    inline void SetTimerSupportedOnRemoteEnd(IN IMS_SINT32 nFlag)
    {
        m_nTimerSupportedOnRemoteEnd |= nFlag;
    }
    inline void SetOrClearTimerSupportedOnRemoteEnd(IN IMS_BOOL bSet, IN IMS_SINT32 nFlag)
    {
        if (bSet)
        {
            m_nTimerSupportedOnRemoteEnd |= nFlag;
        }
        else
        {
            m_nTimerSupportedOnRemoteEnd &= (~nFlag);
        }
    }

    void SetSessionRefreshParameters(IN IMS_SINT32 nRefresher, IN IMS_SINT32 nRefreshRequest,
            IN IMS_SINT32 nMinSe, IN IMS_SINT32 nSessionTimerDuration,
            IN IMS_SINT32 nLocalSessionTimerDuration);
    static IMS_BOOL IsUpdateMethodSupported(IN const ISipMessage* piSipMsg);

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

    IMS_SINT32 m_nMinSe;
    IMS_SINT32 m_nSessionTimerDuration;
    // This duration is only used when the local session timer is required
    // even though the remote end doesn't support the session timer or
    // support the session timer, but doesn't specify the Session-Expires header.
    IMS_SINT32 m_nLocalSessionTimerDuration;
    IMS_SINT32 m_nRefresher;
    IMS_SINT32 m_nRefreshRequest;
    IMS_BOOL m_bUpdateMethodAllowed;

    enum
    {
        /// UAS: Incoming initial INVITE request
        TIMER_SUPPORTED_ON_INITIAL_INVITE = 0x0001,
        /// UAS: Incoming re-INVITE or UPDATE request (session update)
        TIMER_SUPPORTED_ON_SESSION_UPDATE = 0x0002,
        /// UAC/UAS: Most recent 200 OK response
        TIMER_SUPPORTED_ON_REMOTE_UA = 0x0004,
        /// UAS: Temporary condition
        TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_INVITE = 0x4000,
        TIMER_SUPPORTED_TEMPORARY_ON_INCOMING_UPDATE = 0x8000
    };

    IMS_SINT32 m_nTimerSupportedOnRemoteEnd;
    // SipConfigV :: SESSION_HEADER_XXX
    IMS_SINT32 m_nSipHeaders;
    Service* m_pService;
};

#endif
