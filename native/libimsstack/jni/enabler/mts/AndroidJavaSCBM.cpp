#include <utils/String8.h>
#include <cutils/properties.h>

#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceThread.h"
#include "ServiceMessage.h"

#include "ByteArray.h"

#include "IUSMS.h"
#include "IUSmsSCBMService.h"

#include "AndroidJavaSCBM.h"
#include "EnablerUtils.h"

__IMS_TRACE_TAG_ADAPT__;

static CBJniSmsSCBMService gpCBJniSmsSCBMService = IMS_NULL;
static AndroidJavaSCBM* pAndroidJavaSCBM = IMS_NULL;

void androidJavaSCBM_SetJniSmsService(IN CBJniSmsSCBMService pCB)
{
    gpCBJniSmsSCBMService = pCB;
}

void androidJavaSCBM_sendMsgToJava(IN IMS_UINT32 nMsg, IMS_UINT32 pParam, IN IMS_SINT32 nSlotID)
{
    IMSMSG objUIMsg(nMsg, 0, pParam);
    IMS_UINT32 nSlotId = nSlotID;
    MessageService::PostMessage(STR_SMS_SCBM_SVC_THREAD_NAME[nSlotId], objUIMsg);
}

PRIVATE
AndroidJavaSCBM::AndroidJavaSCBM()
    : IMSActivityEx(AString("AndroidJavaSCBM"))
    , isSmsScbmServerReady(IMS_FALSE)
{
    IMS_TRACE_D("AndroidJavaSCBM() :: Name(%s)", GetName().GetStr(), 0, 0);
    return;
}

PRIVATE VIRTUAL
AndroidJavaSCBM::~AndroidJavaSCBM()
{
    IMS_TRACE_D("~AndroidJavaSCBM() :: Name(%s)", GetName().GetStr(), 0, 0);
}


PUBLIC GLOBAL
AndroidJavaSCBM* AndroidJavaSCBM::GetAndroidJavaSCBM()
{
    if (IMS_NULL == pAndroidJavaSCBM)
    {
        pAndroidJavaSCBM = new AndroidJavaSCBM();

        if (IMS_NULL == pAndroidJavaSCBM)
        {
            IMS_TRACE_D("AndroidJavaSCBM is failed", 0, 0, 0);
            return IMS_NULL;
        }
    }
    return pAndroidJavaSCBM;
}

PUBLIC GLOBAL
void AndroidJavaSCBM::DestroyAndroidJavaSCBM(IN IMS_SINT32 /*nSlotID*/)
{
    if(pAndroidJavaSCBM != IMS_NULL)
    {
        IMS_TRACE_D("DestroyAndroidJavaSCBM: pAndroidJavaSCBM: %" PFLS_d,
                pAndroidJavaSCBM, 0, 0); // will be deleted
        delete pAndroidJavaSCBM;
        pAndroidJavaSCBM = IMS_NULL;
    }
}

PUBLIC GLOBAL
IMS_BOOL AndroidJavaSCBM::StartUp()
{
    return IMS_TRUE;
}

PUBLIC GLOBAL
void AndroidJavaSCBM::CleanUp()
{
}

/*
PUBLIC VIRTUAL
IMS_RESULT AndroidJavaSCBM::Init()
{
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
IMS_RESULT AndroidJavaSCBM::Release()
{
    return IMS_SUCCESS;
}
*/

