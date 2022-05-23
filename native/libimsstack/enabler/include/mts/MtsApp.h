#ifndef MTS_APP_H_
#define MTS_APP_H_

#include "IMSApp.h"
#include "IMSService.h"
#include "IMtsCallTrackerListener.h"
#include "IMtsMessageControllerListener.h"
#include "IMtsApp.h"
#include "MtsServiceState.h"
#include "utility/MtsDynamicLoader.h"

class IMtsCallTracker;
class MtsCallTracker;
class MtsDynamicLoader;
class MtsMessageController;
class MtsService;
class MtsServiceState;

class MtsApp :
        public IMSApp,
        public IMtsApp,
        public IMtsCallTrackerListener,
        public IMtsMessageControllerListener
{
public:
    MtsApp(IN IMS_SINT32 nSlotId);
    virtual ~MtsApp();

    void AddService(IN MtsService* pService);
    MtsServiceState* GetMtsServiceState();
    void RemoveMtsServices();
    void RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType);

    // IMtsApp
    virtual void Start() override;
    virtual void Stop() override;

    // IMtsMessageControllerListener
    virtual void MtsMessageController_NoTransaction();

    // IMtsCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

private:
    void CreateMtsService(IN IMS_SINT32 nSlotId);
    void CreateMtsMessageController(
            IN IMS_SINT32 nSlotId, IN MtsDynamicLoader* pMtsDynamicLoader);
    void CreateMtsUtils(IN IMS_SINT32 nSlotId);
    void DestroyMtsUtils();
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

    enum
    {
        SMSFORMAT_3GPP = 1,
        SMSFORMAT_3GPP2,
        SMSFORMAT_INVALID
    };

protected:
    IMS_SINT32 m_nSlotId;
    IMSList<MtsService*> m_lstMtsServices;
    MtsService* m_pMtsService;
    MtsMessageController* m_pMtsMessageController;
    MtsDynamicLoader* m_pMtsDynamicLoader;
    MtsServiceState* m_pMtsServiceState;
    MtsCallTracker* m_pCallTracker;
};

#endif
