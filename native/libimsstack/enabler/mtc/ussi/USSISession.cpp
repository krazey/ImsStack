/*
 * author : aromi.kwak@
 * date : 201510
 * brief : Create USSISession
 */


#include "ServiceTrace.h"

#include "IMessage.h"
#include "ISipMessage.h"

#include "ussi/UssiConstants.h"
#include "ussi/USSISession.h"
#include "ussi/USSIEarlySession.h"
#include "ussi/USSIConfirmedSession.h"
#include "ussi/UssiDataCreator.h"

#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

#include "MtcDef.h"

__IMS_TRACE_TAG_COM_UC__;

// TODO, MTC BUILD
PUBLIC
USSISession::USSISession(IN IMtcApp* pApp, IN IMtcService* pService, IN AString aStrUIKey,
        IN IMS_UINTP nIMSKey)
// PUBLIC
// USSISession::USSISession(IN IMtcApp* pApp, IN IMtcService* pService, IN AString aStrUIKey,
//         IN IMS_UINTP nIMSKey, IN IUCSessionListener* pListener)
//     : UCSession(pApp, pService, aStrUIKey, nIMSKey, pListener)
{
    UNUSED_PARAM(pApp);
    UNUSED_PARAM(pService);
    UNUSED_PARAM(aStrUIKey);
    UNUSED_PARAM(nIMSKey);
    IMS_TRACE_MEM("uc", "uc_M : USSISession[%" PFLS_u "][%" PFLS_x "]", sizeof(USSISession), this,
            0);
    //---------------------------------------------------------------------------------------------
}

PUBLIC VIRTUAL
USSISession::~USSISession()
{
    IMS_TRACE_MEM("uc", "uc_F : USSISession[%" PFLS_u "][%" PFLS_x "]", sizeof(USSISession), this,
            0);
    //---------------------------------------------------------------------------------------------
}

// TODO, MTC BUILD

// /* M ----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PUBLIC VIRTUAL
// AString USSISession::GetToURI(IN ICoreService* /*pIService*/, IN ISession* /*pISession*/,
//         IN const AString &/*aStrNumber*/)
// {
//     return AString::ConstNull();
// }

// // TODO, MTC BUILD
// // /* M ----------------------------------------------------------------------------------------------
// // ------------------------------------------------------------------------------------------------ */
// // PROTECTED VIRTUAL
// // EarlySession* USSISession::EarlySession_CreateCom(IN ISession* pISession)
// // {
// //     USSIEarlySession *pUSSIEarlySession = new USSIEarlySession(pISession, this);
// //     return pUSSIEarlySession;
// // }

// // /* M ----------------------------------------------------------------------------------------------
// // ------------------------------------------------------------------------------------------------ */
// // PROTECTED VIRTUAL
// // ConfirmedSession* USSISession::ConfirmedSession_CreateCom(IN ISession* /*pISession*/)
// // {
// //     USSIConfirmedSession *pUSSIConfirmedSession = new USSIConfirmedSession(this);
// //     return pUSSIConfirmedSession;
// // }

// /* ------------------------------------------------------------------------------------------------
//     State Machine METHODS
// ------------------------------------------------------------------------------------------------ */

// /* ------------------------------------------------------------------------------------------------
//  ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::StateIDLE_Start(IN IMSMSG &objMsg)
// {
//     IUUCSessionStartParam* pParam = reinterpret_cast<IUUCSessionStartParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateIDLE_Start", 0, 0, 0);

//     m_pSessInfo->ePeerType = PEERTYPE_MO;
//     SetSessionType(pParam->eSessionType);
//     Media_Start(pParam->pMediaInfo);

//     LoadConfig();
//     AddEventListn();

//     ISession* pISession = CreateISession(START_NORMAL, pParam->aStrTarget, pParam->pDialog);
//     if (pISession == IMS_NULL)
//     {
//         HandleTerminating(FailReason(FAIL_REASON_SESSION_UNKNOWN, -1), IMS_TRUE);

//         delete pParam;
//         return IMS_TRUE;
//     }

//     EarlySession* pEarlySession = EarlySession_Create(pISession);
//     EarlySession_Start(pEarlySession, IMSList<ConfUser*>(), pParam->pDialog);

