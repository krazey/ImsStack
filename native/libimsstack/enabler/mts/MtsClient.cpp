#include "ImsMessage.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "SIPStatusCode.h"
#include "SystemConfig.h"
#include "IMtsClient.h"
#include "MtsClientFactory.h"
#include "MtsApp.h"
#include "utility/MtsStrName.h"
#include "utility/MtsSmUtils.h"
#include "utility/MtsDynamicLoader.h"
#include "MtsClient.h"
#include "IUSMS.h"
#include "MtsFactory.h"

__IMS_TRACE_TAG_COM_SMS__;

static MtsClient* s_pMtsClient[] = {IMS_NULL, IMS_NULL};

PUBLIC GLOBAL
MtsClient* MtsClient::GetInstance(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        IMS_TRACE_E(0,"MtsClient::GetInstance() Error! Invalid SlotId:%d", nSlotId,0,0);
        nSlotId = 0;
    }

    IMS_TRACE_D("MtsClient::GetInstance(): slot[%d]", nSlotId, 0, 0);
    if(s_pMtsClient[nSlotId] == IMS_NULL)
    {
        IMS_TRACE_D("MtsClient::GetInstance: Generate New MtsClient slot[%d]", nSlotId, 0, 0);
        s_pMtsClient[nSlotId] = new MtsClient(nSlotId);
        IMS_TRACE_D("GetInstance: MtsClient[%d]: %d", nSlotId, s_pMtsClient[nSlotId], 0);
    }
    return s_pMtsClient[nSlotId];
}

void MtsClient::DestroyMtsClient(IN IMS_SINT32 nSlotId)
{
    if(s_pMtsClient[nSlotId] != IMS_NULL)
    {
        IMS_TRACE_D("DestroyMtsClient: MtsClient[%d]: %d", nSlotId, s_pMtsClient[nSlotId], 0);
        delete s_pMtsClient[nSlotId];
        s_pMtsClient[nSlotId] = IMS_NULL;
    }
}

PRIVATE
MtsClient::MtsClient(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_pMtsServiceState(IMS_NULL),
        m_piSCRetryTimer(IMS_NULL),
        m_bIsConnectSC(IMS_FALSE),
        m_nSCCnxRetryCnt(0),
        m_nTempRetryAfterValue(0)
{
    IMS_TRACE_I("+MtsClient : slotId:[%d]", nSlotId, 0, 0);
}

PRIVATE
MtsClient::~MtsClient()
{
    IMS_TRACE_I("~MtsClient : slotId:[%d]", m_nSlotId, 0, 0);
    RemoveIMtsClient(m_nSlotId);
}

PUBLIC
IMS_BOOL MtsClient::HasIpcRouterFeature(IN IMS_SINT32 nSlotId)
{
    IMtsClient* piClient = GetIMtsClient(nSlotId);

    if (piClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return IMS_FALSE;
    }

    return ((piClient->GetFeature() & IMtsClient::FEATURE_IPC_ROUTER) > 0);
}

PUBLIC
IMS_UINT32 MtsClient::PassReceivedMessage(
        IN const ByteArray& objSms, IN const IMS_UINT32 nSmsType, IN IMS_SINT32 nSlotId)
{
    IMtsClient* pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return MtsClient::MT_FAILURE;
    }

    if (objSms.GetLength() > 255) // WMS_MAX_LEN
    {
        IMS_TRACE_E(0, "Too Large RPDU(%d)", objSms.GetLength(), 0, 0);
        return MtsClient::MT_FAILURE;
    }

    IMS_UINT32 nReportResult = pIMtsClient->ReportMtSMS(
            nSmsType, objSms.GetLength(), (const IMS_BYTE*)objSms.GetData(), nSlotId);

    if (nReportResult == IMtsClient::MT_SUCCESS)
    {
        return MtsClient::MT_SUCCESS;
    }
    else if (nReportResult == IMtsClient::MT_SMS_FORMAT_FAILURE)
    {
        return MtsClient::MT_SMS_FORMAT_FAILURE;
    }
    else if (nReportResult == IMtsClient::MT_SMS_NODATA_FAILURE)
    {
        return MtsClient::MT_SMS_NODATA_FAILURE;
    }
    else
    {
        return MtsClient::MT_FAILURE;
    }
}

