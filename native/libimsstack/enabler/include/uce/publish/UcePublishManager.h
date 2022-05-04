/*
    Author
    <table>
    date              author                        description
    --------      --------------                ----------
    20120525      saurabh31.srivastava@           Created
    20121102    hyunho.shin@                   Re-Factorying
    20130819    jaesik.kong@                   Re-Factorying for one source
    </table>

    Description - UcePublishManager.h

*/

#ifndef _UCE_PUBLISH_MANAGER_H_
#define _UCE_PUBLISH_MANAGER_H_

#include "IMSStateMachine.h"
#include "IPublicationListener.h"
#include "ITimer.h"
#include "base/IMessageMediator.h"

#define GET_MESSAGE_FROM_RESPONSE IMS_TRUE
#define GET_MESSAGE_FOR_REQUEST IMS_FALSE

#define INITIAL IMS_FALSE
#define REFRESH IMS_TRUE

class ICoreService;
class IPublication;
class ISipMessage;

class IPublicationData  // internal Param
{
 public:
  inline IPublicationData() {
    m_nKey = 0;
    m_nExtended = 1;
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
  inline IPublishResponseData() {
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

class UcePublishManager : public IPublicationListener,
                          public ITimerListener,
                          public IMessageMediator,
                          public IMSStateMachine {
  /* ------------------------------------------------------------------------------------------
      State Machine
  ---------------------------------------------------------------------------------------------
*/
  DECLARE_STATE_MAP()

  DECLARE_STATE_MSG_MAP(IDLE)  // before registration
  DECLARE_STATE_MSG_MAP(
      ON)  // registration done and don`t send any publish message to server.
  DECLARE_STATE_MSG_MAP(PUBLISHING)   // send publish message to server
  DECLARE_STATE_MSG_MAP(PUBLISHED)    // receive success response from server
  DECLARE_STATE_MSG_MAP(REFRESHING)   // receive success response from server
  DECLARE_STATE_MSG_MAP(TERMINATING)  // send unpublish message to server
  /* ------------------------------------------------------------------------------------------
      Constructor, Destructor, Operator Overloading
  ---------------------------------------------------------------------------------------------
*/
 public:
  UcePublishManager(IN ICoreService *_piCoreService,
                    IN CONST AString &strAppName, IN IMS_SINT32 nSimSlot = 0);
  virtual ~UcePublishManager();

 public:
  enum INTERNAL_TIMER {
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
      IN_OUT ISipMessage *piSIPMsg, IN IMS_SINT32 nMessage = MESSAGE_NORMAL);
  IMS_BOOL SendPublishRequest(IN IMS_UINT32 key, IN AString pidfXml,
                              IN AString eTag, IN IMS_UINT32 capability,
                              IN IMS_UINT32 extended);
  IMS_BOOL AosConnected(IMS_UINT32 conectedService);  // AoS-connected
  IMS_BOOL AosDisConnected();                         // AoS-disconnected
  IMS_BOOL AosDisConnecting();                        // AoS-disconnecting
  void ClosedService();                               // core service closed
 protected:
  virtual void Timer_TimerExpired(IN ITimer *piTimer);
  // IPublicationListener - start
  virtual void PublicationDelivered(IN IPublication *piPublication);
  virtual void PublicationDeliveryFailed(IN IPublication *piPublication);
  virtual void PublicationTerminated(IN IPublication *piPublication);
  virtual void PublicationRefreshStarted(IN IPublication *piPublication);
  virtual void PublicationRefreshCompleted(IN IPublication *piPublication);
  // IPublicationListener - end

  // IDLE
  virtual IMS_BOOL StateIDLE_PublishRequested(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateIDLE_AoSConnected(IN IMSMSG &objMsg);
  // ON
  virtual IMS_BOOL StateON_PublishRequested(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateON_AoSDisConnected(IN IMSMSG &objMsg);
  // PUBLISHING
  virtual IMS_BOOL StatePUBLISHING_PublishRequested(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHING_AoSDisConnecting(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHING_AoSDisConnected(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHING_Published(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHING_Failed(IN IMSMSG &objMsg);
  // PUBLISHED
  virtual IMS_BOOL StatePUBLISHED_PublishRequested(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHED_AoSDisconnecting(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHED_AoSDisconnected(IN IMSMSG &objMsg);
  virtual IMS_BOOL StatePUBLISHED_RefreshStarted(IN IMSMSG &objMsg);
  // REFRESHING
  virtual IMS_BOOL StateREFRESHING_PublishRequested(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateREFRESHING_AoSDisConnecting(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateREFRESHING_AoSDisConnected(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateREFRESHING_Refreshed(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateREFRESHING_RefreshFailed(IN IMSMSG &objMsg);
  // TERMINATING
  virtual IMS_BOOL StateTERMINATING_PublishRequested(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateTERMINATING_AoSDisconnected(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateTERMINATING_Published(IN IMSMSG &objMsg);
  virtual IMS_BOOL StateTERMINATING_Failed(IN IMSMSG &objMsg);
  // ALL
  virtual IMS_BOOL StateALL_Terminated(IN IMSMSG &objMsg);

 private:
  void LoadConfigValue();
  IPublishResponseData *GetPublishResponseData(ISipMessage *piMessage);
  ISipMessage *GetISIPMessage(IMS_BOOL bRequireResponseMessage = IMS_FALSE);
  void SendPublishCommandErrorInd(IMS_UINT32 nKey, IMS_UINT32 nCommandError);
  void SendPublishResponseInd(IMS_UINT32 nKey, IMS_SINT32 nResponseCode,
                              AString strReason, IMS_SINT32 nReasonHeaderCause,
                              AString strReasonHeaderText, AString eTag,
                              IMS_BOOL bNeedToRetry = IMS_FALSE);
  void SendUnpublishedInd();
  IMS_BOOL CreatePublication();
  void DestroyPublication();

  IMS_BOOL SetPublish(IN IMS_BOOL bIsRefresh,
                      AString strMinExpiryValue = AString::ConstEmpty());
  void SetRefreshPolicy(ISipMessage *piMessage,
                        AString strMinExpiryValue = AString::ConstEmpty());
  IMS_BOOL Publish();
  IMS_BOOL Unpublish();
  IMS_BOOL SetPidfXmlBody(ISipMessage *piMessage);
  void GetEtagAndExpireValue(ISipMessage *piMessage);
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

  IMS_BOOL RetryPublish(IMS_BOOL bRefresh,
                        AString strMinExpiryValue = AString::ConstEmpty());

  void SendPendingPublishRequest();
  void ClearPendingPublishRequest();

  IMS_BOOL SetState(IMS_UINT32 _eState);
  const IMS_CHAR *StateToString(IMS_UINT32 _eState);
  /* ------------------------------------------------------------------------------------------
      Variables
  ---------------------------------------------------------------------------------------------
*/
 protected:
  enum INTERNAL_MSG {
    PUBLISH_SUCCEEDED = 1,
    PUBLISH_FAILED,
    PUBLISH_TERMINATED,
    PUBLISH_REFRESH_STARTED,
    PUBLISH_REFRESHED,
    PUBLISH_REFRESH_FAILED,
    TIMER_EXPIRED,
    AOS_CONNECTED,
    AOS_DISCONNECTED,
    AOS_DISCONNECTING,
    PUBLISH_REQUESTED,
    SERVICE_CLOSED,
  };

  enum {
    IDLE,
    ON,
    PUBLISHING,
    PUBLISHED,
    REFRESHING,
    TERMINATING,
  };
  enum {
    PUBLISH_STARTED,
    PUBLISH_STOPPED,
  };
  enum { SC_NO_RESPONSE = 4 };

 private:
  IMS_UINT32 m_nKey;
  AString m_strPidfXml;
  AString m_strEtag;
  IMS_UINT32 m_nCapability;
  IMS_SINT32 m_nSimSlot;

  ICoreService *m_piCoreService;
  IPublication *m_piPublication;
  AString m_strAppName;

  IMS_UINT32 m_eState;
  // variable that caches the configuration value.
  IMSVector<IMS_SINT32> m_objExponentialRetryTimeSec;
  // saving registered service for add feature tags to contact header.
  IMS_UINT32 m_nConnectedServices;
  IMS_BOOL m_bAoSConnected;
  IMS_UINT32 m_nExtended;  // service availability = timer of publish refresh
  // If TMUS supports the encoded body, the pidf xml must be gzipped. If a
  // publish request with a compressed body fails, the publish request is
  // retried with the original body.
  IMS_BOOL m_bReceivedFailResponse;
  IPublicationData *m_pPendingPublicationData;
  IMS_BOOL m_bReceivedUnPublishRequest;
  IMS_BOOL m_bSetPublishStarted;
  IMS_BOOL m_bUnpublishSent;

  ITimer *m_pExponentialTimer;
  ITimer *m_pRetryTimer;
  ITimer *m_pRetryAfterTimer;

  IMS_UINT32 m_nImmediatelyRetryCount;
  IMS_UINT32 m_nRetryCount;
  IMS_UINT32 m_nExponentialRetryCount;
};
#endif  // _UCE_PUBLISH_MANAGER_H_