//     IMessage* pIMessage = m_pISession->GetNextRequest();
//     if (pIMessage != IMS_NULL)
//     {
//         SetUSSIBody(pIMessage->GetMessage(), pParam->aStrTarget);

//         SendRingBackToListn();
//         SetState(RINGBACK);
//     }
//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::StateIDLE_IncomingSession(IN IMSMSG &objMsg)
// {
//     IUUCServiceIncomingSessionParam* pParam
//             = reinterpret_cast<IUUCServiceIncomingSessionParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateIDLE_IncomingSession", 0, 0, 0);

//     m_pSessInfo->ePeerType = PEERTYPE_MT;
//     SessionTransactionHelper::GetInstance()->AddISession(pParam->pISession);
//     pParam->pISession->SetListener(this);
//     Proxy_RegisterService(pParam->aStrJNIServiceName);
//     Media_Start(pParam->pMediaInfo);

//     SuppService_Create();
//     UpdateIncomingInformation(pParam->pISession);
//     LoadConfig();
//     AddEventListn();

//     EarlySession* pEarlySession = EarlySession_Create(pParam->pISession);

//     IEarlySessionIncomingParam* pESParam = new IEarlySessionIncomingParam();
//     pESParam->pCallInfo = new CallInfo(*m_pSessInfo);
//     pESParam->nSessionKey = reinterpret_cast<IMS_SINTP>(this);
//     GetCallerInfo(&(pESParam->stCallerInfo));
//     pEarlySession->PostMessage(EarlySession::EARLY_SESSION_INCOMING, 0, (IMS_UINTP)pESParam);

//     SetState(RINGING);
//     HandleCallState(VOLTE_CALL_STATE_RINGING);

//     SendRingingToListn();

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::StateRINGING_Started(IN IMSMSG &objMsg)
// {
//     IEarlySessionRequestReceivedParam* pParam
//             = reinterpret_cast<IEarlySessionRequestReceivedParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateRINGING_Started", 0, 0, 0);

//     SetSessInfo(pParam->pCallInfo);
//     m_pISession = pParam->pISession;
//     m_nRequireFeatures = pParam->nRequireFeatures;
//     m_nSupportedFeatures = pParam->nSupportFeatures;

//     IMessage* pIMessage = m_pISession->GetPreviousRequest(IMessage::SESSION_START);

//     if (pIMessage == IMS_NULL)
//     {
//         delete pParam;
//         return IMS_TRUE;
//     }

//     ConfirmedSession_Create(pParam->pISession, pParam->lstLocalQosTable, pParam->pQoSInfo);
//     EarlySession_Destroy(FailReason());

//     USSDDataParser* pUSSDData = GetParsedUSSIData(pIMessage->GetMessage());

//     if (pUSSDData == IMS_NULL || (m_pConfirmedSession == IMS_NULL))
//     {
//         SetState(CONVERSATION);
//         HandleCallState(VOLTE_CALL_STATE_OFFHOOK);

//         SendStartedToUI();
//         SendStartedToListn();

//         delete pParam;
//         return IMS_TRUE;
//     }

//     USSIConfirmedSession* pUSSIConfirmedSession
//             = DYNAMIC_CAST(USSIConfirmedSession*, m_pConfirmedSession);
//     pUSSIConfirmedSession->SetUSSType(pUSSDData->GetAnyExtension().GetUSSType());

//     if (pUSSDData->GetUSSDString().GetLength() <= 0)
//     {
//         m_pConfirmedSession->PostMessage(
//                 USSIConfirmedSession::USSI_CONFIRMED_SEND_ERROR_INFO, 0,
//                 DYNAMIC_CAST(IMS_UINTP, USSDDataParser::ERROR_CODE_1));
//     }
//     else if ((m_pSessList != IMS_NULL) && (m_pSessList->GetNum() > 1))
//     {
//         IMS_TRACE_I("There's other ongoing session, so drop this ussi session", 0, 0, 0);
//         m_pConfirmedSession->PostMessage(
//                 USSIConfirmedSession::USSI_CONFIRMED_SEND_ERROR_INFO, 0,
//                 DYNAMIC_CAST(IMS_UINTP, USSDDataParser::ERROR_CODE_4));

//         m_pSessList->Terminate(
//                 GetKey(), FailReason(FAIL_REASON_SESSION_UNKNOWN, -1), IMS_FALSE);
//     }
//     else
//     {
//         SendUSSIResultToUI(pUSSDData);
//     }

