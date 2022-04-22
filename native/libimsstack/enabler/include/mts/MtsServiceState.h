#ifndef MTS_SERVICESTATE_H_
#define MTS_SERVICESTATE_H_

#include "IMSTypeDef.h"
#include "message/MtsMessageController.h"

class MtsServiceState final
{
public:
    MtsServiceState(IN IMS_SINT32 nSlotId);
    ~MtsServiceState();

    static MtsServiceState* GetInstance(IN IMS_SINT32 nSlotId);
    void SetImsRegConnected(IN IMS_BOOL bConnected);
    void SetImsRegConnected(IN IMS_BOOL bConnected, IMS_BOOL bIsEmergencyType);
    void SetImsSuspendState(IN IMS_BOOL bState);
    void SetSmsOverIpState(IN IMS_BOOL bState);
    void SetTemporaryServiceBlocked(IN IMS_BOOL bBlocked);
    void SetMtsMessageController(IN MtsMessageController* pMtsMessageController);
    void SetMtsServiceState(IN IMS_SINT32 nServiceState);
    void SetConnectedServices(IN IMS_UINT32 nServices);

    IMS_BOOL GetImsRegState();
    IMS_BOOL GetImsRegMod();
    IMS_BOOL GetImsSuspendState();
    IMS_BOOL GetSmsOverIpState();
    IMS_SINT32 GetMtsServiceState();
    IMS_UINT32 GetConnectedServices();
    MtsMessageController* GetMtsMessageController();
    IMS_BOOL IsServiceConnected(IN IMS_UINT32 nService);

    void OnImsConnected();
    void OnImsConnected(IN IUSendSmsRequestParam* pToBeSentSms);
    void OnImsDisconnected(IN IMS_UINT32 nReason);
    void OnImsDisconnecting(IN IMS_UINT32 nReason);
    void OnImsSuspended(IN IMS_UINT32 nReason);
    void OnImsResumed();
    void NotifySpecificMessage(IN IMS_UINT32 nMsg, IN IMS_UINT32 nWparam, IN IMS_UINT32 nLparam);

    IMS_SINT32 GetServiceState();
    void UpdateServiceState();
    IMS_BOOL IsMoServiceBlocked();
    IMS_BOOL IsMtServiceBlocked();
    IMS_BOOL IsTemporaryServiceBlocked();
    IMS_SINT32 GetSlotId();

protected:
    IMS_SINT32  m_nMtsServiceState;

    //Check Condition for SMS SERVICE MODE
    IMS_BOOL                m_bIsImsConnected; //if Connected true enable sms mo/mt service.
    IMS_BOOL                m_bIsAosRegModAdmin; //if Mod Admin true. block mo service.
    IMS_BOOL                m_bIsImsSuspend; //if IMSAoSApp_IMSSuspended true. block mo service
    //if sms_over_ip_network Ind is false. block mo service
    IMS_BOOL                m_bIsSmsOverIpConf;
    IMS_BOOL                m_bIsTemporaryBlocked;
    IMS_UINT32              m_nConnectedServices;
    IMS_SINT32              m_nSlotId;
    MtsMessageController*   m_pMtsMessageController;
};

#endif
