/*
 * author : aromi.kwak
 * version : v1.0
 * date : 2015.12
 * brief : Create USSIEarlySession
 */

#include "ServiceTrace.h"
#include "ServiceEvent.h"

#include "ISipHeader.h"
#include "ISession.h"
#include "IMessage.h"

#include "utility/MessageUtil.h"
#include "call/IMtcCall.h"

#include "ussi/USSIEarlySession.h"
#include "ussi/USSISession.h"
#include "ussi/UssiConstants.h"

__IMS_TRACE_TAG_COM_UC__;

/* ------------------------------------------------------------------------------------------------
    Constructor, Destructor
------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
USSIEarlySession::USSIEarlySession(IN ISession* pISession, IN IMtcCall* pSession)
// TODO, MTC BUILD
// : EarlySession(pISession, pSession)
{
    // TODO, MTC BUILD
    UNUSED_PARAM(pISession);
    UNUSED_PARAM(pSession);
    IMS_TRACE_MEM("uc", "uc_M : USSIEarlySession[%" PFLS_u "][%" PFLS_x "]",
            sizeof(USSIEarlySession), this, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL USSIEarlySession::~USSIEarlySession()
{
    IMS_TRACE_MEM("uc", "uc_F : USSIEarlySession[%" PFLS_u "][%" PFLS_x "]",
            sizeof(USSIEarlySession), this, 0);
}

// TODO, MTC BUILD
// /*

//     State Machine METHODS

// */
// /*

//  -----------------------------------------------------------------------------------------------
//  */
// PROTECTED VIRTUAL
// IMS_BOOL USSIEarlySession::StateIDLE_Start(IN IMSMSG &objMsg)
// {
//     IEarlySessionStartParam* pParam = reinterpret_cast<IEarlySessionStartParam*>(objMsg.nLparam);
//     IMessage* pIMessage = m_pISession->GetNextRequest();
//     FailReason failReason;

//     IMS_TRACE_I("StateIDLE_Start", 0, 0, 0);

//     SetSessInfo(pParam->pCallInfo);
//     m_pMediaMngr->SetMediaInfo(pParam->pMediaInfo);

//     AddUSSIHeaders(pIMessage);

//     if (Media_FormSDP(m_pSessInfo->eSessionType) == IMS_FAILURE)
//     {
//         failReason.eReason = FAIL_REASON_MEDIA_NEGOFAIL;
//         goto Exit_StateIDLE_Start;
//     }

//     if (m_pMessage->Start(m_pSession, m_pSessInfo) != IMS_SUCCESS)
//     {
//         failReason.eReason = FAIL_REASON_SESSION_UNKNOWN;
//         goto Exit_StateIDLE_Start;
//     }

//     SetState(ESTABLISHING);

//     Exit_StateIDLE_Start:

//     if (failReason.eReason != FAIL_REASON_SESSION_NONE)
//     {
//         IMS_BOOL bTerminated = !HandleCancel(&failReason);
//         SendStartFailedToListn(failReason, bTerminated);
//     }

//     delete pParam;
//     return IMS_TRUE;

// }

// /*

// */ PROTECTED VIRTUAL IMS_BOOL USSIEarlySession::StateIDLE_IncomingSession(IN IMSMSG &objMsg)
// {
//     IEarlySessionIncomingParam* pParam
//             = reinterpret_cast<IEarlySessionIncomingParam*>(objMsg.nLparam);
//     IMessage* pIMessage = m_pISession->GetPreviousRequest(IMessage::SESSION_START);
//     FailReason rejectReason;
//     USSISession* pUSSISession = DYNAMIC_CAST(USSISession*, m_pSession);

//     IMS_TRACE_I("StateIDLE_IncomingSession", 0, 0, 0);

//     SetSessInfo(pParam->pCallInfo);
//     m_nSessionKey = pParam->nSessionKey;
//     m_stCallerInfo = pParam->stCallerInfo;

//     if (!IsAvailableFeatures(pIMessage))
//     {
//         rejectReason.eReason= REJECT_REASON_SESSION_NOTSUPPORT;
//         goto Exit_StateIDLE_IncomingSession;
//     }

//     if (Media_NegoSDP(pIMessage) == IMS_FAILURE)
//     {
//         rejectReason.eReason = REJECT_REASON_MEDIA_NEGOFAIL;
//         goto Exit_StateIDLE_IncomingSession;
//     }

