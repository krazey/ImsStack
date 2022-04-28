#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ServiceMessage.h"

#include "ByteArray.h"

#include "IUMts.h"
#include "IUMtsService.h"

#include "AndroidJavaWms.h"
#include "EnablerUtils.h"

#define WMS_CONNECTIONID_INVALID    (-1)
#define WMS_DEFAULT_SLOTID          (0)
#define WMS_SLOTID_0                (0)
#define WMS_SLOTID_1                (1)

__IMS_TRACE_TAG_ADAPT__;

LOCAL IMS_UINT32 s_nSlotId = 0;
LOCAL CBJniMtsService s_pCBJniMtsService = IMS_NULL;
AndroidJavaWMS* AndroidJavaWMS::s_pAndroidJavaWms[] = {IMS_NULL, IMS_NULL};

static IMS_UINT32 androidJavaWms_GetSlotId()
{
    return s_nSlotId;
}

static void androidJavaWms_SetSlotId(IN IMS_UINT32 nSlotId)
{
    s_nSlotId = nSlotId;
}

void androidJavaWms_SetJniMtsService(IN CBJniMtsService pCB)
{
    s_pCBJniMtsService = pCB;
}

void androidJavaWms_UpdateSlotId(IN IMS_SINT32 nSlotId)
{
    char byData[2] = { 0x30, 0x00 };

    IMS_TRACE_I("androidJavaWms_UpdateSlotId :: slot id (%s), nSlotId (%d), prevSlotID (%d)",
            byData, nSlotId, androidJavaWms_GetSlotId());

    if (strcmp(byData, "1") == 0)
    {
        androidJavaWms_SetSlotId(WMS_SLOTID_1);
    }
    else
    {
        androidJavaWms_SetSlotId(WMS_SLOTID_0);
    }
}

void androidJavaWms_sendMsgToJava(IN IMS_UINT32 nMsg, IMS_UINTP pParam, IN IMS_SINT32 nSlotId)
{
    IMSMSG objUIMsg(nMsg, 0, pParam);
    IMS_UINT32 nSlotId_ = nSlotId;
    MessageService::PostMessage(STR_MTS_SVC_THREAD_NAME[nSlotId_], objUIMsg);
}

