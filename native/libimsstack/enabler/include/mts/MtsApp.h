#ifndef MTS_APP_H_
#define MTS_APP_H_

#include "IMSApp.h"
#include "IMtsMessageControllerListener.h"
#include "IMtsApp.h"
#include "IMSService.h"
#include "utility/MtsDynamicLoader.h"
#include "MtsServiceState.h"

#include "IMtsCallTrackerListener.h"

#include "utility/MtsTrm.h"

class MtsService;
class MtsClient;
class MtsMessageController;
class MtsDynamicLoader;
class MtsServiceState;
class MtsCallTracker;
class IMtsCallTracker;

class MtsApp :
        public IMtsApp,
        public IMSApp,
        public IIMSActivityControl,
        public IMtsMessageControllerListener,
        public MtsTrmListener,
        public IMtsCallTrackerListener
{
public:
    MtsApp(IN IMS_SINT32 nSlotId);
    virtual ~MtsApp();

    static IMSApp* GetInstance(IN IMS_SINT32 nSlotId);
    void RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType);
    virtual void RequestRegistrationSwitch(
            IN IUSendSmsRequestParam* pToBeSentSms, IN IMS_BOOL bIsSmsEServiceType);
    MtsServiceState* GetMtsServiceState();

    // IMtsApp
    virtual void Start();
    virtual void Stop();

    // IIMSActivityControl
    virtual IMS_BOOL Control(
            IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam, OUT IMS_UINTP* pnOutParam);

    // IMtsMessageControllerListener
    virtual void MtsMessageController_NoTransaction();

    // MtsTrmListener
    virtual void Trm_PriorityChanged();

    // IMtsCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

protected:
    // IMSApp
    virtual IMS_BOOL OnPreprocess(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG);
    virtual IMS_BOOL OnPostprocess(IN IMSMSG& objMSG);
    virtual IIMSActivityControl* GetController();

    virtual void CreateMtsService(IN IMS_SINT32 nSlotId);
    virtual void CreateMtsClient(IN IMS_SINT32 nSlotId);
    virtual void RemoveMtsClient(IN IMS_SINT32 nSlotId);
    virtual void CreateMtsMessageController(
            IN IMS_SINT32 nSlotId, IN MtsDynamicLoader* pMtsDynamicLoader);
    virtual void CreateMtsUtils(IN IMS_SINT32 nSlotId);
    virtual void DestroyMtsUtils();

protected:
    void AddService(IN MtsService* pService);
    void RemoveMtsService();

private:
    void GetSmOverIpConfigInfo(IN IMS_SINT32 nSlotId);
    void SetSlotId(IN IMS_SINT32 nSlotId);

public:
    // SDM item definitions
    enum
    {
        SDM_I_BASE = 0,

        SDM_I_HOME_DOMAIN_NAME,
        SDM_I_TV_T1,
        SDM_I_TV_T2,
        SDM_I_TV_TF,
        SDM_I_SMS_FORMAT,
        SDM_I_SMS_OVER_IP_NETWORK_INDICATION,

        SDM_I_MAX,
    };

    // PST item definitions
    enum
    {
        PST_I_BASE = 0,

        PST_I_PCSCF_ADDRESS,
        PST_I_PCSCF_PORT,
        PST_I_TV_T1,
        PST_I_TV_T2,
        PST_I_TV_TF,
        PST_I_SMS_FORMAT,
        PST_I_SMS_OVER_IP_NETWORK_INDICATION,

        PST_I_MAX
    };

    // TIMER item definitions
    enum
    {
        TIMER_SMS_CLIENT_RETRY = 0
    };

protected:
    IMS_SINT32 m_nSlotId;
    IMSList<MtsService*> m_lstMtsServices;
    MtsClient* m_pMtsClient;
    MtsService* m_pMtsService;
    MtsMessageController* m_pMtsMessageController;
    MtsDynamicLoader* m_pMtsDynamicLoader;
    MtsServiceState* m_pMtsServiceState;
    MtsCallTracker* m_pCallTracker;
    MtsTrm* m_pMtsAppTrm;
    IMS_BOOL m_bTrmBlock;
};

#endif
