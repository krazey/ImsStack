#include "Connector.h"
#include "EnablerUtils.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "IMSCore.h"
#include "IPageMessage.h"
#include "IServiceFilterCriteria.h"
#include "ISipHeader.h"
#include "IImsAos.h"
#include "IImsAosInfo.h"
#include "IIpcan.h"
#include "ImsAos.h"
#include "IUMts.h"
#include "MtsService.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_SMS__;

MtsService::MtsService(IN const AString& strMtsAppId, IN const AString& strServiceId,
        IN IMS_SINT32 nSlotId, IN MtsDynamicLoader* pMtsDynamicLoader) :
        IMSService(AString::ConstNull()),
        m_strAppId(strMtsAppId),
        m_strServiceId(strServiceId),
        m_nSlotId(nSlotId),
        m_pMtsDynamicLoader(pMtsDynamicLoader)
{
    Init(strMtsAppId, strServiceId, nSlotId);
}

PUBLIC
MtsService::~MtsService()
{
    IMS_TRACE_I("CoreService_PageMessageReceived :: ~MtsService", 0, 0, 0);

    DeInit();
}

PRIVATE
void MtsService::Init(
        IN const AString& strMtsAppId, IN const AString& strServiceId, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("Init", 0, 0, 0);

    // Get the interface of AoS and register as the listener.
    m_piCoreService = IMS_NULL;
    m_piImsAos = IMS_NULL;

    // Get the interface of AoS and register as the listener.
    m_piImsAos = ImsAos::GetImsAos(strMtsAppId, strServiceId, nSlotId);

    if (m_piImsAos == IMS_NULL)
    {
        IMS_TRACE_E(0, "MtsService::Init: m_piImsAos is null", 0, 0, 0);
        return;
    }

    // Get an ICoreService and register as the listener.
    AString aStrParams;
    aStrParams.Sprintf("%s=%s", "serviceId", GetId().GetStr());

    m_piCoreService = DYNAMIC_CAST(
            ICoreService*, (Connector::Open(IMSCore::CONNECTION_SCHEME, GetAppId(), aStrParams)));

    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "MtsService::Init: m_piCoreService is null", 0, 0, 0);
        return;
    }

    m_piCoreService->SetListener(this);

    //// iFC -- starts
    // It MUST be applied if the feature-tag property is not supported (no Accept-Contact).
    IServiceFilterCriteria* piSFC = m_piCoreService->GetFilterCriteria();

    if (piSFC != IMS_NULL)
    {
        SipMethod objMethod(SipMethod::MESSAGE);
        TriggerPoint objTP(objMethod);

        // Add iFC for 3GPP2 SMS format (Content-Type: application/vnd.3gpp2.sms)
        objTP.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp2.sms");
        piSFC->AddTriggerPoint(objTP);

        // Add iFC for 3GPP SMS format (Content-Type: application/vnd.3gpp.sms)
        objTP.RemoveAllHeaders();
        objTP.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp.sms");
        piSFC->AddTriggerPoint(objTP);
    }
    //// iFC -- ends

    m_piImsAos->SetListener(this);
    m_piImsAos->SetMonitor(this);
}

PRIVATE
void MtsService::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);
    if (m_piCoreService != IMS_NULL)
    {
        m_piCoreService->Close();
        m_piCoreService = IMS_NULL;
    }

    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->SetListener(IMS_NULL);
        m_piImsAos->SetMonitor(IMS_NULL);
        m_piImsAos = IMS_NULL;
    }
}

PUBLIC
const AString& MtsService::GetId() const
{
    return m_strServiceId;
}

PUBLIC
ICoreService* MtsService::GetICoreService()
{
    return m_piCoreService;
}

PUBLIC
void MtsService::CoreService_PageMessageReceived(
        IN ICoreService* piService, IN IPageMessage* piMessage)
{
    IMS_TRACE_I("CoreService_PageMessageReceived :: SMS message has been received", 0, 0, 0);

    if (piService != m_piCoreService)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived : not my ICoreService", 0, 0, 0);
        return;
    }

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived : no IPageMessage", 0, 0, 0);
        return;
    }

    AString strTargetActivity = EnablerUtils::GetEnablerThreadName(m_nSlotId);
    strTargetActivity.Append(".MtsApp");
    IMSMSG objMSG(MtsServiceInternal::MTS_MT_RECVD, 0, reinterpret_cast<IMS_UINTP>(piMessage));
    MessageService::PostMessage(strTargetActivity, objMSG);
}