//     if (m_pSuppService->UpdateUSSD(m_pISession, pIMessage)
//             && pUSSISession->Control(
//                     USSISession::CMD_CHECK_INCOMING_USSI_DATA,
//                     reinterpret_cast<IMS_UINTP>(pIMessage->GetMessage()), 0))
//     {
//         SetState(ALERTING);
//         SendAlertingToListn();
//     }
//     else
//     {
//         rejectReason.eReason = REJECT_REASON_SERVICE_UNAVAILABLE;
//     }

//     Exit_StateIDLE_IncomingSession:

//     if (rejectReason.eReason != REJECT_REASON_INVALID)
//     {
//         IMS_BOOL bTerminated = !HandleReject(&rejectReason);
//         SendStartFailedToListn(rejectReason, bTerminated);
//     }

//     delete pParam;
//     return IMS_TRUE;
// }

// /*

// */ PROTECTED VIRTUAL IMS_BOOL USSIEarlySession::StateESTABLISHING_SSStarted(IN IMSMSG
// &/*objMsg*/)
// {
//     IMessage* pIMessage = MessageUtil::GetPreviousResponse(m_pISession, IMessage::SESSION_START);
//     FailReason failReason;

//     IMS_TRACE_I("StateESTABLISHING_SSStarted", 0, 0, 0);

//     UpdateServiceType(pIMessage);
//     UpdateSessionType(pIMessage, IMS_FALSE);

//     if (UpdateRemoteFeatures(pIMessage)) {
//     }

//     if (m_pMessage->SendAck(m_pISession, m_pService, m_pSessInfo, m_pSuppService,
//     m_nLocalFeatures,
//             m_nSupportedFeatures, m_nRequireFeatures) == IMS_FAILURE)
//     {
//         failReason.eReason = FAIL_REASON_SESSION_UNKNOWN;
//         goto Exit_StateESTABLISHING_SSStarted;
//     }

//     SetState(CONVERSATION);
//     DeInit();
//     SendStartedToListn();

//     Exit_StateESTABLISHING_SSStarted:

//     if (failReason.eReason != FAIL_REASON_SESSION_NONE)
//     {
//         if (m_pMessage->SendAck(m_pISession, m_pService, m_pSessInfo,
//                 m_pSuppService,m_nLocalFeatures, m_nSupportedFeatures, m_nRequireFeatures)
//                 == IMS_FAILURE)
//         {
//             //do nothing
//         }

//         IMS_BOOL bTerminated = !HandleCancel(&failReason);
//         SendStartFailedToListn(failReason, bTerminated);
//     }

//     return IMS_TRUE;
// }

// /*

// */ PROTECTED VIRTUAL IMS_BOOL USSIEarlySession::StateALERTING_UserAlert(IN IMSMSG &objMsg)
// {
//     IEarlySessionUserAlertParam* pParam
//             = reinterpret_cast<IEarlySessionUserAlertParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateALERTING_UserAlert", 0, 0, 0);

//     IEarlySessionAcceptParam* pESParam = new IEarlySessionAcceptParam();
//     pESParam->pCallInfo = new CallInfo(*m_pSessInfo);
//     pESParam->pMediaInfo = new MediaInfo(*m_pMediaMngr->GetMediaInfo());

//     IMSMSG objAcceptMsg(EarlySession::EARLY_SESSION_ACCEPT, 0, (IMS_UINTP)pESParam);

//     StateALERTING_Accept(objAcceptMsg);

//     delete pParam;
//     return IMS_TRUE;
// }

// /*

// */ PROTECTED VIRTUAL IMS_BOOL USSIEarlySession::StateALERTING_Accept(IN IMSMSG &objMsg)
// {
//     IEarlySessionAcceptParam* pParam =
//     reinterpret_cast<IEarlySessionAcceptParam*>(objMsg.nLparam); FailReason rejectReason;
//     IMessage* pIMessage = m_pISession->GetNextResponse();

//     IMS_TRACE_I("StateALERTING_Accept", 0, 0, 0);

//     SetSessInfo(pParam->pCallInfo);
//     m_pMediaMngr->SetMediaInfo(pParam->pMediaInfo);

//     m_pMessage->AddRecvInfoHeader(pIMessage, FormRecvInfoHeader());
//     m_pMessage->AddAcceptHeader(pIMessage, FormAcceptHeader());