//     SetState(CONVERSATION);
//     HandleCallState(VOLTE_CALL_STATE_OFFHOOK);

//     SendStartedToUI();
//     SendStartedToListn();

//     // USS-Type : Notify case, it should send INFO request with USS-Type
//     if ((pUSSDData->GetUSSDString().GetLength() > 0)
//             && (pUSSDData->GetAnyExtension().GetUSSType()
//                 == USSDDataParser::AnyExtension::USS_TYPE_NOTIFY))
//     {
//         IConfirmedSendTransactionParam* pSendTransactionParam
//                 = new IConfirmedSendTransactionParam();
//         m_pConfirmedSession->PostMessage(ConfirmedSession::CONFIRMED_SS_SENDTRANSACTION,
//                 0, (IMS_UINTP)pSendTransactionParam);
//     }

//     delete pUSSDData;
//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::StateCONVERSATION_Terminate(IN IMSMSG &objMsg)
// {
//     IUUCSessionTerminateParam* pParam
//             = reinterpret_cast<IUUCSessionTerminateParam*>(objMsg.nLparam);
//     FailReason terminateReason(ConvertTerminateReasonToFailReason(pParam->failReason.eReason),
//             pParam->failReason.eCode);

//     IMS_TRACE_I("StateCONVERSATION_Terminate : %s", PS_FR(terminateReason), 0, 0);

//     if (terminateReason.eReason == FAIL_REASON_SESSION_USERTERMINATE)
//     {
//         IMS_TRACE_I("StateCONVERSATION_Terminate : ussi session is rejected by user", 0, 0, 0);
//         m_pConfirmedSession->PostMessage(
//                 USSIConfirmedSession::USSI_CONFIRMED_SEND_ERROR_INFO, 0,
//                 DYNAMIC_CAST(IMS_UINTP, USSDDataParser::ERROR_CODE_1));
//     }

//     StopSendingKeepAlive();

//     if (ConfirmedSession_Terminate(terminateReason))
//     {
//     }

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::StateCONVERSATION_SendTransaction(IN IMSMSG &objMsg)
// {
//     IUUCSessionSendTransactionParam* pParam
//             = reinterpret_cast<IUUCSessionSendTransactionParam*>(objMsg.nLparam);

//     IMS_TRACE_D("StateCONVERSATION_SendTransaction : USSI[%s]", pParam->aStrUSSI.GetStr(), 0, 0);

//     IConfirmedSendTransactionParam* pSendTransactionParam = new IConfirmedSendTransactionParam();
//     pSendTransactionParam->aStrUSSIString = pParam->aStrUSSI;

//     m_pConfirmedSession->PostMessage(ConfirmedSession::CONFIRMED_SS_SENDTRANSACTION, 0,
//             (IMS_UINTP)pSendTransactionParam);

//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::StateXXX_SS_UpdateReceived(IN IMSMSG &objMsg)
// {
//     IConfirmedTransactionReceivedParam* pParam
//             = reinterpret_cast<IConfirmedTransactionReceivedParam*>(objMsg.nLparam);

//     IMS_TRACE_I("StateXXX_SS_UpdateReceived[%s]", PrintState(), 0, 0);

//     if (m_pConfirmedSession == IMS_NULL)
//     {
//         delete pParam;
//         return IMS_TRUE;
//     }

//     USSDDataParser* pUSSDData = GetParsedUSSIData(pParam->pISIPMessage);

//     if (SendUSSIResultToUI(pUSSDData))
//     {
//         m_pConfirmedSession->Control(USSIConfirmedSession::CMD_HANDLING_RESULT, IMS_SUCCESS, 0);

//         if (IsPreviousRequestByeMessage())
//         {
//             IMS_TRACE_I("StateXXX_SS_UpdateReceived : prev. request was BYE, no action", 0, 0, 0);
//             delete pUSSDData;
//             delete pParam;
//             return IMS_TRUE;
//         }

//         USSIConfirmedSession* pUSSIConfirmedSession
//                 = DYNAMIC_CAST(USSIConfirmedSession*, m_pConfirmedSession);
//         pUSSIConfirmedSession->SetUSSType(pUSSDData->GetAnyExtension().GetUSSType());

