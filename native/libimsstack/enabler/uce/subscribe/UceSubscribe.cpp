#include "subscribe/UceSubscribe.h"

#include "AoSAppRequestType.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISubscription.h"
#include "ImsAosParameter.h"
#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceUtil.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "TextParser.h"
#include "config/UceConfig.h"
#include "def/UceDef.h"
#include "subscribe/UceNonCapabilityUser.h"
#include "subscribe/UceNotifyBodyPartData.h"
#include "subscribe/UceNotifyMessageBody.h"
#include "subscribe/UceRlmiComposer.h"
#include "subscribe/UceXmlDocumentHelperThread.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

BEGIN_STATE_MAP(UceSubscribe)
STATE_ENTRY(ON)
STATE_ENTRY(SUBSCRIBING)
STATE_ENTRY(SUBSCRIBED)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(UceSubscribe, ON)
STATE_MSG_ENTRY(SINGLE_REQUESTED,
                &UceSubscribe::StateON_SingleSubscribeRequested)
STATE_MSG_ENTRY(LIST_REQUESTED, &UceSubscribe::StateON_ListSubscribeRequested)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UceSubscribe, SUBSCRIBING)
STATE_MSG_ENTRY(AOS_DISCONNECTED,
                &UceSubscribe::StateSUBSCRIBING_AoSDisConnected)
STATE_MSG_ENTRY(SUBSCRIBE_SUCCEED, &UceSubscribe::StateSUBSCRIBING_Subscribed)
STATE_MSG_ENTRY(SUBSCRIBE_FAILED,
                &UceSubscribe::StateSUBSCRIBING_SubscribeFailed)
STATE_MSG_ENTRY(SUBSCRIBE_TERMINATED,
                &UceSubscribe::StateSUBSCRIBING_SubscribeTerminated)
STATE_MSG_ENTRY(RECEIVE_NOTIFIED,
                &UceSubscribe::StateSUBSCRIBING_NotifyReceived)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(UceSubscribe, SUBSCRIBED)
STATE_MSG_ENTRY(AOS_DISCONNECTED,
                &UceSubscribe::StateSUBSCRIBED_AoSDisConnected)
STATE_MSG_ENTRY(SUBSCRIBE_TERMINATED,
                &UceSubscribe::StateSUBSCRIBED_SubscribeTerminated)
STATE_MSG_ENTRY(RECEIVE_NOTIFIED, &UceSubscribe::StateSUBSCRIBED_NotifyReceived)
END_STATE_MSG_MAP()
/* -------------------------------------------------------------------------------------------------
    Constructor, Destructor, Operator Overloading
-------------------------------------------------------------------------------------------------
*/
UceSubscribe::UceSubscribe(IN ICoreService* piCoreService,
                           IN CONST AString& strAppName,
                           IN CONST AString& strManagerName,
                           IN IMS_UINT32 conectedService,
                           IN IMS_SINT32 nSimSlot)
    : IMSActivityEx(AString::ConstNull()),
      m_nKey(0),
      m_piCoreService(piCoreService),
      m_piSubscription(IMS_NULL),
      m_strAppName(strAppName),
      m_nSimSlot(nSimSlot),
      m_nConnectedServices(conectedService),
      m_eState(ON),
      m_pUceXmlDocumentHelperThread(IMS_NULL),
      m_nThreadRunningCompleted(0),
      m_strExpireValueInListSub("30"),
      m_nAnonymousMethod(0),
      m_eQueryType(QUERY_CAPABILITY_TYPE_NONE),
      m_bSubscriptionTerminated(IMS_FALSE),
      m_strRemoteUser(AString::ConstNull()),
      m_strUceSubscribeManagerName(strManagerName),
      m_pRLMIComposer(IMS_NULL),
      m_nWaitNotiTimerValue(15000),
      m_pWaitNotifyMsgTimer(IMS_NULL),
      m_pRetryAfterTimer(IMS_NULL) {
  IMS_TRACE_D("UCE_M : UceSubscribe = %" PFLS_u, sizeof(UceSubscribe), 0, 0);
  IMS_TRACE_I("UceSubscribe", 0, 0, 0);
  if (m_piCoreService == IMS_NULL) {
    IMS_TRACE_E(0, "[ERROR]UceSubscribe - m_piCoreService is NULL", 0, 0, 0);
    return;
  }
  m_strXMLDocumentHelperThreadName =
      AString().Sprintf("UceXmlDocumentHelperThread%d", GetSlotId());
  LoadConfigValue();
  SetState(ON);
}

