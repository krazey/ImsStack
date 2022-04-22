/*
    Author
    <table>
    date              author                        description
    --------      --------------            ----------
    20140530      hoonsang.yun@           Created
    </table>

    Description
    SMSPhone JNI.
*/

#include <utils/String8.h>

#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "IMSProcess.h"

#include "IUSMS.h"
#include "IUSmsService.h"
#include "OsMutex.h"
#include "JNISmsService.h"
#include "JNISmsServiceThread.h"
#include "EnablerUtils.h"

#define EAB_SOLUTION_SERVICE_ID_LEN    128

__IMS_TRACE_TAG_USER_DECL__("JNISmsService");

extern void androidJavaWms_SetJniSmsService(IN CBJniSmsService pCB);

class JNISmsServicePrivate
{
public:
    inline JNISmsServicePrivate()
        : nMtResult(0)
        , bConditionAwaked(IMS_FALSE)
    {
        pthread_cond_init(&stSignal, IMS_NULL);
    };

    inline virtual ~JNISmsServicePrivate()
    {
        pthread_cond_destroy(&stSignal);
    };

    inline static JNISmsServicePrivate* GetInstance()
    {
        static JNISmsServicePrivate *pPrivate = IMS_NULL;

        if (pPrivate == IMS_NULL)
        {
            pPrivate = new JNISmsServicePrivate();
        }

        return pPrivate;
    };

    inline void AwakeCondition()
    {
        IMS_TRACE_I("AwakeCondition", 0, 0, 0);

        objMutex4Signal.Lock();
        bConditionAwaked = IMS_TRUE;
        pthread_cond_signal(&stSignal);
        objMutex4Signal.Unlock();
    };

    inline IMS_SINT32 GetMtResult()
    {
        return nMtResult;
    };

    inline void SetMtResult(IN IMS_SINT32 nResult)
    {
        nMtResult = nResult;
    };

    inline int WaitCondition()
    {
        IMS_SINT32 nWaitResult = -1;

        objMutex4Signal.Lock();

        if (bConditionAwaked)
        {
            nWaitResult = 1;
            IMS_TRACE_I("WaitCondition :: condition is already awaked", 0, 0, 0);
        }
        else
        {
            nWaitResult = pthread_cond_wait(&stSignal,
                reinterpret_cast<pthread_mutex_t*>(objMutex4Signal.GetMutexObj()));
        }

        bConditionAwaked = IMS_FALSE;

        objMutex4Signal.Unlock();

        return nMtResult;
    };

private:
    IMS_SINT32 nMtResult;
    pthread_cond_t stSignal;
    OsMutex objMutex4Signal;
    IMS_BOOL bConditionAwaked;
};

static int jniSmsService_ReportSMS(IN IMS_UINT32 nType, IN IMS_UINTP pParam, IN IMS_UINT32 nSimSlot)
{
    IMS_TRACE_I("jniSmsService_ReportSMS : nType (%d), slotid : (%d)", nType, nSimSlot, 0);

    IMS_SINT32 nResult = -1;

    if (nType == IUSmsService::REPORT_MTS_MT_SMS)
    {
        IMSMSG objMsg(nType, 0, pParam);
        MessageService::PostMessage(STR_SMS_SVC_THREAD_NAME[nSimSlot], objMsg);

        return JNISmsServicePrivate::GetInstance()->WaitCondition();
    }

    return nResult;
}

PUBLIC
JNISmsService::JNISmsService(IN IMS_SINT32 nSimSlot /*= 0*/)
    : pJNISmsServiceThread(IMS_NULL)
    , m_nSlotID(nSimSlot)
{
    m_aStrTargetActivity = EnablerUtils::GetEnablerThreadName(nSimSlot);
    m_aStrTargetActivity.Append(".MtsApp");
    IMS_TRACE_D("+JNISMSService [%s]", m_aStrTargetActivity.GetStr(), 0, 0);
}