//         if (pUSSDData->GetUSSDString().GetLength() <= 0)
//         {
//             m_pConfirmedSession->PostMessage(
//                     USSIConfirmedSession::USSI_CONFIRMED_SEND_ERROR_INFO, 0,
//                     DYNAMIC_CAST(IMS_UINTP, USSDDataParser::ERROR_CODE_1));
//         }
//         else if (pUSSDData->GetAnyExtension().GetUSSType()
//                 == USSDDataParser::AnyExtension::USS_TYPE_NOTIFY)
//         {
//             IConfirmedSendTransactionParam* pSendTransactionParam
//                     = new IConfirmedSendTransactionParam();
//             m_pConfirmedSession->PostMessage(ConfirmedSession::CONFIRMED_SS_SENDTRANSACTION, 0,
//                     (IMS_UINTP)pSendTransactionParam);
//         }
//     }
//     else
//     {
//         m_pConfirmedSession->Control(USSIConfirmedSession::CMD_HANDLING_RESULT, IMS_FAILURE, 0);
//     }

//     delete pUSSDData;
//     delete pParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PUBLIC VIRTUAL
// IMS_BOOL USSISession::Control(IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam,
//         OUT IMS_UINTP* pnOutParam)
// {
//     IMS_TRACE_I("Control : CmdType [%d]", nCmdType, 0, 0);

//     if (nCmdType == CMD_CHECK_INCOMING_USSI_DATA)
//     {
//         ISIPMessage* pISIPMessage = reinterpret_cast<ISIPMessage*>(nInParam);
//         if (pISIPMessage == IMS_NULL)
//         {
//             IMS_TRACE_E(0, "SIPMessage is null", 0, 0, 0);
//             return IMS_FALSE;
//         }

//         USSDDataParser* pParsedUSSIData = GetParsedUSSIData(pISIPMessage);
//         if (pParsedUSSIData == IMS_NULL)
//         {
//             IMS_TRACE_E(0, "USSI Data unavailable", 0, 0, 0);
//             return IMS_FALSE;
//         }

//         if (pParsedUSSIData->GetAnyExtension().GetUSSType()
//                 == USSDDataParser::AnyExtension::USS_TYPE_NONE)
//         {
//             IMS_TRACE_E(0, "no USS-Type", 0, 0, 0);
//             return IMS_FALSE;
//         }

//         delete pParsedUSSIData;
//         return IMS_TRUE;
//     }
//     else
//     {
//         return UCSession::Control(nCmdType, nInParam, pnOutParam);
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PUBLIC VIRTUAL
// void USSISession::Confirmed_TransactionReceived(IN IMS_UINTP nParam)
// {
//     IMS_TRACE_I("Confirmed_TransactionReceived[%s]", PrintState(), 0, 0);

//     PostMessage(UCSESSION_SS_UPDATERECEIVED, 0, nParam);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::SetUSSIBody(IN ISIPMessage* pISIPMessage, IN const AString& strUSSDStr,
//         IN IMS_BOOL bMultiPart/* = IMS_TRUE*/)
// {
//     ISIPMessageBodyPart *piBodyPart = pISIPMessage->CreateBodyPart();
//     if (piBodyPart == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "piBodyPart is NULL", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     IMS_TRACE_I("SetUSSIBody Multi Part(%s)", PS_BOOL(bMultiPart), 0, 0);

//     if (bMultiPart)
//     {
//         piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE,
//                 USSDConstants::HEADER_APPLICATION_USSDXML);
//         piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_DISPOSITION,
//                 USSDConstants::HEADER_RENDER_HANDLING);
//     }

//     AStringBuffer objXML(USSDConstants::XML_BUFFER_SIZE);
//     USSDDataCreator::GetXMLBody(
//             strUSSDStr, objXML, m_nSlotID,
//             USSDDataParser::AnyExtension::USS_TYPE_NONE, USSDDataParser::ERROR_CODE_NONE);
//     piBodyPart->SetContent(objXML.GetString());

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// USSDDataParser* USSISession::GetParsedUSSIData(IN ISIPMessage* pISIPMessage)
// {
//     IMSList<ISIPMessageBodyPart*> objBodyParts = pISIPMessage->GetBodyParts();