PUBLIC VIRTUAL UceSubscribe::~UceSubscribe() {
  IMS_TRACE_D("UCE_F : UceSubscribe = %" PFLS_u, sizeof(UceSubscribe), 0, 0);
  IMS_TRACE_I("~UceSubscribe", 0, 0, 0);
  DestroyXMLDocumentHelperThread();
  /*delete the presence source if it is there*/
  DestroySubscription();
  StopWaitingNotifyMessageTimer();
  StopRetryAfterTimer();
  if (m_pRLMIComposer != IMS_NULL) {
    delete m_pRLMIComposer;
    m_pRLMIComposer = IMS_NULL;
  }
}
/* -------------------------------------------------------------------------------------------------
    Methods
-------------------------------------------------------------------------------------------------
*/
PUBLIC
IMS_RESULT UceSubscribe::MessageMediator_AdjustMessage(
    IN_OUT ISipMessage* piSIPMsg, IN IMS_SINT32 nMessage) {
  IMS_TRACE_D("MessageMediator_AdjustMessage:nMessage [%d]", nMessage, 0, 0);
  if (piSIPMsg == null) {
    IMS_TRACE_I("MessageMediator_AdjustMessage:piSIPMsg is null", 0, 0, 0);
    return IMS_SUCCESS;
  }
  if (piSIPMsg->GetMethod().Equals(SipMethod::SUBSCRIBE) == IMS_FALSE) {
    IMS_TRACE_I("MessageMediator_AdjustMessage:it is not subscribe method.", 0,
                0, 0);
    return IMS_SUCCESS;
  }
  AString strContactHeader = piSIPMsg->GetHeader(ISipHeader::CONTACT_NORMAL);
  if (UceConfig::GetInstance()->GetBoolValue(
          UceConfig::KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE, m_nSimSlot) ==
      IMS_FALSE) {
    IMS_TRACE_I("MessageMediator_AdjustMessage:m_bUseContactHeader is false", 0,
                0, 0);
    return IMS_SUCCESS;
  }

  IMS_BOOL bAppendIARITag = IMS_FALSE;
  IMS_BOOL bAppendICSITag = IMS_FALSE;
  IMS_BOOL bNeedToComma = IMS_FALSE;

  AString strContactHeaderValue = IMS_NULL;

  if ((m_nConnectedServices & CONNECTED_SERVICE_CPM_SESSION) !=
      CONNECTED_SERVICE_CPM_SESSION) {
    IMS_TRACE_I("MessageMediator_AdjustMessage : CPM is not registered.", 0, 0,
                0);
    return IMS_SUCCESS;
  }

  if (strContactHeader.Contains(UceTag::TAG_CHAT) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CPM_SESSION) ==
          CONNECTED_SERVICE_CPM_SESSION) {
    if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE &&
        bAppendICSITag == IMS_FALSE) {
      bAppendICSITag = IMS_TRUE;
      strContactHeaderValue.Append(UceTag::TAG_ICSI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    strContactHeaderValue.Append(UceTag::TAG_CHAT);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_CALL_COMPOSER) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CALL_COMPOSER) ==
          CONNECTED_SERVICE_CALL_COMPOSER) {
    if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE &&
        bAppendICSITag == IMS_FALSE) {
      bAppendICSITag = IMS_TRUE;
      strContactHeaderValue.Append(UceTag::TAG_ICSI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_CALL_COMPOSER);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_STANDALONE_PAGER_MESSAGING) ==
          IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CPM_MSG) ==
          CONNECTED_SERVICE_CPM_MSG) {
    if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE &&
        bAppendICSITag == IMS_FALSE) {
      bAppendICSITag = IMS_TRUE;
      strContactHeaderValue.Append(UceTag::TAG_ICSI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_STANDALONE_PAGER_MESSAGING);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_STANDALONE_LARGE_MESSAGING) ==
          IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CPM_LARGEMSG) ==
          CONNECTED_SERVICE_CPM_LARGEMSG) {
    if (strContactHeader.Contains(UceTag::TAG_ICSI) == IMS_FALSE &&
        bAppendICSITag == IMS_FALSE) {
      bAppendICSITag = IMS_TRUE;
      strContactHeaderValue.Append(UceTag::TAG_ICSI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_STANDALONE_LARGE_MESSAGING);
    bNeedToComma = IMS_TRUE;
  }
  if (bAppendICSITag == IMS_TRUE) {
    strContactHeaderValue.Append(TextParser::STR_DQUOTE);
  }
  bNeedToComma = IMS_FALSE;

  if (strContactHeader.Contains(UceTag::TAG_PRESENCE) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_PRESENCE) ==
          CONNECTED_SERVICE_PRESENCE) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    strContactHeaderValue.Append(UceTag::TAG_PRESENCE);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_FILE_TRANSFER_HTTP) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_HTTPFT) ==
          CONNECTED_SERVICE_HTTPFT) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_FILE_TRANSFER_HTTP);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_GEOLOCATION_PUSH) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_GEOPUSH) ==
          CONNECTED_SERVICE_GEOPUSH) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_GEOLOCATION_PUSH);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_FT_SMS) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_FTSMS) ==
          CONNECTED_SERVICE_FTSMS) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_FT_SMS);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_GEOLOCATIONPUSH_SMS) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_GEOSMS) ==
          CONNECTED_SERVICE_GEOSMS) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_GEOLOCATIONPUSH_SMS);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_CHATBOT_SESSION) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT) ==
          CONNECTED_SERVICE_CHATBOT) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_CHATBOT_SESSION);
    bNeedToComma = IMS_TRUE;
  }
  if (strContactHeader.Contains(UceTag::TAG_CHATBOT_STANDALONE_MESSAGE) ==
          IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG) ==
          CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG) {
    if (strContactHeader.Contains(UceTag::TAG_IARI) == IMS_FALSE &&
        bAppendIARITag == IMS_FALSE) {
      bAppendIARITag = IMS_TRUE;
      if (bAppendICSITag == IMS_TRUE) {
        strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
      }
      strContactHeaderValue.Append(UceTag::TAG_IARI);
      strContactHeaderValue.Append(TextParser::STR_DQUOTE);
    }
    if (bNeedToComma == IMS_TRUE) {
      strContactHeaderValue.Append(TextParser::CHAR_COMMA);
    }
    strContactHeaderValue.Append(UceTag::TAG_CHATBOT_STANDALONE_MESSAGE);
    bNeedToComma = IMS_TRUE;
  }
  if (bAppendIARITag == IMS_TRUE) {
    strContactHeaderValue.Append(TextParser::STR_DQUOTE);
  }
  bNeedToComma = IMS_FALSE;

  if (strContactHeader.Contains(UceTag::TAG_CHATBOT_VERSION_V1) == IMS_FALSE &&
      (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT_V1) ==
          CONNECTED_SERVICE_CHATBOT_V1) {
    strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
    strContactHeaderValue.Append(UceTag::TAG_CHATBOT_VERSION_V1);
  } else if (strContactHeader.Contains(UceTag::TAG_CHATBOT_VERSION_V2) ==
                 IMS_FALSE &&
             (m_nConnectedServices & CONNECTED_SERVICE_CHATBOT_V2) ==
                 CONNECTED_SERVICE_CHATBOT_V2) {
    strContactHeaderValue.Append(TextParser::CHAR_SEMICOLON);
    strContactHeaderValue.Append(UceTag::TAG_CHATBOT_VERSION_V2);
  }

  if (strContactHeader.IsEmpty() == IMS_TRUE) {
    piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeaderValue);
  } else {
    strContactHeader = strContactHeader + ";" + strContactHeaderValue;
    piSIPMsg->SetHeader(ISipHeader::CONTACT_NORMAL, strContactHeader);
  }
  return IMS_SUCCESS;
}

