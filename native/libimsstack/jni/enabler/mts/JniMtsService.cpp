#include <utils/String8.h>

#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "IMSProcess.h"

#include "IUMts.h"
#include "IUMtsService.h"
#include "OsMutex.h"
#include "JniMtsService.h"
#include "JniMtsServiceThread.h"
#include "EnablerUtils.h"

#define EAB_SOLUTION_SERVICE_ID_LEN    128

__IMS_TRACE_TAG_USER_DECL__("JniMtsService");

extern void androidJavaWms_SetJniMtsService(IN CBJniMtsService pCB);

class JniMtsServicePrivate
{
public:
    inline JniMtsServicePrivate() :
            nMtResult(0),
            bConditionAwaked(IMS_FALSE)
    {
        pthread_cond_init(&stSignal, IMS_NULL);
    }

    inline virtual ~JniMtsServicePrivate()
    {
        pthread_cond_destroy(&stSignal);
    }

    inline static JniMtsServicePrivate* GetInstance()
    {
        static JniMtsServicePrivate* pPrivate = IMS_NULL;

        if (pPrivate == IMS_NULL)
        {
            pPrivate = new JniMtsServicePrivate();
        }

        return pPrivate;
    }

    inline void AwakeCondition()
    {
        IMS_TRACE_I("AwakeCondition", 0, 0, 0);

        objMutex4Signal.Lock();
        bConditionAwaked = IMS_TRUE;
        pthread_cond_signal(&stSignal);
        objMutex4Signal.Unlock();
    }

    inline IMS_SINT32 GetMtResult()
    {
        return nMtResult;
    }

    inline void SetMtResult(IN IMS_SINT32 nResult)
    {
        nMtResult = nResult;
    }

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
            if (nWaitResult == 0)
            {
                // no_op
            }
        }

        bConditionAwaked = IMS_FALSE;

        objMutex4Signal.Unlock();

        return nMtResult;
    }

private:
    IMS_SINT32 nMtResult;
    pthread_cond_t stSignal;
    OsMutex objMutex4Signal;
    IMS_BOOL bConditionAwaked;
};

static int jniSmsService_ReportSMS(
        IN IMS_UINT32 nType, IN IMS_UINTP pParam, IN IMS_UINT32 nSlotId)
{
    IMS_TRACE_I("jniSmsService_ReportSMS : nType (%d), slotid : (%d)", nType, nSlotId, 0);

    IMS_SINT32 nResult = -1;

    if (nType == IUMtsService::REPORT_MTS_MT_SMS)
    {
        IMSMSG objMsg(nType, 0, pParam);
        MessageService::PostMessage(STR_MTS_SVC_THREAD_NAME[nSlotId], objMsg);

        return JniMtsServicePrivate::GetInstance()->WaitCondition();
    }

    return nResult;
}

PUBLIC
JniMtsService::JniMtsService(IN IMS_SINT32 nSlotId /*= 0*/) :
        m_pJniMtsServiceThread(IMS_NULL),
        m_nSlotId(nSlotId)
{
    m_strTargetActivity = EnablerUtils::GetEnablerThreadName(nSlotId);
    m_strTargetActivity.Append(".MtsApp");
    IMS_TRACE_D("+JNISMSService [%s]", m_strTargetActivity.GetStr(), 0, 0);
}

PUBLIC
JniMtsService::JniMtsService(IN CBServiceNoti pCBServiceNoti, IN IMS_SINT32 nSlotId /*= 0*/) :
        m_pJniMtsServiceThread(IMS_NULL),
        m_nSlotId(nSlotId)
{
    IMS_TRACE_I("JniMtsService : nSlotId[%d]", nSlotId, 0, 0);

    if (pCBServiceNoti == IMS_NULL)
    {
        IMS_TRACE_E(0,"JniMtsService:pCBServiceNoti is null", 0, 0, 0);
        return;
    }
    m_strThreadName.Sprintf("%s", STR_MTS_SVC_THREAD_NAME[nSlotId]);

    IMSProcess::GetInstance()->LoadAppThread(m_strThreadName, JniMtsServiceThread::GetInstance);

    m_pJniMtsServiceThread = (JniMtsServiceThread*)(IMSProcess::GetInstance()
            ->GetApplicationThread(m_strThreadName));

    if (m_pJniMtsServiceThread != IMS_NULL)
    {
        m_pJniMtsServiceThread
                ->SetCallback(reinterpret_cast<IMS_SINTP>(this), pCBServiceNoti, nSlotId);
    }
    else
    {
        IMS_TRACE_E(0, "can't create listener thread", 0, 0, 0);
    }
    JniMtsServicePrivate::GetInstance();
    androidJavaWms_SetJniMtsService(jniSmsService_ReportSMS);
}

PUBLIC VIRTUAL
JniMtsService::~JniMtsService()
{
    IMS_TRACE_I("~JniMtsService :", 0, 0, 0);

    if (m_pJniMtsServiceThread != IMS_NULL)
    {
        IMSProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pJniMtsServiceThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL
int JniMtsService::SendData(IN const Parcel& objParcel)
{
    int nMsg = objParcel.readInt32();

    HandleMessage(nMsg, objParcel);

    return 1;
}

PRIVATE
void JniMtsService::HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_I("HandleMessage :: msg = %d", nMsg, 0, 0);

    switch (nMsg)
    {
        case IUMtsService::NOTI_MTSENABLER_SEND_MO_SMS:
            {
                IUMtsServiceSendMoSmsParam* pMoSms = new IUMtsServiceSendMoSmsParam();
                pMoSms->nSmsFormat = objParcel.readInt32();
                android::String8 astrEncodedData(objParcel.readString16());
                android::String8 astrAddress(objParcel.readString16());
                pMoSms->nSeqId = objParcel.readInt32();
                AString strData = AString::FromBase64(astrEncodedData.string());
                pMoSms->objData.Append(
                        reinterpret_cast<const IMS_BYTE*>(strData.GetStr()),
                        static_cast<IMS_SINT32>(strData.GetLength()));
                pMoSms->strAddr = astrAddress.string();

                IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pMoSms));
                AString aStrActivity = EnablerUtils::GetEnablerThreadName(m_nSlotId);
                aStrActivity.Append(".AndroidJavaWms");
                IMS_TRACE_I("m_strTargetActivity [%s]", aStrActivity.GetStr(), 0, 0);
                MessageService::PostMessage(aStrActivity, objMSG);
            }
            break;

        case IUMtsService::NOTI_MTSENABLER_SEND_MT_RESULT:
            {
                IMS_SINT32 nMtResult = objParcel.readInt32();

                JniMtsServicePrivate::GetInstance()->SetMtResult(nMtResult);
                IMS_TRACE_I("MT result = (%d)", nMtResult, 0, 0);
                JniMtsServicePrivate::GetInstance()->AwakeCondition();
            }
            break;

        default:
            {
                IMS_TRACE_E(0, "unknown message : %d", nMsg, 0, 0);
            }
            break;
    }
}
