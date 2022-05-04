#include "UceApp.h"

#include "AoSAppRequestType.h"
#include "Configuration.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "IIpcan.h"
#include "IUUceService.h"
#include "ImsAos.h"
#include "ImsAosParameter.h"
#include "ImsServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceMessage.h"
#include "ServicePhoneInfo.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"
#include "UceService.h"
#include "config/UceConfig.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

/* -------------------------------------------------------------------------------------------------
    ConstrEABtor, DestrEABtor
-------------------------------------------------------------------------------------------------
*/
UceApp::UceApp(IN CONST IMS_SINT32 nSlotId, IN CONST AString &strAppName)
    : IMSApp(strAppName),
      m_nSlotId(nSlotId),
      m_strAppName(strAppName),
      m_piImsAos(IMS_NULL),
      m_eAoSStatus(AOS_DISCONNECTED),
      m_piNetWatcherInfo(IMS_NULL),
      m_piDeBounceTimer(IMS_NULL),
      m_RegisteredNetwork(eUCE_RAT_INVALID),
      m_eCurrentNetwork(eUCE_RAT_INVALID),
      m_pUceService(IMS_NULL) {
  IMS_TRACE_D("UCE_M : UceApp = %" PFLS_u, sizeof(UceApp), 0, 0);
  IMS_TRACE_I("UceApp - strName (%s)", strAppName.GetStr(), 0, 0);
  m_strAppID = ImsServiceConfig::GetAppName(ImsAppId::UCE);
  m_strServiceID = ImsServiceConfig::GetServiceName(ImsServiceId::UCE);
  m_piNetWatcherInfo =
      PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(m_nSlotId);
  if (m_piNetWatcherInfo != IMS_NULL) {
    m_piNetWatcherInfo->RegisterObserver(this);
  } else {
    IMS_TRACE_E(0, "[ERROR]m_piNetWatcherInfo is null", 0, 0, 0);
  }
  UceConfig::GetInstance()->Init(m_nSlotId);
  Configuration::GetInstance()->SetAppConfig(
      ImsServiceConfig::GetAppName(ImsAppId::UCE), m_nSlotId);

  PostMessage(AMSG_CREATE_SERVICE, 0, 0);
}

PUBLIC VIRTUAL UceApp::~UceApp() {
  IMS_TRACE_D("UCE_F : UceApp = %" PFLS_u, sizeof(UceApp), 0, 0);
  IMS_TRACE_I("~UceApp", 0, 0, 0);
  if (m_piNetWatcherInfo != IMS_NULL) {
    m_piNetWatcherInfo->RemoveObserver(this);
    m_piNetWatcherInfo = IMS_NULL;
  }
  ClearTimer();
  DisableUceService();
}

/* -------------------------------------------------------------------------------------------------
    Methods
-------------------------------------------------------------------------------------------------
*/
PUBLIC
IMSApp *UceApp::GetInstance(IN CONST IMS_SINT32 nSlotId) {
  return new UceApp(nSlotId, GetUceAppName(nSlotId));
}

PROTECTED VIRTUAL IMS_BOOL UceApp::OnPreprocess(IN IMSMSG &objMSG) {
  (void)objMSG;
  return IMS_FALSE;
}

IMS_BOOL UceApp::OnMessage(IN IMSMSG &objMSG) {
  IMS_TRACE_I("OnMessage:MSG [%d]", objMSG.GetName(), 0, 0);

  switch (objMSG.GetName()) {
    case AMSG_CREATE_SERVICE:
      EnableUceService();
      break;

    case AoSAppRequest::COMMAND_SET_PUBLISH_STARTED:
      SetPublishStatusToAos(IMS_TRUE);
      break;

    case AoSAppRequest::COMMAND_SET_PUBLISH_TERMINATED:
      SetPublishStatusToAos(IMS_FALSE);
      break;

    case AoSAppRequest::COMMAND_REGISTER_RECOVERY: {
      if (m_RegisteredNetwork != eUCE_RAT_WIFI &&
          m_eCurrentNetwork == eUCE_RAT_INVALID) {
        IMS_TRACE_I("No valid network. Don't recovery registration", 0, 0, 0);
        return IMS_TRUE;
      }
      IMS_UINT32 nParam = LONG_TO_INT(objMSG.nLparam);
      SendRegistrationRecoveryRequestToAos(nParam);
    } break;
    default:
      if (m_pUceService != IMS_NULL) {
        m_pUceService->PostMessage(objMSG);
      }
      break;
  }

  return IMS_TRUE;
}