PUBLIC
void MtsClient::ReportTransmissionResult(
        IN IMS_UINT32 nResponse,
        IN IMS_UINT32 nSmsType,
        IN IMS_SINT32 nSeqId /*= -1*/,
        IN IMS_SINT32 nSlotId)
{
    IMtsClient* pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("ReportTransmissionResult : nResponse (%d) nSmsType (%d) nSlotId (%d)", nResponse,
            nSmsType, nSlotId);

    IMS_UINT32 nResultCode = 0;

    if ((nResponse == SIPStatusCode::SC_200) || (nResponse == SIPStatusCode::SC_202))
    {
        IMS_TRACE_I("Reporting SUCCESS, nResponse is %d", nResponse, 0, 0);

        nResultCode = IMtsClient::MO_SUCCESS;
    }
    else if (nResponse == MtsClient::MO_IMS_LIMITEDSMSSVCREGI)
    {
        IMS_TRACE_E(0, "Reporting LIMITEDSMSSVCREGI FAILURE, nResponse is %d", nResponse, 0, 0);

        nResultCode = IMtsClient::MO_IMS_LIMITEDSMSSVCREGI;
    }
    else if (nResponse == MtsClient::MO_IMS_PERM_FAILURE)
    {
        IMS_TRACE_E(0, "Reporting PERMANENT FAILURE, nResponse is %d", nResponse, 0, 0);

        nResultCode = IMtsClient::MO_IMS_PERM_FAILURE;
    }
    else
    {
        IMS_TRACE_E(0, "ReportingTEMPORARY FAILURE, nResponse is %d", nResponse, 0, 0);

        // any provisional or failure final responses are failures.
        nResultCode = IMtsClient::MO_IMS_TEMP_FAILURE;
    }

    pIMtsClient->ReportMoStatus(nResultCode, nSmsType,
            (nResultCode == IMtsClient::MO_IMS_TEMP_FAILURE) ? m_nTempRetryAfterValue : 0, nSeqId,
            nSlotId);
}

PUBLIC
void MtsClient::ReportTransmissionFailureWithRetryTime(
        IN const IMS_UINT32 nSmsType,
        IN const IMS_UINT8 nRetryTime,
        IN IMS_SINT32 nSeqId /*= -1*/,
        IN IMS_SINT32 nSlotId)
{
    IMS_UINT32 nResultCode = IMtsClient::MO_IMS_TEMP_FAILURE;
    IMtsClient* pIMtsClient = IMS_NULL;

    pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("ReportTransmissionResult w/ RetryTime :: resultCode=%d, smsType=%d, retryTime=%d",
            nResultCode, nSmsType, nRetryTime);

    pIMtsClient->ReportMoStatus(nResultCode, nSmsType, nRetryTime, nSeqId, nSlotId);
}

PUBLIC
IMS_BOOL MtsClient::ConnectSC(IN IMS_SINT32 nSlotId)
{
    // TODO: this method is deprecated. It will be removed.
    IMtsClient* pIMtsClient = IMS_NULL;

    pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return IMS_FALSE;
    }

    // Initialize WMS client
    if (pIMtsClient->Init() == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Initialize WMS client failed", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strSmsFormat = "3GPP";

    IMS_UINT32 nSmsFormat = IMtsClient::SMSFORMAT_3GPP2;

    if (strSmsFormat.EqualsIgnoreCase("3gpp"))
    {
        nSmsFormat = IMtsClient::SMSFORMAT_3GPP;
    }
    else if (strSmsFormat.EqualsIgnoreCase("3gpp2"))
    {
        nSmsFormat = IMtsClient::SMSFORMAT_3GPP2;
    }
    else
    {
        IMS_TRACE_E(0, "Invalid sms format=%s, use default: 3gpp2", strSmsFormat.GetStr(), 0, 0);
        nSmsFormat = IMtsClient::SMSFORMAT_3GPP2;
    }

    // Make a connection.
    if (pIMtsClient->ConnectSC(nSmsFormat, nSlotId) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "connecting Mts Client failed", 0, 0, 0);
        return IMS_FALSE;
    }
    SetSCCnxState(IMS_TRUE);

    return IMS_TRUE;
}

PUBLIC
void MtsClient::DisconnectSC(IN IMS_SINT32 nSlotId)
{
    IMtsClient* pIMtsClient = IMS_NULL;

    IMS_TRACE_I( "DisconnectSC nSlotId(%d)", nSlotId, 0, 0);

    pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return;
    }

    pIMtsClient->DisconnectSC(nSlotId);
    SetSCCnxState(IMS_FALSE);
}

PUBLIC
void MtsClient::Retry_SCCnx()
{
    IMS_TRACE_I( "Retry connecting to MtsClient", 0, 0, 0);

    if (m_bIsConnectSC)
    {
        IMS_TRACE_E(0, "MtsClient connection already established", 0, 0, 0);
        return;
    }

    if (m_nSCCnxRetryCnt > 60)
    {
        m_nSCCnxRetryCnt = 0;
        IMS_TRACE_E(0, "Connecting to MtsClient permanently failed !!", 0, 0, 0);
        return;
    }

    m_nSCCnxRetryCnt++;
    StartTimer(TIMER_SMS_CLIENT_CNX_RETRY,3000);
}

