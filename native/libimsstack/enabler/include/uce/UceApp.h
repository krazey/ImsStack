
#ifndef _UCE_APP_H_
#define _UCE_APP_H_

#include "IMSApp.h"
#include "ImsMessageDef.h"
#include "interface/aos/IImsAosListener.h"
#include "interface/aos/IImsAosMonitor.h"
#include "INetworkWatcher.h"
#include "ITimer.h"
#include "def/UceDef.h"
#include "IUce.h"

class IImsAos;
class IMSService;
class UceService;

class UceApp
    : public IMSApp
    , public IIMSActivityControl
    , public IImsAosListener
    , public IImsAosMonitor
    , public INetworkWatcherListener
    , public ITimerListener
{
/* -------------------------------------------------------------------------------------------------
    Constructor, Destructor, Operator Overloading
------------------------------------------------------------------------------------------------- */
public:
    UceApp(IN CONST IMS_SINT32 nSlotId, IN CONST AString &strAppName);
    virtual ~UceApp();
/* -------------------------------------------------------------------------------------------------
    Methods
------------------------------------------------------------------------------------------------- */
public:
    static IMSApp* GetInstance(IN CONST IMS_SINT32 nSlotId);
protected:
    virtual IMS_BOOL OnPreprocess( IN IMSMSG &objMSG );
    virtual IMS_BOOL OnMessage( IN IMSMSG &objMSG );
    virtual IMS_BOOL OnPostprocess( IN IMSMSG &objMSG );
    virtual IIMSActivityControl* GetController();
    virtual IMS_BOOL Control(IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam,
            OUT IMS_UINTP* pnOutParam);
    void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo);
    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);
    void ClearTimer();

    // ITimerListener Interface
    void Timer_TimerExpired(IN ITimer *piTimer);

    virtual void ImsAos_Connected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAos_Connecting() override;
    virtual void ImsAos_Disconnecting(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Disconnected(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Suspended(IN IMS_UINT32 nReason) override;
    virtual void ImsAos_Resumed() override;

    virtual void ImsAosMonitor_Connected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan) override;
    virtual void ImsAosMonitor_Notify(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;

private:
    void EnableUceService(void);
    void DisableUceService(void);
    IMS_SINT32 GetServiceNetworkType(IN IMS_UINT32 nIpcan);
    IMS_SINT32 ConvertNetworkType(IN IMS_SINT32 nRAT);
    void NotifyRATChanged(void);
    void EnableAllAoSApps();
    void SelectActiveAoSApp();
    static const IMS_CHAR* GetPrefixForMultiApp();
    static AString GetUceAppName(IN IMS_SINT32 nSlotId);
    void SetPublishStatusToAos(IN IMS_BOOL bIsPublishStarted);
    void SendRegistrationRecoveryRequestToAos(IN IMS_UINT32 nAosControlType);
/* -------------------------------------------------------------------------------------------------
    Variables
------------------------------------------------------------------------------------------------- */
protected:
    enum
    {
        AMSG_BASE = IMS_MSG_USER + 1,
        AMSG_CREATE_SERVICE,
        AMSG_MAX
    };
    enum
    {
        TIMER_NETWORK_CHANGED = 0,
    };
    IMS_SINT32                      m_nSlotId;
    AString                         m_strAppName;
    AString                         m_strAppID;
    AString                         m_strServiceID;
private:
    enum
    {
        AOS_CONNECTED,
        AOS_DISCONNECTING,
        AOS_DISCONNECTED,
        AOS_SUSPENDED,
        AOS_RESUMED,
    };

    IImsAos*                        m_piImsAos;
    IMS_SINT32                      m_eAoSStatus;
    INetworkWatcher*                m_piNetWatcherInfo;
    ITimer*                         m_piDeBounceTimer;
    IMS_SINT32                      m_RegisteredNetwork;
    IMS_SINT32                      m_eCurrentNetwork;
    UceService*                     m_pUceService;
};
#endif    /* _UCE_APP_H_ */