IMS_BOOL UceApp::OnPostprocess(IN IMSMSG &objMSG) {
  (void)objMSG;
  return IMS_TRUE;
}

IIMSActivityControl *UceApp::GetController() { return this; }

IMS_BOOL UceApp::Control(IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam,
                         OUT IMS_UINTP *pnOutParam) {
  (void)nCmdType;
  (void)nInParam;
  (void)pnOutParam;
  return IMS_FALSE;
}

void UceApp::NetworkWatcher_NotifyStatus(IN INetworkWatcher *piNetWatcherInfo) {
  IMS_TRACE_I("NotifyNetWatcherStatus", 0, 0, 0);

  if (m_piNetWatcherInfo != piNetWatcherInfo) {
    IMS_TRACE_E(0, "[ERROR]NotifyNetWatcherStatus - not matched", 0, 0, 0);
    return;
  }

  IMS_SINT32 nNetworkType = eUCE_RAT_INVALID;
  if (m_RegisteredNetwork != eUCE_RAT_WIFI) {
    nNetworkType = ConvertNetworkType(m_piNetWatcherInfo->GetNetworkType());
    if (m_eCurrentNetwork != nNetworkType) {
      IMS_TRACE_I("NotifyNetWatcherStatus:NetworkType is changed [%d] -> [%d]",
                  m_eCurrentNetwork, nNetworkType, 0);
      StartTimer(TIMER_NETWORK_CHANGED, 2000);
      m_eCurrentNetwork = nNetworkType;
    }
  }
}

void UceApp::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration) {
  IMS_UINTP nTimerID = 0;

  if (nType == TIMER_NETWORK_CHANGED) {
    if (m_piDeBounceTimer != IMS_NULL) {
      StopTimer(TIMER_NETWORK_CHANGED);
    }
    m_piDeBounceTimer = TimerService::GetTimerService()->CreateTimer();
    nTimerID = m_piDeBounceTimer->SetTimer(nDuration, this);
    IMS_TRACE_I("StartTimer:Type[%d], Duration[%d], Timer ID[%d] ", nType,
                nDuration, nTimerID);
    return;
  }
}

void UceApp::StopTimer(IN IMS_UINT32 nType) {
  if (nType == TIMER_NETWORK_CHANGED) {
    if (m_piDeBounceTimer != IMS_NULL) {
      m_piDeBounceTimer->KillTimer();
      TimerService::GetTimerService()->DestroyTimer(m_piDeBounceTimer);
      m_piDeBounceTimer = IMS_NULL;
      return;
    }
  }
}

void UceApp::ClearTimer() {
  IMS_TRACE_I("ClearTimer", 0, 0, 0);

  if (m_piDeBounceTimer != IMS_NULL) {
    StopTimer(TIMER_NETWORK_CHANGED);
  }
}

void UceApp::Timer_TimerExpired(IN ITimer *piTimer) {
  if (piTimer == m_piDeBounceTimer) {
    IMS_TRACE_I("Timer_TimerExpired:TIMER_NETWORK_CHANGED", 0, 0, 0);
    StopTimer(TIMER_NETWORK_CHANGED);
    NotifyRATChanged();
  }
}

void UceApp::ImsAos_Connected(IN IMS_UINT32 /*nFeatures*/,
                              IN IMS_UINT32 /*nIpcan*/) {
  IMS_TRACE_I("ImsAos_Connected()", 0, 0, 0);
  SelectActiveAoSApp();
}

void UceApp::ImsAos_Connecting() {
  IMS_TRACE_I("ImsAos_Connecting()", 0, 0, 0);
}

void UceApp::ImsAos_Disconnecting(IN IMS_UINT32 nReason) {
  (void)nReason;
  if (m_eAoSStatus == AOS_DISCONNECTING) {
    IMS_TRACE_I("ImsAos_Disconnecting : Not Changed", 0, 0, 0);
    return;
  }
  m_eAoSStatus = AOS_DISCONNECTING;
  if (m_pUceService != IMS_NULL) {
    m_pUceService->AosDisConnecting();
  }
}

