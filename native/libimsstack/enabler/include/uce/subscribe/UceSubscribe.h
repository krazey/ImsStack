/*
 */

#ifndef _UCE_SUBSCRIBE_H_
#define _UCE_SUBSCRIBE_H_

#include "IMSActivityEx.h"
#include "IMSStateMachine.h"
#include "ISubscriptionListener.h"
#include "ITimer.h"
#include "IUce.h"
#include "base/IMessageMediator.h"

class ICoreService;
class ISubscription;
class Subscription;
class ITimer;
class ISipMessage;
class UceXmlDocumentHelperThread;
class UceRlmiComposer;
class UceNonCapabilityUser;

class ISubscribeResponseData  // internal Param
{
 public:
  inline ISubscribeResponseData() {
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

class UceSubscribe : public IMSActivityEx,
                     public IMSStateMachine,
                     public ISubscriptionListener,
                     public ITimerListener,
                     public IMessageMediator {
  DECLARE_STATE_MAP()

  DECLARE_STATE_MSG_MAP(ON)
  DECLARE_STATE_MSG_MAP(SUBSCRIBING)
  DECLARE_STATE_MSG_MAP(SUBSCRIBED)
  /* ------------------------------------------------------------------------------------------
      Constructor, Destructor, Operator Overloading
  ---------------------------------------------------------------------------------------------
*/
 public:
  UceSubscribe(IN ICoreService* piCoreService, IN CONST AString& strAppName,
               IN CONST AString& strManagerName, IN IMS_UINT32 conectedService,
               IN IMS_SINT32 nSimSlot = 0);
  virtual ~UceSubscribe();
  virtual IMS_RESULT MessageMediator_AdjustMessage(
      IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL);

  IMS_BOOL QuerySingleCapability(IN AString strUser, IN IMS_UINT32 key);
  IMS_BOOL QueryMultiCapability(IN IMSList<AString> objUsers,
                                IN IMS_UINT32 key);
  IMS_BOOL AosDisConnected();  // AoS-disconnected
  /* ------------------------------------------------------------------------------------------
      Methods
  -------------------------------------------------------------------------------------------*/
 protected:
  virtual void Timer_TimerExpired(IN ITimer* piTimer);

  virtual IMS_BOOL OnMessage(IN IMSMSG& objMsg);
  // IWatcherListener - start
  virtual void SubscriptionForkedNotify(IN ISubscription* piSubscription,
                                        IN ISubscription* piForkedSubscription);
  virtual void SubscriptionNotify(IN ISubscription* piSubscription,
                                  IN IMessage* piNotify);
  virtual void SubscriptionStarted(IN ISubscription* piSubscription);
  virtual void SubscriptionStartFailed(IN ISubscription* piSubscription);
  virtual void SubscriptionTerminated(IN ISubscription* piSubscription);
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

 private:
  void LoadConfigValue();
  void CreateXMLDocumentHelperThread();
  void DestroyXMLDocumentHelperThread();
  IMS_BOOL CreateSubscription(IN CONST AString& strToURI);
  void DestroySubscription();
  void SubscribeTerminated();
  IMS_BOOL SetState(IMS_UINT32 _eState);
  const IMS_CHAR* StateToString(void);
  const IMS_CHAR* StateToString(IMS_UINT32 _eState);

  void SendSubscribeResponseInd(IMS_SINT32 nResponseCode, AString strReason,
                                IMS_SINT32 nReasonHeaderCause,
                                AString strReasonHeaderText);
  void SendSubscribeCommandErrorInd(IMS_UINT32 nCommandError);
  void SendPresenceNotifyInd(IMSList<AString> pidfXmls);
  void SendSubscribeResourceTerminatedInd(
      IMSList<UceNonCapabilityUser*>* pList);
  void SendSubscribeTerminatedInd();

  IMS_BOOL SetHeaderForSingleSubscription(IN_OUT ISipMessage* piSIPMessage);
  AString GetListSubscribeUri();
  IMS_BOOL SetHeaderForListSubscription(
      IN_OUT ISipMessage* piSIPMessage,
      IN CONST AString& strListSubscriptionRequestUri);
  IMS_BOOL SetContentBody(IN_OUT ISipMessage* piSIPMessage,
                          IN CONST AString& strXMLBody);
  ISipMessage* GetISIPMessage();

  IMS_BOOL SendSingleSubscribe();
  IMS_BOOL SendListSubscribe();

  ISubscribeResponseData* GetSubscribeResponseData(ISipMessage* piMessage);

  IMS_BOOL HandleRetryAfterHeader(ISipMessage* piSIPMessage);
  IMS_BOOL Handle403FailureResponse(ISipMessage* piSIPMessage);
  IMS_BOOL Handle423FailureResponse(ISipMessage* piSIPMessage);
  IMS_BOOL HandleNotifyInd(IN ISipMessage* piSIPMessage);

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
  enum {
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

  enum INTERNAL_MSG {
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
  IMS_UINT32 m_eState;
  UceXmlDocumentHelperThread* m_pUceXmlDocumentHelperThread;
  AString m_strXMLDocumentHelperThreadName;
  IMS_SINT32 m_nThreadRunningCompleted;

  AString m_strExpireValueInListSub;
  IMS_UINT32 m_nAnonymousMethod;
  IMS_UINT32 m_eQueryType;
  IMS_BOOL m_bSubscriptionTerminated;
  AString m_strRemoteUser;
  IMSList<AString> m_objRemoteUsers;
  AString m_strUceSubscribeManagerName;  // deleted
  UceRlmiComposer* m_pRLMIComposer;
  IMS_UINT32 m_nWaitNotiTimerValue;
  ITimer* m_pWaitNotifyMsgTimer;
  ITimer* m_pRetryAfterTimer;
};
#endif  // _UCE_SUBSCRIBE_H_