PUBLIC
void MtsClient::SetSCCnxState(IN IMS_BOOL bState)
{
    if (m_bIsConnectSC == bState)
    {
        return;
    }

    m_bIsConnectSC = bState;

    IMS_TRACE_I("MtsClient Connection State is (%s)", _TRACE_B_(m_bIsConnectSC), 0, 0);

    if (!m_bIsConnectSC)
    {
        // Do nothing...
        return;
    }

    m_nSCCnxRetryCnt = 0;

    if (m_pMtsServiceState != IMS_NULL)
    {
        m_pMtsServiceState->UpdateServiceState();
    }
    else
    {
        IMS_TRACE_E(0, "Fail to get MtsServiceState instance !!", 0, 0, 0);
    }
}

PUBLIC
IMS_BOOL MtsClient::GetSCCnxState()
{
    // TODO: The CP SMS stack connection is deprecated and will be deleted.
    return IMS_TRUE;
}

PUBLIC
void MtsClient::UpdateMtsServiceState(IN IMS_UINT32 nStatus, IN IMS_SINT32 nSlotId)
{
    IMtsClient* pIMtsClient = IMS_NULL;

    pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return;
    }

    switch (nStatus)
    {
    case IMtsClient::STATE_LIMITED:
        // SMS MO BLOCK and MT OK
        pIMtsClient->UpdateServiceStatus(IMtsClient::STATE_LIMITED, nSlotId);
        IMS_TRACE_I("Update Mts Service Mode :: nSlotId(%d) STATE_LIMIT !!", nSlotId, 0, 0);
        break;

    case IMtsClient::STATE_READY:
        // SMS MO and MT OK
        pIMtsClient->UpdateServiceStatus(IMtsClient::STATE_READY, nSlotId);
        IMS_TRACE_I("Update Mts Service Mode :: nSlotId(%d) STATE_READY", nSlotId, 0, 0);
        break;

    case IMtsClient::STATE_NOTREADY:
        // SMS MO and MT BLOCK
        pIMtsClient->UpdateServiceStatus(IMtsClient::STATE_NOTREADY, nSlotId);
        IMS_TRACE_I("Update Mts Service Mode :: nSlotId(%d) STATE_NOTREADY", nSlotId, 0, 0);
        break;

    default :
        IMS_TRACE_E(0, "Update Mts Service Mode :: nSlotId(%d) INVALID STATE", nSlotId, 0, 0);
        break;
    }
}

PUBLIC
void MtsClient::UpdateSmsFormat(IN IMS_UINT32 nSmsFormat, IN IMS_SINT32 nSlotId)
{
    IMtsClient* pIMtsClient = IMS_NULL;

    pIMtsClient = GetIMtsClient(nSlotId);

    if (pIMtsClient == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtsClient is null", 0, 0, 0);
        return;
    }

    pIMtsClient->UpdateSmsFormat(nSmsFormat, nSlotId);
    IMS_TRACE_I("MtsClient::UpdateSmsFormat::(%d) nSlotId:(%d)", nSmsFormat, nSlotId, 0);
}

PUBLIC
void MtsClient::RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType, IN IMS_SINT32 nSlotId)
{
    MtsApp* pMtsApp = GetMtsApp(nSlotId);

    if (pMtsApp != IMS_NULL)
    {
        pMtsApp->RequestRegistrationRecovery(nRecoveryType);
    }
}

PUBLIC
void MtsClient::Timer_TimerExpired(IN ITimer *piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piSCRetryTimer)
    {
        IMS_TRACE_I("Timer_TimerExpired() --- Expired SMS Client re-connection Timer", 0, 0, 0);
        StopTimer(TIMER_SMS_CLIENT_CNX_RETRY);

        if (!GetSCCnxState())
        {
            if (!ConnectSC(m_nSlotId))
            {
                Retry_SCCnx();
            }
        }
    }
}

PUBLIC
void MtsClient::ClearTimer()
{
    IMS_TRACE_I("ClearTimer()", 0, 0, 0);

    if (m_piSCRetryTimer != IMS_NULL)
    {
        StopTimer(TIMER_SMS_CLIENT_CNX_RETRY);
    }
}

