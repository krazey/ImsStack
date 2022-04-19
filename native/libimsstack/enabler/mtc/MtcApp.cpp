#include "AString.h"
#include "Configuration.h"
#include "IMSList.h"
#include "ImsServiceConfig.h"
#include "ServiceMSG.h"
#include "ServiceTrace.h"

#include "JniConnector.h"
#include "JniConnectorFactory.h"
#include "JniMtcCall.h"
#include "IMtcService.h"
#include "MtcApp.h"
#include "MtcImsEventReceiver.h"
#include "call/MtcCallManager.h"
#include "MtcService.h"
#include "helper/CallStateProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "call/MtcCallController.h"
#include "conferencecall/ConferenceManager.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR MTC_APP_NAME[] = "MtcApp";


PUBLIC
MtcApp::MtcApp(IN IMS_SINT32 nSlotId) :
        IMSApp(MTC_APP_NAME),
        m_nSlotId(nSlotId),
        m_objConfigurationProxy(MtcConfigurationProxy()),
        m_lstServices(IMSList<MtcService*>()),
        m_objDialingPlan(MtcDialingPlan(*this)),
        m_objCallManager(MtcCallManager(*this)),
        m_objCallController(MtcCallController(*this)),
        m_objVonrManager(MtcVonrManager()),
        m_objCallStateProxy(CallStateProxy(m_objCallManager)),
        m_objImsEventReceiver(MtcImsEventReceiver(nSlotId)),
        m_objSipInterfaceFactory(MtcSipInterfaceFactory()),
        m_objConferenceManager(ConferenceManager(*this))
{
    IMS_TRACE_I("+MtcApp [slot_%d]", nSlotId, 0, 0);
    Configuration::GetInstance()->SetAppConfig(
            ImsServiceConfig::GetAppName(ImsAppId::MTC), nSlotId);
}

PUBLIC VIRTUAL
MtcApp::~MtcApp()
{
    IMS_TRACE_I("~MtcApp [slot_%d]", m_nSlotId, 0, 0);
}

PUBLIC VIRTUAL
void MtcApp::Start()
{
    IMS_TRACE_I("Start", 0, 0, 0);

    InitConfiguration();
    CreateServices();
    InitCallManager();
}

PUBLIC VIRTUAL
void MtcApp::Stop()
{
    IMS_TRACE_I("Stop", 0, 0, 0);
    DestroyServices();
}

PUBLIC VIRTUAL
IMtcService* MtcApp::GetServiceByType(IN ServiceType eServiceType)
{
    for (IMS_UINT32 i = 0; i < m_lstServices.GetSize(); i++)
    {
        IMtcService* piService = m_lstServices.GetAt(i);

        if (eServiceType == piService->GetServiceType())
        {
            IMS_TRACE_I("GetServiceByType : Type[%d]", eServiceType, 0, 0);
            return piService;
        }
    }

    IMS_TRACE_D("GetServiceByType : Service Type[%d] Not Found", eServiceType, 0, 0);
    return IMS_NULL;
}

PUBLIC VIRTUAL
MtcAosConnector* MtcApp::GetAosConnector(IN ServiceType eServiceType)
{
    for (IMS_UINT32 i = 0; i < m_lstServices.GetSize(); i++)
    {
        IMtcService* piService = m_lstServices.GetAt(i);

        if (eServiceType == piService->GetServiceType())
        {
            return piService->GetAosConnector();
        }
    }

    return IMS_NULL;
}

PRIVATE
void MtcApp::InitConfiguration()
{
    m_objConfigurationProxy.Init();
}

PRIVATE
void MtcApp::CreateServices()
{
    m_lstServices.Append(new MtcService(*this, ServiceType::NORMAL));
    //m_lstServices.Append(new MtcService(*this, ServiceType::EMERGENCY));
}

PRIVATE
void MtcApp::InitCallManager()
{
    m_objCallManager.Init();
    JniConnectorFactory::GetInstance()->GetMtcCallConnector(m_nSlotId)->
            SetEnablerService(static_cast<MtcCallController*>(&m_objCallController));
}

PRIVATE
void MtcApp::DestroyServices()
{

}
