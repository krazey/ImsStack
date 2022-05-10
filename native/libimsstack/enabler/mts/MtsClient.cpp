#include "ImsMessage.h"
#include "ServiceTrace.h"
#include "ServiceTimer.h"
#include "SipStatusCode.h"
#include "SystemConfig.h"

#include "IMtsClient.h"
#include "IUMts.h"

#include "MtsApp.h"
#include "MtsClient.h"
#include "MtsClientFactory.h"
#include "MtsFactory.h"

#include "utility/MtsDynamicLoader.h"
#include "utility/MtsSmUtils.h"
#include "utility/MtsStrName.h"

__IMS_TRACE_TAG_COM_SMS__;

LOCAL
MtsClient* s_pMtsClient[] = {IMS_NULL, IMS_NULL};

PUBLIC GLOBAL MtsClient* MtsClient::GetInstance(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        IMS_TRACE_E(0, "MtsClient::GetInstance() Error! Invalid SlotId:%d", nSlotId, 0, 0);
        nSlotId = 0;
    }

    IMS_TRACE_D("MtsClient::GetInstance(): slot[%d]", nSlotId, 0, 0);
    if (s_pMtsClient[nSlotId] == IMS_NULL)
    {
        IMS_TRACE_D("MtsClient::GetInstance: Generate New MtsClient slot[%d]", nSlotId, 0, 0);
        s_pMtsClient[nSlotId] = new MtsClient(nSlotId);
        IMS_TRACE_D("GetInstance: MtsClient[%d]: %d", nSlotId, s_pMtsClient[nSlotId], 0);
    }
    return s_pMtsClient[nSlotId];
}

void MtsClient::DestroyMtsClient(IN IMS_SINT32 nSlotId)
{
    if (s_pMtsClient[nSlotId] != IMS_NULL)
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

    if (objSms.GetLength() > 255)  // WMS_MAX_LEN
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
void MtsClient::ReportTransmissionResult(IN IMS_UINT32 nResponse, IN IMS_UINT32 nSmsType,
        IN IMS_SINT32 nSeqId /*= -1*/, IN IMS_SINT32 nSlotId)
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

    if ((nResponse == SipStatusCode::SC_200) || (nResponse == SipStatusCode::SC_202))
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
void MtsClient::ReportTransmissionFailureWithRetryTime(IN const IMS_UINT32 nSmsType,
        IN const IMS_UINT8 nRetryTime, IN IMS_SINT32 nSeqId /*= -1*/, IN IMS_SINT32 nSlotId)
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
void MtsClient::RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType, IN IMS_SINT32 nSlotId)
{
    MtsApp* pMtsApp = GetMtsApp(nSlotId);

    if (pMtsApp != IMS_NULL)
    {
        pMtsApp->RequestRegistrationRecovery(nRecoveryType);
    }
}

// TODO: VZW SCBM mode timer
PUBLIC
void MtsClient::Timer_TimerExpired(IN ITimer* piTimer)
{
    (void)piTimer;
}

// TODO: VZW SCBM mode timer
PUBLIC
void MtsClient::ClearTimer()
{
    return;
}

// TODO: VZW SCBM mode timer
PUBLIC
void MtsClient::StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration)
{
    IMS_UINTP nTimerId = 0;
    IMS_TRACE_I("StartTimer() - Type [%d] , Duration [%d] , Timer ID [%" PFLS_u "] ", nType,
            nDuration, nTimerId);

    return;
}

// TODO: VZW SCBM mode timer
PUBLIC
void MtsClient::StopTimer(IN IMS_UINT32 nType)
{
    IMS_TRACE_I("StopTimer() - Type [%d]", nType, 0, 0);
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
    IMS_TRACE_I("RemoveIMtsClient()", 0, 0, 0);
    MtsClientFactory::DestroyIMtsJavaClient(nSlotId);
}

PUBLIC
void MtsClient::Client_SendMo(IN IMSWMS_UINTP /*nWparam_*/, IN IWMSSmsSendRequestParam* nLparam)
{
    IMS_TRACE_I("Client_SendMo() nLparam->m_nSlotId (%d)", nLparam->m_nSlotId, 0, 0);
    IMSMSG objMSG(IUMts::MTS_MO_SEND_REQUEST, 0, reinterpret_cast<IMSWMS_UINTP>(nLparam));

    MtsApp* pMtsApp = GetMtsApp(nLparam->m_nSlotId);

    if (pMtsApp != IMS_NULL)
    {
        pMtsApp->PostMessage(objMSG);
    }
    else
    {
        IMS_TRACE_E(0, "pMtsApp is NULL", 0, 0, 0);
        ReportTransmissionResult(
                MtsClient::MO_IMS_PERM_FAILURE, nLparam->m_nSmsType, nLparam->m_nSeqId, m_nSlotId);
    }
}

PROTECTED
IMtsClient* MtsClient::GetIMtsClient(IN IMS_SINT32 nSlotId)
{
    IMtsClient* piClient = IMS_NULL;
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        IMS_TRACE_E(0, "MtsClient::GetIMtsClient() Error! Invalid SlotId:%d", nSlotId, 0, 0);
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
