#define IMS_STL_USE

#include <utils/String8.h>
#include "ServiceTrace.h"
#include "IMSProcess.h"
#include "JniAosServiceThread.h"
#include "JniAosService.h"
#include "JniConnectorFactory.h"
#include "IIAosService.h"
#include "IAosService.h"

using namespace android;

__IMS_TRACE_TAG_USER_DECL__("JNI.AOS");

JniAosService::JniAosService(IN CBServiceNoti pCbServiceNoti, IN IMS_SINT32 nSlotId)
    : m_nSlotId(nSlotId)
    , m_strThreadName(AString::ConstNull())
    , m_piAosService(IMS_NULL)
    , m_pJniAosServiceThread(IMS_NULL)
{
    IMS_TRACE_D("+JniAosService SlotId[%d]", m_nSlotId, 0, 0);

    Initialize(pCbServiceNoti);
}

JniAosService::~JniAosService()
{
    IMS_TRACE_D("~JniAosService SlotId[%d]", m_nSlotId, 0, 0);

    if (m_piAosService)
    {
        m_piAosService->SetJniAosService(IMS_NULL);
    }

    if (m_pJniAosServiceThread != IMS_NULL)
    {
        IMSProcess::GetInstance()->UnloadAppThread(m_strThreadName);
        m_pJniAosServiceThread = IMS_NULL;
    }
}

PUBLIC
int JniAosService::SendData(const Parcel& objParcel)
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
void JniAosService::Initialize(IN CBServiceNoti pCbServiceNoti)
{
    if (pCbServiceNoti == NULL)
    {
        return;
    }

    m_strThreadName.Sprintf("JniAosServiceThread_%d", m_nSlotId);

    IMS_TRACE_D("Initialize()", 0, 0, 0);
    auto fnEntry = []() -> BaseThread*
    {
        return new JniAosServiceThread();
    };

    IMSProcess::GetInstance()->LoadThread(m_strThreadName, fnEntry);
    m_pJniAosServiceThread = (JniAosServiceThread*)(IMSProcess::GetInstance()->
            GetThread(m_strThreadName));

    if (m_pJniAosServiceThread == IMS_NULL)
    {
        IMS_TRACE_E(0, "JniAosService : can't create listener thread", 0, 0, 0);
        return;
    }

    m_pJniAosServiceThread->SetSlotId(m_nSlotId);
    m_pJniAosServiceThread->SetCallback(reinterpret_cast<IMS_SINTP>(this), pCbServiceNoti);

    Attach();
}

PUBLIC
void JniAosService::SetAosService(IN IAosService* piAosService)
{
    IMS_TRACE_D("SetAosService()", 0, 0, 0);
    m_piAosService = piAosService;
}

PUBLIC
JniAosServiceThread* JniAosService::GetThread()
{
    return m_pJniAosServiceThread;
}

