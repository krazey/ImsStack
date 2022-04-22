#ifndef MTC_VONR_H_
#define MTC_VONR_H_


#include "INetworkWatcher.h"
#include "IVoNr.h"
#include "ServiceTimer.h"

#include "call/IMtcCall.h"
//#include "call/IMtcCallManager.h"
class IMtcVonrListener;

class MtcVonr :
        public IVoNrUacListener, public IVoNrCallPreferenceListener, public IVoNrHandoffListener,
        public INetworkWatcherListener, public ITimerListener
{
public:
    explicit MtcVonr(IN IMS_UINT32 nSlotId, IN IMtcVonrListener* piListener);
    virtual ~MtcVonr();

    enum class VonrInitType { NONE, LTE, NR, WIFI, EMERGENCY/* differently defined?*/ };

public:
    // IVoNrUacListener interface implementation
    virtual void VoNrUac_NotifyResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
            IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime);

    // IVoNrCallPreferenceListener interface implementation
    virtual void VoNrCallPreference_NotifyCallReady(IN IMS_UINT32 nSysMode);

    // IVoNrHandoffListener interface implementation
    virtual void VoNrHandoff_NotifyInformation(IN IMS_UINT32 nStatus,
            IN IMS_UINT32 nSourceRAT, IN IMS_UINT32 nTargetRAT, IN IMS_SINT32 nReason);

    // INetworkWatcherListener interface implementation
    virtual void NetworkWatcher_NotifyStatus(IN INetworkWatcher* piNetWatcherInfo);

    // ITimerListener interface implementation.
    virtual void Timer_TimerExpired(IN ITimer *piTimer);

    // for MtcVonr child classes
    virtual void CheckBarring(IN IMtcCall* piMtcCall, IN CallType eCallType,
            IN IMS_BOOL bEmergency);

    IMS_UINT32 GetUacType();

protected:
    virtual void OnSessionStopped(IN IMS_UINTP nParam);
    virtual void NotifyCallState(IN IMS_UINT32 nState);
    virtual void OnNotifyUacResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
            IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime);
    virtual void OnNotifyCallPreferenceReady(IN IMS_UINT32 nSysMode);
    virtual IMS_BOOL IsUacCheckRequired();

    void Initialize();
    void RequestCallPreference(IN IMS_UINT32 nRat);
    IMS_RESULT UpdateSessionInfo(IN IMtcCall* piMtcCall);

    void SetUacType(IN CallType eCallType, IN IMS_BOOL bEmergency);
    IMS_UINT32 GetSysMode();
    IMS_UINT32 GetConvertedDirection(IN PeerType ePeerType);
    VonrInitType GetConvertedInitType(IN IMS_UINT32 nSysMode);
    IMtcCall* GetCall();
    IMS_BOOL IsAvailableNetwork(IN IMS_SINT32 nRadiotechType);
    void StartTimer(IN IMS_UINT32 nTime);
    void StopTimer();

protected:
    enum class UacStatus {
            IDLE, WAIT_RESPONSE /*waiting?*/, FAILURE /*failed?*/, SUCCESS /*succeeded?*/ };

    enum { TIME_WAIT_UAC_RESPONSE = 3000 };
    enum { TIME_WAIT_CALL_READY = 5000 };

    IMS_UINT32          m_nSlotId;
    IMtcVonrListener*   m_piListener;

    IVoNr*              m_piVonr;
    IMS_SINTP           m_nCallKey;
    //IMtcCallManager*    m_piCallManager;
    IMS_UINT32          m_nDirection;
    IMS_UINT32          m_nUacType;
    INetworkWatcher*    m_piNetWatcherInfo;
    UacStatus           m_eUacStatus;
    IMS_SINT32          m_nCurrentNetwork;
    ITimer*             m_piTimer;
};
#endif // MTC_VONR_H_
