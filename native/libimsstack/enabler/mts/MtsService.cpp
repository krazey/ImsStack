#include "Connector.h"
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
#include "IuMts.h"
#include "JniConnectorFactory.h"
#include "JniMtsService.h"
#include "JniMtsServiceThread.h"
#include "MtsApp.h"
#include "MtsFactory.h"
#include "MtsService.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_SMS__;

MtsService::MtsService(IN const AString& strMtsAppId, IN const AString& strServiceId,
        IN IMS_SINT32 nSlotId, IN MtsDynamicLoader* pMtsDynamicLoader) :
        IMSService(AString::ConstNull()),
        m_piImsAos(IMS_NULL),
        m_strAppId(strMtsAppId),
        m_strServiceId(strServiceId),
        m_nSlotId(nSlotId),
        m_piCoreService(IMS_NULL),
        m_piMtsServiceListener(IMS_NULL),
        m_pJniMtsService(IMS_NULL),
        m_pMtsDynamicLoader(pMtsDynamicLoader)
{
    Init(strMtsAppId, strServiceId, nSlotId);
}

PUBLIC
MtsService::~MtsService()
{
    IMS_TRACE_I("~MtsService", 0, 0, 0);

    if (m_pJniMtsService != IMS_NULL) {
        m_pJniMtsService->SetMtsService(IMS_NULL);
    }

    DeInit();
}

PUBLIC
const AString& MtsService::GetId() const
{
    return m_strServiceId;
}

PUBLIC
ICoreService* MtsService::GetICoreService() const
{
    return m_piCoreService;
}

PUBLIC VIRTUAL
void MtsService::SetListener(IN IMtsServiceListener* piMtsServiceListener)
{
    m_piMtsServiceListener = piMtsServiceListener;
}

PUBLIC VIRTUAL
void MtsService::SetJniMtsService(IN JniMtsService* pJniMtsService)
{
    IMS_TRACE_I("SetJniMtsService", 0, 0, 0);
    m_pJniMtsService = pJniMtsService;
}

PUBLIC VIRTUAL
void MtsService::SendMoSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData,
        IN const AString& strAddress, IN IMS_SINT32 nSeqId)
{
    IMS_TRACE_I("SendMoSms", 0, 0, 0);
    m_piMtsServiceListener->NotifyMoSms(nSmsFormat, objData, strAddress, nSeqId);
}

PUBLIC VIRTUAL
void MtsService::SendMtResult(IN IMS_BOOL bMtResult)
{
    IMS_TRACE_I("SendMtResult", 0, 0, 0);
    // TODO: Call back is being considered
    (void)bMtResult;
}

PUBLIC
void MtsService::ReportMoStatus(IN IMS_UINT32 nReason, IN IMS_UINT32 nSmsformat,
        IN IMS_UINT8 nRetryAfter, IN IMS_SINT32 nSeqId)
{
    IMS_TRACE_I("ReportMoStatus", 0, 0, 0);

    if (Attach())
    {
        m_pJniMtsService->GetThread()->ReportMoStatus(
                nReason, nSmsformat, nRetryAfter, nSeqId, m_nSlotId);
    }
}

PUBLIC
void MtsService::ReportMtSms(IN IMS_UINT32 nSmsFormat, IN const ByteArray& objData)
{
    IMS_TRACE_I("ReportMtSms", 0, 0, 0);

    if (Attach())
    {
        m_pJniMtsService->GetThread()->ReportMtSms(nSmsFormat, objData, m_nSlotId);
    }
}

PUBLIC
void MtsService::CoreService_PageMessageReceived(
        IN ICoreService* piService, IN IPageMessage* piMessage)
{
    IMS_TRACE_I("CoreService_PageMessageReceived() - SMS message has been received", 0, 0, 0);

    if (piService != m_piCoreService)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived() - not my ICoreService", 0, 0, 0);
        return;
    }

    if (piMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "CoreService_PageMessageReceived() - no IPageMessage", 0, 0, 0);
        return;
    }

    m_piMtsServiceListener->NotifyMtSms(piMessage);
}