PUBLIC
void MtsClient::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    IMS_UINTP nTimerId = 0;

    if (nType == TIMER_SMS_CLIENT_CNX_RETRY)
    {
        if (m_piSCRetryTimer != IMS_NULL)
        {
            StopTimer(TIMER_SMS_CLIENT_CNX_RETRY);
        }
        m_piSCRetryTimer = TimerService::GetTimerService()->CreateTimer();
        nTimerId = m_piSCRetryTimer->SetTimer(nDuration, this);
    }
    else
    {
        IMS_TRACE_E(0, "Invalid Timer Type", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("StartTimer() - Type [%d] , Duration [%d] , Timer ID [%" PFLS_u "] ", \
            nType, nDuration, nTimerId);
}

PUBLIC
void MtsClient::StopTimer(IN IMS_UINT32 nType)
{
    if (nType == TIMER_SMS_CLIENT_CNX_RETRY)
    {
        if (m_piSCRetryTimer != IMS_NULL)
        {
            m_piSCRetryTimer->KillTimer();
            TimerService::GetTimerService()->DestroyTimer(m_piSCRetryTimer);
            m_piSCRetryTimer = IMS_NULL;
        }
        else
        {
            IMS_TRACE_E(0, "Already Service In Out Timer Expired", 0, 0, 0);
        }
    }
    else
    {
        IMS_TRACE_E(0, "Invalid Timer Type", 0, 0, 0);
        return;
    }
}

PUBLIC
void MtsClient::SetTempRetryAfterValue(IN IMS_UINT8 nValue)
{
    m_nTempRetryAfterValue = nValue;
}

PUBLIC
void MtsClient::SetServiceState(IN MtsServiceState* pMtsServiceState)
{
    if (pMtsServiceState == IMS_NULL)
    {
        IMS_TRACE_E(0, "pMtsServiceState is NULL", 0, 0, 0);
        return;
    }

    m_pMtsServiceState = pMtsServiceState;
}

PROTECTED
void MtsClient::RemoveIMtsClient(IN IMS_SINT32 nSlotId)
{
    if (m_piSCRetryTimer != IMS_NULL)
    {
        m_piSCRetryTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piSCRetryTimer);
        m_piSCRetryTimer = IMS_NULL;
    }

    MtsClientFactory::DestroyIMtsJavaClient(nSlotId);
}

PUBLIC
void MtsClient::Client_SendMo(
        IN IMSWMS_UINTP /*nWparam_*/, IN IWMSSmsSendRequestParam* nLparam)
{
    IMS_TRACE_I("Client_SendMo() nLparam->nSlotId (%d)", nLparam->nSlotId, 0, 0);
    IMSMSG objMSG(IUSMS::SMSMO_SEND_REQUEST, 0, reinterpret_cast<IMSWMS_UINTP>(nLparam));

    MtsApp* pMtsApp = GetMtsApp(nLparam->nSlotId);

    if (pMtsApp != IMS_NULL)
    {
        pMtsApp->PostMessage(objMSG);
    }
    else
    {
        IMS_TRACE_E(0, "pMtsApp is NULL", 0, 0, 0);
        ReportTransmissionResult(MtsClient::MO_IMS_PERM_FAILURE, nLparam->nSmsType,
                nLparam->nSeqId, m_nSlotId);
    }
}

PUBLIC
void MtsClient::Client_ControlService(
        IN IMSWMS_UINTP /*nWparam_*/, IN IWMSSmsServiceControlParam* nLparam)
{
    IMS_TRACE_I("Client_ControlService(), slot[%d]", nLparam->nSlotId, 0, 0);
    IMSMSG objMSG(IUSMS::SMS_SERVICE_CONTROL, 0, reinterpret_cast<IMSWMS_UINTP>(nLparam));

    MtsApp* pMtsApp = GetMtsApp(nLparam->nSlotId);
    if (pMtsApp != IMS_NULL)
    {
        pMtsApp->PostMessage(objMSG);
    }
}

PROTECTED
IMtsClient* MtsClient::GetIMtsClient(IN IMS_SINT32 nSlotId)
{
    IMtsClient* piClient = IMS_NULL;
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        IMS_TRACE_E(0,"MtsClient::GetIMtsClient() Error! Invalid SlotId:%d", nSlotId,0,0);
        return IMS_NULL;
    }

    IMS_TRACE_I("GetIMtsClient() : SoiClient slotId:[%d]", nSlotId, 0, 0);
    piClient = MtsClientFactory::GetIMtsJavaClient(nSlotId);

    return piClient;
}

PUBLIC
MtsApp* MtsClient::GetMtsApp(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("GetMtsApp nSlotId : (%d)", nSlotId, 0, 0);

    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        IMS_TRACE_E(0, "Invalid nSlotId Type", 0, 0, 0);
        return IMS_NULL;
    }

    MtsApp* pMtsApp = MtsFactory::GetInstance()->GetMtsApp(nSlotId);
    if (pMtsApp == IMS_NULL)
    {
        IMS_TRACE_I("cannot find MtsApp from nSlotId [%d]", nSlotId, 0, 0);
    }
    else
    {
        IMS_TRACE_I("Check to find MtsApp from nSlotId [%d]", nSlotId, 0, 0);
    }

    return pMtsApp;
}