void UceApp::ImsAos_Disconnected(IN IMS_UINT32 nReason) {
  (void)nReason;
  if (m_eAoSStatus == AOS_DISCONNECTED) {
    IMS_TRACE_I("ImsAos_Disconnected : Not Changed", 0, 0, 0);
    return;
  }
  m_eAoSStatus = AOS_DISCONNECTED;
  IMSMSG objUIMsg(IUUceService::UCE_IMS_AGENT_DISCONNECTED_IND, 0, 0);
  MessageService::PostMessage(AString("JNIUCEServiceThread"), objUIMsg);

  if (m_pUceService != IMS_NULL) {
    m_pUceService->AoSDisconnected();
  }
}

void UceApp::ImsAos_Suspended(IN IMS_UINT32 nReason) {
  IMS_TRACE_I("ImsAos_Suspended : Reason[%d]", nReason, 0, 0);
}

void UceApp::ImsAos_Resumed() { IMS_TRACE_I("ImsAos_Resumed ", 0, 0, 0); }

void UceApp::ImsAosMonitor_Connected(IN IMS_UINT32 nServices,
                                     IN IMS_UINT32 nIpcan) {
  IMS_UINT32 m_RegisteredServiceForEnabler = 0;
  IMS_UINT32 m_RegisteredServiceForJAVA = 0;

  m_RegisteredNetwork = GetServiceNetworkType(nIpcan);
  m_eCurrentNetwork = m_RegisteredNetwork;

  IMS_TRACE_I(
      "ImsAosMonitor_Connected:nServices [%08x], m_RegisteredNetwork [%d]",
      nServices, m_RegisteredNetwork, 0);

  if (m_piImsAos != null) {
    IImsAosInfo *pIImsAosInfo = m_piImsAos->GetAosInfo();
    if (pIImsAosInfo != IMS_NULL) {
      IMS_UINT32 registeredFeatures = pIImsAosInfo->GetImsFeatures();
      if (registeredFeatures & ImsAosFeature::VIDEO) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_VIDEO;
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_VIDEO;
        IMS_TRACE_I("ImsAosMonitor_Connected -Video tag", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::STANDALONE_MSG) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CPM_MSG;
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CPM_LARGEMSG;
        IMS_TRACE_I("ImsAosMonitor_Connected -Standalone Message", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::CHAT_SESSION) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CPM_SESSION;
        IMS_TRACE_I("ImsAosMonitor_Connected -CPM Session", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::FILE_TRANSFER) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_HTTPFT;
        IMS_TRACE_I("ImsAosMonitor_Connected -FT via HTTP", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::FILE_TRANSFER_VIA_SMS) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_FTSMS;
        IMS_TRACE_I("ImsAosMonitor_Connected -FT via SMS", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::CALL_COMPOSER_ENRICHED_CALLING) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CALL_COMPOSER;
        IMS_TRACE_I("ImsAosMonitor_Connected -Call Composer", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::GEO_PUSH) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_GEOPUSH;
        IMS_TRACE_I("ImsAosMonitor_Connected -Geolocation Push", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::GEO_PUSH_VIA_SMS) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_GEOSMS;
        IMS_TRACE_I("ImsAosMonitor_Connected -Geo push via SMS", 0, 0, 0);
      }

      if (registeredFeatures &
          ImsAosFeature::CHATBOT_COMMUNICATION_USING_SESSION) {
        m_RegisteredServiceForJAVA |= CONNECTED_SERVICE_CHATBOT;
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CHATBOT;
        IMS_TRACE_I("ImsAosMonitor_Connected -Chatbot Session", 0, 0, 0);
      }

      if (registeredFeatures &
          ImsAosFeature::CHATBOT_COMMUNICATION_USING_STANDALONE_MSG) {
        m_RegisteredServiceForJAVA |= CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG;
        m_RegisteredServiceForEnabler |=
            CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG;
        IMS_TRACE_I("ImsAosMonitor_Connected -Chatbot Standalone message", 0, 0,
                    0);
      }

      if (registeredFeatures & ImsAosFeature::CHATBOT_VERSION_SUPPORTED) {
        m_RegisteredServiceForJAVA |= CONNECTED_SERVICE_CHATBOT_V1;
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CHATBOT_V1;
        IMS_TRACE_I("ImsAosMonitor_Connected -botVersion=#1", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::CHATBOT_VERSION_V2_SUPPORTED) {
        m_RegisteredServiceForJAVA |= CONNECTED_SERVICE_CHATBOT_V2;
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_CHATBOT_V2;
        IMS_TRACE_I("ImsAosMonitor_Connected -botVersion=#1,#2", 0, 0, 0);
      }

      if (registeredFeatures & ImsAosFeature::PRESENCE) {
        m_RegisteredServiceForEnabler |= CONNECTED_SERVICE_PRESENCE;
        IMS_TRACE_I("ImsAosMonitor_Connected -Presence", 0, 0, 0);
      }
    }
  }

  if (m_eAoSStatus == AOS_CONNECTED) {
    IMSMSG objUIMsg(IUUceService::UCE_IMS_AGENT_REFRESHED_IND,
                    m_RegisteredNetwork, 0);
    MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
  } else {
    IMSMSG objUIMsg(IUUceService::UCE_IMS_AGENT_CONNECTED_IND,
                    m_RegisteredServiceForJAVA, m_RegisteredNetwork);
    MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
  }

  m_eAoSStatus = AOS_CONNECTED;
  if (m_pUceService != IMS_NULL) {
    m_pUceService->AoSConnected(m_RegisteredServiceForEnabler);
  }
}