PRIVATE VIRTUAL
void JniAosService::HandleMessage(IN IMS_SINT32 nMsg, IN const Parcel& objParcel)
{
    IMS_TRACE_D("HandleMessage() MSG=[%d]", nMsg, 0, 0);

    switch (nMsg)
    {
        case IIAosService::J2N_REQUEST_REGISTRATION:
            UpdateSipDelegateRegistration(objParcel);
            break;

        case IIAosService::J2N_REQUEST_DEREGISTRATION:
            TriggerSipDelegateDeregistration(objParcel);
            break;

        case IIAosService::J2N_REQUEST_FULL_REGISTRATION:
            TriggerFullNetworkRegistration(objParcel);
            break;

        case IIAosService::J2N_REQUEST_CAPABILITIES_CHANGED:
            NotifyCapabilitiesChanged(objParcel);
            break;

        case IIAosService::J2N_REQUEST_CONTROL_REGISTRATION:
            ControlRegistration(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_AIRPLANE_SETTING:
            NotifyAirplaneSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_DATA_ROAMING_SETTING:
            NotifyDataRoamingSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_MOBILE_DATA_SETTING:
            NotifyMobileDataSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_ROAMING_PREFERRED_VOICE_NETWORK:
            NotifyRoamingPreferredVoiceNetwork(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_SERVICE_SETTING:
            NotifyServiceSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_TTY_SETTING:
            NotifyTtySetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_VIDEO_SETTING:
            NotifyVideoSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_VOLTE_SETTING:
            NotifyVolteSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_WFC_SETTING:
            NotifyWfcSetting(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_AOS_START:
            NotifyAosStart(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_IPCAN_HANDOVER_FAILURE:
            NotifyIpcanHandoverFailure(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_ISIM_STATE:
            NotifyIsimState(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_LOCATION_INFO:
            NotifyLocationInfo(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_MOBILE_DATA_LIMIT:
            NotifyMobileDataLimit(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_NETWORK_VIDEO_CAPABILITY:
            NotifyNetworkVideoCapability(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_PHONE_NUMBER_STATE:
            NotifyPhoneNumberState(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_PLMN_CHANGED:
            NotifyPlmnChanged(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_POWER_OFF:
            NotifyPowerOff(objParcel);
            break;

        case IIAosService::J2N_NOTIFY_PRECISE_CALL_STATE:
            NotifyPreciseCallState(objParcel);
            break;

        default:
            break;
    }
}

PRIVATE
IMS_BOOL JniAosService::Attach()
{
    IMS_BOOL bIsAttached = IMS_FALSE;

    if (m_piAosService)
    {
        IMS_TRACE_D("Attach()::Attached", 0, 0, 0);
        return IMS_TRUE;
    }

    m_piAosService = JniConnectorFactory::GetInstance()->GetAosServiceConnector(m_nSlotId)->
            GetEnablerService();
    if (m_piAosService)
    {
        m_piAosService->SetJniAosService(this);
        bIsAttached = IMS_TRUE;
    }
    else
    {
        JniConnectorFactory::GetInstance()->GetAosServiceConnector(m_nSlotId)->SetJniService(this);
    }

    IMS_TRACE_I("Attach() :: %s", _TRACE_B_(bIsAttached), 0, 0);
    return bIsAttached;
}

PRIVATE
void JniAosService::UpdateSipDelegateRegistration(IN const Parcel& /*objParcel*/)
{
    if (Attach())
    {
        m_piAosService->UpdateSipDelegateRegistration();
    }
}

PRIVATE
void JniAosService::TriggerSipDelegateDeregistration(IN const Parcel& /*objParcel*/)
{
    if (Attach())
    {
        m_piAosService->TriggerSipDelegateDeregistration();
    }
}

PRIVATE
void JniAosService::TriggerFullNetworkRegistration(IN const Parcel& objParcel)
{
    IMS_SINT32 nSipCode = objParcel.readInt32();
    AString strSipReason;
    ConvertString(objParcel.readString16(), strSipReason);

    if (Attach())
    {
        m_piAosService->TriggerFullNetworkRegistration(nSipCode, strSipReason);
    }
}

PRIVATE
void JniAosService::NotifyCapabilitiesChanged(IN const Parcel& objParcel)
{
    IMSMap<IMS_UINT32, IMS_UINT32> objCapabilities;

    IMS_SINT32 nSize = objParcel.readInt32();
    for (int i = 0; i < nSize; ++i)
    {
        objCapabilities.Add(objParcel.readInt32(), objParcel.readInt32());
    }

    if (Attach())
    {
        m_piAosService->NotifyCapabilitiesChanged(objCapabilities);
    }
}

PRIVATE
void JniAosService::ControlRegistration(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->ControlRegistration(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyAirplaneSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyAirplaneSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyDataRoamingSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyDataRoamingSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyMobileDataSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyMobileDataSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyRoamingPreferredVoiceNetwork(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyRoamingPreferredVoiceNetwork(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyServiceSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyServiceSetting(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyTtySetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyTtySetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyVideoSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyVideoSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyVolteSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyVolteSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyWfcSetting(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyWfcSetting(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyAosStart(IN const android::Parcel& /*objParcel*/)
{
    if (Attach())
    {
        m_piAosService->NotifyAosStart();
    }
}

PRIVATE
void JniAosService::NotifyIpcanHandoverFailure(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyIpcanHandoverFailure(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyIsimState(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyIsimState(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyLocationInfo(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyLocationInfo(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyMobileDataLimit(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyMobileDataLimit(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyNetworkVideoCapability(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyNetworkVideoCapability(objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyPhoneNumberState(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyPhoneNumberState(objParcel.readInt32(), objParcel.readInt32());
    }
}

PRIVATE
void JniAosService::NotifyPlmnChanged(IN const android::Parcel& /*objParcel*/)
{
    if (Attach())
    {
        m_piAosService->NotifyPlmnChanged();
    }
}

PRIVATE
void JniAosService::NotifyPowerOff(IN const android::Parcel& /*objParcel*/)
{
    if (Attach())
    {
        m_piAosService->NotifyPowerOff();
    }
}

PRIVATE
void JniAosService::NotifyPreciseCallState(IN const android::Parcel& objParcel)
{
    if (Attach())
    {
        m_piAosService->NotifyPreciseCallState(objParcel.readInt32());
    }
}

PRIVATE GLOBAL
void JniAosService::ConvertString(IN const String16& strSource, OUT AString& strDest)
{
    String8 str8(strSource);
    strDest = str8.string();
}