//     if (m_pMessage->Accept(m_pISession, m_pService, m_pSessInfo, m_pSuppService,
//     m_nLocalFeatures,
//             m_nSupportedFeatures, m_nRequireFeatures) == IMS_FAILURE)
//     {
//         rejectReason.eReason = REJECT_REASON_SESSION_FAIL;
//         goto Exit_StateALERTING_Accept;
//     }

//     Exit_StateALERTING_Accept:

//     if (rejectReason.eReason == REJECT_REASON_INVALID)
//     {
//         SetState(CONVERSATION);
//         DeInit();
//         SendStartedToListn();
//     }
//     else
//     {
//         IMS_BOOL bTerminated = !HandleReject(&rejectReason);
//         SendStartFailedToListn(rejectReason, bTerminated);
//     }

//     delete pParam;
//     return IMS_TRUE;
// }

// /*

// */ PROTECTED VIRTUAL void USSIEarlySession::SendStartedToListn()
// {
//     IMS_TRACE_I("SendStartedToListn", 0, 0, 0);
//     IMessage* pIMessage = m_pISession->GetPreviousRequest(IMessage::SESSION_START);

//     IEarlySessionRequestReceivedParam* pParam = new IEarlySessionRequestReceivedParam();
//     pParam->pISession = m_pISession;
//     pParam->pCallInfo = new CallInfo(*m_pSessInfo);
//     MessageUtil::GetHeader(pIMessage, ISipHeader::UNKNOWN, "Session-ID", pParam->aStrSessionID);
//     pParam->stTIPInfo = m_stTIPInfo;
//     pParam->nRequireFeatures = m_nRequireFeatures;
//     pParam->nSupportFeatures = m_nSupportedFeatures;

//     Precondition_GetLocalQoSTables(pParam->lstLocalQosTable);

//     pParam->pQoSInfo = new UCQoS::QoSInfo(m_pSession);

//     Precondition_GetLocalQoSInfo(pParam->pQoSInfo);
//     pParam->pQoSInfo->PrintQoSInfo();

//     m_pListener->Early_Started((IMS_UINTP)pParam);
// }

// /*

// */ PROTECTED VIRTUAL void USSIEarlySession::AddUSSIHeaders(IN IMessage* pIMessage)
// {
//     IMS_TRACE_I("AddUSSIHeaders", 0, 0, 0);

//     m_pMessage->AddRecvInfoHeader(pIMessage, FormRecvInfoHeader());
//     m_pMessage->AddAcceptHeader(pIMessage, FormAcceptHeader());
//     m_pMessage->AddContentTypeHeader(pIMessage, FormContentTypeHeader());
// }

// /* ----------------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------------- */
// PROTECTED VIRTUAL
// IMSList<AString> USSIEarlySession::FormRecvInfoHeader()
// {
//     IMS_TRACE_I("FormRecvInfoHeader", 0, 0, 0);

//     IMSList<AString> objRecvInfoHeaders;
//     objRecvInfoHeaders.Append(USSDConstants::HEADER_USSD_PACKAGE);

//     return objRecvInfoHeaders;
// }

// /*

// */ PROTECTED VIRTUAL IMSList<AString> USSIEarlySession::FormAcceptHeader()
// {
//     IMS_TRACE_I("FormAcceptHeader", 0, 0, 0);

//     IMSList<AString> objAcceptHeaders;
//     objAcceptHeaders.Append(USSDConstants::HEADER_APPLICATION_SDP);
//     objAcceptHeaders.Append(USSDConstants::HEADER_APPLICATION_IMSXML);
//     objAcceptHeaders.Append(USSDConstants::HEADER_APPLICATION_USSDXML);
//     objAcceptHeaders.Append(USSDConstants::HEADER_MULTIPART_MIXED);

//     return objAcceptHeaders;
// }

// /*

// */ PROTECTED VIRTUAL IMSList<AString> USSIEarlySession::FormContentTypeHeader()
// {
//     IMS_TRACE_I("FormContentTypeHeader", 0, 0, 0);

//     IMSList<AString> objContentTypeHeaders;
//     objContentTypeHeaders.Append(USSDConstants::HEADER_MULTIPART_MIXED);

//     return objContentTypeHeaders;
// }

// /*

// */ PROTECTED VIRTUAL void USSIEarlySession::LoadConfig()
// {
//     return EarlySession::LoadConfig();
// }
