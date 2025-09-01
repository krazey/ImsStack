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

#ifndef _UCE_SUBSCRIBE_H_
#define _UCE_SUBSCRIBE_H_

#include "ImsActivityEx.h"
#include "ImsStateMachine.h"
#include "ISubscriptionListener.h"
#include "ITimer.h"
#include "base/IMessageMediator.h"

class ICoreService;
class ISubscription;
class ITimer;
class ISipMessage;
class UceXmlDocumentHelperThread;
class UceRlmiComposer;
class UceNonCapabilityUsers;
class IUceJniThread;

class ISubscribeResponseData  // internal Param
{
public:
    inline ISubscribeResponseData()
    {
        m_nResponseCode = 0;
        m_nReasonCause = 0;
    }
    inline virtual ~ISubscribeResponseData() {}

public:
    IMS_SINT32 m_nResponseCode;
    AString m_strReason;
    IMS_SINT32 m_nReasonCause;
    AString m_strReasonText;
};

class UceSubscribe :
        public ImsActivityEx,
        public ImsStateMachine,
        public ISubscriptionListener,
        public ITimerListener,
        public IMessageMediator
{
    DECLARE_STATE_MAP()

    DECLARE_STATE_MSG_MAP(ON)
    DECLARE_STATE_MSG_MAP(SUBSCRIBING)
    DECLARE_STATE_MSG_MAP(SUBSCRIBED)
    /* ------------------------------------------------------------------------------------------
        Constructor, Destructor, Operator Overloading
    ---------------------------------------------------------------------------------------------
  */
public:
    explicit UceSubscribe(IN ICoreService* piCoreService, IN const AString& strAppName,
            IN const AString& strManagerName, IN IMS_UINT32 conectedService,
            IN IMS_SINT32 nSimSlot = 0);
    virtual ~UceSubscribe() override;
    virtual IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL) override;

    IMS_BOOL QuerySingleCapability(IN const AString& strUser, IN IMS_UINT32 key);
    IMS_BOOL QueryMultiCapability(IN const ImsList<AString>& objUsers, IN IMS_UINT32 key);
    IMS_BOOL AosDisConnected();  // AoS-disconnected
    /* ------------------------------------------------------------------------------------------
        Methods
    -------------------------------------------------------------------------------------------*/
