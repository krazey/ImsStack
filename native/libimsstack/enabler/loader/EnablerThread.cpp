/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20170320  hwangoo.park@             Created
    </table>

    Description

*/
#include "DeviceConfig.h"
#include "ServiceConfig.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "ServiceEvent.h"
#include "SystemConfigManager.h"
#include "Configuration.h"
#include "GeolocationHelper.h"
#include "EngineLoader.h"
#include "EnablerFactory.h"
#include "EnablerThread.h"

__IMS_TRACE_TAG_USER_DECL__("EnablerThread");

PUBLIC
EnablerThread::EnablerThread(IN EnablerFactory *pEnablerFactory_, IN IMS_SINT32 nSlotId_)
    : IMSAppThread()
    , pEnablerFactory(pEnablerFactory_)
    , nSlotId(nSlotId_)
    , nState(STATE_INACTIVE)
{
}

PUBLIC VIRTUAL
EnablerThread::~EnablerThread()
{
}

PUBLIC
IMS_SINT32 EnablerThread::GetSlotId() const
{
    return nSlotId;
}

PUBLIC
void EnablerThread::ControlEnablers(IN IMS_SINT32 nCtrlFlags)
{
    IMS_MSG_CreateNPostThreadMessage(GetThread(), TMSG_CONTROL_ENABLERS, nCtrlFlags, 0);
}

