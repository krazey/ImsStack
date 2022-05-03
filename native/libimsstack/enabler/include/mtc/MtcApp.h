#ifndef MTC_APP_H_
#define MTC_APP_H_

#include "IMSApp.h"

#include "MtcImsEventReceiver.h"
#include "helper/CallStateProxy.h"
#include "IMtcApp.h"
#include "call/IMtcCallManager.h"
#include "IMtcContext.h"
#include "IMtcService.h"
#include "call/MtcCallController.h"
#include "call/MtcCallManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "vonr/MtcVonrManager.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "conferencecall/ConferenceManager.h"

class MtcService;
class EctManager;

class MtcApp : public IMSApp, public IMtcApp, public IMtcContext
{
public:
    MtcApp(IN IMS_SINT32 nSlotId);
    virtual ~MtcApp();
    MtcApp(IN const MtcApp&) = delete;
    MtcApp& operator=(IN const MtcApp&) = delete;

    // IMtcApp implementation
    virtual void Start() override;
    virtual void Stop() override;

    // IMtcContext implementation
    inline IMS_SINT32 GetSlotId() override { return m_nSlotId; }
    IMtcService* GetServiceByType(IN ServiceType eServiceType) override;
    inline MtcDialingPlan& GetDialingPlan() override { return m_objDialingPlan; }
    inline MtcCallController& GetCallController() override { return m_objCallController; }
    inline IMtcCallManager& GetCallManager() override { return m_objCallManager; }
    inline MtcVonrManager& GetVonrManager() override { return m_objVonrManager; }
    inline MtcConfigurationProxy& GetConfigurationProxy() override
    {
        return m_objConfigurationProxy;
    }
    inline CallStateProxy& GetCallStateProxy() override { return m_objCallStateProxy; }
    inline MtcImsEventReceiver& GetImsEventReceiver() override { return m_objImsEventReceiver; }
    MtcAosConnector* GetAosConnector(IN ServiceType eServiceType) override;
    inline MtcSipInterfaceFactory& GetSipInterfaceFactory() override
    {
        return m_objSipInterfaceFactory;
    }
    inline ConferenceManager& GetConferenceManager() override { return m_objConferenceManager; }
    EctManager* GetEctManager() override;

private:
    void InitConfiguration();
    void CreateServices();
    void InitCallManager();
    void DestroyServices();

private:
    IMS_SINT32 m_nSlotId;
    MtcConfigurationProxy m_objConfigurationProxy;
    IMSList<MtcService*> m_lstServices;
    MtcDialingPlan m_objDialingPlan;
    MtcCallManager m_objCallManager;
    MtcCallController m_objCallController;
    MtcVonrManager m_objVonrManager;
    CallStateProxy m_objCallStateProxy;
    MtcImsEventReceiver m_objImsEventReceiver;
    MtcSipInterfaceFactory m_objSipInterfaceFactory;
    ConferenceManager m_objConferenceManager;
    EctManager* m_pEctManager;
};

#endif
