/*
 * author : dongo.yi@
 * version : 1.0
 * date : 20150907
 * brief :
 */

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceSystemTime.h"
#include "ImsMessage.h"

#include "ISession.h"
#include "ISessionDescriptor.h"
#include "IReference.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ICoreService.h"

#include "SipParsingHelper.h"
#include "SipParameter.h"
#include "SipAddress.h"
#include "SipStatusCode.h"
#include "SipHeaderName.h"
#include "IMessageBodyPart.h"
#include "IMessage.h"

#include "IuMtcCall.h"

#include "MtcDef.h"
#include "define/MtcStringDef.h"
#include "configuration/ConfigDef.h"
#include "utility/MessageUtil.h"
#include "ect/EctReference.h"
#include "ect/IEctReferenceListener.h"
#include "IMtcService.h"

__IMS_TRACE_TAG_COM_UC__;

const IMS_CHAR UCECTReference::STR_NOTIFY_100TRYING[] = "SIP/2.0 100 Trying";
const IMS_CHAR UCECTReference::STR_NOTIFY_200OK[] = "SIP/2.0 200 OK";
const IMS_CHAR UCECTReference::STR_NOTIFY_403FORBIDDEN[] = "SIP/2.0 403 Forbidden";
const IMS_CHAR UCECTReference::STR_NOTIFY_603DECLINED[] = "SIP/2.0 603 Declined";

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCECTReference::UCECTReference(
        IN IMtcApp* pApp, IN IUCECTReferenceListener* pListener, IN IMtcCall* pBySession) :
        IMSActivityEx(AString::ConstNull()),
        m_eState(ECT_REFER_IDLE),
        m_eOldState(ECT_REFER_IDLE),
        m_pApp(pApp),
        m_nSlotID(0),
        m_pListener(pListener),
        m_bReferSub(IMS_TRUE),
        m_pBySession(pBySession),
        m_aStrReferTo(AString::ConstNull()),
        m_aStrReplace(AString::ConstNull()),
        m_aStrReferToEx(AString::ConstNull()),
        m_aStrMethod(AString::ConstNull()),
        m_aStrUserID(AString::ConstNull()),
        m_pIReference(IMS_NULL),
        m_pTimer(new MtcTimerWrapper()),
        m_n1xxWaitTime(0),
        m_nFinalWaitTime(0),
        m_bReferredBy(IMS_FALSE)
{
    // TODO, MTC BUILD
    // IMS_TRACE_MEM("uc", "uc_M[%d] : UCECTReference[%" PFLS_u "][%" PFLS_x "]",
    //         m_pApp->GetSlotID(), sizeof(UCECTReference), this);

    IMS_TRACE_D(
            "+UCECTReference : To[%s] By[%" PFLS_u "]", m_aStrReferTo.GetStr(), m_pBySession, 0);
    SetState(ECT_REFER_IDLE);

    // TODO, MTC BUILD
    m_nSlotID = 0;
    // m_nSlotID = m_pApp->GetSlotID();
    m_pTimer->SetListener(this);

    m_n1xxWaitTime = 60000;
    m_nFinalWaitTime = 60000;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL UCECTReference::~UCECTReference()
{
    IMS_TRACE_MEM("uc", "uc_F[%d] : UCECTReference[%" PFLS_u "][%" PFLS_x "]", m_nSlotID,
            sizeof(UCECTReference), this);

    if (m_pIReference != IMS_NULL)
    {
        m_pIReference->SetNotificationListener(IMS_NULL);
        m_pIReference->Destroy();
        m_pIReference = IMS_NULL;
    }

    if (m_pTimer != IMS_NULL)
    {
        delete m_pTimer;
        m_pTimer = IMS_NULL;
    }

    if (m_pListener != IMS_NULL)
    {
        m_pListener = IMS_NULL;
    }

    if (m_pBySession != IMS_NULL)
    {
        m_pBySession = IMS_NULL;
    }

    if (m_pApp != IMS_NULL)
    {
        m_pApp = IMS_NULL;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCECTReference::Init(IN const AString& aStrSipMethod, IN const AString& aStrReferTo,
        IN const AString& aStrReplace /*= AString::ConstNull()*/,
        IN const AString& aStrReferToEx /*= AString::ConstNull()*/)
{
    IMS_TRACE_I("Init : [%s]", aStrSipMethod.GetStr(), 0, 0);
    IMS_TRACE_D("Init : [%s][%s]", aStrReferTo.GetStr(), aStrReplace.GetStr(), 0);

    LoadConfig();

    m_aStrMethod = aStrSipMethod;
    m_aStrReferTo = aStrReferTo;
    m_aStrReplace = aStrReplace;
    m_aStrReferToEx = aStrReferToEx;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::ReferenceDelivered(IN IReference* piReference)
{
    IMS_SINT32 eFailReason = FAIL_REASON_NONE;

    SetState(ECT_REFER_DELIVERED);

    IMSList<IMessage*> objListIMsg = piReference->GetPreviousResponses(IMessage::REFERENCE_REFER);
    if (objListIMsg.IsEmpty())
    {
        // time out
        IMS_TRACE_I("ReferenceDelivered[%s] : Refer time out", PrintState(m_eOldState), 0, 0);
        eFailReason = FAIL_REASON_CONF_TIMEOUT;
    }
    else
    {
        IMessage* pLastMsg = piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);

        if (pLastMsg != IMS_NULL)
        {
            IMS_SINT32 nStatusCode = pLastMsg->GetStatusCode();
            IMS_TRACE_I("ReferenceDelivered[%s] : Response[%d]", PrintState(m_eOldState),
                    nStatusCode, 0);

            if (SipStatusCode::IsFinalSuccess(nStatusCode))
            {
            }
            else
            {
                eFailReason = GetFailReason(nStatusCode);
            }
        }
        else
        {
            IMS_TRACE_E(
                    0, "ReferenceDelivered[%s] : IMessage is NULL", PrintState(m_eOldState), 0, 0);
            eFailReason = FAIL_REASON_CONF_INTERNAL_ERROR;
        }
    }

    if (eFailReason == FAIL_REASON_NONE)
    {
        if (m_bReferSub)
        {
            SetState(ECT_REFER_SUBSCRIBED);
        }

        SendDeliveredToListn();
    }
    else
    {
        SetState(ECT_REFER_DELIVERYFAILED);
        SendDeliveryFailedToListn(eFailReason);
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::ReferenceDeliveryFailed(IN IReference* piReference)
{
    IMS_TRACE_I("ReferenceDeliveryFailed[%s]", PrintState(), 0, 0);

    SetState(ECT_REFER_DELIVERYFAILED);

    IMessage* pLastMsg = piReference->GetPreviousResponse(IMessage::REFERENCE_REFER);

    if (pLastMsg != IMS_NULL)
    {
        IMS_SINT32 nStatusCode = pLastMsg->GetStatusCode();
        SendDeliveryFailedToListn(GetFailReason(nStatusCode));
    }
    else
    {
        SendDeliveryFailedToListn(FAIL_REASON_CONF_INTERNAL_ERROR);
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::ReferenceNotify(
        IN IReference* /*piReference*/, IN IMessage* piNotify)
{
    AString aStrSubState;
    MessageUtil::GetHeaderValue(piNotify, ISipHeader::SUBSCRIPTION_STATE, aStrSubState);
    IMS_SINT32 nStatusCode = MessageUtil::GetStatusCodeInNotify(piNotify);
    AString aStrID;
    MessageUtil::GetParameterValue(piNotify, MessageUtil::STR_ID, ISipHeader::EVENT, aStrID);

    IMS_TRACE_I("ReferenceNotify[%s]", PrintState(), 0, 0);

    HandleNotify(piNotify, aStrSubState, nStatusCode);
    SendNotifyToListn(piNotify, aStrSubState, nStatusCode, aStrID);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::ReferenceTerminated(IN IReference* /*piReference*/)
{
    IMS_TRACE_I("ReferenceTerminated[%s]", PrintState(), 0, 0);

    if (GetState() == ECT_REFER_TERMINATED)
    {
        return;
    }

    SetState(ECT_REFER_TERMINATED);
    SendTerminatedToListn(FAIL_REASON_NONE);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::NotificationDelivered(IN IServiceMethod* piMethod)
{
    IMS_TRACE_I("NotificationDelivered[%s]", PrintState(), 0, 0);

    m_pTimer->Stop(TIMER_WAIT_NOTIFYDELIVERY);
    m_pIReference->SetNotificationListener(IMS_NULL);
    SendNotifyDeliveredToListn(piMethod);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::NotificationDeliveryFailed(
        IN IServiceMethod* piMethod, IN IMS_SINT32 nStatusCode)
{
    IMS_TRACE_I("NotificationDeliveryFailed[%s] : nStatusCode[%d]", PrintState(), nStatusCode, 0);

    m_pTimer->Stop(TIMER_WAIT_NOTIFYDELIVERY);
    m_pIReference->SetNotificationListener(IMS_NULL);
    SendNotifyDeliveryFailedToListn(piMethod, nStatusCode);
}

PUBLIC
void UCECTReference::SetIReference(IN IReference* pIReference)
{
    m_pIReference = pIReference;
    m_pIReference->SetNotificationListener(this);
}

PUBLIC
void UCECTReference::SendNotificationTrying()
{
    m_pTimer->Start(TIMER_WAIT_NOTIFYDELIVERY, ECT_NOTIFYDELIVERY_WAIT_TIME);
    m_pIReference->SendNotification(ISubscriptionState::STATE_ACTIVE, STR_NOTIFY_100TRYING,
            ISubscriptionState::REASON_NONE);
}

PUBLIC
void UCECTReference::SendNotificationSuccess()
{
    m_pTimer->Start(TIMER_WAIT_NOTIFYDELIVERY, ECT_NOTIFYDELIVERY_WAIT_TIME);
    m_pIReference->SendNotification(ISubscriptionState::STATE_TERMINATED, STR_NOTIFY_200OK,
            ISubscriptionState::REASON_NONE);
}

PUBLIC
void UCECTReference::SendNotificationFailure()
{
    m_pIReference->SendNotification(ISubscriptionState::STATE_TERMINATED, STR_NOTIFY_403FORBIDDEN,
            ISubscriptionState::REASON_NONE);
}

PUBLIC
void UCECTReference::SendNotificationDeclined()
{
    m_pIReference->SendNotification(ISubscriptionState::STATE_TERMINATED, STR_NOTIFY_603DECLINED,
            ISubscriptionState::REASON_NONE);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_UINT32 UCECTReference::GetState()
{
    return m_eState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_UINT32 UCECTReference::GetOldState()
{
    return m_eOldState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCECTReference::SetState(IN IMS_UINT32 eState)
{
    IMS_TRACE_I("SetState : [%s] -> [%s]", PrintState(m_eState), PrintState(eState), 0);

    m_eOldState = m_eState;
    m_eState = eState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL UCECTReference::Refer()
{
    IMS_TRACE_I("Refer", 0, 0, 0);

    if (!CreateRefer())
    {
        return IMS_FALSE;
    }

    SendRefer();

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_SINT32 UCECTReference::GetFailReason(IN IMS_SINT32 nStatusCode)
{
    IMS_SINT32 nReason = FAIL_REASON_NONE;

    switch (nStatusCode)
    {
        default:
            nReason = FAIL_REASON_UNKNOWN;
            break;
    }
    IMS_TRACE_I("GetFailReason - nReason[%d]", nReason, 0, 0);
    return nReason;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCECTReference::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("UCECTReference : Msg[%d]", objMSG.nMSG, 0, 0);

    HandleMessage(objMSG);

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void UCECTReference::UCTimer_Expired(IN IMS_SINT32 eType)
{
    // TODO, MTC BUILD
    // IMS_UINT32 eSize = m_pTimer->GetSize();

    // IMS_TRACE_I("UCTimer_Expired : Size[%d] Type[%d]", eSize, eType, 0);

    switch (eType)
    {
        case TIMER_MO_1XX_WAIT:
        case TIMER_MO_NOANSWER:
            ProcessTimer_Notify_Completed();
            break;
        case TIMER_WAIT_NOTIFYDELIVERY:
            SendNotifyDeliveryFailedToListn(IMS_NULL, 0);
            break;

        default:
            break;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IMS_BOOL UCECTReference::HandleMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("HandleMessage : Msg[%d]", objMSG.nMSG, 0, 0);

    switch (objMSG.nMSG)
    {
        default:
            break;
    }

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCECTReference::CreateRefer()
{
    if (m_pIReference != IMS_NULL)
    {
        return IMS_TRUE;
    }

    // TODO, MTC BUILD
    // if (m_pBySession->GetISession() == IMS_NULL)
    // {
    //     IMS_TRACE_E( 0, "ISession is NULL.", 0, 0, 0 );
    //     return IMS_FALSE;
    // }

    // m_pIReference = m_pBySession->GetISession()->CreateReference(m_aStrReferTo, m_aStrMethod);
    if (m_pIReference == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pIReference is NULL.", 0, 0, 0);
        return IMS_FALSE;
    }
    m_pIReference->SetListener(this);

    IMS_TRACE_I("CreateRefer", 0, 0, 0);
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendDeliveredToListn()
{
    IMS_TRACE_I("SendDeliveredToListn", 0, 0, 0);

    IECTReferListenDeliveredParam* pParam = new IECTReferListenDeliveredParam();
    pParam->pRefer = this;
    m_pListener->Refer_Delivered((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendDeliveryFailedToListn(
        IN IMS_SINT32 eReason, IN IMS_SINT32 eCode /*= -1*/)
{
    IMS_TRACE_I("SendDeliveryFailedToListn : Reason[%d]Code[%d]", eReason, eCode, 0);

    IECTReferListenDeliveryFailedParam* pParam = new IECTReferListenDeliveryFailedParam();
    pParam->pRefer = this;

    pParam->eReason = eReason;
    pParam->eCode = eCode;

    m_pListener->Refer_DeliveryFailed((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendNotifyToListn(IN IMessage* /*pNotify*/,
        IN AString aStrSubState, IN IMS_SINT32 nStatusCode, IN AString aStrEventID)
{
    IMS_TRACE_I("SendNotifyToListn ", 0, 0, 0);

    IECTReferListenNotifyParam* pParam = new IECTReferListenNotifyParam();
    pParam->pRefer = this;

    pParam->aStrSubState = aStrSubState;
    pParam->nStatusCode = nStatusCode;
    pParam->aStrEventID = aStrEventID;

    m_pListener->Refer_Notify((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendTerminatedToListn(
        IN IMS_SINT32 eReason, IN IMS_SINT32 eCode /*= -1*/)
{
    IMS_TRACE_I("SendTerminatedToListn : Reason[%d]Code[%d]", eReason, eCode, 0);

    IECTReferListenTerminatedParam* pParam = new IECTReferListenTerminatedParam();
    pParam->pRefer = this;

    pParam->eReason = eReason;
    pParam->eCode = eCode;

    m_pListener->Refer_Terminated((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendFailedToListn(
        IN IMS_SINT32 eReason, IN IMS_SINT32 eCode /*= -1*/)
{
    IMS_TRACE_I("SendFailedToListn : Reason[%d]Code[%d]", eReason, eCode, 0);

    IECTReferListenFailedParam* pParam = new IECTReferListenFailedParam();
    pParam->pRefer = this;

    pParam->eReason = eReason;
    pParam->eCode = eCode;

    m_pListener->Refer_Failed((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendNotifyDeliveredToListn(IN IServiceMethod* /*piMethod*/)
{
    IMS_TRACE_I("SendNotifyDeliveredToListn ", 0, 0, 0);

    IECTReferListenNotifyDeliveredParam* pParam = new IECTReferListenNotifyDeliveredParam();
    pParam->pRefer = this;

    m_pListener->Refer_Notify_Delivered((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SendNotifyDeliveryFailedToListn(
        IN IServiceMethod* /*piMethod*/, IN IMS_SINT32 nStatusCode)
{
    IMS_TRACE_I("SendNotifyDeliveryFailedToListn ", 0, 0, 0);

    IECTReferListenNotifyDeliveryFailedParam* pParam =
            new IECTReferListenNotifyDeliveryFailedParam();
    pParam->pRefer = this;
    pParam->nStatusCode = nStatusCode;

    m_pListener->Refer_Notify_DeliveryFailed((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::SetReferredByH(IN IMessage* pIMessage)
{
    AString aStrReferBy = GetReferredByHrd();

    if (aStrReferBy.IsEmpty() || aStrReferBy.IsNULL())
    {
        IMS_TRACE_I("SetReferredByH : aStrReferBy is NULL", 0, 0, 0);
        return;
    }

    ISipMessage* pISIPMessage = pIMessage->GetMessage();
    if (pISIPMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "pSIPMsg is Null", 0, 0, 0);
        return;
    }

    IMS_TRACE_D("SetReferredByH : [%s]", aStrReferBy.GetStr(), 0, 0);
    pISIPMessage->AddHeader(ISipHeader::REFERRED_BY, aStrReferBy);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL AString UCECTReference::GetReferredByHrd()
{
    AString aStrReferBy = AString::ConstNull();

    if (m_pBySession == IMS_NULL)
    {
        IMS_TRACE_I("GetReferredByHrd : m_pBySession is NULL", 0, 0, 0);
        return aStrReferBy;
    }

    // TODO, MTC BUILD
    // aStrReferBy = m_pBySession->GetService()->GetICoreService()->GetLocalUserId();
    if (aStrReferBy == AString::ConstEmpty() || aStrReferBy == AString::ConstNull())
    {
        IMS_TRACE_E(0, "aStrReferBy is NULL", 0, 0, 0);
    }

    IMS_TRACE_D("GetReferredByHrd [%s]", aStrReferBy.GetStr(), 0, 0);
    return aStrReferBy;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL AString UCECTReference::GetReferToExHdr()
{
    IMS_TRACE_D("GetReferToExHdr : Ext[%s]", m_aStrReferToEx.GetStr(), 0, 0);
    return m_aStrReferToEx;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCECTReference::SendRefer()
{
    IMS_TRACE_I(
            "SendRefer : Sub[%s] ReferredBy[%s]", PS_BOOL(m_bReferSub), PS_BOOL(m_bReferredBy), 0);

    if (m_bReferredBy)
    {
        SetReferredByH(m_pIReference->GetNextRequest());
    }

    if (!m_aStrReplace.IsNULL() && !m_aStrReplace.IsEmpty())
    {
        m_pIReference->SetReplaces(m_aStrReplace);
    }
    m_pIReference->ReferEx(m_bReferSub, GetReferToExHdr());

    if (m_bReferSub)
    {
        m_pTimer->Start(TIMER_MO_1XX_WAIT, m_n1xxWaitTime);
    }

    SetState(ECT_REFER_SENT);
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCECTReference::HandleNotify(
        IN IMessage* /*pNotify*/, IN AString aStrSubState, IN IMS_SINT32 nStatusCode)
{
    IMS_TRACE_I("HandleNotify", 0, 0, 0);

    m_pTimer->Stop(TIMER_WAIT_NOTIFY);

    if (SipStatusCode::Is1XX(nStatusCode))
    {
        m_pTimer->Stop(TIMER_MO_1XX_WAIT);
        m_pTimer->Start(TIMER_MO_NOANSWER, m_nFinalWaitTime);
    }

    if (aStrSubState.Equals("terminated") || SipStatusCode::IsFinal(nStatusCode))
    {
        // TODO, MTC BUILD
        // m_pTimer->AllStop();
    }

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL UCECTReference::ProcessTimer_Notify_Completed()
{
    IMS_TRACE_I("ProcessTimer_Notify_Completed :", 0, 0, 0);

    // TODO, MTC BUILD
    // m_pTimer->AllStop();
    SendFailedToListn(0 /*FAIL_REASON_CONF_TIMEOUT*/);
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
const IMS_CHAR* UCECTReference::PrintState(IN IMS_SINT32 eState /*= -1*/)
{
    if (eState <= -1)
    {
        eState = m_eState;
    }

    switch (eState)
    {
        case ECT_REFER_IDLE:
            return "ECT_REFER_IDLE";
        case ECT_REFER_SENT:
            return "ECT_REFER_SENT";
        case ECT_REFER_DELIVERED:
            return "ECT_REFER_DELIVERED";
        case ECT_REFER_DELIVERYFAILED:
            return "ECT_REFER_DELIVERYFAILED";
        case ECT_REFER_SUBSCRIBED:
            return "ECT_REFER_SUBSCRIBED";
        case ECT_REFER_TERMINATED:
            return "ECT_REFER_TERMINATED";
        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void UCECTReference::LoadConfig()
{
    m_bReferSub = IMS_TRUE;
    m_bReferredBy = IMS_TRUE;

    IMS_TRACE_I("loadConfig : [%s][%s]", PS_BOOL(m_bReferSub), PS_BOOL(m_bReferSub), 0);
}