PUBLIC
JNISmsService::JNISmsService(CBServiceNoti pCBServiceNoti, IN IMS_SINT32 nSimSlot /*= 0*/)
    : pJNISmsServiceThread(IMS_NULL)
    , m_nSlotID(nSimSlot)
{
    IMS_TRACE_I("JNISmsService : simslot[%d]", nSimSlot, 0, 0);

    if (pCBServiceNoti != IMS_NULL)
    {
        strThreadName.Sprintf("%s", STR_SMS_SVC_THREAD_NAME[nSimSlot]);

        IMSProcess::GetInstance()->LoadAppThread(strThreadName, JNISmsServiceThread::GetInstance);

        pJNISmsServiceThread = (JNISmsServiceThread *)(IMSProcess::GetInstance()->GetApplicationThread(strThreadName));

        if (pJNISmsServiceThread != IMS_NULL)
        {
            pJNISmsServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pCBServiceNoti, nSimSlot);
        }
        else
        {
            IMS_TRACE_E(0, "can't create listener thread", 0, 0, 0);
        }

        JNISmsServicePrivate::GetInstance();
        androidJavaWms_SetJniSmsService(jniSmsService_ReportSMS);
    }
}

PUBLIC VIRTUAL
JNISmsService::~JNISmsService()
{
    IMS_TRACE_I("~JNISmsService :", 0, 0, 0);

    if (pJNISmsServiceThread != IMS_NULL)
    {
        IMSProcess::GetInstance()->UnloadAppThread(strThreadName);
        pJNISmsServiceThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL
int JNISmsService::SendData(const Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    HandleMessage(nMsg, pParcel);

    return 1;
}

PRIVATE
void JNISmsService::HandleMessage(int nMsg, const Parcel& pParcel)
{
    IMS_TRACE_I("HandleMessage :: msg = %d", nMsg, 0, 0);

    switch (nMsg)
    {
        case IUSmsService::NOTI_SMSSERVICE_SEND_MO_SMS:
            {
                IUSmsServiceSendMoSMSParam *pMoSms = new IUSmsServiceSendMoSMSParam();

                if(pMoSms != IMS_NULL)
                {
                    pMoSms->nSmsFormat = pParcel.readInt32();
                    android::String8 astrEncodedData(pParcel.readString16());
                    android::String8 astrAddress(pParcel.readString16());
                    pMoSms->nSeqId = pParcel.readInt32();
                    AString strData = AString::FromBase64(astrEncodedData.string());
                    pMoSms->objData.Append(
                        reinterpret_cast<const IMS_BYTE*>(strData.GetStr()),
                        static_cast<IMS_SINT32>(strData.GetLength()));
                    pMoSms->strAddr = astrAddress.string();

                    IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pMoSms));
                    AString aStrActivity = EnablerUtils::GetEnablerThreadName(m_nSlotID);
                    aStrActivity.Append(".AndroidJavaWms");
                    IMS_TRACE_I("m_aStrTargetActivity [%s]", aStrActivity.GetStr(), 0, 0);
                    MessageService::PostMessage(aStrActivity, objMSG);
                }
                else
                {
                    IMS_TRACE_E(0,"NOTI_SMSSERVICE_SEND_MO_SMS is failed", 0, 0, 0);
                }
            }
            break;

        case IUSmsService::NOTI_IMS_PHONE_RESTARTED:
        case IUSmsService::NOTI_SMS_SERVER_READY: // FALL-THROUGH
        case IUSmsService::NOTI_SMS_SERVER_NOT_READY: // FALL-THROUGH
            {
                IMSMSG objMSG(nMsg, 0, 0);
                AString aStrActivity = EnablerUtils::GetEnablerThreadName(m_nSlotID);
                aStrActivity.Append(".AndroidJavaWms");
                IMS_TRACE_I("m_aStrTargetActivity [%s]", aStrActivity.GetStr(), 0, 0);
                MessageService::PostMessage(aStrActivity, objMSG);
            }
            break;

        case IUSmsService::NOTI_SMSSERVICE_SEND_MT_RESULT:
            {
                IMS_SINT32 nMtResult = pParcel.readInt32();

                JNISmsServicePrivate::GetInstance()->SetMtResult(nMtResult);
                IMS_TRACE_I("MT result = (%d)", nMtResult, 0, 0);
                JNISmsServicePrivate::GetInstance()->AwakeCondition();
            }
            break;

        default:
            {
                IMS_TRACE_E(0, "unknown message : %d", nMsg, 0, 0);
            }
            break;
    }
}