IMS_BOOL UceSubscribe::QuerySingleCapability(IN AString strUser,
                                             IN IMS_UINT32 key) {
  IMS_TRACE_I("QuerySingleCapability - Trigger single capability discovery", 0,
              0, 0);
  m_nKey = key;
  m_strRemoteUser = strUser;
  IMSMSG objMSG(SINGLE_REQUESTED, 0, 0);
  OnStateMessage(objMSG);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::QueryMultiCapability(IN IMSList<AString> objUsers,
                                            IN IMS_UINT32 key) {
  IMS_TRACE_I("QueryMultiCapability - Trigger multi capability discovery", 0, 0,
              0);
  m_nKey = key;
  m_objRemoteUsers = objUsers;
  IMSMSG objMSG(LIST_REQUESTED, 0, 0);
  OnStateMessage(objMSG);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::AosDisConnected() {
  IMS_TRACE_I("AosDisConnected", 0, 0, 0);
  IMSMSG objMSG(AOS_DISCONNECTED, 0, 0);
  OnStateMessage(objMSG);
  return IMS_TRUE;
}

PROTECTED VIRTUAL void UceSubscribe::Timer_TimerExpired(IN ITimer* piTimer) {
  IMS_TRACE_I("Timer_TimerExpired ", 0, 0, 0);
  if (piTimer == IMS_NULL) {
    IMS_TRACE_I("Timer_TimerExpired:piTimer is null", 0, 0, 0);
  } else if (piTimer == m_pWaitNotifyMsgTimer) {
    IMS_TRACE_I("Timer_TimerExpired:Wait Notify Timer is expired", 0, 0, 0);
    HandleWaitingNotifyMessageTimer();
  } else if (piTimer == m_pRetryAfterTimer) {
    IMS_TRACE_I("Timer_TimerExpired:Retry After Timer is expired", 0, 0, 0);
    HandleRetryAfterTimer();
  }
}

IMS_BOOL UceSubscribe::OnMessage(IN IMSMSG& objMsg) {
  if (objMsg.nMSG != IUUceService::UCE_XML_PARSE_COMPLETED_IND) {
    IMS_TRACE_I("OnMessage() - Not Support MSG", 0, 0, 0);
    return IMS_FALSE;
  }

  m_nThreadRunningCompleted--;
  IMSList<UceNonCapabilityUser*>* pNonCapabilities =
      (IMSList<UceNonCapabilityUser*>*)objMsg.nWparam;

  IMSList<AString> objPidfXmls = *(IMSList<AString>*)objMsg.nLparam;

  if (pNonCapabilities->GetSize() != 0) {
    SendSubscribeResourceTerminatedInd(pNonCapabilities);
  }
  if (objPidfXmls.GetSize() != 0) {
    SendPresenceNotifyInd(objPidfXmls);
  }

  if (m_bSubscriptionTerminated == IMS_TRUE && m_nThreadRunningCompleted == 0) {
    IMS_TRACE_I(
        "OnMessage:Subscription was terminated and XML Thread was completed", 0,
        0, 0);
    SendSubscribeTerminatedInd();
    SubscribeTerminated();
    return IMS_TRUE;
  }
  IMS_TRACE_I(
      "OnMessage:subscribe is terminated[%d] or xml thread completed [%d]",
      m_bSubscriptionTerminated, m_nThreadRunningCompleted, 0);
  return IMS_TRUE;
}

void UceSubscribe::SubscriptionForkedNotify(
    IN ISubscription* piSubscription, IN ISubscription* piForkedSubscription) {
  IMS_TRACE_D("SubscriptionForkedNotify", 0, 0, 0);
  (void)piSubscription;
  (void)piForkedSubscription;
}

void UceSubscribe::SubscriptionNotify(IN ISubscription* piSubscription,
                                      IN IMessage* piNotify) {
  if (piNotify == IMS_NULL || piSubscription == IMS_NULL ||
      piSubscription != m_piSubscription) {
    IMS_TRACE_I("SubscriptionNotify:Not matched", 0, 0, 0);
    return;
  }
  IMS_TRACE_D("SubscriptionNotify:m_nThreadRunningCompleted[%d]",
              m_nThreadRunningCompleted, 0, 0);
  IMSMSG objMSG(RECEIVE_NOTIFIED, reinterpret_cast<IMS_UINTP>(piNotify), 0);
  OnStateMessage(objMSG);
}

void UceSubscribe::SubscriptionStarted(IN ISubscription* piSubscription) {
  if (piSubscription == IMS_NULL || piSubscription != m_piSubscription) {
    IMS_TRACE_I("SubscriptionStarted:Not matched", 0, 0, 0);
    return;
  }
  IMS_TRACE_I("SubscriptionStarted:received response", 0, 0, 0);
  IMSMSG objMSG(SUBSCRIBE_SUCCEED, 0, 0);
  OnStateMessage(objMSG);
}

void UceSubscribe::SubscriptionStartFailed(IN ISubscription* piSubscription) {
  if (piSubscription == IMS_NULL || piSubscription != m_piSubscription ||
      m_eQueryType == QUERY_CAPABILITY_TYPE_NONE) {
    IMS_TRACE_I("SubscriptionStartFailed:Not matched", 0, 0, 0);
    return;
  }
  IMessage* piMessage = IMS_NULL;
  if (m_eQueryType == QUERY_CAPABILITY_TYPE_SINGLE) {
    piMessage =
        piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_POLL);
  } else {
    piMessage =
        piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE);
  }

  if (piMessage == IMS_NULL || piMessage->GetMessage() == IMS_NULL) {
    IMS_TRACE_I("SubscriptionStartFailed:IMessage or ISipMessage is null", 0, 0,
                0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return;
  }

  IMS_SINT32 nErrorResponse = 0;
  nErrorResponse = piMessage->GetMessage()->GetStatusCode();
  if (nErrorResponse < 0) {
    nErrorResponse = SipStatusCode::SC_408;
  }
  IMS_TRACE_I("SubscriptionStartFailed:received response[%d]", nErrorResponse,
              0, 0);
  IMSMSG objMSG(SUBSCRIBE_FAILED, nErrorResponse, 0);
  OnStateMessage(objMSG);
}

void UceSubscribe::SubscriptionTerminated(IN ISubscription* piSubscription) {
  IMS_TRACE_I("SubscriptionTerminated:m_nThreadRunningCompleted[%d]",
              m_nThreadRunningCompleted, 0, 0);
  if (m_piSubscription == piSubscription) {
    DestroySubscription();
  }
  IMSMSG objMSG(SUBSCRIBE_TERMINATED, 0, 0);
  OnStateMessage(objMSG);
  return;
}

IMS_BOOL UceSubscribe::StateON_SingleSubscribeRequested(IN IMSMSG& objMsg) {
  (void)objMsg;
  m_eQueryType = QUERY_CAPABILITY_TYPE_SINGLE;
  if (m_strRemoteUser == IMS_NULL || m_strRemoteUser.IsEmpty()) {
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_INVALID_PARAM);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  IMS_TRACE_I("StateON_SingleSubscribeRequested:remote user[%s]",
              m_strRemoteUser.GetStr(), 0, 0);
  if (m_strRemoteUser.StartsWith("sip:") != IMS_TRUE &&
      m_strRemoteUser.StartsWith("tel:") != IMS_TRUE) {
    IMS_TRACE_I("StateON_SingleSubscribeRequested:user uri is wrong", 0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_INVALID_PARAM);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  if (CreateSubscription(m_strRemoteUser) != IMS_TRUE) {
    IMS_TRACE_I("StateON_SingleSubscritionReq:CreateSubscription() is failed",
                0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  if (SetHeaderForSingleSubscription(GetISIPMessage()) != IMS_TRUE) {
    IMS_TRACE_I(
        "StateON_SingleSubscritionReq:SetHeaderForSingleSubscription() is "
        "failed",
        0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  if (SendSingleSubscribe() != IMS_TRUE) {
    IMS_TRACE_I("StateON_SingleSubscritionReq:SendSingleSubscribe() is failed",
                0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  SetState(SUBSCRIBING);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateON_ListSubscribeRequested(IN IMSMSG& objMsg) {
  (void)objMsg;
  IMS_TRACE_I("StateON_ListSubscribeRequested", 0, 0, 0);
  m_eQueryType = QUERY_CAPABILITY_TYPE_LIST;
  if (m_objRemoteUsers.IsEmpty() || m_objRemoteUsers.GetSize() <= 0 ||
      m_piCoreService == IMS_NULL) {
    IMS_TRACE_I("mUsers is empty or m_piCoreService is null", 0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_INVALID_PARAM);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  AString strListSubscriptionRequestUri = GetListSubscribeUri();
  if (strListSubscriptionRequestUri == IMS_NULL ||
      CreateSubscription(strListSubscriptionRequestUri) != IMS_TRUE) {
    IMS_TRACE_I("Create subscription is failed", 0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }

  ISipMessage* piSIPMessage = GetISIPMessage();
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("ISipMessage is null", 0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }

  m_pRLMIComposer = new UceRlmiComposer();
  AString strXMLBody = m_pRLMIComposer->ComposeRLMIList(m_objRemoteUsers);
  IMS_TRACE_D("strXMLBody [%s]", strXMLBody.GetStr(), 0, 0);

  if (SetHeaderForListSubscription(piSIPMessage,
                                   strListSubscriptionRequestUri) != IMS_TRUE ||
      SetContentBody(piSIPMessage, strXMLBody) != IMS_TRUE) {
    IMS_TRACE_I("SetHeaderForListSubscription() or SetContentBody() is failed",
                0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  if (SendListSubscribe() != IMS_TRUE) {
    IMS_TRACE_I("SendListSubscribe() is failed", 0, 0, 0);
    SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
    SubscribeTerminated();
    return IMS_TRUE;
  }
  IMS_TRACE_I("Send list users size [%d]", m_objRemoteUsers.GetSize(), 0, 0);
  SetState(SUBSCRIBING);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBING_AoSDisConnected(IN IMSMSG& objMsg) {
  (void)objMsg;
  IMS_TRACE_I("StateSUBSCRIBING_AoSDisConnected", 0, 0, 0);
  StopRetryAfterTimer();
  SendSubscribeCommandErrorInd(IUUceService::COMMAND_CODE_GENERIC_FAILURE);
  SubscribeTerminated();
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBING_Subscribed(IN IMSMSG& objMsg) {
  (void)objMsg;
  IMS_TRACE_I("StateSUBSCRIBING_Subscribed", 0, 0, 0);
  StopRetryAfterTimer();
  SetState(SUBSCRIBED);
  IMessage* piMessage = IMS_NULL;
  if (m_eQueryType == QUERY_CAPABILITY_TYPE_LIST) {
    piMessage =
        m_piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE);
  } else {
    piMessage =
        m_piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_POLL);
  }

  ISipMessage* piSIPMessage = IMS_NULL;
  if (piMessage != IMS_NULL) {
    /* get sip message */
    piSIPMessage = piMessage->GetMessage();
    if (piSIPMessage != IMS_NULL) {
      AString strExpireHeader =
          piSIPMessage->GetHeader(ISipHeader::EXPIRES_ANY);
      if (strExpireHeader.IsNULL() != IMS_TRUE) {
        IMS_TRACE_I("StateSUBSCRIBING_Subscribed:expire value is [%s]",
                    strExpireHeader.GetStr(), 0, 0);
        m_nWaitNotiTimerValue += strExpireHeader.ToInt32() * 1000;
      }
    }
  }

  ISubscribeResponseData* pResponseData = IMS_NULL;
  pResponseData = GetSubscribeResponseData(piSIPMessage);
  if (pResponseData != IMS_NULL) {
    SendSubscribeResponseInd(
        pResponseData->m_nResponseCode, pResponseData->m_strReason,
        pResponseData->m_nReasonCause, pResponseData->m_strReasonText);
    delete pResponseData;
  } else {
    SendSubscribeResponseInd(SipStatusCode::SC_200, "OK", 0, "");
  }
  StartWaitingNotifyMessageTimer(m_nWaitNotiTimerValue);
  SetState(SUBSCRIBED);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBING_SubscribeFailed(IN IMSMSG& objMsg) {
  IMS_SINT32 nErrorResponse = LONG_TO_SINT(objMsg.nWparam);
  IMS_TRACE_I("StateSUBSCRIBING_SubscribeFailed - reason [%d]", nErrorResponse,
              0, 0);
  StopWaitingNotifyMessageTimer();
  StopRetryAfterTimer();
  IMessage* piMessage = IMS_NULL;
  if (m_eQueryType == QUERY_CAPABILITY_TYPE_LIST) {
    piMessage =
        m_piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE);
  } else {
    piMessage =
        m_piSubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_POLL);
  }

  if (piMessage == IMS_NULL) {
    IMS_TRACE_I("StateSUBSCRIBING_SubscribeFailed:IMessage is null", 0, 0, 0);
    SendSubscribeResponseInd(nErrorResponse, "Request Timeout", 0, "");
    SubscribeTerminated();
    SetState(ON);
    return IMS_TRUE;
  }
  ISipMessage* piSIPMessage = piMessage->GetMessage();
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("StateSUBSCRIBING_SubscribeFailed:ISipMessage is null", 0, 0,
                0);
    SendSubscribeResponseInd(nErrorResponse, "Request Timeout", 0, "");
    SubscribeTerminated();
    SetState(ON);
    return IMS_TRUE;
  }

  switch (nErrorResponse) {
    case SipStatusCode::SC_404:  // FALL-THROUGH //rfc3261 20.33 Retry-After
    case SipStatusCode::SC_413:  // FALL-THROUGH
    case SipStatusCode::SC_480:  // FALL-THROUGH
    case SipStatusCode::SC_486:  // FALL-THROUGH
    case SipStatusCode::SC_500:  // FALL-THROUGH
    case SipStatusCode::SC_503:  // FALL-THROUGH
    case SipStatusCode::SC_600:  // FALL-THROUGH
    case SipStatusCode::SC_603:  // FALL-THROUGH
    {
      if (HandleRetryAfterHeader(piSIPMessage) == IMS_TRUE) {
        return IMS_TRUE;
      }
    } break;
    case SipStatusCode::SC_423:  // min_expire
    {
      if (Handle423FailureResponse(piSIPMessage) == IMS_TRUE) {
        return IMS_TRUE;
      }
    } break;
    case SipStatusCode::SC_403:  // forbidden
    {
      if (Handle403FailureResponse(piSIPMessage) == IMS_TRUE) {
        return IMS_TRUE;
      }
    } break;
    default:
      break;
  }

  ISubscribeResponseData* pResponseData =
      GetSubscribeResponseData(piSIPMessage);
  if (pResponseData != IMS_NULL) {
    SendSubscribeResponseInd(
        pResponseData->m_nResponseCode, pResponseData->m_strReason,
        pResponseData->m_nReasonCause, pResponseData->m_strReasonText);
    delete pResponseData;
  } else {
    SendSubscribeResponseInd(nErrorResponse, "Request Timeout", 0, "");
  }
  SubscribeTerminated();
  SetState(ON);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBING_SubscribeTerminated(IN IMSMSG& objMsg) {
  (void)objMsg;
  IMS_TRACE_I("StateSUBSCRIBING_SubscribeTerminated", 0, 0, 0);
  StopRetryAfterTimer();
  if (m_nThreadRunningCompleted == 0) {
    SendSubscribeResponseInd(SipStatusCode::SC_408, "Request Timeout", 0, "");
    SubscribeTerminated();
    SetState(ON);
  } else {
    m_bSubscriptionTerminated = IMS_TRUE;
  }

  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBING_NotifyReceived(IN IMSMSG& objMsg) {
  IMS_TRACE_I("StateSUBSCRIBING_NotifyReceived", 0, 0, 0);
  StopRetryAfterTimer();
  IMessage* piNotify = (IMessage*)objMsg.nWparam;
  if (piNotify == IMS_NULL) {
    IMS_TRACE_I("StateSUBSCRIBING_NotifyReceived:piNotify is null", 0, 0, 0);
    return IMS_TRUE;
  }

  ISipMessage* piSIPMessage = piNotify->GetMessage();
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("StateSUBSCRIBING_NotifyReceived:piSIPMessage is null", 0, 0,
                0);
    return IMS_TRUE;
  }
  if (HandleNotifyInd(piSIPMessage) == IMS_FALSE &&
      m_nThreadRunningCompleted == 0) {
    SendSubscribeResponseInd(SipStatusCode::SC_408, "Request Timeout", 0, "");
    SubscribeTerminated();
    SetState(ON);
    return IMS_TRUE;
  }
  IMS_TRACE_I("StateSUBSCRIBING_NotifyInd:m_nThreadRunningCompleted [%d]",
              m_nThreadRunningCompleted, 0, 0);
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBED_AoSDisConnected(IN IMSMSG& objMsg) {
  (void)objMsg;
  IMS_TRACE_I("StateSUBSCRIBED_AoSDisConnected", 0, 0, 0);
  StopWaitingNotifyMessageTimer();
  if (m_nThreadRunningCompleted == 0) {
    SendSubscribeTerminatedInd();
    SubscribeTerminated();
    SetState(ON);
  } else {
    m_bSubscriptionTerminated = IMS_TRUE;
  }

  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBED_SubscribeTerminated(IN IMSMSG& objMsg) {
  (void)objMsg;
  IMS_TRACE_I(
      "StateSUBSCRIBED_SubscribeTerminated:m_nThreadRunningCompleted[%d]",
      m_nThreadRunningCompleted, 0, 0);
  StopWaitingNotifyMessageTimer();
  if (m_nThreadRunningCompleted == 0) {
    SendSubscribeTerminatedInd();
    SubscribeTerminated();
    SetState(ON);
  } else {
    m_bSubscriptionTerminated = IMS_TRUE;
  }
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::StateSUBSCRIBED_NotifyReceived(IN IMSMSG& objMsg) {
  IMS_TRACE_I("StateSUBSCRIBED_NotifyReceived", 0, 0, 0);
  if (m_eQueryType == QUERY_CAPABILITY_TYPE_SINGLE) {
    StopWaitingNotifyMessageTimer();
  }
  IMessage* piNotify = (IMessage*)objMsg.nWparam;
  if (piNotify == IMS_NULL) {
    IMS_TRACE_I("StateSUBSCRIBED_NotifyReceived:piNotify is null", 0, 0, 0);
    return IMS_TRUE;
  }

  ISipMessage* piSIPMessage = piNotify->GetMessage();
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("StateSUBSCRIBED_NotifyReceived:piSIPMessage is null", 0, 0, 0);
    return IMS_TRUE;
  }
  if (HandleNotifyInd(piSIPMessage) == IMS_FALSE &&
      m_nThreadRunningCompleted == 0) {
    SubscribeTerminated();
    SetState(ON);
    return IMS_TRUE;
  }
  IMS_TRACE_I("StateSUBSCRIBED_NotifyReceived:nThreadRunningCompleted [%d]",
              m_nThreadRunningCompleted, 0, 0);
  return IMS_TRUE;
}

PRIVATE
void UceSubscribe::LoadConfigValue() {
  IMS_TRACE_I("LoadConfigValue : read config", 0, 0, 0);
  m_strExpireValueInListSub.SetNumber(UceConfig::GetInstance()->GetIntValue(
      UceConfig::KEY_EXPIRE_VALUE_LIST_SUBSCRIBE, m_nSimSlot));
  m_nAnonymousMethod = UceConfig::GetInstance()->GetIntValue(
      UceConfig::KEY_ANONYMOUS_FETCH_METHOD_INT, m_nSimSlot);
}

void UceSubscribe::CreateXMLDocumentHelperThread() {
  IMS_TRACE_D("CreateXMLDocumentHelperThread() ", 0, 0, 0);
  if (m_pUceXmlDocumentHelperThread == IMS_NULL) {
    m_pUceXmlDocumentHelperThread =
        new UceXmlDocumentHelperThread(GetName(), m_nSimSlot);
    if (m_pUceXmlDocumentHelperThread != IMS_NULL) {
      m_pUceXmlDocumentHelperThread->Start(m_strXMLDocumentHelperThreadName);
    }
  }
}

void UceSubscribe::DestroyXMLDocumentHelperThread() {
  IMS_TRACE_D("DestroyXMLDocumentHelperThread() ", 0, 0, 0);
  if (m_pUceXmlDocumentHelperThread != IMS_NULL) {
    m_pUceXmlDocumentHelperThread->Terminate();
    delete m_pUceXmlDocumentHelperThread;
    m_pUceXmlDocumentHelperThread = IMS_NULL;
  }
}

IMS_BOOL UceSubscribe::CreateSubscription(IN CONST AString& strToURI) {
  IMS_TRACE_D("CreateSubscription:ToURI [%s]", strToURI.GetStr(), 0, 0);
  if (m_piCoreService == IMS_NULL || strToURI == IMS_NULL ||
      strToURI.IsEmpty()) {
    IMS_TRACE_I("CreateSubscription:m_piCoreService or strToURI is null", 0, 0,
                0);
    return IMS_FALSE;
  }
  m_piSubscription = m_piCoreService->CreateSubscription(AString::ConstNull(),
                                                         strToURI, "presence");
  if (m_piSubscription == IMS_NULL) {
    IMS_TRACE_I("CreateSubscription:ISubscription create failed", 0, 0, 0);
    return IMS_FALSE;
  }
  m_piSubscription->SetListener(this);
  if (UceConfig::GetInstance()->GetBoolValue(
          UceConfig::KEY_USE_CONTACT_HEADER_IN_SUBSCRIBE, m_nSimSlot) ==
      IMS_TRUE) {
    m_piSubscription->SetMessageMediator(this);
  }
  return IMS_TRUE;
}

void UceSubscribe::DestroySubscription() {
  IMS_TRACE_D("DestroySubscription", 0, 0, 0);
  if (m_piSubscription != IMS_NULL) {
    m_piSubscription->Destroy();
    m_piSubscription = IMS_NULL;
  }
}

void UceSubscribe::SubscribeTerminated() {
  IMS_TRACE_I("SubscribeTerminated:send UCE_SUBSCRIBE_DELETED_IND to manager",
              0, 0, 0);
  DestroySubscription();
  StopWaitingNotifyMessageTimer();
  m_eQueryType = QUERY_CAPABILITY_TYPE_NONE;

  IMSMSG objUIMsg(IUUceService::UCE_SUBSCRIBE_DELETED_IND, 0,
                  reinterpret_cast<IMS_UINTP>(this));
  MessageService::PostMessage(m_strUceSubscribeManagerName, objUIMsg);
  SetState(ON);
}

IMS_BOOL UceSubscribe::SetState(IMS_UINT32 _eState) {
  IMS_TRACE_I("SetState:State [ %s ] -> [ %s ]", StateToString(),
              StateToString(m_eState), 0);
  m_eState = _eState;
  IMSStateMachine::SetState(m_eState);
  return IMS_TRUE;
}

const IMS_CHAR* UceSubscribe::StateToString() {
  static const char* szState[] = {
      "ON",
      "SUBSCRIBING",
      "SUBSCRIBED",
      "ERROR",
  };

  if (sizeof(szState) / sizeof(char*) - 1 <= m_eState) {
    IMS_TRACE_E(0, "StateToString:State Error", 0, 0, 0);
    return szState[sizeof(szState) / sizeof(char*) - 1];
  }

  return szState[m_eState];
}

const IMS_CHAR* UceSubscribe::StateToString(IMS_UINT32 _eState) {
  static const char* szState[] = {
      "ON",
      "SUBSCRIBING",
      "SUBSCRIBED",
      "ERROR",
  };

  if (sizeof(szState) / sizeof(char*) - 1 <= _eState) {
    IMS_TRACE_E(0, "StateToString:State Error", 0, 0, 0);
    return szState[sizeof(szState) / sizeof(char*) - 1];
  }
  return szState[_eState];
}

void UceSubscribe::SendSubscribeResponseInd(IMS_SINT32 nResponseCode,
                                            AString strReason,
                                            IMS_SINT32 nReasonHeaderCause,
                                            AString strReasonHeaderText) {
  IUceSubResponseIndPrm* pParam = new IUceSubResponseIndPrm();

  pParam->m_nKey = m_nKey;
  pParam->m_nResponseCode = nResponseCode;
  pParam->m_strReason = strReason;
  pParam->m_nReasonHeaderCause = nReasonHeaderCause;
  pParam->m_strReasonHeaderText = strReasonHeaderText;

  IMSMSG objUIMsg(IUUceService::UCE_SUBSCRIBE_RESPONSE_IND, 0,
                  reinterpret_cast<IMS_UINTP>(pParam));
  MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
}

void UceSubscribe::SendSubscribeCommandErrorInd(IMS_UINT32 nCommandError) {
  if (m_nKey == 0) {
    return;
  }
  IMS_TRACE_I("SendSubscribeCommandErrorInd:key[%d], error[%d]", m_nKey,
              nCommandError, 0);
  IUceSubCmdErrorIndPrm* pParam = new IUceSubCmdErrorIndPrm();

  pParam->m_nKey = m_nKey;
  pParam->m_nCommandError = nCommandError;

  IMSMSG objUIMsg(IUUceService::UCE_SUBSCRIBE_CMD_ERROR_IND, 0,
                  reinterpret_cast<IMS_UINTP>(pParam));
  MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
  m_nKey = 0;
}

void UceSubscribe::SendPresenceNotifyInd(IMSList<AString> pidfXmls) {
  if (m_nKey == 0) {
    return;
  }
  IMS_TRACE_I("SendPresenceNotifyInd:key[%d]", m_nKey, 0, 0);
  IUcePreNotifyIndPrm* pParam = new IUcePreNotifyIndPrm();

  pParam->m_nKey = m_nKey;
  pParam->m_nCount = pidfXmls.GetSize();
  pParam->m_lstPidfXmls = pidfXmls;

  IMSMSG objUIMsg(IUUceService::UCE_PRESENCE_NOTIFY_IND, 0,
                  reinterpret_cast<IMS_UINTP>(pParam));
  MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
}

void UceSubscribe::SendSubscribeResourceTerminatedInd(
    IMSList<UceNonCapabilityUser*>* pList) {
  if (m_nKey == 0) {
    return;
  }
  IMS_TRACE_I("SendSubscribeResourceTerminatedInd:key[%d],size[%d]", m_nKey,
              pList->GetSize(), 0);
  IUceSubResourceTerminatedIndPrm* pParam =
      new IUceSubResourceTerminatedIndPrm();
  IMS_UINT32 nCount = 0;
  pParam->m_nKey = m_nKey;
  for (IMS_UINT32 i = 0; i < pList->GetSize(); i++) {
    UceNonCapabilityUser* user = pList->GetAt(i);
    if (user != IMS_NULL) {
      IUceTerminatedReason* reason = new IUceTerminatedReason();
      reason->m_strContact = user->GetId();
      reason->m_strReason = user->GetReason();
      delete user;
      user = IMS_NULL;
      nCount++;
      pParam->m_lstTerminateContacts.Append(reason);
    }
  }
  pParam->m_nCount = nCount;
  pList->Clear();
  IMSMSG objUIMsg(IUUceService::UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND, 0,
                  reinterpret_cast<IMS_UINTP>(pParam));
  MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
}

void UceSubscribe::SendSubscribeTerminatedInd() {
  if (m_nKey == 0) {
    return;
  }
  IMS_TRACE_I("SendSubscribeTerminatedInd:key[%d]", m_nKey, 0, 0);
  IUceSubTerminatedIndPrm* pParam = new IUceSubTerminatedIndPrm();

  pParam->m_nKey = m_nKey;
  pParam->m_strReason = AString::ConstEmpty();
  pParam->m_nRetryAfterMillsecond = 0;

  IMSMSG objUIMsg(IUUceService::UCE_SUBSCRIBE_TERMINATED_IND, 0,
                  reinterpret_cast<IMS_UINTP>(pParam));
  MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
  m_nKey = 0;
}

IMS_BOOL UceSubscribe::SetHeaderForSingleSubscription(
    IN_OUT ISipMessage* piSIPMessage) {
  IMS_TRACE_D("SetHeaderForSingleSubscription", 0, 0, 0);
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("SetHeaderForSingleSubscription:piSIPMessage is null", 0, 0, 0);
    return IMS_FALSE;
  }

  if (m_nAnonymousMethod == 0) {  // set only privacy header
    piSIPMessage->SetHeader(ISipHeader::PRIVACY, "id");
  } else if (m_nAnonymousMethod == 1) {  // set only anonymous from header
    piSIPMessage->SetHeader(ISipHeader::FROM,
                            "Anonymous <sip:anonymous@anonymous.invalid>");
  } else {  // set privacy and anonymous from header
    piSIPMessage->SetHeader(ISipHeader::PRIVACY, "id");
    piSIPMessage->SetHeader(ISipHeader::FROM,
                            "Anonymous <sip:anonymous@anonymous.invalid>");
  }
  piSIPMessage->AddHeader(ISipHeader::UNKNOWN, "gzip", "Accept-Encoding");
  piSIPMessage->AddHeader(ISipHeader::EXPIRES_ANY, "0");
  // Accept
  piSIPMessage->AddHeader(ISipHeader::ACCEPT, "application/pidf+xml");
  return IMS_TRUE;
}

AString UceSubscribe::GetListSubscribeUri() {
  AString szRequestUri = m_piCoreService->GetAuthorizedUserId().ToString();
  IMS_TRACE_D("GetListSubscribeUri:AuthorizedUserId [%s]",
              szRequestUri.GetStr(), 0, 0);
  szRequestUri += UceConfig::GetInstance()->GetAStringValue(
      UceConfig::KEY_RLS_URI, m_nSimSlot);
  IMS_TRACE_D("GetListSubscribeUri:full request uri [%s]",
              szRequestUri.GetStr(), 0, 0);
  return szRequestUri;
}

IMS_BOOL UceSubscribe::SetHeaderForListSubscription(
    IN_OUT ISipMessage* piSIPMessage,
    IN CONST AString& strListSubscriptionRequestUri) {
  IMS_TRACE_D("SetHeaderForListSubscription", 0, 0, 0);

  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("SetHeaderForListSubscription:piSIPMessage is null", 0, 0, 0);
    return IMS_FALSE;
  }

  if (m_nAnonymousMethod == 0) {  // set only privacy header
    piSIPMessage->SetHeader(ISipHeader::PRIVACY, "id");
  } else if (m_nAnonymousMethod == 1) {  // set only anonymous from header
    piSIPMessage->SetHeader(ISipHeader::FROM,
                            "Anonymous <sip:anonymous@anonymous.invalid>");
  } else {  // set privacy and anonymous from header
    piSIPMessage->SetHeader(ISipHeader::PRIVACY, "id");
    piSIPMessage->SetHeader(ISipHeader::FROM,
                            "Anonymous <sip:anonymous@anonymous.invalid>");
  }
  piSIPMessage->AddHeader(ISipHeader::UNKNOWN, "gzip", "Accept-Encoding");
  AString strToHeaderRequestUri = "<" + strListSubscriptionRequestUri + ">";
  piSIPMessage->SetHeader(ISipHeader::TO, strToHeaderRequestUri);
  piSIPMessage->AddHeader(ISipHeader::REQUIRE, "recipient-list-subscribe");
  piSIPMessage->AddHeader(ISipHeader::ACCEPT, "multipart/related");
  piSIPMessage->AddHeader(ISipHeader::ACCEPT, "application/rlmi+xml");
  piSIPMessage->AddHeader(ISipHeader::CONTENT_TYPE,
                          "application/resource-lists+xml");
  piSIPMessage->AddHeader(ISipHeader::CONTENT_DISPOSITION, "recipient-list");
  IMS_TRACE_D("SetHeaderForSingleSubscription:Expire value[%s]",
              m_strExpireValueInListSub.GetStr(), 0, 0);
  piSIPMessage->AddHeader(ISipHeader::EXPIRES_ANY, m_strExpireValueInListSub);
  piSIPMessage->AddHeader(ISipHeader::SUPPORTED, "eventlist");
  piSIPMessage->AddHeader(ISipHeader::ACCEPT, "application/pidf+xml");
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::SetContentBody(IN_OUT ISipMessage* piSIPMessage,
                                      IN CONST AString& strXMLBody) {
  IMS_TRACE_D("SetContentBody", 0, 0, 0);
  if (piSIPMessage == IMS_NULL || strXMLBody.GetLength() <= 0) {
    IMS_TRACE_I("SetContentBody:piSIPMessage or szXMLBody is null", 0, 0, 0);
    return IMS_FALSE;
  }

  ISipMessageBodyPart* piBodyPart = piSIPMessage->CreateBodyPart();
  if (piBodyPart == IMS_NULL) {
    IMS_TRACE_I("SetContentBody:ISipMessageBodyPart is null", 0, 0, 0);
    return IMS_FALSE;
  }

  ByteArray objContent;
  const IMS_BYTE* pbyXML =
      reinterpret_cast<const IMS_BYTE*>(strXMLBody.GetStr());
  if (pbyXML == IMS_NULL) {
    IMS_TRACE_I("SetContentBody:pbyXML is null", 0, 0, 0);
    return IMS_FALSE;
  }
  objContent.Attach(pbyXML, strXMLBody.GetLength());

  if (UceConfig::GetInstance()->GetBoolValue(
          UceConfig::KEY_ENCODE_SUBSCRIBE_BODY, m_nSimSlot) == IMS_TRUE) {
    ByteArray objCompressedContent;
    if (IMS_UTIL_ZLIB_Compress(objContent, objCompressedContent) == IMS_TRUE) {
      IMS_TRACE_I("SetSubscribeXMLBody - compressing a body part", 0, 0, 0);
      piBodyPart->SetContent(objCompressedContent);
      piSIPMessage->AddHeader(ISipHeader::CONTENT_ENCODING, "gzip");
      return IMS_TRUE;
    }
  }
  piBodyPart->SetContent(objContent);
  return IMS_TRUE;
}

ISipMessage* UceSubscribe::GetISIPMessage() {
  if (m_piSubscription == IMS_NULL) {
    IMS_TRACE_I("GetISIPMessage:m_piSubscription is null", 0, 0, 0);
    return IMS_NULL;
  }

  IMessage* piMessage = m_piSubscription->GetNextRequest();
  if (piMessage == IMS_NULL) {
    IMS_TRACE_I("GetISIPMessage:Getting a IMessage failed", 0, 0, 0);
    return IMS_NULL;
  }

  /* get sip message */
  ISipMessage* piSIPMessage = piMessage->GetMessage();
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("GetISIPMessage:Getting a ISipMessage failed", 0, 0, 0);
    return IMS_NULL;
  }
  return piSIPMessage;
}

IMS_BOOL UceSubscribe::SendSingleSubscribe() {
  if (m_piSubscription == IMS_NULL) {
    IMS_TRACE_I("SingleSubscribe:m_piSubscription is null", 0, 0, 0);
    return IMS_FALSE;
  }
  IMS_TRACE_D("SendSingleSubscribe ", 0, 0, 0);
  m_piSubscription->SetRefreshPolicy(ISubscription::REFRESH_POLICY_NO_REFRESH,
                                     0, 0, 0);
  /* fetch */
  if (m_piSubscription->Poll() != IMS_SUCCESS) {
    IMS_TRACE_I("SingleSubscribe:Send Poll() is failed", 0, 0, 0);
    return IMS_FALSE;
  }
  return IMS_TRUE;
}

IMS_BOOL UceSubscribe::SendListSubscribe() {
  IMS_TRACE_D("SendListSubscribe", 0, 0, 0);
  if (m_piSubscription == IMS_NULL) {
    IMS_TRACE_I("SendListSubscribe:m_piSubscription is null", 0, 0, 0);
    return IMS_FALSE;
  }
  m_piSubscription->SetRefreshPolicy(ISubscription::REFRESH_POLICY_NO_REFRESH,
                                     0, 0, 0);

  if (m_piSubscription->Subscribe() != IMS_SUCCESS) {
    IMS_TRACE_I("SendListSubscribe: Send Subscribe() is failed", 0, 0, 0);
    return IMS_FALSE;
  }
  return IMS_TRUE;
}

ISubscribeResponseData* UceSubscribe::GetSubscribeResponseData(
    ISipMessage* piMessage) {
  ISubscribeResponseData* pSubscribeResponseData = IMS_NULL;

  IMS_SINT32 nReasonCause = -1;
  AString strReasonText = "";
  if (piMessage != IMS_NULL) {
    pSubscribeResponseData = new ISubscribeResponseData();

    IMS_TRACE_D("GetSubscribeResponseData:StatusCode[%d], reason[%s]",
                piMessage->GetStatusCode(),
                piMessage->GetReasonPhrase().GetStr(), 0);
    pSubscribeResponseData->m_nResponseCode = piMessage->GetStatusCode();
    pSubscribeResponseData->m_strReason = piMessage->GetReasonPhrase();

    IMSList<AString> objReasonHeaders =
        piMessage->GetHeaders(ISipHeader::UNKNOWN, "Reason");
    if (objReasonHeaders.IsEmpty()) {
      return pSubscribeResponseData;
    }
    AString strReasonHdr =
        SipParsingHelper::GetSipReasonHeader(objReasonHeaders);
    SipParsingHelper::ParseReasonHeader(strReasonHdr, nReasonCause,
                                        strReasonText);
    IMS_TRACE_D("GetSubscribeResponseData:cause[%d], text[%s]", nReasonCause,
                strReasonText.GetStr(), 0);

    pSubscribeResponseData->m_nReasonCause = nReasonCause;
    pSubscribeResponseData->m_strReasonText = strReasonText;
  }
  return pSubscribeResponseData;
}

IMS_BOOL UceSubscribe::HandleRetryAfterHeader(ISipMessage* piSIPMessage) {
  IMS_TRACE_D("HandleRetryAfterHeader", 0, 0, 0);
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("HandleRetryAfterHeader:ISipMessage is null", 0, 0, 0);
    return IMS_FALSE;
  }
  AString strHeader = piSIPMessage->GetHeader(ISipHeader::RETRY_AFTER_SEC);
  if (strHeader.GetLength() <= 0) {
    IMS_TRACE_I("HandleRetryAfterHeader:strHeader less than 0", 0, 0, 0);
    return IMS_FALSE;
  }

  ISipHeader* piHeader =
      SipParsingHelper::CreateHeader(ISipHeader::RETRY_AFTER_SEC, strHeader);
  if (piHeader == IMS_NULL) {
    IMS_TRACE_I("HandleRetryAfterHeader:piHeader is null.", 0, 0, 0);
    return IMS_FALSE;
  }
  IMS_SINT32 nValue = piHeader->GetValueInt();
  piHeader->Destroy();

  IMS_TRACE_I("HandleRetryAfterHeader:retry after header nValue [%d]", nValue,
              0, 0);
  return StartRetryAfterTimer(nValue);
}

IMS_BOOL UceSubscribe::Handle403FailureResponse(ISipMessage* piSIPMessage) {
#define NOT_AUTHORIZED_FOR_PRESENCE "NOT AUTHORIZED FOR PRESENCE"

  IMS_TRACE_D("Handle403FailureResponse", 0, 0, 0);
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("Handle403FailureResponse:piSIPMessage is null", 0, 0, 0);
    return IMS_FALSE;
  }
  AString strReasonPhrs = piSIPMessage->GetReasonPhrase().MakeUpper();
  IMS_TRACE_D("REASON PHRASE = [%s]", strReasonPhrs.GetStr(), 0, 0);

  if (strReasonPhrs.Contains(NOT_AUTHORIZED_FOR_PRESENCE) == IMS_TRUE) {
    IMS_TRACE_I("ReasonPhrase:Not Authorized for presence", 0, 0, 0);
    return IMS_FALSE;
  }
  IMSList<AString> objReasonList =
      piSIPMessage->GetHeaders(ISipHeader::UNKNOWN, "Reason");
  if (objReasonList.IsEmpty() == IMS_TRUE) {
    IMS_TRACE_D("No Reason header present", 0, 0, 0);
    IMSMSG objMsg(AoSAppRequest::COMMAND_REGISTER_RECOVERY, 0,
                  ImsAosControl::REGISTER_REINITIATE);
    MessageService::PostMessage(m_strAppName, objMsg);
    return IMS_FALSE;
  }
  for (IMS_UINT32 i = 0; i < objReasonList.GetSize(); i++) {
    AString strReason = objReasonList.GetAt(i).MakeUpper();
    IMS_TRACE_D("REASON header = %d:[%s]", i, strReason.GetStr(), 0);
    if (strReason.Contains(NOT_AUTHORIZED_FOR_PRESENCE) == IMS_TRUE) {
      IMS_TRACE_I("ReasonPhrase:Not Authorized for presence", 0, 0, 0);
      return IMS_FALSE;
    }
  }
  IMSMSG objMsg(AoSAppRequest::COMMAND_REGISTER_RECOVERY, 0,
                ImsAosControl::REGISTER_REINITIATE);
  MessageService::PostMessage(m_strAppName, objMsg);
  return IMS_FALSE;
}

IMS_BOOL UceSubscribe::Handle423FailureResponse(ISipMessage* piSIPMessage) {
  IMS_TRACE_D("Handle423FailureResponse", 0, 0, 0);
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("Handle423FailureResponse:piSIPMessage is null", 0, 0, 0);
    return IMS_FALSE;
  }

  AString strMinExpireHeader = piSIPMessage->GetHeader(ISipHeader::MIN_EXPIRES);
  if (strMinExpireHeader.GetLength() == 0) {
    IMS_TRACE_I("Handle423FailureResponse:strMinExpireHeader is empty", 0, 0,
                0);
    return IMS_FALSE;
  }
  ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::MIN_EXPIRES,
                                                        strMinExpireHeader);
  if (piHeader == IMS_NULL) {
    IMS_TRACE_I("Handle423FailureResponse:piHeader is null", 0, 0, 0);
    return IMS_FALSE;
  }
  m_strExpireValueInListSub = piHeader->GetValue();
  piHeader->Destroy();
  DestroySubscription();

  if (m_eQueryType == QUERY_CAPABILITY_TYPE_SINGLE) {
    IMS_TRACE_I("Handle423FailureResponse:current is single request", 0, 0, 0);
    return IMS_FALSE;
  } else {
    IMS_TRACE_I("Handle423FailureResponse:list subscribe.min Expire Value [%s]",
                m_strExpireValueInListSub.GetStr(), 0, 0);
    SetState(ON);
    return QueryMultiCapability(m_objRemoteUsers, m_nKey);
  }
}

IMS_BOOL UceSubscribe::HandleNotifyInd(IN ISipMessage* piSIPMessage) {
  IMS_TRACE_D("HandleNotifyInd:m_nThreadRunningCompleted [%d]",
              m_nThreadRunningCompleted, 0, 0);
  if (piSIPMessage == IMS_NULL) {
    IMS_TRACE_I("HandleNotifyInd:piSIPMessage is null", 0, 0, 0);
    return IMS_FALSE;
  }
  AString strSIPSubState =
      piSIPMessage->GetHeader(ISipHeader::SUBSCRIPTION_STATE);
  IMSList<ISipMessageBodyPart*> objBodyParts = piSIPMessage->GetBodyParts();
  IMS_TRACE_D("HandleNotifyInd:Subscription-State [%s]",
              strSIPSubState.GetStr(), 0, 0);

  AString strContentType = piSIPMessage->GetHeader(ISipHeader::CONTENT_TYPE);
  if (strContentType.Contains("application/pidf+xml")) {
    IMSList<AString> pidfXmls = IMSList<AString>();
    for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); i++) {
      ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);
      pidfXmls.Append(piBodyPart->GetContent().ToString());
    }
    if (pidfXmls.IsEmpty()) {
      return IMS_FALSE;
    } else {
      SendPresenceNotifyInd(pidfXmls);
      return IMS_TRUE;
    }
  }
  UceNotifyMessageBody* pUceNotifyMessageBody = new UceNotifyMessageBody();
  pUceNotifyMessageBody->SetContentType(strContentType);
  IMS_TRACE_D("HandleNotifyInd:Content-Type [%s]", strContentType.GetStr(), 0,
              0);

  IMS_TRACE_D("HandleNotifyInd:objBodyParts.GetSize() [%d]",
              objBodyParts.GetSize(), 0, 0);
  for (IMS_UINT32 i = 0; i < objBodyParts.GetSize(); i++) {
    ISipMessageBodyPart* piBodyPart = objBodyParts.GetAt(i);
    ByteArray objContent = piBodyPart->GetContent();
    IMS_TRACE_D("HandleNotifyInd:XML data [%s]", objContent.ToString().GetStr(),
                0, 0);
    AString strContentId =
        piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_ID);
    AString strContentTypeOfBody =
        piBodyPart->GetHeader(ISipMessageBodyPart::CONTENT_TYPE);
    UceNotifyBodyPartData* pBodyData = new UceNotifyBodyPartData(
        strContentId, strContentTypeOfBody, objContent);
    pUceNotifyMessageBody->SetNotifyBodyPartData(pBodyData);
  }

  if (pUceNotifyMessageBody->GetContentType().Contains("multipart/related") ||
      pUceNotifyMessageBody->GetContentType().Contains(
          "application/rlmi+xml")) {
    m_nThreadRunningCompleted++;
    IMS_TRACE_D("HandleNotifyInd:Content-Type related RLMI", 0, 0, 0);
    m_pUceXmlDocumentHelperThread->SendMsg(
        MSG_THREAD_RECEIVED_NOTIFY_RLMI, 0,
        reinterpret_cast<IMS_UINTP>(pUceNotifyMessageBody));
  } else {
    IMS_TRACE_D("HandleNotifyInd:Can not handle Content-Type", 0, 0, 0);
    return IMS_FALSE;
  }
  IMS_TRACE_I("HandleNotifyInd:m_nThreadRunningCompleted [%d]",
              m_nThreadRunningCompleted, 0, 0);
  return IMS_TRUE;
}

void UceSubscribe::StartWaitingNotifyMessageTimer(IMS_UINT32 nDuration) {
  StopWaitingNotifyMessageTimer();
  m_pWaitNotifyMsgTimer = TimerService::GetTimerService()->CreateTimer();
  if (m_pWaitNotifyMsgTimer == IMS_NULL || nDuration <= 0) {
    IMS_TRACE_I("StartWaitingNotifyMessageTimer:CreateTimer failed", 0, 0, 0);
    return;
  }
  IMS_TRACE_I("StartWaitingNotifyMessageTimer:duration [%d]", nDuration, 0, 0);
  m_pWaitNotifyMsgTimer->SetTimer(nDuration, this);
}

void UceSubscribe::StopWaitingNotifyMessageTimer() {
  if (m_pWaitNotifyMsgTimer == IMS_NULL) {
    return;
  }
  m_pWaitNotifyMsgTimer->KillTimer();

  if (TimerService::GetTimerService() != IMS_NULL) {
    IMS_TRACE_I("StopWaitingNotifyMessageTimer:Destroy Timer", 0, 0, 0);
    TimerService::GetTimerService()->DestroyTimer(m_pWaitNotifyMsgTimer);
    m_pWaitNotifyMsgTimer = IMS_NULL;
  }
}

void UceSubscribe::HandleWaitingNotifyMessageTimer() {
  IMS_TRACE_I("HandleWaitingNotifyMessageTimer", 0, 0, 0);
  StopWaitingNotifyMessageTimer();
  if (GetState() == SUBSCRIBING) {
    SendSubscribeResponseInd(SipStatusCode::SC_200, "OK", 0, "");
    SendSubscribeTerminatedInd();
    SubscribeTerminated();
    SetState(ON);
  } else if (GetState() == SUBSCRIBED) {
    SendSubscribeTerminatedInd();
    SubscribeTerminated();
    SetState(ON);
  }
}

IMS_BOOL UceSubscribe::StartRetryAfterTimer(IMS_UINT32 nDuration) {
  if (nDuration <= 0) {
    IMS_TRACE_I("StartRetryAfterTimer:invalid duration[%d]", nDuration, 0, 0);
    return IMS_FALSE;
  }
  StopRetryAfterTimer();
  m_pRetryAfterTimer = TimerService::GetTimerService()->CreateTimer();
  if (m_pRetryAfterTimer == IMS_NULL) {
    IMS_TRACE_I("StartRetryAfterTimer:CreateTimer failed", 0, 0, 0);
    return IMS_FALSE;
  }
  IMS_TRACE_I("StartRetryAfterTimer:duration [%d]", nDuration, 0, 0);
  m_pRetryAfterTimer->SetTimer(nDuration, this);
  return IMS_TRUE;
}

void UceSubscribe::StopRetryAfterTimer() {
  if (m_pRetryAfterTimer == IMS_NULL) {
    return;
  }
  m_pRetryAfterTimer->KillTimer();
  if (TimerService::GetTimerService() != IMS_NULL) {
    IMS_TRACE_I("StopRetryAfterTimer:Destroy Timer", 0, 0, 0);
    TimerService::GetTimerService()->DestroyTimer(m_pRetryAfterTimer);
    m_pRetryAfterTimer = IMS_NULL;
  }
}

void UceSubscribe::HandleRetryAfterTimer() {
  StopRetryAfterTimer();
  DestroySubscription();
  if (m_eQueryType == QUERY_CAPABILITY_TYPE_SINGLE) {
    IMS_TRACE_I("HandleRetryAfterTimer:send single request", 0, 0, 0);
    SetState(ON);
    QuerySingleCapability(m_strRemoteUser, m_nKey);
  } else {
    IMS_TRACE_I("HandleRetryAfterTimer:send list request", 0, 0, 0);
    SetState(ON);
    QueryMultiCapability(m_objRemoteUsers, m_nKey);
  }
}
