#include <utils/String8.h>

#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "IMSProcess.h"

#include "IUSMS.h"
#include "IUSmsSCBMService.h"
#include "OsMutex.h"
#include "JNISmsSCBMService.h"
#include "JNISmsSCBMServiceThread.h"
#include "EnablerUtils.h"

#include "AndroidJavaSCBM.h"

__IMS_TRACE_TAG_USER_DECL__("JNISmsSCBMService");

extern void androidJavaSCBM_SetJniSmsService(IN CBJniSmsSCBMService pCB);

class JNISmsSCBMServicePrivate
{
public:
    inline JNISmsSCBMServicePrivate()
        : bConditionAwaked(IMS_FALSE)
    {
        if (pthread_cond_init(&stSignal, IMS_NULL) == 0)
        {
            // OK
        }
    };

    inline virtual ~JNISmsSCBMServicePrivate()
    {
        if (pthread_cond_destroy(&stSignal) == 0)
        {
            // OK
        }
    };

    inline static JNISmsSCBMServicePrivate* GetInstance()
    {
        static JNISmsSCBMServicePrivate *pPrivate = IMS_NULL;

        if (pPrivate == IMS_NULL)
        {
            pPrivate = new JNISmsSCBMServicePrivate();
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

        if (nWaitResult == 0)
        {
            // no_op
        }

        objMutex4Signal.Unlock();

        return bConditionAwaked;
    };

private:
    pthread_cond_t stSignal;
    OsMutex objMutex4Signal;
    IMS_BOOL bConditionAwaked;
};

static int jniSmsSCBMService_ReportSCBM(IN IMS_UINT32 nType, IN IMS_UINTP pParam, IN IMS_UINT32 nSimSlot)
{
    IMS_TRACE_I("jniSmsSCBMService_ReportSMSSCBM : nType (%d)", nType, 0, 0);

    IMS_SINT32 nResult = -1;

    if (nType == IUSmsSCBMService::REQUEST_SMS_RAT_SELECTION)
    {
        IMSMSG objMsg(nType, 0, pParam);
        MSGService::PostMessage(STR_SMS_SCBM_SVC_THREAD_NAME[nSimSlot], objMsg);

        return JNISmsSCBMServicePrivate::GetInstance()->WaitCondition();
    }
    else if (nType == IUSmsSCBMService::REQUEST_SMS_EXIT_RAT_SELECTION)
    {
        IMSMSG objMsg(nType, 0, pParam);
        MSGService::PostMessage(STR_SMS_SCBM_SVC_THREAD_NAME[nSimSlot], objMsg);
    }

    return nResult;
}

PUBLIC
JNISmsSCBMService::JNISmsSCBMService(IN IMS_SINT32 nSimSlot /*= 0*/)
    : pJNISmsSCBMServiceThread(IMS_NULL)
    , m_nSlotID(nSimSlot)
{
    m_aStrTargetActivity = EnablerUtils::GetEnablerThreadName(m_nSlotID);
    m_aStrTargetActivity.Append(".AndroidJavaSCBM");
    IMS_TRACE_D("+JNISmsSCBMService [%s]", m_aStrTargetActivity.GetStr(), 0, 0);
}

PUBLIC
JNISmsSCBMService::JNISmsSCBMService(CBServiceNoti pCBServiceNoti, IN IMS_SINT32 nSimSlot /*= 0*/)
    : pJNISmsSCBMServiceThread(IMS_NULL)
    , m_nSlotID(nSimSlot)
{
    m_aStrTargetActivity = EnablerUtils::GetEnablerThreadName(m_nSlotID);
    m_aStrTargetActivity.Append(".AndroidJavaSCBM");
    IMS_TRACE_D("+JNISmsSCBMService [%s]", m_aStrTargetActivity.GetStr(), 0, 0);

    if (pCBServiceNoti != IMS_NULL)
    {
        strThreadName.Sprintf("%s", STR_SMS_SCBM_SVC_THREAD_NAME[nSimSlot]);

        IMSProcess::GetInstance()->LoadAppThread(strThreadName, JNISmsSCBMServiceThread::GetInstance);

        pJNISmsSCBMServiceThread = (JNISmsSCBMServiceThread *)(IMSProcess::GetInstance()->GetApplicationThread(strThreadName));

        if (pJNISmsSCBMServiceThread != IMS_NULL)
        {
            pJNISmsSCBMServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pCBServiceNoti, nSimSlot);
        }
        else
        {
            IMS_TRACE_E(0, "can't create listener thread", 0, 0, 0);
        }

        JNISmsSCBMServicePrivate::GetInstance();
        androidJavaSCBM_SetJniSmsService(jniSmsSCBMService_ReportSCBM);
    }
}

