#include <utils/String8.h>

#define IMS_STL_USE

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "ImsProcess.h"
#include "IMtsService.h"
#include "IuMts.h"
#include "IuMtsService.h"
#include "OsMutex.h"
#include "JniConnectorFactory.h"
#include "JniMtsService.h"
#include "JniMtsServiceThread.h"
#include "EnablerUtils.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.MTS");

JniMtsService::JniMtsService(IN Jni_SendDataToJava pfnSendDataToJava, IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_strThreadName(AString::ConstNull()),
        m_piMtsService(IMS_NULL),
        m_pJniMtsServiceThread(IMS_NULL)
{
    IMS_TRACE_D("+JniMtsService SlotId[%d]", m_nSlotId, 0, 0);

    Initialize(pfnSendDataToJava);
}

JniMtsService::~JniMtsService()
{
    IMS_TRACE_D("~JniMtsService SlotId[%d]", m_nSlotId, 0, 0);

    if (m_piMtsService)
    {
        m_piMtsService->SetJniMtsService(IMS_NULL);
    }

    if (m_pJniMtsServiceThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pJniMtsServiceThread = IMS_NULL;
    }
}

PUBLIC VIRTUAL
int JniMtsService::SendData(const Parcel& objParcel)
{
    int nMessage = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMessage))
    {
        SendDataUsingEnablerThread(objParcel, m_nSlotId);
    }
    else
    {
        HandleMessage(nMessage, objParcel);
    }

    return 1;
}

PUBLIC
void JniMtsService::SetMtsService(IN IMtsService* piMtsService)
{
    IMS_TRACE_D("SetMtsService()", 0, 0, 0);
    m_piMtsService = piMtsService;
}

PUBLIC
JniMtsServiceThread* JniMtsService::GetThread() const
{
    return m_pJniMtsServiceThread;
}

PROTECTED VIRTUAL void
JniMtsService::HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IuMtsService::NOTI_MTSENABLER_SEND_MO_SMS:
            TriggerSendMoSms(objParcel);
            break;

        case IuMtsService::NOTI_MTSENABLER_SEND_MT_RESULT:
            NotifyMtResult(objParcel);
            break;

        default:
            break;
    }
}

PRIVATE
IMS_BOOL JniMtsService::Attach()
{
    IMS_BOOL bIsAttached = IMS_FALSE;

    if (m_piMtsService)
    {
        IMS_TRACE_D("Attach()::Attached", 0, 0, 0);
        return IMS_TRUE;
    }

    m_piMtsService = JniConnectorFactory::GetInstance()->GetMtsServiceConnector(m_nSlotId)
            ->GetEnablerService();
    if (m_piMtsService)
    {
        m_piMtsService->SetJniMtsService(this);
        bIsAttached = IMS_TRUE;
    }
    else
    {
        JniConnectorFactory::GetInstance()->GetMtsServiceConnector(m_nSlotId)->SetJniService(this);
    }

    IMS_TRACE_I("Attach() :: %s", _TRACE_B_(bIsAttached), 0, 0);
    return bIsAttached;
}

PRIVATE
void JniMtsService::Initialize(IN Jni_SendDataToJava pfnSendDataToJava)
{
    if (pfnSendDataToJava == IMS_NULL)
    {
        return;
    }

    m_strThreadName.Sprintf("JniMtsServiceThread_%d", m_nSlotId);

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread * { return new JniMtsServiceThread(); };

    ImsProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry, m_nSlotId);
    m_pJniMtsServiceThread =
            (JniMtsServiceThread*)(ImsProcess::GetInstance()->GetThread(m_strThreadName));

    if (m_pJniMtsServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtsService : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniMtsServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnSendDataToJava);
    Attach();
}

PRIVATE
void JniMtsService::TriggerSendMoSms(IN const Parcel& objParcel)
{
    IMS_UINT32 nSmsFormat_ = objParcel.readInt32();
    android::String8 strEncodedData(objParcel.readString16());
    android::String8 strAddress_(objParcel.readString16());
    IMS_SINT32 nSeqId = objParcel.readInt32();
    AString strData = AString::FromBase64(strEncodedData.string());
    ByteArray objData(reinterpret_cast<const IMS_BYTE*>(strData.GetStr()),
            static_cast<IMS_SINT32>(strData.GetLength()));
    AString strAddress = strAddress_.string();

    SmsFormatType eSmsFormat;
    if (nSmsFormat_ == (IMS_UINT32)SmsFormatType::SMSFORMAT_3GPP)
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_3GPP;
    }
    else if (nSmsFormat_ == (IMS_UINT32)SmsFormatType::SMSFORMAT_3GPP2)
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_3GPP2;
    }
    else
    {
        eSmsFormat = SmsFormatType::SMSFORMAT_INVALID;
    }

    if (m_piMtsService == IMS_NULL)
    {
        Attach();

        if (m_piMtsService == IMS_NULL)
        {
            IMS_TRACE_D("MtsEnabler is not bound.", 0, 0, 0);
            m_pJniMtsServiceThread->ReportMoStatus(
                    MO_IMS_TEMP_FAILURE, eSmsFormat, 0, nSeqId, m_nSlotId);
            return;
        }
    }

    m_piMtsService->SendMoSms(eSmsFormat, objData, strAddress, nSeqId);
}

PRIVATE
void JniMtsService::NotifyMtResult(IN const Parcel& objParcel)
{
    IMS_SINT32 nMtResult = objParcel.readInt32();
    IMS_TRACE_I("MT result = (%d)", nMtResult, 0, 0);

    if (m_piMtsService == IMS_NULL)
    {
        Attach();

        if (m_piMtsService == IMS_NULL)
        {
            // TODO: error handling is needed when call back is added
            IMS_TRACE_D("MtsEnabler is not bound.", 0, 0, 0);
            return;
        }
    }

    m_piMtsService->SendMtResult(nMtResult);
}
