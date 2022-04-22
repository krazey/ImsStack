/*
 * author : seyoon0802.jung@
 * version : 0.1
 * date : 201507
 * brief :
 */


#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceEvent.h"
#include "ServiceMessage.h"

#include "ISession.h"
#include "IMessage.h"
#include "ISIPServerConnection.h"
#include "IReference.h"
#include "call/IMtcCall.h"

#include "SIPStatusCode.h"
#include "SIP.h"
#include "SIPHeaderName.h"
#include "SIPAddress.h"
#include "dialingplan/MtcDialingPlan.h"
#include "IMSTypeDef.h"

#include "configuration/ConfigDef.h"
#include "configuration/ConfigDef.h"
#include "utility/MessageUtil.h"
#include "helper/CallStateProxy.h"
#include "MtcDef.h"
#include "define/MtcStringDef.h"
#include "ect/EctManager.h"
#include "call/MtcUiNotifier.h"

__IMS_TRACE_TAG_COM_UC__;

// TODO, MTC BUILD

// BEGIN_STATE_MAP(UCECTMngr)
//     STATE_ENTRY(IDLE)
//     STATE_ENTRY(NOTIFYING)
//     STATE_ENTRY(INHR)
//     STATE_ENTRY(RINGBACK)
//     STATE_ENTRY(RINGING)
//     STATE_ENTRY(REFERRING)
//     STATE_ENTRY(WAITINGNOTIFY)
//     STATE_ENTRY(WAITINGBYE)
//     STATE_ENTRY(TERMINATING)
// END_STATE_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, IDLE)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_ECT_START, &UCECTMngr::StateIDLE_ECTStart)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_ECT_START_BLIND, &UCECTMngr::StateIDLE_ECTStartBlind)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_REFERENCE_RECEIVED,
//             &UCECTMngr::StateIDLE_ReferenceReceived)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_INVITE_RECEIVED, &UCECTMngr::StateIDLE_IncomingSession)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateIDLE_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, NOTIFYING)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_NOTIFY_DELIVERED,
//             &UCECTMngr::StateNOTIFYING_NOTIFYDelivered)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_NOTIFY_DELIVERYFAILED,
//             &UCECTMngr::StateNOTIFYING_NOTIFYDeliveryFailed)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateNOTIFYING_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, INHR)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_HR_HELD, &UCECTMngr::StateINHR_Held)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_HR_HOLDFAILED, &UCECTMngr::StateINHR_HoldFailed)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateINHR_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, RINGBACK)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_STARTED, &UCECTMngr::StateRINGBACK_Started)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_STARTFAILED, &UCECTMngr::StateRINGBACK_StartFailed)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateRINGBACK_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, RINGING)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_STARTED, &UCECTMngr::StateRINGING_Started)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_STARTFAILED, &UCECTMngr::StateRINGING_StartFailed)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateRINGING_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, REFERRING)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_REFERENCE_DELIVERED,
//             &UCECTMngr::StateREFERRING_REFERDelivered)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_REFERENCE_DELIVERFAILED,
//             &UCECTMngr::StateREFERRING_REFERDeliverFailed)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_NOTIFY_RECEIVED, &UCECTMngr::StateREFERRING_REFERNotify)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateREFERRING_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, WAITINGNOTIFY)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_NOTIFY_RECEIVED,
//             &UCECTMngr::StateWAITINGNOTIFY_REFERNotify)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateWAITINGNOTIFY_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, WAITINGBYE)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateWAITINGBYE_Terminated)
// END_STATE_MSG_MAP()

// BEGIN_STATE_MSG_MAP(UCECTMngr, TERMINATING)
//     STATE_MSG_ENTRY(UCECTMngr::UCECTMNGR_SS_TERMINATED, &UCECTMngr::StateTERMINATING_Terminated)
// END_STATE_MSG_MAP()


