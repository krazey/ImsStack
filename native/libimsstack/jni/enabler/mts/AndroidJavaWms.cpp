/*
    Author
    <table>
    date                author                          description
    --------        --------------                  ----------
    20100208        hwangoo.park@               Created
    20101006        hwangoo.park@               Add EventSender related code
    20111206        soonwoo.hwang@              refactoring for Common Mts
    20140226        hoonsnag.yun@               Create AndroidJavaWms
    20140511        hoonsang.yun@               modify for using SmsServiceInterface
    20181001        narae.eo@                   refactoring for MTK DSDV : TSC-1181
    20191016        rian.kim@                   Modify Rety to connectSC
    20200228        rian.kim@                   Modify info log for SMS format when sending SMS based MTK
    </table>

    Description
    This file defines the factory methods for IMS client platform.
    It's only for creating an instance of the platform-related class.
*/
#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ServiceMessage.h"

#include "ByteArray.h"

#include "IUSMS.h"
#include "IUSmsService.h"

#include "AndroidJavaWms.h"
#include "EnablerUtils.h"


#define WMS_CONNECTIONID_INVALID            (-1)
#define WMS_DEFAULT_SLOTID  (0)
#define WMS_SLOTID_0        (0)
#define WMS_SLOTID_1        (1)

__IMS_TRACE_TAG_ADAPT__;

static IMS_UINT32 geSlotId = 0;
static CBJniSmsService gpCBJniSmsService = IMS_NULL;
AndroidJavaWMS* AndroidJavaWMS::pAndroidJavaWMS[] = {IMS_NULL, IMS_NULL};


static IMS_UINT32 androidJavaWms_GetSlotId()
{
    return geSlotId;
}

static void androidJavaWms_SetSlotId(IN IMS_UINT32 slotId)
{
    geSlotId = slotId;
}

void androidJavaWms_SetJniSmsService(IN CBJniSmsService pCB)
{
    gpCBJniSmsService = pCB;
}

void androidJavaWms_UpdateSlotId(IN IMS_SINT32 nSlotID)
{
    char byData[2] = { 0x30, 0x00 };

    IMS_TRACE_I("androidJavaWms_UpdateSlotId :: slot id (%s), nSlotID (%d), prevSlotID (%d)",
                    byData, nSlotID, androidJavaWms_GetSlotId());

    if (strcmp(byData, "1") == 0)
    {
        androidJavaWms_SetSlotId(WMS_SLOTID_1);
    }
    else
    {
        androidJavaWms_SetSlotId(WMS_SLOTID_0);
    }
}

void androidJavaWms_sendMsgToJava(IN IMS_UINT32 nMsg, IMS_UINTP pParam, IN IMS_SINT32 nSlotID)
{
    IMSMSG objUIMsg(nMsg, 0, pParam);
    IMS_UINT32 nSlotId = nSlotID;
    MessageService::PostMessage(STR_SMS_SVC_THREAD_NAME[nSlotId], objUIMsg);
}

