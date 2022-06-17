/*
 * author : hyunbin.shin@
 * version : 2.0
 * date : 201503
 * brief :
 */

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMessage.h"
#include "ServiceTimer.h"

#include "ICoreService.h"
#include "ISession.h"
#include "ISubscription.h"
#include "ISipMessage.h"
#include "ISipHeader.h"
#include "SipStatusCode.h"
#include "IMessageBodyPart.h"
#include "IMessage.h"
#include "TextParser.h"

#include "DomDocumentBuilderFactory.h"
#include "DocumentBuilder.h"
#include "IElement.h"
#include "INodeList.h"
#include "INamedNodeMap.h"
#include "IText.h"

#include "IuMtcService.h"
#include "IuMtcCall.h"
#include "configuration/ConfigDef.h"
#include "utility/MessageUtil.h"
#include "call/IMtcCall.h"
#include "dialogevent/DialogEventManager.h"
#include "dialogevent/DialogEvent.h"
#include "helper/CallStateProxy.h"

__IMS_TRACE_TAG_COM_UC__;

/* ------------------------------------------------------------------------------------------------
    Constructor, Destructor
------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
DEMngr::DEMngr(IN IMtcApp* pApp) :
        IMSActivityEx(AString::ConstNull()),
        m_pApp(pApp),
        m_pListener(IMS_NULL),
        m_pTimer(IMS_NULL),
        m_aStrJNIServiceName(AString::ConstNull()),
        m_bDestroy(IMS_FALSE),
        m_eCallState(0),
        m_nSlotID(0),
        m_pService(IMS_NULL),
        m_pISubscription(IMS_NULL),
        m_objDialogs(IMSMap<AString, IDialogEvent*>()),
        m_nExpireTime(7200),
        m_nVersion(0),
        m_eState(0),
        m_aStrEntity(AString::ConstNull()),
        m_eTerminatedReason(0),
        m_nRetryAfter(0)
{
    // TODO, MTC BUILD
    // IMS_TRACE_MEM("uc", "uc_M[%d] : DEMngr[%" PFLS_u "][%" PFLS_x "]", m_pApp->GetSlotID(),
    //         sizeof(DEMngr), this);

    m_pService = IMS_NULL;
    m_pISubscription = IMS_NULL;
    m_objDialogs.Clear();
    m_aStrJNIServiceName = AString::ConstNull();
    m_bDestroy = IMS_FALSE;
    m_eCallState = 0;
    // TODO, MTC BUILD
    m_nSlotID = 0;
    // m_nSlotID = m_pApp->GetSlotID();

    m_pTimer = new MtcTimerWrapper();
    m_pTimer->SetListener(this);

    m_nVersion = 0;
    m_eState = DIALOGINFO_STATE_IDLE;
    m_aStrEntity = AString::ConstNull();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
DEMngr::~DEMngr()
{
    IMS_TRACE_MEM(
            "uc", "uc_F[%d] : DEMngr[%" PFLS_u "][%" PFLS_x "]", m_nSlotID, sizeof(DEMngr), this);

    DestroyAllDialog();

    if (m_pTimer != IMS_NULL)
    {
        delete m_pTimer;
        m_pTimer = IMS_NULL;
    }

    if (m_pISubscription != IMS_NULL)
    {
        m_pISubscription->Destroy();
        m_pISubscription = IMS_NULL;
    }

    if (m_pListener != IMS_NULL)
    {
        m_pListener = IMS_NULL;
    }

    if (m_pService != IMS_NULL)
    {
        m_pService = IMS_NULL;
    }

    if (m_pApp != IMS_NULL)
    {
        m_pApp = IMS_NULL;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::Init(IN IDEMngrListener* pListener)
{
    IMS_TRACE_D("Init", 0, 0, 0);

    m_pListener = pListener;

    LoadConfig();

    m_eTerminatedReason = DIALOG_TERMINATED_REASON_NONE;
    m_nRetryAfter = 0;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);

    if (m_pTimer)
    {
        m_pTimer->Stop(TIMER_BASE_TERMINATED_RETRY);
    }

    // TODO, MTC BUILD
    // CallStateProxy::GetInstance()->RemoveListener(m_nSlotID, this);
    DeleteEventListn();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::Start(IN AString aStrJNIServiceName)
{
    IMS_TRACE_I("Start : [%s]", aStrJNIServiceName.GetStr(), 0, 0);

    AddEventListn();
    // TODO, MTC BUILD
    // CallStateProxy::GetInstance()->AddListener(m_nSlotID, this);

    m_aStrJNIServiceName = aStrJNIServiceName;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL DEMngr::Stop(IN IMS_BOOL bDestroy /*= IMS_FALSE*/)
{
    IMS_BOOL bReturn = IMS_FALSE;

    bReturn = UnSubscribe();

    // TODO, MTC BUILD
    // CallStateProxy::GetInstance()->RemoveListener(m_nSlotID, this);
    DeleteEventListn();

    IMS_TRACE_I("Stop : Destroy[%s][%s] [%s]", PS_BOOL(m_bDestroy), PS_BOOL(bDestroy),
            PS_BOOL(bReturn));

    m_bDestroy = bDestroy;
    return bReturn;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL IMS_BOOL DEMngr::OnMessage(IN IMSMSG& objMSG)
{
    IMS_TRACE_I("OnMessage[%d]", objMSG.nMSG, 0, 0);

    switch (objMSG.nMSG)
    {
        default:
            HandleThisMsg(objMSG);
            break;
    }

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::SubscriptionForkedNotify(
        IN ISubscription* piSubscription, IN ISubscription* piForkedSubscription)
{
    PostMessage(DEMNGR_S_FORKEDNOTIFY, (IMS_UINTP)piSubscription, (IMS_UINTP)piForkedSubscription);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::SubscriptionNotify(
        IN ISubscription* piSubscription, IN IMessage* piNotify)
{
    IMS_BOOL bUpdated = IMS_FALSE;
    IMSList<IMessageBodyPart*> objBodyParts = piNotify->GetBodyParts();
    AString aStrSubState;
    MessageUtil::GetHeader(piNotify, ISipHeader::SUBSCRIPTION_STATE, aStrSubState);

    // set terminated reason & retry-after
    if (HandleSubState(aStrSubState))
    {
        IMS_TRACE_I("SubscriptionNotify - SubState[%s]", aStrSubState.GetStr(), 0, 0);
        return;
    }

    for (IMS_UINT32 index = 0; index < objBodyParts.GetSize(); index++)
    {
        IMessageBodyPart* pIBodyPart = objBodyParts.GetAt(index);
        if (pIBodyPart == IMS_NULL)
        {
            IMS_TRACE_D("SubscriptionNotify : pIBodyPart[%d] is NULL", index, 0, 0);
            continue;
        }

        AString strDialogInfo = pIBodyPart->GetContent().ToString();
        if (strDialogInfo.IsNULL())
        {
            IMS_TRACE_D("strDialogInfo is NULL : objBodyParts[%d] out of [%d]", index,
                    objBodyParts.GetSize(), 0);
            continue;
        }

        if (HandleDialogInfo(strDialogInfo))
        {
            bUpdated = IMS_TRUE;
        }
    }

    IMS_TRACE_I("SubscriptionNotify : Update[%s]", PS_BOOL(bUpdated), 0, 0);

    if (bUpdated)
    {
        PostMessage(DEMNGR_S_NOTIFY, reinterpret_cast<IMS_UINTP>(piSubscription), 0);
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::SubscriptionStarted(IN ISubscription* piSubscription)
{
    PostMessage(DEMNGR_S_STARTED, (IMS_UINTP)piSubscription, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::SubscriptionStartFailed(IN ISubscription* piSubscription)
{
    PostMessage(DEMNGR_S_STARTFAILED, (IMS_UINTP)piSubscription, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::SubscriptionTerminated(IN ISubscription* piSubscription)
{
    PostMessage(DEMNGR_S_TERMINATED, (IMS_UINTP)piSubscription, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL void DEMngr::UCTimer_Expired(IN IMS_SINT32 eType)
{
    // TODO, MTC BUILD
    // IMS_UINT32 eSize = m_pTimer->GetSize();
    // IMS_TRACE_I("HandleTimerExpired : Size[%d] Type[%d]", eSize, eType, 0);

    switch (eType)
    {
        case TIMER_BASE_TERMINATED_RETRY:
        {
            if (m_pTimer)
            {
                m_pTimer->Stop(TIMER_BASE_TERMINATED_RETRY);
            }

            Subscribe(m_pService->GetICoreService());
            break;
        }

        default:
            break;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL DEMngr::UpdateService(IN IMtcService* pService)
{
    if (pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateService : m_pService is NULL.", 0, 0, 0);
        m_pService = IMS_NULL;
        return IMS_FALSE;
    }

    if (m_pService == pService)
    {
        IMS_TRACE_D("UpdateService : m_pService == pService", 0, 0, 0);
        return IMS_FALSE;
    }

    ICoreService* pICoreService = pService->GetICoreService();
    if (pICoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "UpdateService : pICoreService is not NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("UpdateService : [%" PFLS_u "]", pService, 0, 0);

    m_pService = pService;

    if (Subscribe(pICoreService))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL DEMngr::Subscribe(IN ICoreService* pICoreService)
{
    if (pICoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Subscribe : pICoreService is NULL.", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pService == IMS_NULL)
    {
        IMS_TRACE_E(0, "Subscribe : m_pService is NULL.", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pISubscription != IMS_NULL)
    {
        IMS_TRACE_D("Subscribe : m_pISubscription is not NULL", 0, 0, 0);
        UnSubscribe();
        m_pISubscription->Destroy();
        m_pISubscription = IMS_NULL;
    }

    m_pISubscription = pICoreService->CreateSubscription(GetFromURI(), GetToURI(), "dialog");
    if (m_pISubscription == IMS_NULL)
    {
        IMS_TRACE_E(0, "m_pISubscription is NULL.", 0, 0, 0);
        return IMS_FALSE;
    }
    m_pISubscription->SetListener(this);

    SetHeaderSubscribe();

    if (m_pISubscription->Subscribe() == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "Subscribe : Subscribe is FAIL", 0, 0, 0);
        return IMS_FALSE;
    }

    m_eTerminatedReason = DIALOG_TERMINATED_REASON_NONE;
    m_nRetryAfter = 0;

    if (m_pTimer)
    {
        m_pTimer->Stop(TIMER_BASE_TERMINATED_RETRY);
    }

    IMS_TRACE_I("Subscribe : [%" PFLS_u "]", pICoreService, 0, 0);
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_BOOL DEMngr::UnSubscribe()
{
    if (m_pISubscription == IMS_NULL)
    {
        IMS_TRACE_E(0, "UnSubscribe : m_pISubscription is NULL.", 0, 0, 0);
        return IMS_FALSE;
    }

    SetHeaderUnSubscribe();

    if (m_pISubscription->Unsubscribe() == IMS_FAILURE)
    {
        IMS_TRACE_E(0, "UnSubscribe : Subscribe is FAIL", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("UnSubscribe : [%" PFLS_u "]", m_pISubscription, 0, 0);
    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IDialogEvent* DEMngr::CreateDialog(IN IElement* pDialogElement)
{
    IDialogEvent* pDialog = IMS_NULL;
    AString aStrID = AString::ConstNull();

    pDialog = Dialog_Create(pDialogElement);
    if (pDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateDialog : pDialog is NULL", 0, 0, 0);
        return IMS_NULL;
    }
    aStrID = pDialog->GetID();

    IMS_TRACE_D("CreateDialog [%s][%" PFLS_u "]", aStrID.GetStr(), pDialog, 0);
    m_objDialogs.Add(aStrID, pDialog);
    return pDialog;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IDialogEvent* DEMngr::GetDialog(IN IElement* pDialogElement)
{
    for (IMS_UINT32 index = 0; index < m_objDialogs.GetSize(); index++)
    {
        IDialogEvent* pDialog = m_objDialogs.GetValueAt(index);
        if (pDialog == IMS_NULL)
        {
            continue;
        }

        if (pDialog->IsDialog(pDialogElement))
        {
            IMS_TRACE_I("GetDialog : Dialogs[%d] Index[%d]", m_objDialogs.GetSize(), index, 0);
            return pDialog;
        }
    }

    IMS_TRACE_I("GetDialog : Dialogs[%d] Not Found", m_objDialogs.GetSize(), 0, 0);
    return IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IDialogEvent* DEMngr::GetDialog(IN AString aStrID)
{
    for (IMS_UINT32 index = 0; index < m_objDialogs.GetSize(); index++)
    {
        IDialogEvent* pDialog = m_objDialogs.GetValueAt(index);
        if (pDialog == IMS_NULL)
        {
            continue;
        }

        if (pDialog->GetID().Equals(aStrID))
        {
            IMS_TRACE_I("GetDialog : Dialogs[%d] Index[%d] [%s]", m_objDialogs.GetSize(), index,
                    aStrID.GetStr());
            return pDialog;
        }
    }

    IMS_TRACE_D("GetDialog : Dialogs[%d] [%s]", m_objDialogs.GetSize(), aStrID.GetStr(), 0);
    return IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void DEMngr::DestroyDialog(IN AString aStrID)
{
    IDialogEvent* pDialog = IMS_NULL;

    pDialog = m_objDialogs.GetValue(aStrID);
    if (pDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "DestroyDialog : Dialog is NULL", 0, 0, 0);
        return;
    }

    pDialog->DeInit();

    UCDialog* pUCDialog = reinterpret_cast<UCDialog*>(pDialog);
    delete pUCDialog;
    pUCDialog = IMS_NULL;

    m_objDialogs.Remove(aStrID);
    IMS_TRACE_D("DestroyDialog : [%s] Dialogs[%d]", aStrID.GetStr(), m_objDialogs.GetSize(), 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void DEMngr::DestroyAllDialog()
{
    for (IMS_UINT32 index = 0; index < m_objDialogs.GetSize(); index++)
    {
        IDialogEvent* pDialog = m_objDialogs.GetValueAt(index);
        if (pDialog == IMS_NULL)
        {
            continue;
        }

        pDialog->DeInit();

        UCDialog* pUCDialog = reinterpret_cast<UCDialog*>(pDialog);
        delete pUCDialog;
        pUCDialog = IMS_NULL;
    }

    IMS_TRACE_I("DestroyAllDialog : Dialogs[%d]", m_objDialogs.GetSize(), 0, 0);
    m_objDialogs.Clear();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleThisMsg(IN IMSMSG& objMSG)
{
    switch (objMSG.nMSG)
    {
        case DEMNGR_S_FORKEDNOTIFY:
            HandleForkedNotify(objMSG);
            break;
        case DEMNGR_S_NOTIFY:
            HandleNotify(objMSG);
            break;
        case DEMNGR_S_STARTED:
            HandleStarted(objMSG);
            break;
        case DEMNGR_S_STARTFAILED:
            HandleStartFailed(objMSG);
            break;
        case DEMNGR_S_TERMINATED:
            HandleTerminated(objMSG);
            break;
        case DEMNGR_CALLSTATE_CHANGED:
            HandleChangedCallState(objMSG);
            break;
        case DEMNGR_CALLSTATE_TOTALCHANGED:
            HandleChangedTotalCallState(objMSG);
            break;

        default:
            break;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IDialogEvent* DEMngr::Dialog_Create(IN IElement* pDialogElement)
{
    IDialogEvent* pDialog = IMS_NULL;

    pDialog = Dialog_CreateCom();
    if (pDialog == IMS_NULL)
    {
        IMS_TRACE_E(0, "Dialog_Create : Dialog is NULL", 0, 0, 0);
        return IMS_NULL;
    }
    pDialog->Init(pDialogElement);

    IMS_TRACE_I("Dialog_Create : [%" PFLS_u "]", pDialog, 0, 0);
    return pDialog;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IDialogEvent* DEMngr::Dialog_CreateCom()
{
    return new UCDialog(m_pApp);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL AString DEMngr::GetFromURI()
{
    AString aStrURI = AString::ConstNull();

    return aStrURI;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL AString DEMngr::GetToURI()
{
    AString aStrURI = AString::ConstNull();

    return aStrURI;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL AString DEMngr::GetRequestURI()
{
    AString aStrURI = AString::ConstNull();

    return aStrURI;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::SetHeaderSubscribe()
{
    IMessage* pIMessage = IMS_NULL;

    pIMessage = m_pISubscription->GetNextRequest();
    if (pIMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetHeaderSubscribe : pIMessage is NULL.", 0, 0, 0);
        return;
    }

    ISipMessage* pISIPMessage = pIMessage->GetMessage();
    if (pISIPMessage == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetHeaderSubscribe : pISIPMessage is NULL.", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("SetHeaderSubscribe", 0, 0, 0);

    if (m_nExpireTime > 0)
    {
        AString aStrExpireTime = AString::ConstNull();
        aStrExpireTime.SetNumber(m_nExpireTime);
        pISIPMessage->SetHeader(ISipHeader::EXPIRES_SEC, aStrExpireTime);
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::SetHeaderUnSubscribe() {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleForkedNotify(IN IMSMSG& /*objMSG*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleNotify(IN IMSMSG& objMSG)
{
    ISubscription* pISubscription = reinterpret_cast<ISubscription*>(objMSG.nWparam);

    IMS_TRACE_I("HandleNotify :: SubscriptionState[%d]", pISubscription->GetState(), 0, 0);

    SendNotifyInfoToUI(GetDialogInfos());
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleStarted(IN IMSMSG& objMSG)
{
    ISubscription* pISubscription = reinterpret_cast<ISubscription*>(objMSG.nWparam);

    IMS_TRACE_I("HandleStarted :: SubscriptionState[%d]", pISubscription->GetState(), 0, 0);

    HandleUnSubCompleted(pISubscription);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleStartFailed(IN IMSMSG& objMSG)
{
    IMS_UINT32 nStatusCode = SipStatusCode::SC_INVALID;
    ISubscription* pISubscription = reinterpret_cast<ISubscription*>(objMSG.nWparam);
    IMessage* pIMessage = pISubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE);
    if (pIMessage != IMS_NULL)
    {
        nStatusCode = pIMessage->GetStatusCode();
    }

    if (HandleFailureRes(pISubscription, nStatusCode))
    {
        return;
    }

    if (m_pISubscription != IMS_NULL)
    {
        m_pISubscription->Destroy();
        m_pISubscription = IMS_NULL;
    }

    IMS_TRACE_I("HandleStartFailed : [%d]", nStatusCode, 0, 0);

    SendTerminatedToListn(CallReasonInfo(CODE_UNSPECIFIED, -1));
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleTerminated(IN IMSMSG& /*objMSG*/)
{
    if (m_pISubscription != IMS_NULL)
    {
        m_pISubscription->Destroy();
        m_pISubscription = IMS_NULL;
    }

    if (HandleRetryByTerminated())
    {
        m_eTerminatedReason = DIALOG_TERMINATED_REASON_NONE;
        m_nRetryAfter = 0;
        return;
    }

    IMS_TRACE_I("HandleTerminated", 0, 0, 0);

    SendTerminatedToListn(CallReasonInfo(CODE_UNSPECIFIED, -1));
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::HandleFailureRes(
        IN ISubscription* pISubscription, IN IMS_SINT32 nStatusCode)
{
    IMS_BOOL bHandle = IMS_FALSE;

    if (nStatusCode == SipStatusCode::SC_423)
    {
        bHandle = HandleFailureRes_423(pISubscription);
    }

    IMS_TRACE_I("HandleFailureRes [%d] Handle[%s]", nStatusCode, PS_BOOL(bHandle), 0);
    return bHandle;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::HandleFailureRes_423(IN ISubscription* pISubscription)
{
    IMS_BOOL bHandle = IMS_FALSE;
    IMS_SINT32 nMins = -1;

    nMins = MessageUtil::GetHeaderValueInt(
            pISubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_SUBSCRIBE),
            ISipHeader::MIN_EXPIRES);

    if (nMins == -1)
    {
        IMS_TRACE_I("HandleFailureRes_423 : nMins == -1", 0, 0, 0);
        return bHandle;
    }

    if (m_pISubscription != IMS_NULL)
    {
        m_pISubscription->Destroy();
        m_pISubscription = IMS_NULL;
    }

    m_nExpireTime = (nMins + nMins / 2);
    Subscribe(m_pService->GetICoreService());
    bHandle = IMS_TRUE;

    IMS_TRACE_I("HandleFailureRes_423 : [%d][%d][%s]", nMins, m_nExpireTime, PS_BOOL(bHandle));
    return bHandle;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::HandleSubState(IN AString aStrSubState)
{
    if (aStrSubState == IMS_NULL)
        return IMS_FALSE;

    IMSList<AString> objSplitSemiColon = aStrSubState.Split(';');

    if (objSplitSemiColon.GetSize() == 0)
    {
        return IMS_FALSE;
    }

    // Handles "terminated"
    if (objSplitSemiColon.GetAt(0).EqualsIgnoreCase("terminated"))
    {
        // Default terminated reason & retry-after
        m_eTerminatedReason = DIALOG_TERMINATED_REASON_UNKNOWN;
        m_nRetryAfter = 0;

        for (IMS_UINT32 i = 1; i < objSplitSemiColon.GetSize(); i++)
        {
            IMSList<AString> objSplitEqual = objSplitSemiColon.GetAt(i).Split('=');

            if (objSplitEqual.GetSize() < 2)
            {
                continue;
            }

            if ((objSplitEqual.GetAt(0).GetLength() == 0) ||
                    (objSplitEqual.GetAt(1).GetLength() == 0))
            {
                continue;
            }

            if (objSplitEqual.GetAt(0).EqualsIgnoreCase("reason"))
            {
                m_eTerminatedReason = ConvertTerminatedReason(objSplitEqual.GetAt(1));
            }
            else if (objSplitEqual.GetAt(0).EqualsIgnoreCase("retry-after"))
            {
                m_nRetryAfter = objSplitEqual.GetAt(1).ToUInt32();
            }
        }

        IMS_TRACE_I("HandleSubState : terminated-reason[%u], retry-after[%u]", m_eTerminatedReason,
                m_nRetryAfter, 0);
        return IMS_TRUE;
    }

    IMS_TRACE_I("HandleSubState : not handled", 0, 0, 0);
    return IMS_FALSE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::HandleRetryByTerminated()
{
    IMS_BOOL bHandle = IMS_FALSE;
    IMS_BOOL bReAttemptNeeded = IMS_FALSE;

    // RFC 3265
    switch (m_eTerminatedReason)
    {
        // The subscription has been terminated, but the subscriber SHOULD retry immediately
        // with a new subscription
        // The "retry-after" parameter has no semantics for "deactivated".
        case DIALOG_TERMINATED_REASON_DEACTIVATED:
            bReAttemptNeeded = IMS_TRUE;
            m_nRetryAfter = 0;
            break;

        // The subscription has been terminated, but the client SHOULD retry at some later time.
        // If a "retry-after" parameter isalso present, the client SHOULD wait at least the number
        // of seconds specified by that parameter before attempting to re-subscribe.
        case DIALOG_TERMINATED_REASON_PROBATION:
            bReAttemptNeeded = IMS_TRUE;
            break;

        // Clients SHOULD NOT attempt to re-subscribe
        case DIALOG_TERMINATED_REASON_REJECTED:
            bReAttemptNeeded = IMS_FALSE;
            break;

        // Clients MAY re-subscribe immediately.
        // The "retry-after" parameter has no semantics for "timeout"
        case DIALOG_TERMINATED_REASON_TIMEOUT:
            bReAttemptNeeded = IMS_TRUE;
            m_nRetryAfter = 0;
            break;

        // If a "retry-after" parameter is also present, the client SHOULD wait at least the number
        // of seconds specified by that parameter before attempting to re-subscribe; otherwise,
        // the client MAY retry immediately, but will likely get put back into pending state.
        case DIALOG_TERMINATED_REASON_GIVEUP:
            bReAttemptNeeded = IMS_TRUE;
            break;

        // Clients SHOULD NOT attempt to re-subscribe.
        case DIALOG_TERMINATED_REASON_NORESOURCE:
            bReAttemptNeeded = IMS_FALSE;
            break;

        // If no reason code or an unknown reason code is present,
        // the client MAY attempt to re-subscribe at any time
        case DIALOG_TERMINATED_REASON_UNKNOWN:
            bReAttemptNeeded = IMS_TRUE;
            m_nRetryAfter = 0;
            break;

        case DIALOG_TERMINATED_REASON_NONE:  // FALL-THROUGH
        default:
            bReAttemptNeeded = IMS_FALSE;
            break;
    }

    IMS_TRACE_I("HandleRetryByTerminated : Handle[%s], Reason[%u], RetryAfter[%u]",
            PS_BOOL(bHandle), m_eTerminatedReason, m_nRetryAfter);

    if (bReAttemptNeeded)
    {
        if (m_nRetryAfter > 0)
        {
            if (m_pTimer)
            {
                m_pTimer->Start(TIMER_BASE_TERMINATED_RETRY, m_nRetryAfter * 1000);
            }
        }
        else
        {
            if (m_pService)
            {
                Subscribe(m_pService->GetICoreService());
            }
        }
        bHandle = IMS_TRUE;
    }

    return bHandle;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::HandleUnSubCompleted(IN ISubscription* pISubscription)
{
    IMS_BOOL bHandle = IMS_FALSE;

    if (pISubscription == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMessage* pIMessage = pISubscription->GetPreviousResponse(IMessage::SUBSCRIPTION_UNSUBSCRIBE);
    if (pIMessage == IMS_NULL)
    {
        IMS_TRACE_I("HandleUnSubCompleted : IMessage is NULL", 0, 0, 0);
        return IMS_FALSE;
    }

    if (m_pISubscription != IMS_NULL)
    {
        m_pISubscription->Destroy();
        m_pISubscription = IMS_NULL;
    }

    bHandle = IMS_TRUE;
    IMS_TRACE_I("HandleUnSubCompleted : [%s]", PS_BOOL(bHandle), 0, 0);

    SendTerminatedToListn(CallReasonInfo(CODE_UNSPECIFIED, -1));
    return bHandle;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::HandleDialogInfo(IN const AString& aStrDialogInfo)
{
    IMS_BOOL bUpdated = IMS_FALSE;

    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        IMS_TRACE_E(0, "pDocumentBuilder is null", 0, 0, 0);
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(aStrDialogInfo);
    if (piDocument == IMS_NULL)
    {
        IMS_TRACE_E(0, "Parsing ConfInfo is failed", 0, 0, 0);
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FALSE;
    }

    IElement* pIElement = piDocument->GetDocumentElement();
    if (pIElement == IMS_NULL)
    {
        IMS_TRACE_E(0, "Root element is null", 0, 0, 0);
        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FALSE;
    }

    UpdateDialogInfo(pIElement);

    INodeList* pINodeListDialog = pIElement->GetElementsByTagName("dialog");
    if (pINodeListDialog == IMS_NULL)
    {
        IMS_TRACE_D("updateUsers : pEntpiNodeListUserryElement is null", 0, 0, 0);
        piDocument->DestroyDocument();
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);
        return IMS_FALSE;
    }

    for (IMS_SINT32 Index = 0; Index < pINodeListDialog->GetLength(); Index++)
    {
        INode* pINodeDialog = pINodeListDialog->Item(Index);
        if (pINodeDialog == IMS_NULL)
        {
            IMS_TRACE_D("HandleDialogInfo : There is no 'dialog' Node", 0, 0, 0);
            continue;
        }

        IElement* pDialogElement = DYNAMIC_CAST(IElement*, pINodeDialog);

        IDialogEvent* pDialog = GetDialog(pDialogElement);
        if (pDialog == IMS_NULL)
        {
            IMS_TRACE_D("updateUsers : There is no 'user' Node", 0, 0, 0);
            pDialog = CreateDialog(pDialogElement);
            bUpdated |= IMS_TRUE;
        }
        if (pDialog == IMS_NULL)
        {
            IMS_TRACE_D("updateUsers : There is no 'user' Node", 0, 0, 0);
            continue;
        }
        pDialog->UpdateDialogInfo(m_nVersion, m_eState, m_aStrEntity);
        bUpdated |= pDialog->Update(pDialogElement);
    }

    IMS_TRACE_I("HandleDialogInfo : Dialogs[%d] Updated[%s]", pINodeListDialog->GetLength(),
            PS_BOOL(bUpdated), 0);

    pIElement->DestroyNodeList(pINodeListDialog);
    piDocument->DestroyDocument();
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    return bUpdated;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::UpdateDialogInfo(IN IElement* pDialogInfoElement)
{
    if (pDialogInfoElement == IMS_NULL)
    {
        IMS_TRACE_E(0, "Root element is null", 0, 0, 0);
        return IMS_FALSE;
    }

    AString aStrVersion = pDialogInfoElement->GetAttribute("version");
    AString aStrState = pDialogInfoElement->GetAttribute("state");
    AString aStrEntity = pDialogInfoElement->GetAttribute("entity");

    IMS_TRACE_I("UpdateDialogInfo : Version[%d][%d]", m_nVersion, aStrVersion.ToInt32(), 0);
    IMS_TRACE_I("UpdateDialogInfo : State[%d][%d]", m_eState, ConvertInfoState(aStrState), 0);
    IMS_TRACE_D("UpdateDialogInfo : Entity[%s][%s]", m_aStrEntity.GetStr(), aStrEntity.GetStr(), 0);

    m_nVersion = aStrVersion.ToInt32();
    m_eState = ConvertInfoState(aStrState);
    m_aStrEntity = aStrEntity;

    return IMS_TRUE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleChangedCallState(IN IMSMSG& /*objMsg*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::HandleChangedTotalCallState(IN IMSMSG& /*objMsg*/) {}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMSList<DialogInfo*> DEMngr::GetDialogInfos()
{
    IMSList<DialogInfo*> lstDialogInfos = IMSList<DialogInfo*>();

    for (IMS_UINT32 index = 0; index < m_objDialogs.GetSize(); index++)
    {
        IDialogEvent* pDialog = m_objDialogs.GetValueAt(index);

        if (pDialog == IMS_NULL)
        {
            continue;
        }

        DialogInfo* pDialogInfo = new DialogInfo();

        pDialogInfo->aStrID = pDialog->GetID();
        pDialogInfo->aStrCallID = pDialog->GetCallID();
        pDialogInfo->aStrLocalTag = pDialog->GetLocalTag();
        pDialogInfo->aStrRemoteTag = pDialog->GetRemoteTag();

        pDialogInfo->eState = pDialog->GetState();
        pDialogInfo->eReason = GetStateReason(pDialogInfo->eState, pDialog);
        pDialogInfo->eCode = GetStateCode(pDialogInfo->eState, pDialog);

        pDialogInfo->aStrLocalName = pDialog->GetLocalIdentityDisplay();
        pDialogInfo->aStrLocalNumber = ConvertNumber(pDialog->GetLocalIdentity());

        pDialogInfo->aStrRemoteName = pDialog->GetRemoteIdentityDisplay();
        pDialogInfo->aStrRemoteNumber = ConvertNumber(pDialog->GetRemoteIdentity());

        pDialogInfo->bInitiator = IsInitiator(pDialog);
        pDialogInfo->bEnablePull = IsCallPull(pDialog);
        pDialogInfo->bConference = IsConf(pDialog);

        lstDialogInfos.Append(pDialogInfo);

        pDialogInfo->pMediaInfo = GetMediaInfo(pDialog);
    }

    IMS_TRACE_I("GetDialogInfos : [%d]", lstDialogInfos.GetSize(), 0, 0);
    return lstDialogInfos;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_SINT32 DEMngr::GetStateReason(IN IMS_UINT32 eState, IN IDialogEvent* pDialog)
{
    IMS_SINT32 eReason = -1;
    AString aStrStateEvent = pDialog->GetStateEvent();

    IMS_TRACE_I("GetStateReason : State[%d] [%s][%d]", eState, aStrStateEvent.GetStr(), eReason);
    return eReason;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_SINT32 DEMngr::GetStateCode(IN IMS_UINT32 eState, IN IDialogEvent* pDialog)
{
    IMS_SINT32 eCode = -1;
    AString aStrStateCode = pDialog->GetStateCode();

    IMS_TRACE_I("GetStateCode : State[%d] [%s][%d]", eState, aStrStateCode.GetStr(), eCode);
    return eCode;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::IsInitiator(IN IDialogEvent* pDialog)
{
    IMS_BOOL bIs = IMS_FALSE;
    AString aStrDirection = AString::ConstNull();

    aStrDirection = pDialog->GetDirection();
    if (aStrDirection.EqualsIgnoreCase("initiator"))
    {
        bIs = IMS_TRUE;
    }

    IMS_TRACE_I("IsInitiator : [%s]", PS_BOOL(bIs), 0, 0);
    return bIs;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::IsConf(IN IDialogEvent* /*pDialog*/)
{
    IMS_BOOL bIs = IMS_FALSE;

    IMS_TRACE_I("IsConf : [%s]", PS_BOOL(bIs), 0, 0);
    return bIs;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL IMS_BOOL DEMngr::IsCallPull(IN IDialogEvent* pDialog)
{
    IMS_BOOL bIs = IMS_FALSE;

    bIs = pDialog->EnablePulled();

    IMS_TRACE_I("IsCallPull : [%s]", PS_BOOL(bIs), 0, 0);
    return bIs;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL MediaInfo* DEMngr::GetMediaInfo(IN IDialogEvent* /*pDialog*/)
{
    MediaInfo* pMediaInfo = IMS_NULL;  // CHECK

    IMS_TRACE_I("GetMediaInfo", 0, 0, 0);
    return pMediaInfo;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL AString DEMngr::ConvertNumber(IN AString aStrIdentity)
{
    AString aStrNumber = AString::ConstNull();

    SipAddress objSIPAddress;
    if (objSIPAddress.Create(aStrIdentity))
    {
        if (objSIPAddress.IsSchemeSip() || objSIPAddress.IsSchemeSips())
        {
            const SipAddress::UserInfoPart* pUserInfoPart = objSIPAddress.GetUserInfoPart();
            if (pUserInfoPart != IMS_NULL)
            {
                aStrNumber = pUserInfoPart->GetUser();
            }
            else
            {
                aStrNumber = objSIPAddress.GetUser();
            }
        }
        else if (objSIPAddress.IsSchemeTel())
        {
            aStrNumber = objSIPAddress.GetHost();
        }
    }
    else
    {
        aStrNumber = aStrIdentity;
    }

    IMS_TRACE_D("ConvertNumber : [%s][%s]", aStrIdentity.GetStr(), aStrNumber.GetStr(), 0);
    return aStrNumber;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::SendTerminatedToListn(IN CallReasonInfo terminatedReason)
{
    if (m_pListener == IMS_NULL)
    {
        return;
    }

    IDEMngrTerminatedParam* pParam = new IDEMngrTerminatedParam();
    pParam->terminatedReason = terminatedReason;
    pParam->bDestroy = m_bDestroy;

    IMS_TRACE_I("SendTerminatedToListn : %s Destroy[%s]", PS_FR(terminatedReason),
            PS_BOOL(m_bDestroy), 0);

    m_pListener->DEMngr_Terminated((IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::SendNotifyInfoToUI(IN IMSList<DialogInfo*> lstDialogInfos)
{
    IUUCServiceDialogsNotifyInfoParam* pParam = new IUUCServiceDialogsNotifyInfoParam();
    pParam->lstDialogInfos = lstDialogInfos;

    IMS_TRACE_I("SendNotifyInfoToUI", 0, 0, 0);

    IMS_MSG_CreateNPostThreadMessageByName(
            m_aStrJNIServiceName, IuMtcService::DIALOGS_NOTIFY_INFO, 0, (IMS_UINTP)pParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IMS_UINT32 DEMngr::GetCallState()
{
    IMS_TRACE_I("GetCallState : [%d]", m_eCallState, 0, 0);
    return m_eCallState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IMS_UINT32 DEMngr::ConvertInfoState(IN AString aStrState)
{
    IMS_UINT32 eState = DIALOGINFO_STATE_IDLE;

    if (aStrState.EqualsIgnoreCase("full"))
    {
        eState = DIALOGINFO_STATE_FULL;
    }
    else if (aStrState.EqualsIgnoreCase("partial"))
    {
        eState = DIALOGINFO_STATE_PARTIAL;
    }
    else if (aStrState.EqualsIgnoreCase("deleted"))
    {
        eState = DIALOGINFO_STATE_DELETED;
    }
    else
    {
        eState = DIALOGINFO_STATE_IDLE;
    }

    IMS_TRACE_I("ConvertInfoState : [%s][%d]", aStrState.GetStr(), eState, 0);
    return eState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
IMS_UINT32 DEMngr::ConvertTerminatedReason(IN AString aStrReason)
{
    IMS_UINT32 eReason = DIALOG_TERMINATED_REASON_NONE;

    if (aStrReason.EqualsIgnoreCase("deactivated"))
    {
        eReason = DIALOG_TERMINATED_REASON_DEACTIVATED;
    }
    else if (aStrReason.EqualsIgnoreCase("probation"))
    {
        eReason = DIALOG_TERMINATED_REASON_PROBATION;
    }
    else if (aStrReason.EqualsIgnoreCase("rejected"))
    {
        eReason = DIALOG_TERMINATED_REASON_REJECTED;
    }
    else if (aStrReason.EqualsIgnoreCase("timeout"))
    {
        eReason = DIALOG_TERMINATED_REASON_TIMEOUT;
    }
    else if (aStrReason.EqualsIgnoreCase("giveup"))
    {
        eReason = DIALOG_TERMINATED_REASON_GIVEUP;
    }
    else if (aStrReason.EqualsIgnoreCase("noresource"))
    {
        eReason = DIALOG_TERMINATED_REASON_NORESOURCE;
    }
    else
    {
        eReason = DIALOG_TERMINATED_REASON_UNKNOWN;
    }

    IMS_TRACE_I("ConvertTerminatedReason : [%s][%d]", aStrReason.GetStr(), eReason, 0);
    return eReason;
}

// TODO: VIRTUAL -> UC_CONFG ------------------------------------------ START
/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::LoadConfig()
{
    IMS_TRACE_I("LoadConfig", 0, 0, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::AddEventListn()
{
    // 2 TODO : UC_CONFIG
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL void DEMngr::DeleteEventListn()
{
    // 2 TODO : UC_CONFIG
}