void UceApp::ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) {
  IMS_TRACE_I("ImsAosMonitor_Notify:nType [ %d ], nInfo [ %d ]", nType, nState,
              0);
}

PRIVATE
void UceApp::EnableUceService() {
  m_pUceService = new UceService(m_strAppName, m_nSlotId);
  AttachService(m_pUceService);
  EnableAllAoSApps();
}

void UceApp::DisableUceService() {
  if (m_pUceService != IMS_NULL) {
    DetachService(m_pUceService);
    delete m_pUceService;
    m_pUceService = IMS_NULL;
  }
}

PRIVATE
IMS_SINT32 UceApp::GetServiceNetworkType(IN IMS_UINT32 nIpcan) {
  IMS_SINT32 nNetType = eUCE_RAT_INVALID;
  if (m_piImsAos == IMS_NULL) {
    return nNetType;
  }

  if (m_piImsAos->IsImsConnected() == IMS_FALSE) {
    return nNetType;
  }

  if (nIpcan == IIpcan::CATEGORY_MOBILE) {
    nNetType = ConvertNetworkType(m_piNetWatcherInfo->GetNetworkType());
  } else {
    nNetType = eUCE_RAT_WIFI;
  }
  return nNetType;
}

IMS_SINT32 UceApp::ConvertNetworkType(IN IMS_SINT32 nRAT) {
  switch (nRAT) {
    case INetworkWatcher::RADIOTECH_TYPE_INVALID:  // FALL-THROUGH
    case INetworkWatcher::RADIOTECH_TYPE_UNKNOWN:
      return eUCE_RAT_INVALID;

    case INetworkWatcher::RADIOTECH_TYPE_GPRS:  // FALL-THROUGH
    case INetworkWatcher::RADIOTECH_TYPE_EDGE:
      return eUCE_RAT_GERAN;

    case INetworkWatcher::RADIOTECH_TYPE_HSPA:   // FALL-THROUGH
    case INetworkWatcher::RADIOTECH_TYPE_HSPAP:  // FALL-THROUGH
    case INetworkWatcher::RADIOTECH_TYPE_UMTS:   // FALL-THROUGH
      return eUCE_RAT_UTRAN;

    case INetworkWatcher::RADIOTECH_TYPE_EHRPD:
      return eUCE_RAT_EHRPD;

    case INetworkWatcher::RADIOTECH_TYPE_LTE: {
      if (m_piNetWatcherInfo->IsImsVoiceCallSupported()) {
        return eUCE_RAT_LTE;
      }
      return eUCE_RAT_LTE_NO_VOPS;
    }

    case INetworkWatcher::RADIOTECH_TYPE_NR: {
      if (m_piNetWatcherInfo->IsImsVoiceCallSupported()) {
        return eUCE_RAT_NR;
      }
      return eUCE_RAT_NR_NO_VOPS;
    }

    default:
      return eUCE_RAT_INVALID;
  }
}