PUBLIC
void MtsService::CoreService_ReferenceReceived(
        IN ICoreService* piService, IN IReference* piReference)
{
    (void)piService;
    (void)piReference;

    IMS_TRACE_I("CoreService_ReferenceReceived() - Service Name = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_ServiceClosed(IN ICoreService* piService, IN IReasonInfo* piReasonInfo)
{
    (void)piService;
    (void)piReasonInfo;

    IMS_TRACE_I("CoreService_ServiceClosed() - Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_SessionInvitationReceived(
        IN ICoreService* piService, IN ISession* piSession)
{
    (void)piService;
    (void)piSession;

    IMS_TRACE_I("CoreService_SessionInvitationReceived() - Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_UnsolicitedNotifyReceived(
        IN ICoreService* piService, IN IMessage* piNotify)
{
    (void)piService;
    (void)piNotify;

    IMS_TRACE_I("CoreService_UnsolicitedNotifyReceived() - Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::CoreService_CapabilityQueryReceived(
        IN ICoreService* piService, IN ICapabilities* piCapabilities)
{
    (void)piService;
    (void)piCapabilities;

    IMS_TRACE_I("CoreService_CapabilityQueryReceived() - Service = %s", GetName().GetStr(), 0, 0);
    return;
}

PUBLIC
void MtsService::ImsAos_Connected(IN IMS_UINT32 /*nFeatures*/, IN IMS_UINT32 /*nIpcan*/)
{
    IMS_TRACE_I("ImsAos_Connected() - m_nSlotId[%d]", m_nSlotId, 0, 0);

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
    IMS_TRACE_I("ImsAos_Disconnected() - Reason is (%d)", nReason, 0, 0);

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
    IMS_TRACE_I("ImsAos_Disconnecting() - Reason is (%d)", nReason, 0, 0);

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
    IMS_TRACE_I("ImsAos_Suspended() - Reason is (%d)", nReason, 0, 0);

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
    IMS_TRACE_I("ImsAos_Resumed", 0, 0, 0);

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
    IMS_TRACE_I("ImsAosMonitor_Connected() - [%08x] ", nServices, 0, 0);

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

    IMS_TRACE_I("IMSAoSAppMonitor_Notify() - nType [%d], nInfo [%d]", nType, nState, 0);
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
const AString& MtsService::GetAppId() const
{
    return m_strAppId;
}

PRIVATE
void MtsService::Init(
        IN const AString& strMtsAppId, IN const AString& strServiceId, IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_I("Init", 0, 0, 0);

    Attach();

    // Get the interface of AoS and register as the listener.
    m_piImsAos = ImsAos::GetImsAos(strMtsAppId, strServiceId, nSlotId);

    if (m_piImsAos == IMS_NULL)
    {
        IMS_TRACE_E(0, "Init() - m_piImsAos is null", 0, 0, 0);
        return;
    }

    // Get an ICoreService and register as the listener.
    AString aStrParams;
    aStrParams.Sprintf("%s=%s", "serviceId", GetId().GetStr());

    m_piCoreService = DYNAMIC_CAST(
            ICoreService*, (Connector::Open(IMSCore::CONNECTION_SCHEME, GetAppId(), aStrParams)));

    if (m_piCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Init() - m_piCoreService is null", 0, 0, 0);
        return;
    }

    m_piCoreService->SetListener(this);

    //// iFC -- starts
    // It MUST be applied if the feature-tag property is not supported (no Accept-Contact).
    IServiceFilterCriteria* piSfc = m_piCoreService->GetFilterCriteria();

    if (piSfc != IMS_NULL)
    {
        SipMethod objMethod(SipMethod::MESSAGE);
        TriggerPoint objTp(objMethod);

        // Add iFC for 3GPP2 SMS format (Content-Type: application/vnd.3gpp2.sms)
        objTp.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp2.sms");
        piSfc->AddTriggerPoint(objTp);

        // Add iFC for 3GPP SMS format (Content-Type: application/vnd.3gpp.sms)
        objTp.RemoveAllHeaders();
        objTp.AddHeader(ISipHeader::CONTENT_TYPE, "application/vnd.3gpp.sms");
        piSfc->AddTriggerPoint(objTp);
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

PRIVATE
IMS_BOOL MtsService::Attach()
{
    if (m_pJniMtsService)
    {
        return IMS_TRUE;
    }

    IMS_BOOL bIsAttached = IMS_FALSE;
    m_pJniMtsService =
            JniConnectorFactory::GetInstance()->GetMtsServiceConnector(m_nSlotId)->GetJniService();

    if (m_pJniMtsService)
    {
        m_pJniMtsService->SetMtsService(this);
        bIsAttached = IMS_TRUE;
    }
    else
    {
        JniConnectorFactory::GetInstance()
                ->GetMtsServiceConnector(m_nSlotId)->SetEnablerService(this);
    }

    IMS_TRACE_I("Attach() - %s", _TRACE_B_(bIsAttached), 0, 0);
    return bIsAttached;
}