protected:
    virtual void Timer_TimerExpired(IN ITimer* piTimer) override;

    virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg) override;
    // IWatcherListener - start
    virtual void SubscriptionForkedNotify(
            IN ISubscription* piSubscription, IN ISubscription* piForkedSubscription) override;
    virtual void SubscriptionNotify(
            IN ISubscription* piSubscription, IN IMessage* piNotify) override;
    virtual void SubscriptionStarted(IN ISubscription* piSubscription) override;
    virtual void SubscriptionStartFailed(IN ISubscription* piSubscription) override;
    virtual void SubscriptionTerminated(IN ISubscription* piSubscription) override;
    // IWatcherListener - end

    virtual IMS_BOOL StateON_SingleSubscribeRequested(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateON_ListSubscribeRequested(IN IMSMSG& objMsg);

    virtual IMS_BOOL StateSUBSCRIBING_AoSDisConnected(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateSUBSCRIBING_Subscribed(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateSUBSCRIBING_SubscribeFailed(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateSUBSCRIBING_SubscribeTerminated(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateSUBSCRIBING_NotifyReceived(IN IMSMSG& objMsg);

    virtual IMS_BOOL StateSUBSCRIBED_AoSDisConnected(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateSUBSCRIBED_SubscribeTerminated(IN IMSMSG& objMsg);
    virtual IMS_BOOL StateSUBSCRIBED_NotifyReceived(IN IMSMSG& objMsg);

    void UpdateState(IMS_UINT32 _eState);

private:
    IUceJniThread* GetJniThread();
    void LoadConfigValue();
    void CreateXMLDocumentHelperThread();
    void DestroyXMLDocumentHelperThread();
    IMS_BOOL CreateSubscription(IN const AString& strToURI);
    void DestroySubscription();
    void SubscribeTerminated();

    static const IMS_CHAR* StateToString(IMS_UINT32 _eState);

    void SendSubscribeResponseInd(IMS_SINT32 nResponseCode, const AString& strReason,
            IMS_SINT32 nReasonHeaderCause, const AString& strReasonHeaderText);
    void SendSubscribeCommandErrorInd(IMS_UINT32 nCommandError);
    void SendPresenceNotifyInd(const ImsList<AString>& pidfXmls);
    void SendSubscribeResourceTerminatedInd(const UceNonCapabilityUsers* nonCapUsers);
    void SendSubscribeTerminatedInd();

    IMS_BOOL SetHeaderForSingleSubscription(IN_OUT ISipMessage* piSIPMessage) const;
    AString GetListSubscribeUri();
    void SetHeaderForListSubscription(
            IN_OUT ISipMessage* piSIPMessage, IN const AString& strListSubscriptionRequestUri);
    IMS_BOOL SetContentBody(IN_OUT ISipMessage* piSIPMessage, IN const AString& strXMLBody);
    ISipMessage* GetISIPMessage();

    IMS_BOOL SendSingleSubscribe();
    IMS_BOOL SendListSubscribe();

    static ISubscribeResponseData* GetSubscribeResponseData(const ISipMessage* piMessage);

    IMS_BOOL HandleRetryAfterHeader(const ISipMessage* piSIPMessage);
    IMS_BOOL Handle403FailureResponse(const ISipMessage* piSIPMessage);
    IMS_BOOL Handle423FailureResponse(const ISipMessage* piSIPMessage);
    IMS_BOOL HandleNotifyInd(IN const ISipMessage* piSIPMessage);

    void StartWaitingNotifyMessageTimer(IMS_UINT32 nDuration);
    void StopWaitingNotifyMessageTimer();
    void HandleWaitingNotifyMessageTimer();
    IMS_BOOL StartRetryAfterTimer(IMS_UINT32 nDuration);
    void StopRetryAfterTimer();
    void HandleRetryAfterTimer();

    /* ------------------------------------------------------------------------------------------
        Variable
    ---------------------------------------------------------------------------------------------
  */
public:
    enum
    {
        ON,
        SUBSCRIBING,
        SUBSCRIBED,
    };

protected:
    IMS_UINT32 m_nKey;
    ICoreService* m_piCoreService;
    ISubscription* m_piSubscription;
    AString m_strAppName;
    IMS_SINT32 m_nSimSlot;
    IMS_UINT32 m_nConnectedServices;
    AString m_strRemoteUser;
    ImsList<AString> m_objRemoteUsers;
    IMS_UINT32 m_eState;
    ITimer* m_pWaitNotifyMsgTimer;
    ITimer* m_pRetryAfterTimer;
    IMS_UINT32 m_eQueryType;
    IMS_SINT32 m_nThreadRunningCompleted;
    IMS_BOOL m_bSubscriptionTerminated;
    enum INTERNAL_MSG
    {
        SINGLE_REQUESTED = 1,
        LIST_REQUESTED,
        AOS_DISCONNECTED,
        SUBSCRIBE_SUCCEED,
        SUBSCRIBE_FAILED,
        RECEIVE_NOTIFIED,
        SUBSCRIBE_TERMINATED,
    };

private:
    enum  //
    {
        QUERY_CAPABILITY_TYPE_NONE = 0,
        QUERY_CAPABILITY_TYPE_SINGLE,
        QUERY_CAPABILITY_TYPE_LIST,
    };
    UceXmlDocumentHelperThread* m_pUceXmlDocumentHelperThread;
    AString m_strXMLDocumentHelperThreadName;
    AString m_strExpireValueInListSub;
    IMS_UINT32 m_nAnonymousMethod;

    AString m_strUceSubscribeManagerName;  // deleted
    UceRlmiComposer* m_pRLMIComposer;
    IMS_UINT32 m_nWaitNotiTimerValue;
};
#endif  // _UCE_SUBSCRIBE_H_