const IMS_CHAR UCECTMngr::STR_USER_PHONE[]      = ";user=phone";
const IMS_CHAR UCECTMngr::STR_REFER_REFERTOEX[] = "Require=replaces";

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
UCECTMngr::UCECTMngr(IN IMtcApp* pApp, IN IMtcCall* pTransfereeSession,
        IN IMtcCall* pTransferTargetSession, IN IMtcCall* pTransferredSession)
    : IMSStateMachine(IDLE)
    , IMSActivityEx(AString::ConstNull())
    , m_eState(IDLE)
    , m_eOldState(IDLE)
    , m_pApp(pApp)
    , m_nSlotID(0)
    , m_pTransfereeSessKey(-1)
    , m_pTransferTargetSessKey(-1)
    , m_pTransferredSessKey(-1)
    , m_strUIKey(AString::ConstNull())
    , m_pListener(IMS_NULL)
    , m_pUCSessList(IMS_NULL)
    , m_strTransferTargetURI(AString::ConstNull())
    , m_pECTReference(IMS_NULL)
    , m_pTimer(IMS_NULL)
    , m_bBlindECT(IMS_FALSE)
{
    Init(pTransfereeSession, pTransferTargetSession, pTransferredSession);

    // TODO, MTC BUILD
    m_nSlotID = 0;
    // m_nSlotID = m_pApp->GetSlotID();
    IMS_TRACE_MEM("uc", "uc_M[%d] : UCECTMngr[%" PFLS_u "]", m_nSlotID, sizeof(UCECTMngr), 0);

    // TODO, MTC BUILD
    // SetSessionListn();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
UCECTMngr::~UCECTMngr()
{
    IMS_TRACE_MEM("uc", "uc_F[%d] : UCECTMngr[%" PFLS_u "]", m_nSlotID, sizeof(UCECTMngr), 0);

    DeInit();

    if (m_pTimer != IMS_NULL)
    {
        delete m_pTimer;
        m_pTimer = IMS_NULL;
    }

    if (m_pListener != IMS_NULL)
    {
        m_pListener = IMS_NULL;
    }

    if (m_pUCSessList != IMS_NULL)
    {
        m_pUCSessList = IMS_NULL;
    }

    if (m_pApp != IMS_NULL)
    {
        m_pApp = IMS_NULL;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
IMS_BOOL UCECTMngr::Control(IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam,
        OUT IMS_UINTP* /*pnOutParam*/)
{
    IMS_TRACE_I("Control : App Name = %s, Cmd = %d", GetName().GetStr(), nCmdType, 0);

    IMSMSG objMSG(nCmdType, 0, nInParam);
    OnStateMessage(objMSG);

    return IMS_FALSE;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
IIMSActivityControl* UCECTMngr::GetController()
{
    return IMS_NULL;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::UCTimer_Expired(IN IMS_SINT32 eType)
{
    // TODO, MTC BUILD
    // IMS_UINT32 eSize = m_pTimer->GetSize();

    // IMS_TRACE_I("HandleTimerExpired : State[%s] Size[%d] Type[%d]", PrintState(), eSize, eType);

    switch (eType)
    {
        case TIMER_TRANSFEREE_WAIT_BYE:
            // TODO, MTC BUILD
            // HandleTransfereeWaitBYE();
            break;

        default:
            break;
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED VIRTUAL
IMS_BOOL UCECTMngr::OnMessage(IN IMSMSG &objMSG)
{
    IMS_TRACE_I("OnMessage : Msg[%d]", objMSG.nMSG, 0, 0);
    OnStateMessage(objMSG);

    return IMS_TRUE;
}

// TODO, MTC BUILD
// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::RingBack(IN IMS_UINTP nParam)
// {
//     (void)nParam;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::Ringing(IN IMS_UINTP nParam)
// {
//     (void)nParam;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::Started(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("Started[%s]", PrintState(), 0, 0);
//     PostMessage(UCECTMNGR_SS_STARTED, 0, nParam);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::StartFailed(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("StartFailed[%s]", PrintState(), 0, 0);
//     PostMessage(UCECTMNGR_SS_STARTFAILED, 0, nParam);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::Updated(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("Updated[%s]", PrintState(), 0, 0);

//     if (GetState() == INHR)
//     {
//         PostMessage(UCECTMNGR_HR_HELD, 0, nParam);
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::UpdateFailed(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("UpdateFailed[%s]", PrintState(), 0, 0);

//     if (GetState() == INHR)
//     {
//         PostMessage(UCECTMNGR_HR_HOLDFAILED, 0, nParam);
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::Terminated(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("Terminated[%s]", PrintState(), 0, 0);

//     IUCSessionTerminatedParam* pParam = reinterpret_cast<IUCSessionTerminatedParam*>(nParam);
//     delete pParam;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::Destroyed(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("Destroyed[%s]", PrintState(), 0, 0);

//     IUCSessionDestroyedParam* pParam = reinterpret_cast<IUCSessionDestroyedParam*>(nParam);
//     FailReason terminatedReason(pParam->failReason);

//     m_pUCSessList->Delete(pParam->nSessionKey, terminatedReason, pParam->bTerminated);
//     // TODO, MTC BUILD
//     // CallStateProxy::GetInstance()->SetCallState(m_nSlotID, pParam->nSessionKey,
//     //         VOLTE_CALL_STATE_IDLE, pParam->pCallInfo, pParam->pMediaInfo, FailReason());

//     if (IsECTTerminateRequired((IMS_SINTP)(pParam->nSessionKey)))
//     {
//         PostMessage(UCECTMNGR_SS_TERMINATED, 0, nParam);
//         DeInit();
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::ReferenceReceived(IN IMS_UINTP nParam)
// {
//     (void)nParam;
// }

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::SetListener(IN IUCECTMngrListener* pListener)
{
    IMS_TRACE_I("SetListener : [%" PFLS_u "]", pListener, 0, 0);
    m_pListener = pListener;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_Delivered(IN IMS_UINTP nParam)
{
    IMS_TRACE_I("Refer_Delivered[%s]", PrintState(), 0, 0);
    PostMessage(UCECTMNGR_REFERENCE_DELIVERED, 0, nParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_DeliveryFailed(IN IMS_UINTP nParam)
{
    IMS_TRACE_I("Refer_DeliveryFailed[%s]", PrintState(), 0, 0);
    PostMessage(UCECTMNGR_REFERENCE_DELIVERFAILED, 0, nParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_Notify(IN IMS_UINTP nParam)
{
    IMS_TRACE_I("Refer_Notify[%s]", PrintState(), 0, 0);
    PostMessage(UCECTMNGR_NOTIFY_RECEIVED, 0, nParam);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_Terminated(IN IMS_UINTP /*nParam*/)
{
    IMS_TRACE_I("Refer_Terminated[%s]", PrintState(), 0, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_Failed(IN IMS_UINTP nParam)
{
    IMS_TRACE_I("Refer_Failed[%s]", PrintState(), 0, 0);

    // TODO: failure
    // TODO: send REFER method=CANCEL;
    // TODO, MTC BUILD
    // SendECTResultToUI(IMS_FALSE, 0, 0);

    IECTReferListenFailedParam* pParam = reinterpret_cast<IECTReferListenFailedParam*>(nParam);
    delete pParam;

    m_pListener->ECTMngr_StartFailed(IMS_NULL);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_Notify_Delivered(IN IMS_UINTP /*nParam*/)
{
    IMS_TRACE_I("Refer_Notify_Delivered[%s]", PrintState(), 0, 0);

    IMSMSG objMsg(UCECTMNGR_NOTIFY_DELIVERED, 0, 0);
    OnStateMessage(objMsg);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
void UCECTMngr::Refer_Notify_DeliveryFailed(IN IMS_UINTP nParam)
{
    IMS_TRACE_I("Refer_Notify_DeliveryFailed[%s]", PrintState(), 0, 0);

    IMSMSG objMsg(UCECTMNGR_NOTIFY_DELIVERYFAILED, 0, nParam);
    OnStateMessage(objMsg);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCECTMngr::Init(IN IMtcCall* pTransfereeSession, IN IMtcCall* pTransferTargetSession,
        IN IMtcCall* pTransferredSession)
{
    IMtcCall* pSession = IMS_NULL;

    if (pTransfereeSession != IMS_NULL)
    {
        pSession = pTransfereeSession;
        m_pTransfereeSessKey = pTransfereeSession->GetKey();
    }
    else if (pTransferTargetSession != IMS_NULL)
    {
        pSession = pTransferTargetSession;
        m_pTransferTargetSessKey = pTransferTargetSession->GetKey();
    }
    else if (pTransferredSession != IMS_NULL)
    {
        pSession = pTransferredSession;
        m_pTransferredSessKey = pTransferredSession->GetKey();
    }
    else
    {
        IMS_TRACE_E(0, "Init failed : all sessions are null", 0, 0, 0);
        return;
    }

    // TODO, MTC BUILD
    m_pUCSessList = IMS_NULL;
    // m_pUCSessList = pSession->GetSessList();

    m_strTransferTargetURI = AString::ConstNull();
    m_pECTReference = IMS_NULL;

    m_pTimer = new MtcTimerWrapper();
    m_pTimer->SetListener(this);

    m_bBlindECT = IMS_FALSE;

    m_strUIKey = AString::ConstNull();

    IMS_TRACE_I("Init : Key[%" PFLS_u"] bBlindECT[%s]",
            pSession->GetKey(), PS_BOOL(m_bBlindECT), 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCECTMngr::DeInit()
{
    IMS_TRACE_I("DeInit", 0, 0, 0);

    // TODO, MTC BUILD
    // DestroyRefer();
    // ResetSessionListn();
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
void UCECTMngr::Destroy()
{
    IMS_TRACE_I("Destroy", 0, 0, 0);
    delete this;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_UINT32 UCECTMngr::GetState()
{
    return m_eState;
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
IMS_UINT32 UCECTMngr::GetOldState()
{
    return m_eOldState;
}

// TODO, MTC BUILD
// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateIDLE_ECTStart(IN IMSMSG &objMsg)
// {
//     // Transferor case - it has two sessions : with transferee and transfer target.
//     UCECTMngrStartParam* pParam = reinterpret_cast<UCECTMngrStartParam*>(objMsg.nLparam);
//     IUUCSessionECTStartParam* pJoinParam = (IUUCSessionECTStartParam*)pParam->pMsgParam;
//     AString strReplace;
//     AString strReferTo;
//     SIPAddress objSipAddress;
//     IMS_BOOL bResult = IMS_TRUE;

//     IMtcCall* pTransferTargetSession = GetSession(m_pTransferTargetSessKey);
//     if (pTransferTargetSession == IMS_NULL)
//     {
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStart;
//     }

//     // TODO, MTC BUILD
//     // if (pTransferTargetSession->GetProxy() == IMS_NULL)
//     {
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStart;
//     }

//     // TODO, MTC BUILD
//     // m_strUIKey = pTransferTargetSession->GetProxy()->GetUIKey();
//     if (m_strUIKey.GetLength() <= 0)
//     {
//         IMS_TRACE_E(0, "StateIDLE_ECTStart : UIKey is NULL or Empty", 0, 0, 0);
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStart;
//     }

//     // TODO, MTC BUILD
//     // for (IMS_UINT32 nIndex = 0; nIndex < m_pUCSessList->GetNum(); nIndex++)
//     // {
//     //     if (m_pUCSessList->GetAt(nIndex)->IsECTProgressing() == IMS_FALSE)
//     //     {
//     //         m_pTransfereeSessKey = m_pUCSessList->GetAt(nIndex)->GetKey();
//     //     }
//     // }

//     if (CreateRefer() == IMS_FALSE)
//     {
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStart;
//     }

//     IMS_TRACE_I("StateIDLE_ECTStart", 0, 0, 0);

//     // TODO, MTC BUILD
//     // strReplace = MessageUtil::GetSessionID(pTransferTargetSession->GetISession());
//     // strReferTo = MessageUtil::GetRemoteURI(pTransferTargetSession->GetISession(),
//     //         pTransferTargetSession->GetPeerType());

//     objSipAddress.Create(strReferTo);

//     if (objSipAddress.IsSchemeSIP() || objSipAddress.IsSchemeSIPS())
//     {
//         if (!strReferTo.Contains(STR_USER_PHONE))
//         {
//             strReferTo.Append(STR_USER_PHONE);
//         }
//     }

//     m_pECTReference->Init("INVITE", strReferTo, strReplace, STR_REFER_REFERTOEX);

//     if (SendRefer())
//     {
//         SetState(REFERRING);
//     }
//     else
//     {
//         bResult = IMS_FALSE;
//     }

//     goto Exit_StateIDLE_ECTStart;

//     Exit_StateIDLE_ECTStart:

//     delete pJoinParam;
//     delete pParam;

//     if (bResult == IMS_FALSE)
//     {
//         SendECTResultToUI(IMS_FALSE, 0, 0);
//         m_pListener->ECTMngr_StartFailed(IMS_NULL);
//     }

//     return bResult;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateIDLE_ECTStartBlind(IN IMSMSG &objMsg)
// {
//     // Transferor case - it has only one session which is with transferee.
//     UCECTMngrStartParam* pParam = reinterpret_cast<UCECTMngrStartParam*>(objMsg.nLparam);
//     IUUCSessionECTStartBlindParam* pJoinParam = (IUUCSessionECTStartBlindParam*)pParam->pMsgParam;
//     AString strReferToURI;
//     IMS_BOOL bResult = IMS_TRUE;

//     IMtcCall* pTransfereeSession = GetSession(m_pTransfereeSessKey);
//     if (pTransfereeSession == IMS_NULL)
//     {
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStartBlind;
//     }

//     // TODO, MTC BUILD
//     // if (pTransfereeSession->GetProxy() == IMS_NULL)
//     {
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStartBlind;
//     }

//     // TODO, MTC BUILD
//     // m_strUIKey = pTransfereeSession->GetProxy()->GetUIKey();
//     if (m_strUIKey.GetLength() <= 0)
//     {
//         IMS_TRACE_E(0, "StateIDLE_ECTStartBlind : UIKey is NULL or Empty", 0, 0, 0);
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStartBlind;
//     }

//     strReferToURI = pJoinParam->aStrTarget;
//     if (strReferToURI.GetLength() <= 0)
//     {
//         IMS_TRACE_E(0, "StateIDLE_ECTStartBlind : Target number is NULL or Empty", 0, 0, 0);
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStartBlind;
//     }

//     if (CreateRefer() == IMS_FALSE)
//     {
//         bResult = IMS_FALSE;
//         goto Exit_StateIDLE_ECTStartBlind;
//     }

//     IMS_TRACE_I("StateIDLE_ECTStartBlind", 0, 0, 0);

//     strReferToURI = GetReferToURI(strReferToURI);
//     m_pECTReference->Init("INVITE", strReferToURI);

//     if (SendRefer())
//     {
//         SetState(REFERRING);
//         SetBlindECT(IMS_TRUE);
//     }
//     else
//     {
//         bResult = IMS_FALSE;
//     }

//     goto Exit_StateIDLE_ECTStartBlind;

//     Exit_StateIDLE_ECTStartBlind:

//     delete pJoinParam;
//     delete pParam;

//     if (bResult == IMS_FALSE)
//     {
//         SendECTResultToUI(IMS_FALSE, 0, 0);
//         m_pListener->ECTMngr_StartFailed(IMS_NULL);
//     }

//     return bResult;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateIDLE_ReferenceReceived(IN IMSMSG &objMsg)
// {
//     // Transferee case - REFER received which is in-dialog : session with transferee
//     // TODO, MTC BUILD
//     UNUSED_PARAM(objMsg);
//     // IUCSessionSSReferenceReceivedParam* pParam =
//     //         reinterpret_cast<IUCSessionSSReferenceReceivedParam*>(objMsg.nLparam);

//     // IMS_TRACE_I("StateIDLE_ReferenceReceived", 0, 0, 0);

//     // if (CreateRefer() == IMS_FALSE)
//     // {
//     //     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);
//     //     delete pParam;
//     //     return IMS_FALSE;
//     // }

//     // m_pECTReference->SetIReference(pParam->pReference);
//     // m_pECTReference->SendNotificationTrying();

//     // m_strTransferTargetURI = pParam->pReference->GetReferToUserId();

//     // IMS_TRACE_D("StateIDLE_ReferenceReceived : TransferTargetURI[%s]",
//     //         m_strTransferTargetURI.GetStr(), 0, 0);

//     // SetState(NOTIFYING);

//     // delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateIDLE_IncomingSession(IN IMSMSG &objMsg)
// {
//     // Transfer target case
//     // param will be deleted in UCSession::StateIDLE_IncomingSession();
//     (void)objMsg;

//     SetState(RINGING);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateIDLE_Terminated(IN IMSMSG &objMsg)
// {
//     // All cases
//     // TODO, MTC BUILD
//     UNUSED_PARAM(objMsg);
//     // IUCSessionTerminatedParam* pParam =
//     //         reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateIDLE_Terminated", 0, 0, 0);

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);
//     // TODO, MTC BUILD
//     // delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateNOTIFYING_NOTIFYDelivered(IN IMSMSG &/*objMsg*/)
// {
//     // Transferee case - it has sent NOTIFY(refer) and received 200 OK to that request.
//     IMS_TRACE_I("StateNOTIFYING_NOTIFYDelivered", 0, 0, 0);

//     if (GetOldState() == IDLE) // send NOTIFY with 100 Trying sipfrag.
//     {
//         SendHold();
//     }
//     else if (GetOldState() == RINGBACK) // send NOTIFY with 2xx response sipfrag.
//     {
//         SetState(WAITINGBYE);
//         m_pTimer->Start(TIMER_TRANSFEREE_WAIT_BYE, ECT_WAIT_BYE_TIME);
//     }

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateNOTIFYING_NOTIFYDeliveryFailed(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IECTReferListenNotifyDeliveryFailedParam* pParam =
//             reinterpret_cast<IECTReferListenNotifyDeliveryFailedParam*>(objMsg.nLparam);

//     // status = 0 ==> timer expired.
//     IMS_TRACE_I("StateNOTIFYING_NOTIFYDeliveryFailed[%s] : Code[%d] OldState[%s]", PrintState(),
//             pParam->nStatusCode, PrintState(GetOldState()));

//     if (GetOldState() == RINGBACK)
//     {
//         SetState(WAITINGBYE);
//         m_pTimer->Start(TIMER_TRANSFEREE_WAIT_BYE, ECT_WAIT_BYE_TIME);
//     }
//     else
//     {
//         m_pListener->ECTMngr_SessionNOTIFYDeliveryFailed(IMS_NULL);
//     }

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateNOTIFYING_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IMS_TRACE_I("StateNOTIFYING_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL); //// need to be changed

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateINHR_Held(IN IMSMSG &/*objMsg*/)
// {
//     // Transferee case
//     IMS_TRACE_I("StateINHR_Held", 0, 0, 0);
//     //After holding successfully, create IMtcCall for INVITE
//     //and set Listener IUCSessionListener.
//     CreateECTSession();
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateINHR_HoldFailed(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IMS_TRACE_I("StateINHR_HoldFailed", 0, 0, 0);

//     IUCSessionUpdatFailedParam* pParam =
//             reinterpret_cast<IUCSessionUpdatFailedParam*>(objMsg.nLparam);
//     m_pListener->ECTMngr_SessionUpdateFailed(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateINHR_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IMS_TRACE_I("StateINHR_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);
//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateRINGBACK_Started(IN IMSMSG &objMsg)
// {
//     IMtcCall* pTransfereeSession = GetSession(m_pTransfereeSessKey);
//     // Transferee case
//     if (pTransfereeSession == IMS_NULL)
//     {
//         return IMS_FALSE;
//     }

//     //Replace previous dialog to new dialog
//     IUCSessionStartedParam* pParam = reinterpret_cast<IUCSessionStartedParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateRINGBACK_Started : current[%" PFLS_u "] previous[%" PFLS_u "]",
//             pParam->nSessionKey, m_pTransfereeSessKey, 0);

//     SendResumedByToUI(pTransfereeSession);
//     SendECTStartedByToUI(pTransfereeSession);

//     //send NOTIFY 200 OK
//     m_pECTReference->SendNotificationSuccess();
//     SetState(NOTIFYING);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateRINGBACK_StartFailed(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IMS_TRACE_I("StateRINGBACK_StartFailed", 0, 0, 0);

//     m_pECTReference->SendNotificationDeclined();

//     IUCSessionStartFailedParam* pParam =
//             reinterpret_cast<IUCSessionStartFailedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionStartFailed((IMS_UINTP)pParam);

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateRINGBACK_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IMS_TRACE_I("StateRINGBACK_Terminated", 0, 0, 0);

//     m_pECTReference->SendNotificationFailure();

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateRINGING_Started(IN IMSMSG &objMsg)
// {
//     // Transfer target case - transferred session is started.
//     // It has two sessions : session with transferor and transferred session
//     // TODO, MTC BUILD
//     // IUCSessionStartedParam* pParam = reinterpret_cast<IUCSessionStartedParam*>(objMsg.nLparam);

//     // for (IMS_UINT32 i = 0; i < m_pUCSessList->GetNum(); i++)
//     // {
//     //     if (pParam->nSessionKey != m_pUCSessList->GetAt(i)->GetKey())
//     //     {
//     //         m_pTransferTargetSessKey = m_pUCSessList->GetAt(i)->GetKey();
//     //         break;
//     //     }
//     // }

//     // IMtcCall* pTransferTargetSession = GetSession(m_pTransferTargetSessKey);
//     // if (pTransferTargetSession == IMS_NULL)
//     // {
//     //     IMS_TRACE_E(0, "StateRINGING_Started : session with transfer target is null", 0, 0, 0);
//     //     m_pListener->ECTMngr_SessionStartFailed((IMS_UINTP)pParam);
//     //     return IMS_FALSE;
//     // }

//     // IMS_TRACE_I("StateRINGING_Started : current[%" PFLS_u "] previous[%" PFLS_u "]",
//     //         pParam->nSessionKey, m_pTransferTargetSessKey, 0);

//     // SendResumedByToUI(pTransferTargetSession);
//     // SendECTStartedByToUI(pTransferTargetSession);

//     // m_pUCSessList->Terminate(m_pTransferTargetSessKey,
//     //         FailReason(FAIL_REASON_ECT_REPLACED, -1), IMS_FALSE);

//     // SetState(TERMINATING);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateRINGING_StartFailed(IN IMSMSG &objMsg)
// {
//     //Transfer target case
//     IMS_TRACE_I("StateRINGING_StartFailed", 0, 0, 0);

//     IUCSessionStartFailedParam* pParam =
//             reinterpret_cast<IUCSessionStartFailedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionStartFailed((IMS_UINTP)pParam); //// need to be changed
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateRINGING_Terminated(IN IMSMSG &objMsg)
// {
//     //Transfer target case
//     IMS_TRACE_I("StateRINGING_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateREFERRING_REFERDelivered(IN IMSMSG &/*objMsg*/)
// {
//     // Transferor case
//     IMS_TRACE_I("StateREFERRING_REFERDelivered", 0, 0, 0);

//     if (IsBlindECT())
//     {
//         // terminate the session with transferee
//         SendECTResultToUI(IMS_TRUE, 0, 0);
//         m_pUCSessList->Terminate(m_pTransfereeSessKey,
//                 FailReason(FAIL_REASON_ECT_COMPLETED, -1), IMS_FALSE);
//     }

//     SetState(WAITINGNOTIFY);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateREFERRING_REFERDeliverFailed(IN IMSMSG &objMsg)
// {
//     // Transferor case
//     IMS_TRACE_I("StateREFERRING_REFERDeliverFailed", 0, 0, 0);

//     SendECTResultToUI(IMS_FALSE, 0, 0);

//     IECTReferListenDeliveryFailedParam* pParam =
//             reinterpret_cast<IECTReferListenDeliveryFailedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_StartFailed(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateREFERRING_REFERNotify(IN IMSMSG &objMsg)
// {
//     // Transferor case
//     IMS_TRACE_I("StateREFERRING_REFERNotify", 0, 0, 0);
//     StateWAITINGNOTIFY_REFERNotify(objMsg);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateREFERRING_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferor case
//     IMS_TRACE_I("StateREFERRING_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateWAITINGNOTIFY_REFERNotify(IN IMSMSG &objMsg)
// {
//     // Transferor case
//     IECTReferListenNotifyParam* pParam =
//             reinterpret_cast<IECTReferListenNotifyParam*>(objMsg.nLparam);
//     AString aStrSubState = pParam->aStrSubState;
//     IMS_SINT32 nStatusCode = pParam->nStatusCode;

//     IMS_TRACE_I("StateWAITINGNOTIFY_REFERNotify : [%s][%d]", aStrSubState.GetStr(), nStatusCode,
//             0);

//     if (SIPStatusCode::Is1XX(nStatusCode))
//     {
//     }
//     else if (SIPStatusCode::IsFinalSuccess(nStatusCode))
//     {
//         if (aStrSubState.Equals("terminated"))
//         {
//             if (IsBlindECT() == IMS_FALSE)
//             {
//                 // need terminate the session with transferee - consultative
//                 SendECTResultToUI(IMS_TRUE, 0, 0);
//                 m_pUCSessList->Terminate(m_pTransfereeSessKey,
//                         FailReason(FAIL_REASON_ECT_COMPLETED, -1), IMS_FALSE);
//             }
//             m_pListener->ECTMngr_Started(IMS_NULL);
//         }
//     }
//     else
//     {
//         SendECTResultToUI(IMS_FALSE, 0, 0);
//         m_pListener->ECTMngr_StartFailed(IMS_NULL);
//     }

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateWAITINGNOTIFY_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferor case
//     IMS_TRACE_I("StateWAITINGNOTIFY_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);
//     delete pParam;

//     if (IsBlindECT())
//     {
//         return IMS_FALSE;
//     }

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateWAITINGBYE_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferee case
//     IMS_TRACE_I("StateWAITINGBYE_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);
//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL UCECTMngr::StateTERMINATING_Terminated(IN IMSMSG &objMsg)
// {
//     // Transferor case
//     IMS_TRACE_I("StateTERMINATING_Terminated", 0, 0, 0);

//     IUCSessionTerminatedParam* pParam =
//             reinterpret_cast<IUCSessionTerminatedParam*>(objMsg.nLparam);

//     m_pListener->ECTMngr_SessionTerminated(IMS_NULL);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// AString UCECTMngr::GetReferToURI(IN AString /*aStrReplaceURI*/)
// {
//     return AString::ConstNull();
// }

// /* M -----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMtcCall* UCECTMngr::CreateComSession(IN IMtcService* /*pService*/, IN AString /*aStrUIKey*/,
//         IN IMS_UINTP /*nIMSKey*/)
// {
//     return IMS_NULL;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// IMtcCall* UCECTMngr::CreateSession(IN IMtcService* pService, IN AString UIKey,
//         IN IMS_UINTP IMSKey)
// {
//     IMtcCall* pSession = CreateComSession(pService, UIKey, IMSKey);

//     if (pSession == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "Session is NULL", 0, 0, 0);
//         return pSession;
//     }

//     IMS_TRACE_I("CreateSession : UIKey[%s] IMSKey[%" PFLS_u "]", UIKey.GetStr(), IMSKey, 0);

//     m_pUCSessList->Add(pSession);
//     pSession->Init(m_pUCSessList);
//     return pSession;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// UCECTReference* UCECTMngr::CreateComReference(IN IMtcCall* pSession)
// {
//     return new UCECTReference(m_pApp, this, pSession);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// IMS_BOOL UCECTMngr::UpdateService(IN IMtcCall* pSession, IN IMtcService* pService,
//         IN IMS_UINT32 eLocalServiceType)
// {
//     if (pService == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "pService is NULL", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     IMS_TRACE_I("UpdateService : [%d]", pSession->GetService()->GetServiceType(), 0, 0);

//     if (pSession->GetService()->GetServiceType()== SERVICETYPE_EMERGENCY)
//     {
//         return IMS_TRUE;
//     }

//     pSession->UpdateService(pService, eLocalServiceType);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// AString UCECTMngr::GetTransferTargetURI()
// {
//     IMS_TRACE_D("GetTransferTargetURI() : %s", m_strTransferTargetURI.GetStr(), 0, 0);
//     return m_strTransferTargetURI;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// void UCECTMngr::SetState(IN IMS_UINT32 eState)
// {
//     IMS_TRACE_I("SetState : [%s][%s]->[%s]", PrintState(m_eOldState), PrintState(),
//             PrintState(eState));

//     if (m_eState == TERMINATING)
//     {
//     }
//     else
//     {
//         m_eOldState = m_eState;
//     }

//     m_eState = eState;
//     IMSStateMachine::SetState(m_eState);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// void UCECTMngr::SendResumedByToUI(IN IMtcCall* pExistSession)
// {
//     // Transferee, Transfer target case
//     if (pExistSession == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "SendResumedByToUI : Session is NULL.", 0, 0, 0);
//         return;
//     }

//     MediaInfo* pMediaInfo = new MediaInfo(*pExistSession->GetMediaMngr()->GetMediaInfo());
//     IMS_TRACE_I("SendResumedByToUI :: [%s]", PS_Direction(pMediaInfo->eADir), 0, 0);

//     if (pMediaInfo->eADir == DIRECTION_RECEIVE || pMediaInfo->eADir == DIRECTION_INACTIVE)
//     {
//         pMediaInfo->eADir = DIRECTION_SEND_RECEIVE;

//         IMtcCall* pTransferredSession = GetSession(m_pTransferredSessKey);
//         if (pTransferredSession == IMS_NULL)
//         {
//             delete pMediaInfo;
//             return;
//         }

//         CallInfo* pCallInfo = new CallInfo(*pTransferredSession->GetSessInfo());

//         IMSMap<IMS_UINT32, SuppService*> lstSuppServices;
//         pTransferredSession->GetSuppService()->GetList(lstSuppServices);

//         // TODO, MTC BUILD
//         MtcUiNotifier* pProxy = IMS_NULL;
//         // MtcUiNotifier* pProxy = pExistSession->GetProxy();
//         if (pProxy == IMS_NULL)
//         {
//             IMS_TRACE_D("SendResumedByToUI : m_pProxy is NULL.", 0, 0, 0);

//             delete pCallInfo;
//             delete pMediaInfo;
//             return;
//         }

//         pProxy->SendResumedBy(pCallInfo, pMediaInfo,lstSuppServices);
//     }
//     else
//     {
//         delete pMediaInfo;
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// void UCECTMngr::SendECTStartedByToUI(IN IMtcCall* pExistSession)
// {
//     // Transferee, Transfer target case
//     if (pExistSession == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "SendECTStartedByToUI : Session is NULL.", 0, 0, 0);
//         return;
//     }

//     IMtcCall* pTransferredSession = GetSession(m_pTransferredSessKey);
//     if (pTransferredSession == IMS_NULL)
//     {
//         return;
//     }

//     CallInfo* pCallInfo = new CallInfo(*pTransferredSession->GetSessInfo());
//     MediaInfo* pMediaInfo = new MediaInfo(*pTransferredSession->GetMediaMngr()->GetMediaInfo());

//     IMSMap<IMS_UINT32, SuppService*> lstSuppServices;
//     pTransferredSession->GetSuppService()->GetList(lstSuppServices);

//     // TODO, MTC BUILD
//     MtcUiNotifier* pProxy = IMS_NULL;
//     // MtcUiNotifier* pProxy = pExistSession->GetProxy();

//     if (pProxy == IMS_NULL)
//     {
//         IMS_TRACE_D("SendECTStartedByToUI : Proxy is NULL.", 0, 0, 0);

//         delete pCallInfo;
//         delete pMediaInfo;
//         return;
//     }

//     IMS_UINTP nType = REPALCED_BY_TYPE_ECT;
//     IMS_TRACE_I("SendECTStartedByToUI : new[%" PFLS_u "]", m_pTransferredSessKey, 0, 0);

//     pProxy->SendReplacedBy(
//             pCallInfo, pMediaInfo, lstSuppServices, m_pTransferredSessKey, nType);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void UCECTMngr::SendECTResultToUI(IN IMS_BOOL bResult, IN IMS_SINT32 eReason, IN IMS_SINT32 eCode)
// {
//     if (m_strUIKey.GetLength() <= 0)
//     {
//         IMS_TRACE_E(0, "SendECTResultToUI : UIKey is NULL or Empty", 0, 0, 0);
//         return;
//     }

//     IUUCSessionECTCompletedParam* pParam = new IUUCSessionECTCompletedParam();

//     pParam->bResult = bResult;
//     pParam->failReason = FailReason(eReason, eCode);
//     pParam->aStrPhrase = AString::ConstEmpty();

//     IMS_TRACE_I("SendECTResultToUI [%s]: Reason[%d]Code[%d]", PS_BOOL(bResult), eReason, eCode);
//     IMS_MSG_CreateNPostThreadMessageByName(m_strUIKey,
//             IuMtcCall::ECT_COMPLETED, 0, (IMS_UINTP)pParam);

//     m_strUIKey = AString::ConstNull();
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// IMS_BOOL UCECTMngr::CreateRefer()
// {
//     IMtcCall* pTransfereeSession = GetSession(m_pTransfereeSessKey);
//     if (pTransfereeSession == IMS_NULL)
//     {
//         IMS_TRACE_E(0,"CreateRefer : pTransfereeSession is null", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     m_pECTReference = CreateComReference(pTransfereeSession);
//     if (m_pECTReference == IMS_NULL)
//     {
//         IMS_TRACE_E(0,"pRefer is NULL", 0, 0, 0);
//         return IMS_FALSE;
//     }
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// IMS_BOOL UCECTMngr::SendRefer()
// {
//     if (m_pECTReference == IMS_NULL)
//     {
//         IMS_TRACE_I("SendRefer : ECTReference is null", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     IMS_TRACE_I("SendRefer", 0, 0, 0);

//     if (m_pECTReference->GetState() == UCECTReference::ECT_REFER_IDLE)
//     {
//         return m_pECTReference->Refer();
//     }
//     return IMS_FALSE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// void UCECTMngr::SetBlindECT(IN IMS_BOOL bBlindECT)
// {
//     IMS_TRACE_I("SetBlindECT :[%s] --> [%s]", PS_BOOL(m_bBlindECT), PS_BOOL(bBlindECT), 0);
//     m_bBlindECT = bBlindECT;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// IMS_BOOL UCECTMngr::CreateECTSession()
// {
//     // Transferee case - creates session to send INVITE to transfer target
//     IUUCServiceOpenSessionParam* pParam = new IUUCServiceOpenSessionParam();

//     IMS_TRACE_I("CreateECTSession", 0, 0, 0);

//     IMtcCall* pTransfereeSession = GetSession(m_pTransfereeSessKey);
//     if (pTransfereeSession == IMS_NULL)
//     {
//         IMS_TRACE_D("CreateECTSession : pTransfereeSession is NULL", 0, 0, 0);
//         m_pListener->ECTMngr_SessionStartFailed((IMS_UINTP)pParam);
//         return IMS_FALSE;
//     }

//     pParam->pService = pTransfereeSession->GetService();
//     pParam->nIMSKey = STATIC_CAST(IMS_UINTP, 0);
//     pParam->strUIKey = AString::ConstEmpty();

//     IMtcCall* pTransferredSession = CreateSession(pParam->pService, pParam->strUIKey,
//             pParam->nIMSKey);
//     if (pTransferredSession == IMS_NULL)
//     {
//         IMS_TRACE_D("CreateECTSession : pTransferredSession is NULL", 0, 0, 0);
//         m_pListener->ECTMngr_SessionStartFailed((IMS_UINTP)pParam);
//         return IMS_FALSE;
//     }

//     m_pTransferredSessKey = pTransferredSession->GetKey();

//     UpdateService(pTransferredSession, pParam->pService, pParam->pService->GetServiceType());

//     IUUCSessionStartParam* pStartParam = new IUUCSessionStartParam();

//     MediaInfo* m_pMediaInfo = new MediaInfo();
//     m_pMediaInfo->eAQuality = AUDIO_QUALITY_AMR_WB;
//     m_pMediaInfo->eADir = DIRECTION_SEND_RECEIVE;

//     //need to assign the 3 values
//     pStartParam->pMediaInfo = m_pMediaInfo;
//     pStartParam->pService = pTransferredSession->GetService();
//     pStartParam->aStrTarget = GetTransferTargetURI();

//     IMSMSG objMsg(IuMtcCall::START, 0, (IMS_UINTP)pStartParam);
//     pTransferredSession->SendMsg(objMsg);
//     pTransferredSession->SetIsECTProgressing(IMS_TRUE);

//     SetState(RINGBACK);
//     delete pParam;
//     return IMS_TRUE;
// }

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PROTECTED
const IMS_CHAR* UCECTMngr::PrintState(IN IMS_SINT32 eState)
{
    if (eState <= -1)
    {
        eState = m_eState;
    }

    switch (eState)
    {
        case IDLE:
            return "IDLE";
        case NOTIFYING:
            return "NOTIFYING";
        case RINGING:
            return "RINGING";
        case RINGBACK:
            return "RINGBACK";
        case REFERRING:
            return "REFERRING";
        case WAITINGNOTIFY:
            return "WAITINGNOTIFY";
        case WAITINGBYE:
            return "WAITINGBYE";
        case INHR:
            return "INHR";
        case TERMINATING:
            return "TERMINATING";

        default:
            return "__INVALID__";
    }
}

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED
// IMtcCall* UCECTMngr::GetSession(IMS_SINTP nSessionKey)
// {
//     if (m_pUCSessList == IMS_NULL || nSessionKey == -1)
//     {
//         IMS_TRACE_I("GetSession : can't get a session with key", 0, 0, 0);
//         return IMS_NULL;
//     }

//     return m_pUCSessList->GetWithSessionKey(nSessionKey);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL UCECTMngr::DestroyRefer()
// {
//     if (m_pECTReference == IMS_NULL)
//     {
//         IMS_TRACE_I("DestroyRefer : m_pECTReference is NULL", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     delete m_pECTReference;
//     m_pECTReference = IMS_NULL;

//     IMS_TRACE_I("DestroyRefer", 0, 0, 0);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL UCECTMngr::SendHold()
// {
//     // Transferee case
//     IMS_TRACE_I("SendHold[%s]", PrintState(), 0, 0);

//     IUUCSessionHoldParam* pHRParam = new IUUCSessionHoldParam();

//     IMtcCall* pTransfereeSession = GetSession(m_pTransfereeSessKey);
//     if (pTransfereeSession == IMS_NULL)
//     {
//         return IMS_FALSE;
//     }

//     MediaInfo* m_pMediaInfo = new MediaInfo(*pTransfereeSession->GetMediaInfo());
//     if (m_pMediaInfo->eADir == DIRECTION_RECEIVE)
//     {
//         m_pMediaInfo->eADir = DIRECTION_INACTIVE;
//     }
//     else if (m_pMediaInfo->eADir == DIRECTION_SEND_RECEIVE)
//     {
//         m_pMediaInfo->eADir = DIRECTION_SEND;
//     }

//     pHRParam->pMediaInfo = m_pMediaInfo;

//     IMSMSG objMsg(IuMtcCall::HOLD, 0, (IMS_UINTP)pHRParam);
//     pTransfereeSession->SendMsg(objMsg);

//     SetState(INHR);

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL UCECTMngr::HandleTransfereeWaitBYE()
// {
//     // Transferee case - the timer is expired for waiting BYE request to session with transferee
//     IMtcCall* pTransfereeSession = GetSession(m_pTransfereeSessKey);
//     if (pTransfereeSession == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "HandleTransfereeWaitBYE : Session With Transferee is null", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     m_pTimer->AllStop();

//     IMS_TRACE_I("HandleTransfereeWaitBYE[%s] : current[%" PFLS_u "] 1to1[%" PFLS_u "]",
//             PrintState(), m_pTransferredSessKey, m_pTransfereeSessKey);

//     m_pUCSessList->Terminate(m_pTransfereeSessKey,
//             FailReason(FAIL_REASON_ECT_REPLACED, -1), IMS_FALSE);

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL UCECTMngr::SetSessionListn()
// {
//     if (m_pUCSessList == IMS_NULL)
//     {
//         return IMS_FALSE;
//     }

//     m_pUCSessList->SetSessionListener(this);
//     IMS_TRACE_I("SetSessionListn : Done", 0, 0, 0);
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// void UCECTMngr::ResetSessionListn()
// {
//     if (m_pUCSessList == IMS_NULL)
//     {
//         return;
//     }

//     IMS_TRACE_I("ResetSessionListn : Done", 0, 0, 0);
//     m_pUCSessList->ReleaseSessionListener(this);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL UCECTMngr::IsBlindECT()
// {
//     IMS_TRACE_I("IsBlindECT :[%s]", PS_BOOL(m_bBlindECT), 0, 0);
//     return m_bBlindECT;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL UCECTMngr::IsECTTerminateRequired(IN IMS_SINTP nDestroyedSessionKey)
// {
//     IMS_BOOL bTransfereeSessionDestroyed = IMS_FALSE;
//     IMS_BOOL bTransferTargetSessionDestroyed = IMS_FALSE;
//     IMS_BOOL bTransferredSessionDestroyed = IMS_FALSE;

//     if (nDestroyedSessionKey == m_pTransfereeSessKey)
//     {
//         IMS_TRACE_D("IsECTTerminateRequired : bTransfereeSessionDestroyed", 0, 0, 0);
//         bTransfereeSessionDestroyed = IMS_TRUE;
//     }
//     else if (nDestroyedSessionKey == m_pTransferTargetSessKey)
//     {
//         IMS_TRACE_D("IsECTTerminateRequired : bTransferTargetSessionDestroyed", 0, 0, 0);
//         bTransferTargetSessionDestroyed = IMS_TRUE;
//     }
//     else if (nDestroyedSessionKey == m_pTransferredSessKey)
//     {
//         IMS_TRACE_D("IsECTTerminateRequired : bTransferredSessionDestroyed", 0, 0, 0);
//         bTransferredSessionDestroyed = IMS_TRUE;
//     }
//     else
//     {
//         IMS_TRACE_D("IsECTTerminateRequired : the session is not related to ECT", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     IMS_TRACE_I("IsECTTerminateRequired : Destroyed Session [%" PFLS_u "], state [%s]",
//             nDestroyedSessionKey, PrintState(GetState()), 0);

//     switch (GetState())
//     {
//         /* all cases */
//         case IDLE:
//             return IMS_TRUE;

//         /* transferee case */
//         case NOTIFYING: // [fall-through]
//         case INHR:      // [fall-through]
//         case RINGBACK:
//             return bTransfereeSessionDestroyed;
//         case WAITINGBYE:
//             return (bTransfereeSessionDestroyed || bTransferredSessionDestroyed);

//         /* transferor case */
//         case REFERRING:
//             return (bTransfereeSessionDestroyed || bTransferTargetSessionDestroyed);
//         case WAITINGNOTIFY:
//             return bTransfereeSessionDestroyed;

//         /* transfer target case */
//         case RINGING:
//             return (bTransferTargetSessionDestroyed || bTransferredSessionDestroyed);
//         case TERMINATING:
//             return bTransferTargetSessionDestroyed;

//         default:
//             break;
//     }

//     return IMS_FALSE;
// }
