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

#ifndef _UCE_PUBLISH_MANAGER_H_
#define _UCE_PUBLISH_MANAGER_H_

#include "ImsMessage.h"
#include "ImsStateMachine.h"
#include "IPublicationListener.h"
#include "ITimer.h"
#include "base/IMessageMediator.h"
#include "base/IRefreshListener.h"

#define GET_MESSAGE_FROM_RESPONSE IMS_TRUE
#define GET_MESSAGE_FOR_REQUEST   IMS_FALSE

#define INITIAL                   IMS_FALSE
#define REFRESH                   IMS_TRUE

class ICoreService;
class IPublication;
class ISipMessage;
class IUceJniThread;

class IPublicationData  // internal Param
{
public:
    inline IPublicationData()
    {
        m_nKey = 0;
        m_nExtended = 1;
        m_nCapability = 0;
    }
    inline virtual ~IPublicationData() {}

public:
    AString m_strEtag;
    IMS_UINT32 m_nKey;
    IMS_UINT32 m_nCapability;
    AString m_strPidfXml;
    IMS_UINT32 m_nExtended;
};

class IPublishResponseData  // internal Param
{
public:
    inline IPublishResponseData()
    {
        m_nResponseCode = 0;
        m_nReasonCause = 0;
    }
    inline virtual ~IPublishResponseData() {}

public:
    IMS_SINT32 m_nResponseCode;
    AString m_strReason;
    IMS_SINT32 m_nReasonCause;
    AString m_strReasonText;
};

class UcePublishManager :
        public IPublicationListener,
        public IRefreshListener,
        public ITimerListener,
        public IMessageMediator,
        public ImsStateMachine
{
    /* ------------------------------------------------------------------------------------------
        State Machine
    ---------------------------------------------------------------------------------------------
  */
    DECLARE_STATE_MAP()

    DECLARE_STATE_MSG_MAP(IDLE)  // before registration
    DECLARE_STATE_MSG_MAP(ON)    // registration done and don`t send any publish message to server.
    DECLARE_STATE_MSG_MAP(PUBLISHING)   // send publish message to server
    DECLARE_STATE_MSG_MAP(PUBLISHED)    // receive success response from server
    DECLARE_STATE_MSG_MAP(REFRESHING)   // receive success response from server
    DECLARE_STATE_MSG_MAP(TERMINATING)  // send unpublish message to server
    /* ------------------------------------------------------------------------------------------
        Constructor, Destructor, Operator Overloading
    ---------------------------------------------------------------------------------------------
  */
public:
    explicit UcePublishManager(IN ICoreService* _piCoreService, IN const AString& strAppName,
            IN IMS_SINT32 nSimSlot = 0);
    virtual ~UcePublishManager() override;

public:
    enum INTERNAL_TIMER
    {
        TIMER_EXPONENTIAL = 0,
        TIMER_RETRY = 1,
        TIMER_RETRYAFTER = 2,
        TIMER_ALL = 3
    };
    /* ------------------------------------------------------------------------------------------
        Methods
    ---------------------------------------------------------------------------------------------
  */
public:
    virtual IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL) override;
    IMS_BOOL SendPublishRequest(IN IMS_UINT32 key, IN const AString& pidfXml,
            IN const AString& eTag, IN IMS_UINT32 capability, IN IMS_UINT32 extended);
    IMS_BOOL AosConnected(IMS_UINT32 conectedService);  // AoS-connected
    IMS_BOOL AosDisConnected();                         // AoS-disconnected
    IMS_BOOL AosDisConnecting();                        // AoS-disconnecting
    IMS_BOOL ClosedService();                           // core service closed

    void UpdateState(IMS_UINT32 _eState);