void UceApp::NotifyRATChanged() {
  IMS_SINT32 nNetworkType = eUCE_RAT_INVALID;
  if (m_RegisteredNetwork != eUCE_RAT_WIFI) {
    nNetworkType = ConvertNetworkType(m_piNetWatcherInfo->GetNetworkType());
  }
  IMS_TRACE_I("NotifyRATChanged:network type [%d]", nNetworkType, 0, 0);

  if (nNetworkType == eUCE_RAT_INVALID) {
    IMS_TRACE_I("NotifyRATChanged:Invalid Network", 0, 0, 0);
    return;
  }

  IMSMSG objUIMsg(IUUceService::UCE_NETWORK_CHANGED, 0, nNetworkType);
  MessageService::PostMessage(AString("JniUceServiceThread"), objUIMsg);
}

PRIVATE
void UceApp::EnableAllAoSApps() {
  IMS_TRACE_I("EnableAllAoSApps", 0, 0, 0);

  IMSList<IImsAos *> objImsAosList =
      ImsAos::GetImsAosList(m_strAppID, m_strServiceID, m_nSlotId);

  for (IMS_UINT32 i = 0; i < objImsAosList.GetSize(); ++i) {
    IMS_TRACE_D("EnableAllAoSApps:enable AoS Apps [%d]", i, 0, 0);
    IImsAos *piImsAos = objImsAosList.GetAt(i);

    piImsAos->SetListener(this);
    piImsAos->SetMonitor(this);
  }
}

PRIVATE
void UceApp::SelectActiveAoSApp() {
  IMS_TRACE_I("SelectActiveAoSApp", 0, 0, 0);

  IMSList<IImsAos *> objImsAosList =
      ImsAos::GetImsAosList(m_strAppID, m_strServiceID, m_nSlotId);
  IImsAos *piActiveImsAos = IMS_NULL;

  for (IMS_UINT32 i = 0; i < objImsAosList.GetSize(); ++i) {
    IMS_TRACE_D("SelectActiveAoSApp:enable AoS Apps [%d]", i, 0, 0);
    IImsAos *piImsAos = objImsAosList.GetAt(i);

    if (piImsAos->IsImsConnected()) {
      IMS_TRACE_D("SelectActiveAoSApp:enable AoS Apps was selected [%d]", i, 0,
                  0);
      piActiveImsAos = piImsAos;
      break;
    }
  }

  if ((piActiveImsAos != IMS_NULL) && (m_piImsAos != piActiveImsAos)) {
    IMS_TRACE_I("SelectActiveAoSApp:AoS is changed [%p]>>[%p]", m_piImsAos,
                piActiveImsAos, 0);
    m_piImsAos = piActiveImsAos;
  }
}

PRIVATE
const IMS_CHAR *UceApp::GetPrefixForMultiApp() {
  return UceNamespace::UCE_APP_NAME_PREFIX;
}

PRIVATE
AString UceApp::GetUceAppName(IN IMS_SINT32 nSlotId) {
  if ((nSlotId <= IMS_SLOT_ANY) || (nSlotId >= IMS_SLOT_MAX)) {
    nSlotId = IMS_SLOT_0;
  }

  // AString strName;
  // strName.Sprintf("%s%02d", GetPrefixForMultiApp(), nSlotId);
  AString strName(GetPrefixForMultiApp());

  return strName;
}

PRIVATE
void UceApp::SetPublishStatusToAos(IN IMS_BOOL bIsPublishStarted) {
  if (m_piImsAos != IMS_NULL) {
    IImsAosInfo *pIImsAosInfo = m_piImsAos->GetAosInfo();
    if (pIImsAosInfo != IMS_NULL) {
      pIImsAosInfo->NotifyPublishState(bIsPublishStarted);
    }
  }
}

PRIVATE
void UceApp::SendRegistrationRecoveryRequestToAos(
    IN IMS_UINT32 nAosControlType) {
  if (m_piImsAos != IMS_NULL) {
    m_piImsAos->Control(nAosControlType);
  }
}