IMS_BOOL androidJavaWms_processSendMOSMS(IN IMS_UINT32 nSmsFormat,
            IN CONST  ByteArray &objData, IN CONST AString &strAddr, IN IMS_SINT32 nSeqId, IN IMS_SINT32 nSlotID)
{
    IUSmsSendRequestParam *pParam = IMS_NULL;
    IMS_BOOL bIsAckorError = IMS_FALSE;
    IMS_UINT32 slotId = nSlotID;

    if (objData.GetLength() == 0 || strAddr.IsNULL())
    {
        IMS_TRACE_I("processSendMOSMS :: data or addr is invalid", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("processSendMOSMS :: format (%s) , seq id (%d), slotId (%d)",
        (nSmsFormat == 1) ? "3GPP" : "3GPP2", nSeqId, slotId);

    pParam = new IUSmsSendRequestParam();

    if (IMS_NULL == pParam)
    {
        IMS_TRACE_D("memory allocation for wms request failed!!", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_MEM_Memset(pParam->baSmsData, 0, 512);
    IMS_MEM_Memset(pParam->szDestAddr, 0, IMS_SOLUTION_URI_LEN+1);

    pParam->nSmsType = nSmsFormat;
    pParam->nSmsDataLen = objData.GetLength();
    pParam->nSeqId = nSeqId;

    IMS_TRACE_D("data length (%d) , address (%s)", objData.GetLength(), strAddr.GetStr(), 0);

    IMS_MEM_Memcpy(pParam->baSmsData, objData.GetData(), objData.GetLength());

    if (pParam->nSmsType == IMtsClient::SMSFORMAT_3GPP)
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
        IMS_StrCpy(pParam->szDestAddr, IMS_SOLUTION_URI_LEN + 1, strAddr.GetStr());
    }

    AString aStrTargetActivity = EnablerUtils::GetEnablerThreadName(slotId);
    aStrTargetActivity.Append(".MtsApp");
    IMSMSG objMSG(IUSMS::SMSMO_SEND_REQUEST, 0, reinterpret_cast<IMS_UINTP>(pParam));
    MessageService::PostMessage(aStrTargetActivity, objMSG);

    return IMS_TRUE;
}

PRIVATE
AndroidJavaWMS::AndroidJavaWMS(IN IMS_SINT32 nSlotID)
    : IMSActivityEx("AndroidJavaWms")
    , isWmsConnected(IMS_FALSE)
    , isSmsServerReady(IMS_FALSE)
    , nClientHandle(-1)
    , nTransportID(WMS_CONNECTIONID_INVALID)
    , nSmsFormat(IMtsClient::SMSFORMAT_INVALID)
    , nStatus(IMtsClient::STATE_INIT)
    , mSlotID(nSlotID)
{
    IMS_TRACE_D("AndroidJavaWMS() :: Name(%s)", GetName().GetStr(), 0, 0);
    androidJavaWms_SetSlotId(nSlotID);
}

/* ------------------------------------------------------------------------------------------------
*Destructor of AndroidJavaWMS
------------------------------------------------------------------------------------------------ */

PRIVATE VIRTUAL
AndroidJavaWMS::~AndroidJavaWMS()
{
    IMS_TRACE_D("~AndroidJavaWMS() :: Name(%s)", GetName().GetStr(), 0, 0);

}

/* ------------------------------------------------------------------------------------------------
*Get a instance of AndroidJavaWMS
*
*@param nSlotID : SIM Slot ID
*@return pointer of pAndroidJavaWMS[nSlotID] : static array
------------------------------------------------------------------------------------------------ */

PUBLIC GLOBAL
AndroidJavaWMS* AndroidJavaWMS::GetAndroidJavaWMS(IN IMS_SINT32 nSlotID)
{
    if (nSlotID < 0 || nSlotID > 1) {
        IMS_TRACE_E(0,"GetAndroidJavaWMS() Error! Invalid SlotId:%d", nSlotID,0,0);
        return IMS_NULL;
    }

    if(pAndroidJavaWMS[nSlotID] == IMS_NULL)
    {
        IMS_TRACE_D("GetAndroidJavaWMS: Generate New AndroidJavaWMS slot[%d]", nSlotID, 0, 0);
        pAndroidJavaWMS[nSlotID] = new AndroidJavaWMS(nSlotID);
        IMS_TRACE_D("GetAndroidJavaWMS: pAndroidJavaWMS[%d]: %d", nSlotID, pAndroidJavaWMS[nSlotID], 0);
    }
    return pAndroidJavaWMS[nSlotID];
}

/* ------------------------------------------------------------------------------------------------
*Destroy instance of AndroidJavaWMS
*
*@param nSlotID : SIM Slot ID
------------------------------------------------------------------------------------------------ */
PUBLIC GLOBAL
void AndroidJavaWMS::DestroyAndroidJavaWMS(IN IMS_SINT32 nSlotID)
{
    if(pAndroidJavaWMS[nSlotID] != IMS_NULL)
    {
        IMS_TRACE_D("DestroyAndroidJavaWMS: pAndroidJavaWMS[%d]: %d", nSlotID, pAndroidJavaWMS[nSlotID], 0); // will be deleted
        delete pAndroidJavaWMS[nSlotID];
        pAndroidJavaWMS[nSlotID] = IMS_NULL;
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
IMS_RESULT AndroidJavaWMS::ConnectSC(IN IMS_UINT32 nSmsFormat, IN IMS_SINT32 nSlotID)
{
    this->nSmsFormat = nSmsFormat;

    if (!isSmsServerReady)
    {
        IMS_TRACE_I("ConnectSC :: Sms Server is not Ready", 0, 0, 0);
        return IMS_FAILURE;
    }

    // androidJavaWms_UpdateSlotId(nSlotID);

    IUSmsServiceCreateMtsManagerParam *pSmsServiceCreateMtsManagerParam =
        new IUSmsServiceCreateMtsManagerParam();
    if (pSmsServiceCreateMtsManagerParam == IMS_NULL)
    {
        IMS_TRACE_D("ConnectSC :: param creation is failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    pSmsServiceCreateMtsManagerParam->nSmsFormat = nSmsFormat;
    androidJavaWms_sendMsgToJava(IUSmsService::CREATE_MTS_MANAGER,
        (IMS_UINTP)pSmsServiceCreateMtsManagerParam, nSlotID);

    isWmsConnected = IMS_TRUE;

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
void AndroidJavaWMS::DisconnectSC(IN IMS_SINT32 nSlotID)
{
    this->nSmsFormat = IMtsClient::SMSFORMAT_INVALID;
    isWmsConnected = IMS_FALSE;
    androidJavaWms_sendMsgToJava(IUSmsService::DESTROY_MTS_MANAGER, 0, nSlotID);

    return;
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
void AndroidJavaWMS::UpdateServiceStatus(IN IMS_UINT32 status, IN IMS_SINT32 nSlotID)
{
    IUSmsServiceUpdateServiceStatusParam *pSmsServiceUpdateServiceStatusParam =
        new IUSmsServiceUpdateServiceStatusParam();
    if(pSmsServiceUpdateServiceStatusParam == IMS_NULL)
    {
        IMS_TRACE_D("UpdateServiceStatus :: param creation is failed", 0, 0, 0);
        return;
    }
    pSmsServiceUpdateServiceStatusParam->nImsStatus = status;
    nStatus = status;
    pSmsServiceUpdateServiceStatusParam->nSlotId = nSlotID;

    /*if(IMtsClient::STATE_READY == status)
    {
        androidJavaWms_UpdateSlotId(nSlotID);
    }*/
    androidJavaWms_sendMsgToJava(IUSmsService::UPDATE_MTS_SVC_STATUS,
        (IMS_UINTP)pSmsServiceUpdateServiceStatusParam, nSlotID);

    return;
}

PUBLIC VIRTUAL
void AndroidJavaWMS::UpdateSmsFormat(IN  IMS_UINT32 nSmsFormat, IN IMS_SINT32 nSlotID)
{
    IUSmsServiceUpdateSMSFormatParam *pSmsServiceUpdateSMSFormatParam =
        new IUSmsServiceUpdateSMSFormatParam();
    if(pSmsServiceUpdateSMSFormatParam == IMS_NULL)
    {
        IMS_TRACE_D("UpdateSmsFormat :: param creation is failed", 0, 0, 0);
        return;
    }
    pSmsServiceUpdateSMSFormatParam->nSmsFormat = nSmsFormat;
    this->nSmsFormat = nSmsFormat;
    pSmsServiceUpdateSMSFormatParam->nSlotId = nSlotID;
    androidJavaWms_sendMsgToJava(IUSmsService::UPDATE_MTS_SMS_FORMAT,
        (IMS_UINTP)pSmsServiceUpdateSMSFormatParam, nSlotID);
    return;
}

PUBLIC VIRTUAL
IMS_UINT32 AndroidJavaWMS::ReportMtSMS(IN IMS_UINT32 smsformat, IN IMS_UINT32 smslength,
    IN CONST IMS_BYTE* smsdata, IN IMS_SINT32 nSlotID)
{
    IMS_TRACE_I("ReportMtSMS :: SMS Format(%s) length(%d) slotid(%d)",
        (IMtsClient::SMSFORMAT_3GPP == smsformat) ? "3GPP" : "3GPP2", smslength, nSlotID);

    IUSmsServiceReportMTSMSParam *pSmsServiceReportMTSMSParam = new IUSmsServiceReportMTSMSParam();
    if(pSmsServiceReportMTSMSParam == IMS_NULL)
    {
        IMS_TRACE_D("ReportMtSMS :: param creation is failed", 0, 0, 0);
        return IMtsClient::MT_FAILURE;
    }
    pSmsServiceReportMTSMSParam->nSmsFormat = smsformat;
    AString strData = AString::ConstNull();
    strData.Attach(reinterpret_cast<const IMS_CHAR*>(smsdata), smslength);
    pSmsServiceReportMTSMSParam->objData = strData.ToBase64();
    pSmsServiceReportMTSMSParam->nSlotId = nSlotID;

    if (gpCBJniSmsService != IMS_NULL)
    {
        return (gpCBJniSmsService(IUSmsService::REPORT_MTS_MT_SMS,
            reinterpret_cast<IMS_UINTP>(pSmsServiceReportMTSMSParam), nSlotID) < 0)
            ? IMtsClient::MT_FAILURE : IMtsClient::MT_SUCCESS;
    }

    delete pSmsServiceReportMTSMSParam;

    return IMtsClient::MT_FAILURE;
}

PUBLIC VIRTUAL
IMS_RESULT AndroidJavaWMS::ReportMoStatus(IN IMS_UINT32 reason, IN IMS_UINT32 smsformat,
    IN IMS_UINT8 nRetryAfter /* = 0 */, IN IMS_SINT32 nSeqId /* = -1*/, IN IMS_SINT32 nSlotID)
{
    IMS_CHAR acLog[128+1] = {0, };
    IMS_Sprintf(acLog, 128, "reason (%s, %d) , SMS Format (%s) , nSeqId (%d)",
            (IMtsClient::MO_SUCCESS == reason) ? "success" : "failure", reason,
            (IMtsClient::SMSFORMAT_3GPP == smsformat) ? "3GPP" : "3GPP2", nSeqId);

    IMS_TRACE_I("ReportMoStatus ::  %s", acLog, 0, 0);

    IUSmsServiceReportMOStatusParam *pSmsServiceReportMOStatusParam =
        new IUSmsServiceReportMOStatusParam();
    if(pSmsServiceReportMOStatusParam == IMS_NULL)
    {
        IMS_TRACE_D("ReportMoStatus :: param creation is failed", 0, 0, 0);
        return IMS_FAILURE;
    }
    pSmsServiceReportMOStatusParam->nReason = reason;
    pSmsServiceReportMOStatusParam->nSmsFormat = smsformat;
    pSmsServiceReportMOStatusParam->nRetryAfter = nRetryAfter;
    pSmsServiceReportMOStatusParam->nSeqId = nSeqId;
    pSmsServiceReportMOStatusParam->nSlotId = nSlotID;
    androidJavaWms_sendMsgToJava(IUSmsService::REPORT_MTS_MO_STATUS,
        (IMS_UINTP)pSmsServiceReportMOStatusParam, nSlotID);
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_BOOL AndroidJavaWMS::OnMessage( IN IMSMSG &objMSG )
{
    IMS_TRACE_I( "OnMessage [%d], slotid : [%d]", objMSG.nMSG, mSlotID, 0 );
    IMS_BOOL bResult = IMS_FALSE;

    switch(objMSG.nMSG)
    {
        case IUSmsService::NOTI_SMSSERVICE_SEND_MO_SMS:
        {
            IUSmsServiceSendMoSMSParam *pParam =
                reinterpret_cast<IUSmsServiceSendMoSMSParam*>(objMSG.nLparam);
            androidJavaWms_processSendMOSMS(pParam->nSmsFormat,
                pParam->objData, pParam->strAddr, pParam->nSeqId, mSlotID);
            bResult = IMS_TRUE;
        }
        break;
        case IUSmsService::NOTI_IMS_PHONE_RESTARTED:
        {
            if (isWmsConnected != IMS_TRUE)
            {
                return IMS_TRUE;
            }

            isWmsConnected = IMS_FALSE;
            ConnectSC(nSmsFormat, mSlotID);
            UpdateServiceStatus(nStatus, mSlotID);
        }
        break;
        case IUSmsService::NOTI_SMS_SERVER_READY:
        {
            isSmsServerReady = IMS_TRUE;

            // SMS_Patch_0664
            IUSmsReconnectScParam *pParam = new IUSmsReconnectScParam();
            if (pParam == IMS_NULL)
            {
                IMS_TRACE_D("OnMessage :: Retry ConnectSC, param creation is failed", 0, 0, 0);
            }
            else
            {
                AString aStrTargetActivity = EnablerUtils::GetEnablerThreadName(mSlotID);
                aStrTargetActivity.Append(".MtsApp");
                IMSMSG objMSG(IUSMS::SMS_RECONNECT_SC, 0, reinterpret_cast<IMS_UINTP>(pParam));
                MessageService::PostMessage(aStrTargetActivity, objMSG);
                IMS_TRACE_D("OnMessage :: Retry ConnectSC, nSmsFormat:%d, nStatus:%d, mSlotID:%d", nSmsFormat, nStatus, mSlotID);
            }
        }
        break;
        case IUSmsService::NOTI_SMS_SERVER_NOT_READY:
        {
            isSmsServerReady = IMS_FALSE;
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

PUBLIC VIRTUAL
IMS_BOOL AndroidJavaWMS::Control( IN IMS_UINT32 /*nCmdType*/,
    IN IMS_UINTP /*nInParam*/, OUT IMS_UINTP * /*pnOutParam*/ )
{
    return IMS_FALSE;
}

PUBLIC
IMS_SINT32 AndroidJavaWMS::GetMsgID(IN CONST IMS_BYTE* smsdata, IN CONST IMS_UINT32 smsformat)
{
    IMS_SINT32 nMsgID = (-1);

    if (smsdata == IMS_NULL)
    {
        return (-1);
    }

    if (IMtsClient::SMSFORMAT_3GPP == smsformat)
    {
        nMsgID = smsdata[1];
        IMS_TRACE_I("GetMsgID :: MR of Gsm SMS bin = %d", nMsgID, 0, 0);
        return (IMS_SINT32)nMsgID;
    }
    else
    {
        IMS_TRACE_D("GetMsgID :: Unsupported SMS Format (%d)", smsformat, 0, 0);
        return (-1);
    }
}

PUBLIC
void AndroidJavaWMS::ReleaseWMSClient()
{
    IMS_TRACE_I("ReleaseWMSClient :: client_handle=%d", nClientHandle, 0, 0);

    isWmsConnected = IMS_FALSE;
    SetTransportID(WMS_CONNECTIONID_INVALID);
}