PUBLIC
void MtsService::CoreService_ReferenceReceived(
        IN ICoreService* piService, IN IReference* piReference)
{
    (void)piService;
    (void)piReference;

    IMS_TRACE_I("CoreService_ReferenceReceived : Service Name = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_ServiceClosed(IN ICoreService* piService, IN IReasonInfo* piReasonInfo)
{
    (void)piService;
    (void)piReasonInfo;

    IMS_TRACE_I("CoreService_ServiceClosed : Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_SessionInvitationReceived(
        IN ICoreService* piService, IN ISession* piSession)
{
    (void)piService;
    (void)piSession;

    IMS_TRACE_I("CoreService_SessionInvitationReceived : Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_UnsolicitedNotifyReceived(
        IN ICoreService* piService, IN IMessage* piNotify)
{
    (void)piService;
    (void)piNotify;

    IMS_TRACE_I("CoreService_UnsolicitedNotifyReceived : Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_CapabilityQueryReceived(
        IN ICoreService* piService, IN ICapabilities* piCapabilities)
{
    (void)piService;
    (void)piCapabilities;

    IMS_TRACE_I("CoreService_CapabilityQueryReceived : Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::ImsAos_Connected(IN IMS_UINT32 /*nFeatures*/, IN IMS_UINT32 /*nIpcan*/)
{
    IMS_TRACE_I("MtsService::ImsAos_Connected() m_nSlotId[%d]", m_nSlotId, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->OnImsConnected();
    }
    else
    {
        IMS_TRACE_E(0, "pMtsServiceState is null", 0, 0, 0);
    }
}

PUBLIC
void MtsService::ImsAos_Connecting() {}

PUBLIC
void MtsService::ImsAos_Disconnected(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("MtsService::ImsAos_Disconnected() Reason is (%d)", nReason, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->OnImsDisconnected(nReason);
    }
}

PUBLIC
void MtsService::ImsAos_Disconnecting(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("MtsService::ImsAos_Disconnecting() Reason is (%d)", nReason, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->OnImsDisconnecting(nReason);
    }
}

PUBLIC
void MtsService::ImsAos_Suspended(IN IMS_UINT32 nReason)
{
    IMS_TRACE_I("MtsService::ImsAos_Suspended() Reason is (%d)", nReason, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->OnImsSuspended(nReason);
    }
}

PUBLIC
void MtsService::ImsAos_Resumed()
{
    IMS_TRACE_I("MtsService::ImsAos_Resumed()", 0, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->OnImsResumed();
    }
}

PUBLIC
void MtsService::IMSAoSApp_NotifySpecificMessage(
        IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam)
{
    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->NotifySpecificMessage(nMsg, nWparam, nLparam);
    }
}

PUBLIC
void MtsService::ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 /*nIpcan*/)
{
    IMS_TRACE_I("MtsService::ImsAosMonitor_Connected :[%08x] ", nServices, 0, 0);

    if (m_pMtsDynamicLoader == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pMtsDynamicLoader is null", 0, 0, 0);
        return;
    }

    MtsServiceState* pMtsServiceState = m_pMtsDynamicLoader->GetMtsServiceState();

    if (pMtsServiceState != IMS_NULL)
    {
        pMtsServiceState->SetConnectedServices(nServices);
    }
}

PUBLIC
void MtsService::ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    (void)nType;
    (void)nState;

    IMS_TRACE_I("MtsService::IMSAoSAppMonitor_Notify - nType [%d], nInfo [%d]", nType, nState, 0);
}

PUBLIC
void MtsService::RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType)
{
    if (m_piImsAos != IMS_NULL)
    {
        m_piImsAos->Control(nRecoveryType);
    }
    else
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
    }
}

PUBLIC
void MtsService::RequestRegistrationSwitch(
        IN IUSendSmsRequestParam* /*pToBeSentSms*/, IN IMS_BOOL /*bIsSmsEServiceType*/)
{
    IMS_TRACE_D("MtsService::RequestRegistrationSwitch", 0, 0, 0);
}

PUBLIC
IMS_BOOL MtsService::IsEpdgConnected()
{
    IMS_SINT32 nReportedIpcan = IIpcan::CATEGORY_MOBILE;
    IMS_BOOL bIsEpdg = IMS_FALSE;
    IMS_TRACE_I("Start API IsEpdgConnected", 0, 0, 0);

    if (m_piImsAos != IMS_NULL)
    {
        nReportedIpcan = m_piImsAos->GetAosInfo()->GetIpcanType();
    }
    else
    {
        IMS_TRACE_E(0, "m_piImsAos is null", 0, 0, 0);
    }

    if (nReportedIpcan == IIpcan::CATEGORY_WLAN)
    {
        IMS_TRACE_I("IsEpdgConnected:bIsEpdg is true", 0, 0, 0);
        bIsEpdg = IMS_TRUE;
    }
    else
    {
        IMS_TRACE_I("IsEpdgConnected:bIsEpdg is false (! IPCAN_WLAN)", 0, 0, 0);
    }

    return bIsEpdg;
}

PROTECTED
AString& MtsService::GetAppId()
{
    return m_strAppId;
}
