#ifndef MTS_CLIENT_H_
#define MTS_CLIENT_H_

#include "ByteArray.h"
#include "ITimer.h"
#include "IMtsClientListener.h"
#include "MtsServiceState.h"

class IMtsClient;
class MtsApp;

class MtsClient final :
        public ITimerListener,
        public IMtsClientListener
{
private:
    MtsClient(IN IMS_SINT32 nSlotId);
    ~MtsClient();

public:
    static MtsClient* GetInstance(IN IMS_SINT32 nSlotId);
    static void DestroyMtsClient(IN IMS_SINT32 nSlotId);
    MtsApp* GetMtsApp(IN IMS_SINT32 nSlotId);

    IMS_BOOL HasIpcRouterFeature(IN IMS_SINT32 nSlotId);

    IMS_UINT32 PassReceivedMessage(
            IN const ByteArray& objSms, IN const IMS_UINT32 nSmsType, IN IMS_SINT32 nSlotId);
    void ReportTransmissionResult(
            IN IMS_UINT32 nResponse,
            IN IMS_UINT32 nSmsType,
            IN IMS_SINT32 nSeqId = -1,
            IN IMS_SINT32 nSlotId = -1);
    void ReportTransmissionFailureWithRetryTime(
            IN const IMS_UINT32 nSmsType,
            IN const IMS_UINT8 nRetryTime,
            IN IMS_SINT32 nSeqId = -1,
            IN IMS_SINT32 nSlotId = -1);

    void RequestRegistrationRecovery(IN IMS_SINT32 nRecoveryType, IN IMS_SINT32 nSlotId);
    void SetTempRetryAfterValue(IN IMS_UINT8 nValue);
    void SetServiceState(IN MtsServiceState* pMtsServiceState);

    void ClearTimer();
    void StartTimer(IN IMS_UINT32 nType, IN IMS_UINT32 nDuration);
    void StopTimer(IN IMS_UINT32 nType);

    // ITimerListener
    void Timer_TimerExpired(IN ITimer *piTimer) override;

    // IMtsClientListener
    void Client_SendMo(IN IMSWMS_UINTP nWparam_, IN IWMSSmsSendRequestParam* nLparam) override;

protected:
    void RemoveIMtsClient(IN IMS_SINT32 nSlotId);
    IMtsClient* GetIMtsClient(IN IMS_SINT32 nSlotId);

public:
    // error codes for received sms messages.
    enum
    {
        MT_SUCCESS = 1,
        MT_FAILURE = 2,
        MT_SMS_FORMAT_FAILURE = 3,
        MT_SMS_NODATA_FAILURE = 4
    };

    enum
    {
        MO_IMS_LIMITEDSMSSVCREGI = 1,
        MO_IMS_PERM_FAILURE = 2,
        MO_IMS_TEMP_FAILURE = 3,
        MO_RETRY_CS = 4,
        MO_RETRY_CS_OR_SGS = 5
    };

    // TIMER item definitions
    enum
    {
        TIMER_SMS_CALLBACK_MODE    = 10
    };

protected:
    IMS_SINT32          m_nSlotId;
    IMSList<MtsApp*>    m_lstMtsApp;
    MtsServiceState*    m_pMtsServiceState;

private:
    IMS_UINT8           m_nTempRetryAfterValue;
};

#endif