IMS_BOOL androidJavaWms_processSendMOSMS(
        IN IMS_UINT32 nSmsFormat,
        IN const ByteArray& objData,
        IN const AString& strAddr,
        IN IMS_SINT32 nSeqId,
        IN IMS_SINT32 nSlotId)
{
    IMS_UINT32 slotId = nSlotId;

    if (objData.GetLength() == 0 || strAddr.GetLength() == 0)
    {
        IMS_TRACE_I("processSendMOSMS :: data or addr is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("processSendMOSMS :: format (%s) , seq id (%d), slotId (%d)",
        (nSmsFormat == 1) ? "3GPP" : "3GPP2", nSeqId, slotId);

    IUSendSmsRequestParam* pParam = new IUSendSmsRequestParam();

    if (IMS_NULL == pParam)
    {
        IMS_TRACE_D("memory allocation for wms request failed!!", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_MEM_Memset(pParam->m_baSmsData, 0, 512);
    IMS_MEM_Memset(pParam->m_szDestAddr, 0, IMS_SOLUTION_URI_LEN+1);

    pParam->m_nSmsType = nSmsFormat;
    pParam->m_nSmsDataLen = objData.GetLength();
    pParam->m_nSeqId = nSeqId;

    IMS_TRACE_D("data length (%d) , address (%s)", objData.GetLength(), strAddr.GetStr(), 0);

    IMS_MEM_Memcpy(pParam->m_baSmsData, objData.GetData(), objData.GetLength());

    IMS_BOOL bIsAckorError = IMS_FALSE;

    if (pParam->m_nSmsType == IMtsClient::SMSFORMAT_3GPP)
    {
        bIsAckorError = (((*(objData.GetData()) & 0x07) == 2) ||
        ((*(objData.GetData()) & 0x07) == 4) ||
        ((*(objData.GetData()) & 0x07) == 6))? IMS_TRUE : IMS_FALSE;
    }

    if (bIsAckorError)
    {
        IMS_TRACE_I("destination address is not used for sending sms acknowledgment", 0, 0, 0);
    }
    else
    {
        IMS_StrCpy(pParam->m_szDestAddr, IMS_SOLUTION_URI_LEN + 1, strAddr.GetStr());
    }

    AString aStrTargetActivity = EnablerUtils::GetEnablerThreadName(slotId);
    aStrTargetActivity.Append(".MtsApp");
    IMSMSG objMSG(IUMts::MTS_MO_SEND_REQUEST, 0, reinterpret_cast<IMS_UINTP>(pParam));
    MessageService::PostMessage(aStrTargetActivity, objMSG);

    return IMS_TRUE;
}

PRIVATE
AndroidJavaWMS::AndroidJavaWMS(IN IMS_SINT32 nSlotId) :
        IMSActivityEx("AndroidJavaWms"),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_D("AndroidJavaWMS() :: Name(%s)", GetName().GetStr(), 0, 0);
    androidJavaWms_SetSlotId(nSlotId);
}

PRIVATE VIRTUAL
AndroidJavaWMS::~AndroidJavaWMS()
{
    IMS_TRACE_D("~AndroidJavaWMS() :: Name(%s)", GetName().GetStr(), 0, 0);

}

PUBLIC GLOBAL
AndroidJavaWMS* AndroidJavaWMS::GetAndroidJavaWMS(IN IMS_SINT32 nSlotId)
{
    if (nSlotId < 0 || nSlotId > 1) {
        IMS_TRACE_E(0,"GetAndroidJavaWMS() Error! Invalid SlotId:%d", nSlotId,0,0);
        return IMS_NULL;
    }

    if (s_pAndroidJavaWms[nSlotId] == IMS_NULL)
    {
        IMS_TRACE_D("GetAndroidJavaWMS: Generate New AndroidJavaWMS slot[%d]", nSlotId, 0, 0);
        s_pAndroidJavaWms[nSlotId] = new AndroidJavaWMS(nSlotId);
        IMS_TRACE_D("GetAndroidJavaWMS: s_pAndroidJavaWms[%d]: %d", nSlotId,
                s_pAndroidJavaWms[nSlotId], 0);
    }
    return s_pAndroidJavaWms[nSlotId];
}

PUBLIC GLOBAL
void AndroidJavaWMS::DestroyAndroidJavaWMS(IN IMS_SINT32 nSlotId)
{
    if (s_pAndroidJavaWms[nSlotId] != IMS_NULL)
    {
        IMS_TRACE_D("DestroyAndroidJavaWMS: s_pAndroidJavaWms[%d]: %d", nSlotId,
                s_pAndroidJavaWms[nSlotId], 0);
        delete s_pAndroidJavaWms[nSlotId];
        s_pAndroidJavaWms[nSlotId] = IMS_NULL;
    }
}

PUBLIC GLOBAL
IMS_BOOL AndroidJavaWMS::StartUp()
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
void AndroidJavaWMS::CleanUp()
{
}

PUBLIC VIRTUAL
IMS_RESULT AndroidJavaWMS::Init()
{
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_RESULT AndroidJavaWMS::Release()
{
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_UINT32 AndroidJavaWMS::ReportMtSMS(
        IN IMS_UINT32 nSmsformat,
        IN IMS_UINT32 nSmslength,
        IN CONST IMS_BYTE* pbySmsdata,
        IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("ReportMtSMS :: SMS Format(%s) length(%d) slotid(%d)",
            (IMtsClient::SMSFORMAT_3GPP == nSmsformat) ? "3GPP" : "3GPP2", nSmslength, nSlotId);

    IUMtsServiceReportMtSmsParam* pSmsServiceReportMTSMSParam = new IUMtsServiceReportMtSmsParam();
    pSmsServiceReportMTSMSParam->nSmsFormat = nSmsformat;
    AString strData = AString::ConstNull();
    strData.Attach(reinterpret_cast<const IMS_CHAR*>(pbySmsdata), nSmslength);
    pSmsServiceReportMTSMSParam->objData = strData.ToBase64();
    pSmsServiceReportMTSMSParam->nSlotId = nSlotId;

    if (s_pCBJniMtsService != IMS_NULL)
    {
        return (s_pCBJniMtsService(IUMtsService::REPORT_MTS_MT_SMS,
                reinterpret_cast<IMS_UINTP>(pSmsServiceReportMTSMSParam), nSlotId) < 0)
                ? IMtsClient::MT_FAILURE : IMtsClient::MT_SUCCESS;
    }

    delete pSmsServiceReportMTSMSParam;

    return IMtsClient::MT_FAILURE;
}

PUBLIC VIRTUAL
IMS_RESULT AndroidJavaWMS::ReportMoStatus(
        IN IMS_UINT32 nReason,
        IN IMS_UINT32 nSmsformat,
        IN IMS_UINT8 nRetryAfter /* = 0 */,
        IN IMS_SINT32 nSeqId /* = -1 */,
        IN IMS_SINT32 nSlotId)
{
    IMS_CHAR acLog[128+1] = {0, };
    IMS_Sprintf(acLog, 128, "reason (%s, %d) , SMS Format (%s) , nSeqId (%d)",
            (IMtsClient::MO_SUCCESS == nReason) ? "success" : "failure", nReason,
            (IMtsClient::SMSFORMAT_3GPP == nSmsformat) ? "3GPP" : "3GPP2", nSeqId);

    IMS_TRACE_I("ReportMoStatus ::  %s", acLog, 0, 0);

    IUMtsServiceReportMoStatusParam* pSmsServiceReportMOStatusParam
            = new IUMtsServiceReportMoStatusParam();
    pSmsServiceReportMOStatusParam->nReason = nReason;
    pSmsServiceReportMOStatusParam->nSmsFormat = nSmsformat;
    pSmsServiceReportMOStatusParam->nRetryAfter = nRetryAfter;
    pSmsServiceReportMOStatusParam->nSeqId = nSeqId;
    pSmsServiceReportMOStatusParam->nSlotId = nSlotId;
    androidJavaWms_sendMsgToJava(IUMtsService::REPORT_MTS_MO_STATUS,
            (IMS_UINTP)pSmsServiceReportMOStatusParam, nSlotId);
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_BOOL AndroidJavaWMS::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage [%d], slotid : [%d]", objMSG.nMSG, m_nSlotId, 0);
    IMS_BOOL bResult = IMS_FALSE;

    switch (objMSG.nMSG)
    {
        case IUMtsService::NOTI_MTSENABLER_SEND_MO_SMS:
        {
            IUMtsServiceSendMoSmsParam* pParam
                    = reinterpret_cast<IUMtsServiceSendMoSmsParam*>(objMSG.nLparam);
            androidJavaWms_processSendMOSMS(
                    pParam->nSmsFormat,
                    pParam->objData,
                    pParam->strAddr,
                    pParam->nSeqId,
                    m_nSlotId);
            bResult = IMS_TRUE;
        }
        break;
        default:
        {
            IMS_TRACE_D("OnMessage :: Not Handled Message(%d)", objMSG.nMSG, 0, 0);
            bResult = IMS_FALSE;
        }
        break;
    }

    return bResult;
}
