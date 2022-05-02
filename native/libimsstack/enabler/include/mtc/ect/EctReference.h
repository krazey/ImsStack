/*
 * author : dongo.yi@
 * version : 1.0
 * date : 20150907
 * brief :
 */


#ifndef UC_ECT_REFERENCE_H_
#define UC_ECT_REFERENCE_H_

#include "AString.h"
#include "IMSActivityEx.h"
#include "IMSList.h"

#include "ISession.h"
#include "IMessage.h"
#include "IReference.h"
#include "IReferenceListener.h"

#include "define/MtcInternalMsgDef.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/IMtcTimerListener.h"
#include "ect/IEctReferenceListener.h"
#include "IMtcApp.h"
#include "call/IMtcCall.h"
#include "SipMethod.h"
#include "INotificationListener.h"


class UCECTReference
    : public IReferenceListener
    , public IMSActivityEx
    , public IMtcTimerListener
    , public INotificationListener
{

// ------------------------------------------------------------------------------------------------
// Constructor, Destructor
// ------------------------------------------------------------------------------------------------
public:
    UCECTReference(IN IMtcApp* pApp,
            IN IUCECTReferenceListener* pListener,
            IN IMtcCall* pBySession);
    virtual ~UCECTReference();

private:
    UCECTReference(IN CONST UCECTReference &objRHS);
    UCECTReference& operator=(IN CONST UCECTReference &objRHS);

// ------------------------------------------------------------------------------------------------
// Methods
// ------------------------------------------------------------------------------------------------
public:
    void Init(IN const AString &aStrSipMethod, IN const AString &aStrReferTo,
            IN const AString &aStrReplace = AString::ConstNull(),
            IN const AString &aStrReferToEx = AString::ConstNull());

    //IReferenceListener
    virtual void ReferenceDelivered(IN IReference *piReference);
    virtual void ReferenceDeliveryFailed(IN IReference *piReference);
    virtual void ReferenceNotify(IN IReference * /*piReference*/, IN IMessage *piNotify);
    virtual void ReferenceTerminated(IN IReference * /*piReference*/);

    //INotificationListener
    virtual void NotificationDelivered(IN IServiceMethod *piMethod);
    virtual void NotificationDeliveryFailed(IN IServiceMethod *piMethod,
            IN IMS_SINT32 nStatusCode);

    void SetIReference(IN IReference * piReference);
    void SendNotificationTrying();
    void SendNotificationSuccess();
    void SendNotificationFailure();
    void SendNotificationDeclined();
    IMS_UINT32 GetState();
    IMS_UINT32 GetOldState();
    void SetState(IN IMS_UINT32 eState);

    IMS_BOOL Refer();
    IMS_SINT32 GetFailReason( IN IMS_SINT32 nStatusCode );

    virtual void UCTimer_Expired(IN IMS_SINT32 eType);

protected:
    virtual IMS_BOOL OnMessage(IN IMSMSG& objMSG); // IMSActivityEx class

    IMS_BOOL HandleMessage(IN IMSMSG &objMSG);

    virtual IMS_BOOL CreateRefer();

    virtual void SendDeliveredToListn();
    virtual void SendDeliveryFailedToListn(IN IMS_SINT32 eReason, IN IMS_SINT32 eCode = -1);
    virtual void SendNotifyToListn(IN IMessage* pNotify, IN AString aStrSubState,
            IN IMS_SINT32 nStatusCode, IN AString aStrEventID);
    virtual void SendTerminatedToListn(IN IMS_SINT32 eReason, IN IMS_SINT32 eCode = -1);
    virtual void SendFailedToListn(IN IMS_SINT32 eReason, IN IMS_SINT32 eCode = -1);
    virtual void SendNotifyDeliveredToListn(IN IServiceMethod *piMethod);
    virtual void SendNotifyDeliveryFailedToListn(IN IServiceMethod *piMethod,
            IN IMS_SINT32 nStatusCode);

    virtual void SetReferredByH(IN IMessage* pIMessage);
    virtual AString GetReferredByHrd();
    virtual AString GetReferToExHdr();

    virtual IMS_BOOL SendRefer();

    virtual IMS_BOOL HandleNotify(IN IMessage* pNotify, IN AString aStrSubState,
            IN IMS_SINT32 nStatusCode);
    virtual IMS_BOOL ProcessTimer_Notify_Completed();

    const IMS_CHAR* PrintState(IN IMS_SINT32 eState = -1);

private:

protected:
    virtual void LoadConfig();

// ------------------------------------------------------------------------------------------------
// Variables
// ------------------------------------------------------------------------------------------------
public:
    enum
    {
        REFER_BASE_DEFAULT = MTC_INTERNAL_MSG::REFER_MSG_BASE,

        REFER_REFERENCE_DELIVERED,
        REFER_REFERENCE_DELIVERFILED,
        REFER_REFERENCE_NOTIFY,
        REFER_REFERENCE_TERMINATED,

        REFER_COM_DEFAULT = REFER_BASE_DEFAULT + 50
    };

    enum
    {
        ECT_REFER_IDLE              = 0,
        ECT_REFER_SENT              = 1,
        ECT_REFER_DELIVERED         = 2,
        ECT_REFER_DELIVERYFAILED    = 3,
        ECT_REFER_SUBSCRIBED        = 4,
        ECT_REFER_TERMINATED        = 5
    };

protected:
    enum
    {
        /* Base Timer */
        TIMER_BASE_DEFAULT = 0,
        TIMER_MO_1XX_WAIT = 1,
        TIMER_MO_NOANSWER = 2,
        TIMER_WAIT_NOTIFY = 3,
        TIMER_WAIT_NOTIFYDELIVERY = 4,

        /* Com Timer */
        TIMER_COM_DEFAULT = 20

    }; /*TIMERTYPE */


protected:

    IMS_UINT32                      m_eState;
    IMS_UINT32                      m_eOldState;

    IMtcApp*                         m_pApp;
    IMS_SINT32                      m_nSlotID;
    IUCECTReferenceListener*        m_pListener;

    IMS_BOOL                        m_bReferSub;

    IMtcCall*                     m_pBySession;
    AString                         m_aStrReferTo;
    AString                         m_aStrReplace;
    AString                         m_aStrReferToEx;
    AString                         m_aStrMethod;
    AString                         m_aStrUserID;

    IReference*                     m_pIReference;

    MtcTimerWrapper*                   m_pTimer;
    IMS_SINT32                      m_n1xxWaitTime;
    IMS_SINT32                      m_nFinalWaitTime;

    // CONFIG
    IMS_BOOL                        m_bReferredBy;

private:
    static const IMS_CHAR STR_NOTIFY_100TRYING[];
    static const IMS_CHAR STR_NOTIFY_200OK[];
    static const IMS_CHAR STR_NOTIFY_403FORBIDDEN[];
    static const IMS_CHAR STR_NOTIFY_603DECLINED[];

    static const IMS_SINT32 ECT_NOTIFY_WAIT_TIME = 10000; //ms
    static const IMS_SINT32 ECT_NOTIFYDELIVERY_WAIT_TIME = 5000; //ms
};
#endif /*  UC_ECT_REFERENCE_H_ */
