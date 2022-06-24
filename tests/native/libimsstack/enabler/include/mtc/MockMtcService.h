#ifndef MOCK_MTC_SERVICE_H_
#define MOCK_MTC_SERVICE_H_

#include <gmock/gmock.h>

#include "AString.h"
#include "ICoreService.h"
#include "ICoreServiceListener.h"
#include "IImsAosListener.h"
#include "IImsAosMonitor.h"
#include "ImsService.h"
#include "IMSTypeDef.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "helper/SrvccEventHandler.h"
#include "helper/MtcAosEventHandler.h"
#include "helper/MtcAosConnector.h"
#include "MtcService.h"
class JniMtcService;
class JniMtcServiceThread;
class IMtcAosConnector;

class MockMtcService : public MtcService
{
public:
    ~MockMtcService() override { Die(); }
    MOCK_METHOD(void, Die, ());
    MOCK_METHOD(ServiceType, GetServiceType, (), (const, override));
    MOCK_METHOD(void, AddSrvccStateListener, (IN ISrvccStateListener * piListener), (override));
    MOCK_METHOD(void, RemoveSrvccStateListener, (IN ISrvccStateListener * piListener), (override));
    MOCK_METHOD(IMS_BOOL, IsActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergency, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWlanIpCanType, (), (const, override));
    MOCK_METHOD(ServiceStatus, GetServiceStatus, (), (const, override));
    MOCK_METHOD(ICoreService*, GetICoreService, (), (const, override));
    MOCK_METHOD(IMtcAosConnector*, GetAosConnector, (), (const, override));
    MOCK_METHOD(void, UpdateSrvccState, (IN SrvccState eState), (override));
    MOCK_METHOD(void, SetJniService, (IN JniMtcService * pJniService), (override));
    MOCK_METHOD(void, SetTerminalBasedCallWaiting, (IN IMS_BOOL bProvisioned, IN IMS_BOOL bEnabled),
            (override));
    MOCK_METHOD(IMS_BOOL, IsTerminalBasedCallWaitingEnabled, (), (const, override));
    MOCK_METHOD(void, OpenEmergencyService, (), (override));
    MOCK_METHOD(void, CoreService_PageMessageReceived,
            (IN ICoreService * piService, IN IPageMessage* piMessage), (override));
    MOCK_METHOD(void, CoreService_ReferenceReceived,
            (IN ICoreService * piService, IN IReference* piReference), (override));
    MOCK_METHOD(void, CoreService_ServiceClosed,
            (IN ICoreService * piService, IN IReasonInfo* piReasonInfo), (override));
    MOCK_METHOD(void, CoreService_SessionInvitationReceived,
            (IN ICoreService * piService, IN ISession* piSession), (override));
    MOCK_METHOD(void, CoreService_UnsolicitedNotifyReceived,
            (IN ICoreService * piService, IN IMessage* piNotify), (override));
    MOCK_METHOD(void, CoreService_CapabilityQueryReceived,
            (IN ICoreService * piService, IN ICapabilities* piCapabilities), (override));
    MOCK_METHOD(
            void, ImsAos_Connected, (IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan), (override));
    MOCK_METHOD(void, ImsAos_Connecting, (), (override));
    MOCK_METHOD(void, ImsAos_Disconnecting, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Disconnected, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Suspended, (IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(void, ImsAos_Resumed, (), (override));
    MOCK_METHOD(void, ImsAosMonitor_Connected, (IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan),
            (override));
    MOCK_METHOD(
            void, ImsAosMonitor_Notify, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(AString, GetServiceName, (IN ServiceType eType), (const));
    MOCK_METHOD(void, AttachCoreServiceInterface, (), ());
    MOCK_METHOD(void, AttachAosInterface, (), ());
    MOCK_METHOD(void, SetServiceFilterCriteria, (), ());
    MOCK_METHOD(void, SetAosReady, (IN IMS_BOOL), ());
};

#endif