PUBLIC VIRTUAL
IMS_RESULT AndroidJavaSCBM::ConnectSCBM(IN IMS_SINT32 nSlotID)
{
    if (!isSmsScbmServerReady)
    {
        IMS_TRACE_I("ConnectSCBM :: Sms SCBM Server is not Ready", 0, 0, 0);
        return IMS_FAILURE;
    }

    IMS_TRACE_I("ConnectSCBM", 0, 0, 0);

    androidJavaSCBM_sendMsgToJava(IUSmsSCBMService::CREATE_SCBM_MANAGER, IMS_NULL, nSlotID);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
void AndroidJavaSCBM::DisconnectSCBM(IN IMS_SINT32 nSlotID)
{
    IMS_TRACE_I("SCBM DisconnectSC", 0, 0, 0);

    androidJavaSCBM_sendMsgToJava(IUSmsSCBMService::DESTROY_SCBM_MANAGER, 0, nSlotID);
    return;
}


PUBLIC VIRTUAL
void AndroidJavaSCBM::RequestRATSelection(IN IMS_UINT32 bIsSCBMMode)
{
    IMS_TRACE_I("RequestRATSelection :: SCBM Mode (%d)", bIsSCBMMode, 0, 0);

    IUSmsServiceUpdateSCBMParam *pSmsSCBMParam = new IUSmsServiceUpdateSCBMParam();
    if(pSmsSCBMParam == IMS_NULL)
    {
        IMS_TRACE_D("RequestRATSelection :: param creation is failed", 0, 0, 0);
        return;
    }

    pSmsSCBMParam->nScbmMode = bIsSCBMMode;

    if (gpCBJniSmsSCBMService != IMS_NULL)
    {
        IMS_TRACE_I("RequestRATSelection :: gpCBJniSmsService is Not Null!", 0, 0, 0);
        gpCBJniSmsSCBMService(IUSmsSCBMService::REQUEST_SMS_RAT_SELECTION,
                reinterpret_cast<IMS_UINTP>(pSmsSCBMParam), 0);
    } else {
        IMS_TRACE_I("RequestRATSelection :: gpCBJniSmsService is Null, stop SMS TO 911 processing",
                0, 0, 0);
        delete pSmsSCBMParam;
    }
}

PUBLIC VIRTUAL
void AndroidJavaSCBM::RequestExitRATSelection()
{
    IMS_TRACE_I("RequestExitRATSelection", 0, 0, 0);

    if (gpCBJniSmsSCBMService != IMS_NULL)
    {
        IMS_TRACE_D("RequestExitRATSelection :: gpCBJniSmsService is Not Null!", 0, 0, 0);
        gpCBJniSmsSCBMService(IUSmsSCBMService::REQUEST_SMS_EXIT_RAT_SELECTION, 0, 0);
    }
}


PUBLIC VIRTUAL
void AndroidJavaSCBM::RequestSetE911State()
{
    IMS_TRACE_I("RequestSetE911State===", 0, 0, 0);

    if (gpCBJniSmsSCBMService != IMS_NULL)
    {
        IMS_TRACE_D("RequestSetE911State :: gpCBJniSmsService is Not Null!", 0, 0, 0);
        gpCBJniSmsSCBMService(IUSmsSCBMService::REQUEST_SMS_SET_E911_STATE, 0, 0);
    }
}



PUBLIC VIRTUAL
IMS_BOOL AndroidJavaSCBM::OnMessage( IN IMSMSG &objMSG )
{
    IMS_TRACE_I( "AndroidJavaSCBM - OnMessage [%d]", objMSG.nMSG, 0, 0 );

    switch(objMSG.nMSG)
    {
        case IUSmsSCBMService::NOTI_IMS_PHONE_RESTARTED:
        {
            IMS_TRACE_D("OnMessage :: NOTI_IMS_PHONE_RESTARTED", 0, 0, 0);
        }
        break;
        case IUSmsSCBMService::NOTI_SMS_SCBM_SERVER_READY:
        {
            IMS_TRACE_D("OnMessage :: NOTI_SMS_SCBM_SERVER_READY", 0, 0, 0);
            isSmsScbmServerReady = IMS_TRUE;
            ConnectSCBM(0);
        }
        break;
        case IUSmsSCBMService::NOTI_SMS_SCBM_SERVER_NOT_READY:
        {
            IMS_TRACE_D("OnMessage :: NOTI_SMS_SCBM_SERVER_NOT_READY", 0, 0, 0);
            isSmsScbmServerReady = IMS_FALSE;
            DisconnectSCBM(0);
        }
        break;
        case IUSmsSCBMService::NOTI_SMS_RAT_SELECTION:
        {
            IMS_TRACE_D("OnMessage :: NOTI_SMS_RAT_SELECTION", 0, 0, 0);

            IUSmsServiceUpdateSCBMParam* pParam =
                    reinterpret_cast<IUSmsServiceUpdateSCBMParam*>(objMSG.nLparam);
            IMSMSG objMSG(SmsSvcInternal::SMSMO_RESULT_RAT_SELECTION,
                    0, reinterpret_cast<IMS_UINTP>(pParam));
            AString aStrTargetActivity = EnablerUtils::GetEnablerThreadName(0);
            aStrTargetActivity.Append(".MtsApp");
            MessageService::PostMessage(aStrTargetActivity, objMSG);
        }
        break;
        case IUSmsSCBMService::NOTI_EXIT_SCBM:
        {
            IMS_TRACE_D("OnMessage :: NOTI_EXIT_SCBM", 0, 0, 0);

            IUSmsServiceUpdateSCBMParam* pParam =
                    reinterpret_cast<IUSmsServiceUpdateSCBMParam*>(objMSG.nLparam);
            IMSMSG objMSG(SmsSvcInternal::SMS_REQUEST_EXIT_SCBM,
                    0, reinterpret_cast<IMS_UINTP>(pParam));
            AString aStrTargetActivity = EnablerUtils::GetEnablerThreadName(0);
            aStrTargetActivity.Append(".MtsApp");
            MessageService::PostMessage(aStrTargetActivity, objMSG);
        }
        break;

        default:
        {
            IMS_TRACE_D("OnMessage :: Not Handled Message(%d)", objMSG.nMSG, 0, 0);
        }
        break;
    }

    return IMS_TRUE;
}