//     if (objBodyParts.IsEmpty())
//     {
//         IMS_TRACE_E(0, "objBodyParts IsEmpty", 0, 0, 0);
//         return IMS_NULL;
//     }

//     AString strUSSIBody ;

//     IMS_TRACE_I("pISIPMessage [%" PFLS_u "]", pISIPMessage, 0, 0);

//     for (IMS_UINT32 nIndex = 0; nIndex < objBodyParts.GetSize(); nIndex++)
//     {
//         ISIPMessageBodyPart *piBodyPart = objBodyParts.GetAt(nIndex);
//         if (piBodyPart != IMS_NULL)
//         {
//             const ByteArray &objUSSDBody = piBodyPart->GetContent();
//             strUSSIBody = objUSSDBody.ToString();
//             break;
//         }
//     }

//     USSDDataParser* pUSSDDataParser = new USSDDataParser();
//     if (pUSSDDataParser->Parse(strUSSIBody) == IMS_FALSE)
//     {
//         delete pUSSDDataParser;
//         return IMS_NULL;
//     }
//     return pUSSDDataParser;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSISession::SendUSSIResultToUI(IN USSDDataParser* pUSSDData)
// {
//     if (pUSSDData == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "invalid ussd data", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     IUUCSessionNotifyInfoParam* pNotiInfoParam = new IUUCSessionNotifyInfoParam();
//     m_pProxy->AttachSessProxy();

//     pNotiInfoParam->eType = INFO_TYPE_USSD;
//     pNotiInfoParam->aStrValue = pUSSDData->GetUSSDString();
//     pNotiInfoParam->strUIKey = m_pProxy->GetUIKey();
//     pNotiInfoParam->nIMSKey = m_pProxy->GetIMSKey();

//     IMS_BOOL bBYEMessage = IsPreviousRequestByeMessage();

//     if (pUSSDData->GetErrorCode() != USSDDataParser::ERROR_CODE_NONE)
//     {
//         pNotiInfoParam->nValue = USSD_MODE_ERROR;
//     }
//     else
//     {
//         if (bBYEMessage)
//         {
//             pNotiInfoParam->nValue = USSD_MODE_NOTIFY;
//         }
//         else
//         {
//             if (pUSSDData->GetUSSDString().GetLength() <= 0)
//             {
//                 IMS_TRACE_E(0, "no ussd-string, only not notify to UI", 0, 0, 0);
//                 return IMS_TRUE;
//             }

//             if (IsUEInitiated())
//             {
//                 pNotiInfoParam->nValue = USSD_MODE_REQUEST;
//             }
//             else
//             {
//                 pNotiInfoParam->nValue = pUSSDData->GetAnyExtension().GetUSSType();
//             }
//         }
//     }

//     IMS_TRACE_D("SendUSSIResultToUI type[%d] string[%s]", pNotiInfoParam->nValue,
//             pNotiInfoParam->aStrValue.GetStr(), 0);

//     m_pProxy->SendMsgToUI(IuMtcCall::NOTIFY_INFO, reinterpret_cast<IMS_UINTP>(pNotiInfoParam));

//     return IMS_TRUE;
// }

// /* M ----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void USSISession::LoadConfig()
// {
//     UCSession::LoadConfig();

//     IMS_TRACE_I("LoadConfig", 0, 0, 0);

//     SetLocalSupportedFeature(FEATURE_PRECONDITION, IMS_FALSE);
//     SetLocalSupportedFeature(FEATURE_PEM, IMS_FALSE);
// }

// /* M ----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL USSISession::IsUEInitiated()
// {
//     IMessage* pIMessage = m_pISession->GetPreviousRequest(IMessage::SESSION_START);
//     if (pIMessage != IMS_NULL
//             && pIMessage->GetState() == IMessage::STATE_SENT)
//     {
//         return IMS_TRUE;
//     }

//     return IMS_FALSE;
// }

// /* M ----------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE
// IMS_BOOL USSISession::IsPreviousRequestByeMessage()
// {
//     IMessage* pIMessage = m_pISession->GetPreviousRequest(IMessage::SESSION_TERMINATE);

//     if (pIMessage != IMS_NULL && pIMessage->GetMethod().Equals(SIPMethod::BYE))
//     {
//         return IMS_TRUE;
//     }

//     return IMS_FALSE;
// }
