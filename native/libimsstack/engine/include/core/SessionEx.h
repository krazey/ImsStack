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
#ifndef SESSION_EX_H_
#define SESSION_EX_H_

#include "ITimer.h"
#include "Session.h"

class IOnSessionExListener;
class ReliableProvResponseHelper;

class SessionEx : public Session, public ITimerListener
{
public:
    explicit SessionEx(IN Service* pService);
    ~SessionEx() override;

    SessionEx(IN const SessionEx&) = delete;
    SessionEx& operator=(IN const SessionEx&) = delete;

public:
    IMS_RESULT RespondToEarlyUpdate(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull());
    IMS_RESULT RespondToPrack(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull());
    IMS_RESULT SendPrack();
    IMS_RESULT SendRpr(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_BOOL bSdp = IMS_TRUE,
            IN IMS_SINT32 nFlags = 0);
    IMS_RESULT UpdateEarlyMedia();
    void AbortEarlyUpdateTransaction();
    inline void SetExListener(IN IOnSessionExListener* piListener) { m_piListener = piListener; }

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    IMS_BOOL NotifySipRequest(IN ISipServerConnection* piSsc) override;
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;

    // IDialogMethod interface
    IMS_BOOL Dialog_NotifyRequest(IN ISipServerConnection* piSsc) override;

    // Session class
    Session* CreateSession() override;
    IMS_RESULT HandleProvisionalResponse(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nServiceMethod) override;
    IMS_RESULT HandleRequestToUpdate(IN ISipServerConnection* piSsc) override;
    IMS_RESULT HandleResponseToUpdate(IN ISipClientConnection* piScc) override;
    IMS_BOOL HasPendingPrack() const override;
    IMS_BOOL IsEarlyUpdateInProgress() const override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

private:
    AString AdjustSessionExpiresHeader(
            IN const AString& strRequestSe, IN const AString& strResponseSe);
    IMS_BOOL CheckNCreateRprHelper(IN ISipMessage* piSipMsg);
    void DestroyRprHelper();

    void HandleRequestToPrack(IN ISipServerConnection* piSsc);
    void HandleResponseToPrack(IN ISipClientConnection* piScc);

    // RACE_CONDITION : SESSION_EARLY_UPDATE
    inline IMS_BOOL IsEarlyUpdateNotificationInProgress() const
    {
        return m_bEarlyUpdateNotificationInProgress;
    }
    inline void SetEarlyUpdateNotificationState(IN IMS_BOOL bInProgress)
    {
        m_bEarlyUpdateNotificationInProgress = bInProgress;
    }
    void SetEarlyState(IN IMS_UINT32 nState);

    IMS_BOOL IsIncomingEarlyUpdateReceivedInShortTime() const;
    void SetLastEarlyUpdateCompletedTime(IN IMS_SINT32 nExplicitTime = (-1));

    IMS_BOOL StartPendingUpdateTimer();
    void StopPendingUpdateTimer();

    static const IMS_CHAR* EarlyStateToString(IN IMS_SINT32 nState);

protected:
    enum
    {
        AMSG_SESSIONEX_INVITATION_RECEIVED = AMSG_SESSION_MAX,
        AMSG_SESSIONEX_EARLY_MEDIA_UPDATED,
        AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_FAILED,
        AMSG_SESSIONEX_EARLY_MEDIA_UPDATE_RECEIVED,
        AMSG_SESSIONEX_PRACK_DELIVERED,
        AMSG_SESSIONEX_PRACK_DELIVERY_FAILED,
        AMSG_SESSIONEX_PRACK_RECEIVED,
        AMSG_SESSIONEX_RPR_DELIVERY_FAILED,
        AMSG_SESSIONEX_RPR_RECEIVED,

        AMSG_SESSIONEX_MAX
    };

private:
    enum
    {
        EARLY_STATE_IDLE = 0,
        EARLY_STATE_UPDATE_RECEIVED = 1,
        EARLY_STATE_UPDATE_SENT = 2
    };

    IMS_UINT32 m_nEarlyState;
    ReliableProvResponseHelper* m_pRprHelper;
    IOnSessionExListener* m_piListener;
    // RACE_CONDITION: 200 OK to UPDATE and incoming UPDATE
    IMS_BOOL m_bEarlyUpdateNotificationInProgress;
    ISipServerConnection* m_piSscPendingUpdate;
    ITimer* m_piTimerPendingUpdate;
    IMS_UINT32 m_nLastEarlyUpdateCompletedTimeSec;
    IMS_UINT32 m_nLastEarlyUpdateCompletedTimeMicroSec;
};

#endif