PROTECTED VIRTUAL
IMS_BOOL EnablerThread::Initialize()
{
    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL EnablerThread::OnStart(IN IMSMSG &objMSG)
{
    IMS_TRACE_D("OnStart :: slotId=%d, %s",
            GetSlotId(), DeviceConfig::ToString().GetStr(), 0);

    IMSAppThread::OnStart(objMSG);

    IMS_BOOL bInitOnStart = IMS_FALSE;
    const SystemConfig* pSC = SystemConfigManager::GetInstance()->GetConfig(GetSlotId());

    if ((pSC != IMS_NULL) && (pSC->GetOperator().GetLength() > 0))
    {
        if (!SystemConfig::IsMultiSimEnabled())
        {
            bInitOnStart = IMS_TRUE;
        }
        else if (SystemConfig::IsMultiImsEnabled() || SystemConfig::IsMultiImsEnabledOnDssv())
        {
            bInitOnStart = IMS_TRUE;
        }
        else if (pSC->IsDds())
        {
            bInitOnStart = IMS_TRUE;
        }
    }

    ConfigService::GetConfigService()->LoadCarrierConfig(GetSlotId());

    if (bInitOnStart)
    {
        SystemConfigManager::CacheSystemFeatures();
        Configuration::GetInstance()->InitConfigs(GetSlotId());
        EngineLoader::Initialize(GetSlotId());
        InitializeGlobals();

        pEnablerFactory->CreateEnablers(GetSlotId());

        if (StartEnablers())
        {
            SetState(STATE_ACTIVE);
            NotifyEnablerStartCompleted();
        }
    }

    return IMS_TRUE;
}

PROTECTED VIRTUAL
IMS_BOOL EnablerThread::OnTerminate(IN IMSMSG &objMSG)
{
    IMS_TRACE_D("OnTerminate :: slotId=%d", GetSlotId(), 0, 0);

    if (GetState() == STATE_ACTIVE)
    {
        StopEnablers();
        SetState(STATE_INACTIVE);
    }

    pEnablerFactory->DestroyEnablers(GetSlotId());

    UninitializeGlobals();
    EngineLoader::Uninitialize(GetSlotId());

    return IMSAppThread::OnTerminate(objMSG);
}

PROTECTED VIRTUAL
IMS_BOOL EnablerThread::OnMessage(IN IMSMSG &objMSG)
{
    switch (objMSG.GetName())
    {
    case TMSG_CONTROL_ENABLERS:
        ControlEnablersInternal(LONG_TO_INT(objMSG.nWparam));
        return IMS_TRUE;

    default:
        return IMSAppThread::OnMessage(objMSG);
    }
}

PROTECTED
void EnablerThread::InitializeGlobals()
{
    GeolocationHelper::GetInstance()->CreatePidfCreator(GetSlotId());
}

PROTECTED
void EnablerThread::UninitializeGlobals()
{
    GeolocationHelper::GetInstance()->DestroyPidfCreator(GetSlotId());
}

PROTECTED
void EnablerThread::ControlEnablersInternal(IN IMS_SINT32 nCtrlFlags)
{
    IMS_TRACE_D("ControlEnablersInternal :: ctrlFlags=%08X", nCtrlFlags, 0, 0);

    if (IsControlSet(nCtrlFlags, CONTROL_STOP))
    {
        if (GetState() == STATE_ACTIVE)
        {
            StopEnablers();
            SetState(STATE_INACTIVE);
        }
    }

    if (IsControlSet(nCtrlFlags, CONTROL_DESTROY))
    {
        pEnablerFactory->DestroyEnablers(GetSlotId());

        UninitializeGlobals();
        EngineLoader::Uninitialize(GetSlotId());
    }

    if (IsControlSet(nCtrlFlags, CONTROL_CREATE) || IsControlSet(nCtrlFlags, CONTROL_START))
    {
        EventService::GetEventService()->SetUnregisteredEvents(GetSlotId());
    }

    if (IsControlSet(nCtrlFlags, CONTROL_CREATE))
    {
        // For hot swap, the system features will be re-calculated
        // when re-starting the enablers.
        if (nSlotId == IMS_SLOT_0)
        {
            SystemConfigManager::CacheSystemFeatures();
        }

        Configuration::GetInstance()->RefreshConfigs(GetSlotId());
        EngineLoader::Initialize(GetSlotId());
        InitializeGlobals();

        pEnablerFactory->CreateEnablers(GetSlotId());
    }

    if (IsControlSet(nCtrlFlags, CONTROL_START))
    {
        if (StartEnablers())
        {
            SetState(STATE_ACTIVE);
            NotifyEnablerStartCompleted();
        }
    }
}

PROTECTED
void EnablerThread::NotifyEnablerStartCompleted()
{
    IMS_EVENT_SendEventForSlotId(IMS_EVENT_NATIVE_BOOT_COMPLETED, 0, 0, GetSlotId());
}

PROTECTED
void EnablerThread::SetState(IN IMS_SINT32 nState)
{
    if (this->nState != nState)
    {
        IMS_TRACE_I("ET%02d :: %d >> %d", GetSlotId(), this->nState, nState);
        this->nState = nState;
    }
}

PROTECTED
IMS_BOOL EnablerThread::StartEnablers()
{
    const IMSList<IEnabler*>* pEnablers = pEnablerFactory->GetEnablers(GetSlotId());

    if (pEnablers == IMS_NULL)
    {
        IMS_TRACE_E(0, "No enablers in slot-%d", GetSlotId(), 0, 0);
        return IMS_FALSE;
    }

    IMS_BOOL bStarted = IMS_FALSE;

    IMS_TRACE_I("StartEnablers :: size=%d", pEnablers->GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < pEnablers->GetSize(); i++)
    {
        IEnabler *piEnabler = pEnablers->GetAt(i);

        if (piEnabler != IMS_NULL)
        {
            bStarted = IMS_TRUE;
            piEnabler->Start();
        }
    }

    return bStarted;
}

PROTECTED
void EnablerThread::StopEnablers()
{
    const IMSList<IEnabler*>* pEnablers = pEnablerFactory->GetEnablers(GetSlotId());

    if (pEnablers == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("StopEnablers :: size=%d", pEnablers->GetSize(), 0, 0);

    if (pEnablers->IsEmpty())
    {
        return;
    }

    IMS_SINT32 i = static_cast<IMS_SINT32>(pEnablers->GetSize() - 1);

    for ( ; i >= 0; i--)
    {
        IEnabler *piEnabler = pEnablers->GetAt(i);

        if (piEnabler != IMS_NULL)
        {
            piEnabler->Stop();
        }
    }
}
