#define IMS_STL_USE
#include "ServiceTrace.h"
#include "ImsProcess.h"
#include "EnablerUtils.h"

#include "IuMtcService.h"
#include "JniConnectorFactory.h"
#include "JniMtcService.h"
#include "JniMtcServiceThread.h"

__IMS_TRACE_TAG_USER_DECL__("JNI.MTC");

JniMtcService::JniMtcService(IN CBServiceNoti pfnNotifier, IN IMS_SINT32 nSlotId) :
        m_pThread(IMS_NULL),
        m_nSlotId(nSlotId),
        m_piMtcService(IMS_NULL)
{
    IMS_TRACE_D("+JniMtcService SlotId[%d]", m_nSlotId, 0, 0);

    Initialize(pfnNotifier);
}

JniMtcService::~JniMtcService()
{
    IMS_TRACE_D("~JniMtcService", 0, 0, 0);

    if (m_pThread != IMS_NULL)
    {
        ImsProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pThread = IMS_NULL;
    }

    if (m_piMtcService)
    {
        m_piMtcService->SetJniService(IMS_NULL);
    }
}

PUBLIC VIRTUAL int JniMtcService::SendData(IN const android::Parcel& objParcel)
{
    int nMsg = objParcel.readInt32();

    if (IsThreadSwitchingRequired(nMsg))
    {
        SendDataUsingEnablerThread(objParcel, m_nSlotId);
    }
    else
    {
        HandleMessage(nMsg, objParcel);
    }

    return 1;
}

PUBLIC
void JniMtcService::Initialize(IN CBServiceNoti pfnNotifier)
{
    if (pfnNotifier == IMS_NULL)
    {
        return;
    }
    m_strThreadName.Sprintf("JniMtcServiceThread_%08" PFLS_x, reinterpret_cast<IMS_SINTP>(this));

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniMtcServiceThread();
    };
    ImsProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry, m_nSlotId);
    m_pThread = (JniMtcServiceThread*)(ImsProcess::GetInstance()->GetThread(m_strThreadName));

    if (m_pThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniMtcService : can't create listener thread", 0, 0, 0);
        return;
    }
    IMS_TRACE_D("Initialize()", 0, 0, 0);
    m_pThread->SetSlotId(m_nSlotId);
    m_pThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pfnNotifier);
    Attach();
}

PUBLIC
void JniMtcService::SetMtcService(IN IMtcService* piMtcService)
{
    m_piMtcService = piMtcService;
}

PUBLIC
JniMtcServiceThread* JniMtcService::GetThread()
{
    return m_pThread;
}

PROTECTED VIRTUAL void JniMtcService::HandleMessage(
        IN IMS_SINT32 nMsg, IN const android::Parcel& objParcel)
{
    IMS_TRACE_D("HandleServiceMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IuMtcService::SRVCC_STATE_CHANGED:
            NotifySrvccStateChanged(objParcel);
            break;
        case IuMtcService::SET_TERMINAL_BASED_CALL_WAITING:
            SetTerminalBasedCallWaiting(objParcel);
            break;
        default:
            break;
    }
}

PRIVATE
void JniMtcService::Attach()
{
    if (m_piMtcService)
    {
        IMS_TRACE_E(0, "duplicated Attaching", 0, 0, 0);
        return;
    }

    m_piMtcService = JniConnectorFactory::GetInstance()
                             ->GetMtcServiceConnector(m_nSlotId)
                             ->GetEnablerService();
    if (m_piMtcService)
    {
        m_piMtcService->SetJniService(this);
    }
}

PRIVATE
void JniMtcService::NotifySrvccStateChanged(IN const android::Parcel& objParcel)
{
    m_piMtcService->UpdateSrvccState(static_cast<SrvccState>(objParcel.readInt32()));
}

PRIVATE
void JniMtcService::SetTerminalBasedCallWaiting(IN const android::Parcel& objParcel)
{
    IMS_BOOL bProvisioned = (objParcel.readInt32() == 1) ? IMS_TRUE : IMS_FALSE;
    IMS_BOOL bEnabled = (objParcel.readInt32() == 1) ? IMS_TRUE : IMS_FALSE;
    m_piMtcService->SetTerminalBasedCallWaiting(bProvisioned, bEnabled);
}