PUBLIC VIRTUAL
JNISmsSCBMService::~JNISmsSCBMService()
{
    IMS_TRACE_I("~JNISmsSCBMService :", 0, 0, 0);

    if (pJNISmsSCBMServiceThread != IMS_NULL)
    {
        IMSProcess::GetInstance()->UnloadAppThread(strThreadName);
        pJNISmsSCBMServiceThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL
int JNISmsSCBMService::SendData(const Parcel& pParcel)
{
    int nMsg = pParcel.readInt32();

    HandleMessage(nMsg, pParcel);

    return 1;
}

PRIVATE
void JNISmsSCBMService::HandleMessage(int nMsg, const Parcel& pParcel)
{
    IMS_TRACE_I("JNISmsSCBMService:: HandleMessage :: msg = %d", nMsg, 0, 0);

    switch (nMsg)
    {
        case IUSmsSCBMService::NOTI_SMS_RAT_SELECTION:
            {
                JNISmsSCBMServicePrivate::GetInstance()->AwakeCondition();

                IUSmsServiceUpdateSCBMParam *pResultRATSelection = new IUSmsServiceUpdateSCBMParam();

                if(pResultRATSelection != IMS_NULL)
                {
                    pResultRATSelection->nScbmMode = pParcel.readInt32();

                    IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pResultRATSelection));
                    IMS_TRACE_I("m_aStrTargetActivity [%s]", m_aStrTargetActivity.GetStr(), 0, 0);
                    MSGService::PostMessage(m_aStrTargetActivity, objMSG);
                }
                else
                {
                    IMS_TRACE_E(0,"NOTI_SMS_RAT_SELECTION is failed", 0, 0, 0);
                }
            }
        break;
        case IUSmsSCBMService::NOTI_EXIT_SCBM:
            {
                IUSmsServiceUpdateSCBMParam *pExitSCBM = new IUSmsServiceUpdateSCBMParam();
                if(pExitSCBM != IMS_NULL)
                {
                    pExitSCBM->nScbmMode = pParcel.readInt32();

                    IMSMSG objMSG(nMsg, 0, reinterpret_cast<IMS_UINTP>(pExitSCBM));
                    IMS_TRACE_I("m_aStrTargetActivity [%s]", m_aStrTargetActivity.GetStr(), 0, 0);
                    MSGService::PostMessage(m_aStrTargetActivity, objMSG);
                }
                else
                {
                    IMS_TRACE_E(0,"NOTI_EXIT_SCBM is failed", 0, 0, 0);
                }
            }
            break;
        case IUSmsSCBMService::NOTI_IMS_PHONE_RESTARTED:
        case IUSmsSCBMService::NOTI_SMS_SCBM_SERVER_READY: // FALL-THROUGH
        case IUSmsSCBMService::NOTI_SMS_SCBM_SERVER_NOT_READY: // FALL-THROUGH
            {
                IMSMSG objMSG(nMsg, 0, 0);
                IMS_TRACE_I("m_aStrTargetActivity [%s]", m_aStrTargetActivity.GetStr(), 0, 0);
                MSGService::PostMessage(m_aStrTargetActivity, objMSG);
            }
            break;
        default:
            {
                IMS_TRACE_E(0, "unknown message : %d", nMsg, 0, 0);
            }
            break;
    }
}
