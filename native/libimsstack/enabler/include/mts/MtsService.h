#ifndef MTS_SERVICE_H_
#define MTS_SERVICE_H_

#include "IMSService.h"
#include "ICoreServiceListener.h"
#include "interface/aos/IImsAosListener.h"
#include "interface/aos/IImsAosMonitor.h"
#include "IMtsService.h"

#include "IUMts.h"

class IImsAos;
class MtsDynamicLoader;

class MtsService final :
        public IMSService,
        public ICoreServiceListener,
        public IImsAosListener,
        public IImsAosMonitor,
        public IMtsService
{
public:
    MtsService(IN const AString& strMtsAppId, IN const AString& strServiceId, IN IMS_SINT32 nSlotId,
            IN MtsDynamicLoader* pMtsDynamicLoader);
    ~MtsService();

protected:
    // IMSService
    inline IMS_BOOL OnPreprocess(IN IMSMSG& /*objMSG*/) { return IMS_TRUE; }
    inline IMS_BOOL OnMessage(IN IMSMSG& /*objMSG*/) { return IMS_TRUE; }
    inline IMS_BOOL OnPostprocess(IN IMSMSG& /*objMSG*/) { return IMS_TRUE; }

public:
    // ICoreServiceListener
    void CoreService_PageMessageReceived(IN ICoreService* piService, IN IPageMessage* piMessage);
    void CoreService_ReferenceReceived(IN ICoreService* piService, IN IReference* piReference);
    void CoreService_ServiceClosed(IN ICoreService* piService, IN IReasonInfo* piReasonInfo);
    void CoreService_SessionInvitationReceived(IN ICoreService* piService, IN ISession* piSession);
    void CoreService_UnsolicitedNotifyReceived(IN ICoreService* piService, IN IMessage* piNotify);
    void CoreService_CapabilityQueryReceived(
            IN ICoreService* piService, IN ICapabilities* piCapabilities);

    // IImsAosListener
    void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    void ImsAos_Connecting() override;
    void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    void ImsAos_Resumed() override;

    // IImsAosMonitor
    void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) override;
    void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

    // IMtsService
    const AString& GetId() const;
    ICoreService* GetICoreService();

    void RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType);
    void RequestRegistrationSwitch(
            IN IUSendSmsRequestParam* pToBeSentSms, IN IMS_BOOL bIsSmsEServiceType);
    void IMSAoSApp_NotifySpecificMessage(
            IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam);
    IMS_BOOL IsEpdgConnected();

protected:
    AString& GetAppId();

private:
    void Init(IN const AString& strMtsAppId, IN const AString& strServiceId, IN IMS_SINT32 nSlotId);
    void DeInit();

protected:
    IImsAos* m_piImsAos;
    AString m_strAppId;
    AString m_strServiceId;
    IMS_UINT32 m_nSlotId;
    ICoreService* m_piCoreService;
    MtsDynamicLoader* m_pMtsDynamicLoader;
};

#endif