protected:
    virtual void Timer_TimerExpired(IN ITimer* piTimer) override;
    // IPublicationListener - start
    virtual void PublicationDelivered(IN IPublication* piPublication) override;
    virtual void PublicationDeliveryFailed(IN IPublication* piPublication) override;
    virtual void PublicationTerminated(IN IPublication* piPublication) override;
    virtual void PublicationRefreshStarted(IN IPublication* piPublication) override;
    virtual void PublicationRefreshCompleted(IN IPublication* piPublication) override;
    // IPublicationListener - end

    // IRefreshListener - start
    virtual void Refresh_NotifyCompleted(IN ISipClientConnection* piScc) override;
    virtual void Refresh_NotifyTerminated() override;
    virtual void Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) override;
    // IRefreshListener - end

    // IDLE
    virtual IMS_BOOL StateIDLE_PublishRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateIDLE_AoSConnected(IN IMSMSG& objMsg);
    // ON
    virtual IMS_BOOL StateON_PublishRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateON_AoSDisConnected(IN IMSMSG& objMsg);
    // PUBLISHING
    virtual IMS_BOOL StatePUBLISHING_PublishRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHING_AoSDisConnecting(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHING_AoSDisConnected(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHING_Published(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHING_Failed(IN IMSMSG& objMsg);
    // PUBLISHED
    virtual IMS_BOOL StatePUBLISHED_PublishRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHED_AoSDisconnecting(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHED_AoSDisconnected(IN IMSMSG& objMsg);
    virtual IMS_BOOL StatePUBLISHED_RefreshStarted(IN IMSMSG& objMsg);
    // REFRESHING
    virtual IMS_BOOL StateREFRESHING_PublishRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateREFRESHING_AoSDisConnecting(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateREFRESHING_AoSDisConnected(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateREFRESHING_Refreshed(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateREFRESHING_RefreshFailed(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateREFRESHING_RefreshFailedWithNoResponse(IN IMSMSG& objMsg);
    // TERMINATING
    virtual IMS_BOOL StateTERMINATING_PublishRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateTERMINATING_AoSDisconnected(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateTERMINATING_Published(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateTERMINATING_Failed(IN IMSMSG& objMsg);
    // ALL
    virtual IMS_BOOL StateALL_Terminated(IN IMSMSG& objMsg);

private:
    void LoadConfigValue();
    static IPublishResponseData* GetPublishResponseData(const ISipMessage* piMessage);
    ISipMessage* GetISIPMessage(IMS_BOOL bRequireResponseMessage = IMS_FALSE);
    void SendPublishCommandErrorInd(IMS_UINT32 nKey, IMS_UINT32 nCommandError);
    void SendPublishResponseInd(IMS_UINT32 nKey, IMS_SINT32 nResponseCode, AString strReason,
            IMS_SINT32 nReasonHeaderCause, AString strReasonHeaderText, const AString& eTag,
            IMS_BOOL bNeedToRetry = IMS_FALSE);
    void SendUnpublishedInd();
    IMS_BOOL CreatePublication();
    void DestroyPublication();

    IMS_BOOL SetPublish(IN IMS_BOOL bIsRefresh, AString strMinExpiryValue = AString::ConstEmpty());
    void SetRefreshPolicy(
            ISipMessage* piMessage, AString strMinExpiryValue = AString::ConstEmpty());
    IMS_BOOL Publish();
    IMS_BOOL Unpublish();
    IMS_BOOL SetPidfXmlBody(ISipMessage* piMessage);
    void GetEtagAndExpireValue(const ISipMessage* piMessage);
    IMS_BOOL HandleFailResponse(IMS_SINT32 nResponseCode);
    void SetPublishStateToAoS(IN IMS_UINT32 nState);
    IMS_BOOL ProcessRetryAfterHeader();
    IMS_BOOL Process403Scenario();
    IMS_BOOL Process412Scenario();
    IMS_BOOL Process423Scenario(IMS_BOOL bIsRefresh);
    IMS_BOOL ProcessImmediatelyRetryResponseScenario();
    IMS_BOOL ProcessRetryResponseScenario();
    IMS_BOOL ProcessExponentialRetryResponseScenario();

    IMS_BOOL StartTimer(INTERNAL_TIMER eTimer, IMS_UINT32 nTime = 0);
    void StopTimer(INTERNAL_TIMER eTimer);

    void HandleExponentialRetryTimer();
    void HandleRetryTimer();
    void HandleRetryAfterTimer();

    IMS_BOOL RetryPublish(
            IMS_BOOL bRefresh, const AString& strMinExpiryValue = AString::ConstEmpty());

    void SendPendingPublishRequest();
    void ClearPendingPublishRequest();
    IMS_UINT32 GetState() const;
    static const IMS_CHAR* StateToString(IMS_UINT32 _eState);
    IUceJniThread* GetJniThread();
    /* ------------------------------------------------------------------------------------------
        Variables
    ---------------------------------------------------------------------------------------------
  */
protected:
    enum INTERNAL_MSG
    {
        PUBLISH_SUCCEEDED = 1,
        PUBLISH_FAILED,
        PUBLISH_TERMINATED,
        PUBLISH_REFRESH_STARTED,
        PUBLISH_REFRESHED,
        PUBLISH_REFRESH_FAILED,
        PUBLISH_REFRESH_NO_RESPONSE,
        TIMER_EXPIRED,
        AOS_CONNECTED,
        AOS_DISCONNECTED,
        AOS_DISCONNECTING,
        PUBLISH_REQUESTED,
        SERVICE_CLOSED,
    };

    enum
    {
        IDLE,
        ON,
        PUBLISHING,
        PUBLISHED,
        REFRESHING,
        TERMINATING,
    };
    enum
    {
        PUBLISH_STARTED,
        PUBLISH_STOPPED,
    };
    enum
    {
        SC_NO_RESPONSE = 4
    };
    IMS_UINT32 m_eState;
    IMS_UINT32 m_nKey;
    IMS_BOOL m_bReceivedUnPublishRequest;
    IPublicationData* m_pPendingPublicationData;
    // save registered service feature tags which included in contact header.
    IMS_UINT32 m_nConnectedServices;
    IPublication* m_piPublication;
    ITimer* m_pExponentialTimer;
    ITimer* m_pRetryTimer;
    ITimer* m_pRetryAfterTimer;

private:
    AString m_strPidfXml;
    AString m_strEtag;
    IMS_UINT32 m_nCapability;
    IMS_SINT32 m_nSimSlot;

    ICoreService* m_piCoreService;
    AString m_strAppName;

    // variable that caches the configuration value.
    ImsVector<IMS_SINT32> m_objExponentialRetryTimeSec;
    IMS_BOOL m_bAoSConnected;
    IMS_UINT32 m_nExtended;  // service availability = timer of publish refresh
    // If TMUS supports the encoded body, the pidf xml must be gzipped. If a
    // publish request with a compressed body fails, the publish request is
    // retried with the original body.
    IMS_BOOL m_bEnablePIDFCompression;
    IMS_BOOL m_bSetPublishStarted;
    IMS_BOOL m_bUnpublishSent;

    IMS_UINT32 m_nImmediatelyRetryCount;
    IMS_UINT32 m_nRetryCount;
    IMS_UINT32 m_nExponentialRetryCount;
};
#endif  // _UCE_PUBLISH_MANAGER_H_
