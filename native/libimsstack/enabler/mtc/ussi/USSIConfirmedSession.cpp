/*
 * author : aromi.kwak
 * version : v1.0
 * date : 2015.12
 * brief : Create USSIConfirmedSession
 */

#include "ServiceTrace.h"
#include "AStringBuffer.h"

#include "SIPHeaderName.h"
#include "SIPStatusCode.h"

#include "ISIPHeader.h"
#include "ISIPMessage.h"
#include "ISIPClientConnection.h"
#include "ISIPServerConnection.h"

#include "ussi/USSIConfirmedSession.h"
#include "ussi/UssiConstants.h"
#include "ussi/UssiDataCreator.h"
#include "ussi/UssiDataParser.h"

#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_UC__;

/* ------------------------------------------------------------------------------------------------
    Constructor, Destructor
------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC
USSIConfirmedSession::USSIConfirmedSession(IN IMtcCall* pSession)
    // TODO, MTC BUILD
    // : ConfirmedSession(pSession)
    : m_pISIPClientConnection(IMS_NULL)
    , m_pISIPServerConnection(IMS_NULL)
    , m_nUSSType(USSDDataParser::AnyExtension::USS_TYPE_NONE)
{
    // TODO, MTC BUILD
    UNUSED_PARAM(pSession);
    IMS_TRACE_MEM("uc", "uc_M : USSIConfirmedSession[%" PFLS_u "][%" PFLS_x "]",
            sizeof(USSIConfirmedSession), this, 0);
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
PUBLIC VIRTUAL
USSIConfirmedSession::~USSIConfirmedSession()
{
    IMS_TRACE_MEM("uc", "uc_F : USSIConfirmedSession[%" PFLS_u "][%" PFLS_x "]",
            sizeof(USSIConfirmedSession), this, 0);

    if (m_pISIPClientConnection != IMS_NULL)
    {
        m_pISIPClientConnection->Close();
        m_pISIPClientConnection = IMS_NULL;
    }

    if (m_pISIPServerConnection != IMS_NULL)
    {
        m_pISIPServerConnection->Close();
        m_pISIPServerConnection = IMS_NULL;
    }
}

// TODO, MTC BUILD
// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PUBLIC VIRTUAL
// IMS_BOOL USSIConfirmedSession::Control(IN IMS_UINT32 nCmdType, IN IMS_UINTP nInParam,
//         OUT IMS_UINTP* /*pnOutParam*/)
// {
//     if (nCmdType == CMD_HANDLING_RESULT)
//     {
//         if (nInParam == IMS_SUCCESS)
//         {
//             SendTransactionResponse(SIPStatusCode::SC_200);
//         }
//         else
//         {
//             SendTransactionResponse(SIPStatusCode::SC_469, "Bad Info Package");
//         }
//     }
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PUBLIC VIRTUAL
// void USSIConfirmedSession::SetUSSType(IN IMS_UINT32 nUSSType)
// {
//     if ((m_nUSSType == USSDDataParser::AnyExtension::USS_TYPE_NONE)
//             || (m_nUSSType != nUSSType && nUSSType != USSDDataParser::AnyExtension::USS_TYPE_NONE))
//     {
//         m_nUSSType = nUSSType;
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSIConfirmedSession::OnMessage(IN IMSMSG &objMSG)
// {
//     IMS_TRACE_I("OnMessage : Msg[%d]", objMSG.nMSG, 0, 0);

//     switch (objMSG.nMSG)
//     {
//         case USSI_CONFIRMED_SEND_ERROR_INFO:
//             SendTransactionWithErrorCode(DYNAMIC_CAST(IMS_UINT32, objMSG.nLparam));
//             break;

//         default:
//             ConfirmedSession::OnMessage(objMSG);
//             break;
//     }

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PUBLIC VIRTUAL
// IMS_BOOL USSIConfirmedSession::IsUSSIInfoReceived(IN ISIPMessage* pISIPMessage)
// {
//     IMS_TRACE_I("IsUSSIInfoReceived()", 0, 0, 0);

//     if (!MessageUtil::ContainsValue(pISIPMessage, USSDConstants::HEADER_USSD_PACKAGE,
//             ISIPHeader::UNKNOWN, SIPHeaderName::INFO_PACKAGE))
//     {
//         return IMS_FALSE;
//     }

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSIConfirmedSession::StateXXX_SS_TransactionReceived(IN IMSMSG &objMsg)
// {
//     m_pISIPServerConnection = reinterpret_cast<ISIPServerConnection*>(objMsg.nLparam);

//     IMS_TRACE_I("StateXXX_SS_TransactionReceived", 0, 0, 0);

//     IMS_SINT32 nMethod = m_pISIPServerConnection->GetMethod().ToInt();
//     ISIPMessage* pISIPMessage = m_pISIPServerConnection->GetMessage();

//     switch (nMethod)
//     {
//         case SIPMethod::INFO:
//         {
//             if (IsUSSIInfoReceived(pISIPMessage))
//             {
//                 SendUSSITransactionToListn(pISIPMessage);
//             }
//             else
//             {
//                 SendTransactionResponse(SIPStatusCode::SC_469, "Bad Info Package");
//             }
//         }
//             break;
//         default:
//         {
//             SendTransactionResponse(SIPStatusCode::SC_200);
//         }
//             break;
//     }
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSIConfirmedSession::StateXXX_SS_SendTransaction(IN IMSMSG &objMsg)
// {
//     IConfirmedSendTransactionParam* pTransactionParam
//             = reinterpret_cast<IConfirmedSendTransactionParam*>(objMsg.nLparam);

//     m_pISIPClientConnection = m_pSession->GetISession()->CreateTransaction(SIPMethod::INFO);

//     m_pISIPClientConnection->SetHeader(USSDConstants::HEADER_INFO_PACKAGE,
//             USSDConstants::HEADER_USSD_PACKAGE);
//     m_pISIPClientConnection->SetHeader(SIPHeaderName::CONTENT_TYPE,
//             USSDConstants::HEADER_APPLICATION_USSDXML);
//     m_pISIPClientConnection->SetHeader(SIPHeaderName::CONTENT_DISPOSITION,
//             USSDConstants::HEADER_INFO_PACKAGE);

//     AString strUSSIString = pTransactionParam->aStrUSSIString;
//     IMS_TRACE_D("StateXXX_SS_SendTransaction : USSI[%s]", strUSSIString.GetStr(), 0, 0);

//     if (SetUSSIBody(
//             m_pISIPClientConnection->GetMessage(), strUSSIString, USSDDataParser::ERROR_CODE_NONE))
//     {
//         m_pISIPClientConnection->SetListener(this);
//         m_pISIPClientConnection->SetErrorListener(this);

//         if (m_pISIPClientConnection->Send() == IMS_FAILURE)
//         {
//             //TODO : Fail Handling
//         }
//     }
//     else
//     {
//         m_pISIPClientConnection->Close();
//         m_pISIPClientConnection = IMS_NULL;
//         //TODO : Fail Handling
//     }
//     delete pTransactionParam;
//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// IMS_BOOL USSIConfirmedSession::StateCONVERSATION_SS_Terminated(IN IMSMSG &objMsg)
// {
//     IMS_SINT32 eTerminationReason = LONG_TO_SINT(objMsg.nWparam);
//     FailReason failReason(UC_FAILURE(m_nSlotID)->TerminationReasonToFailReason(eTerminationReason),
//             eTerminationReason);

//     IMS_TRACE_I("StateCONVERSATION_SS_Terminated", 0, 0, 0);

//     IMessage* pIMessage = m_pISession->GetPreviousRequest(IMessage::SESSION_TERMINATE);

//     if (pIMessage != IMS_NULL)
//     {
//         AString strHeader;
//         MessageUtil::GetHeader(pIMessage, ISIPHeader::CONTENT_TYPE, strHeader);
//         if (strHeader.Contains(USSDConstants::HEADER_APPLICATION_USSDXML))
//         {
//             SendUSSITransactionToListn(pIMessage->GetMessage());
//         }
//     }

//     SetState(TERMINATING);
//     HandleTerminated(failReason);

//     return IMS_TRUE;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void USSIConfirmedSession::SendUSSITransactionToListn(IN ISIPMessage* pISIPMessage)
// {
//     IConfirmedTransactionReceivedParam* pParam = new IConfirmedTransactionReceivedParam();
//     pParam->pISIPMessage = pISIPMessage;

//     IMS_TRACE_I("SendUSSITransactionToListn", 0, 0, 0);

//     m_pListener->Confirmed_TransactionReceived((IMS_UINTP)pParam);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void USSIConfirmedSession::ClientConnection_NotifyResponse(IN ISIPClientConnection *piSCC,
//         IN ISIPClientConnection * /* piForkedSCC = IMS_NULL */)
// {
//     if (piSCC == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "ClientConnection_NotifyResponse : piSCC is NULL", 0, 0, 0);
//         return;
//     }

//     if (piSCC != m_pISIPClientConnection)
//     {
//         IMS_TRACE_E(0, "ClientConnection_NotifyResponse : piSCC is invalid", 0, 0, 0);
//         return;
//     }

//     if (piSCC->Receive() != IMS_SUCCESS)
//     {
//         IMS_TRACE_E(0, "No message received", 0, 0, 0);
//         return;
//     }

//     IMS_SINT32 nStatusCode = piSCC->GetStatusCode();

//     IMS_TRACE_I("ClientConnection_NotifyResponse : StatusCode[%d]", nStatusCode, 0, 0);

//     if (SIPStatusCode::IsFinal(nStatusCode))
//     {
//         m_pISIPClientConnection->Close();
//         m_pISIPClientConnection = IMS_NULL;
//     }

// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void USSIConfirmedSession::Error_NotifyError(IN ISIPConnection* /*piSC*/, IN IMS_SINT32 /*nCode*/,
//         IN const AString& /*strMessage*/)
// {
//     if (m_pISIPClientConnection != IMS_NULL)
//     {
//         m_pISIPClientConnection->Close();
//         m_pISIPClientConnection = IMS_NULL;
//     }

//     IMS_TRACE_I("Error_NotifyError", 0, 0, 0);
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void USSIConfirmedSession::SendTransactionWithErrorCode(IMS_UINT32 nErrorCode)
// {
//     IMS_TRACE_D("SendTransactionWithErrorCode", 0, 0, 0);

//     m_pISIPClientConnection = m_pSession->GetISession()->CreateTransaction(SIPMethod::INFO);

//     m_pISIPClientConnection->SetHeader(USSDConstants::HEADER_INFO_PACKAGE,
//             USSDConstants::HEADER_USSD_PACKAGE);
//     m_pISIPClientConnection->SetHeader(SIPHeaderName::CONTENT_TYPE,
//             USSDConstants::HEADER_APPLICATION_USSDXML);
//     m_pISIPClientConnection->SetHeader(SIPHeaderName::CONTENT_DISPOSITION,
//             USSDConstants::HEADER_INFO_PACKAGE);

//     if (SetUSSIBody(m_pISIPClientConnection->GetMessage(), AString::ConstEmpty(), nErrorCode))
//     {
//         m_pISIPClientConnection->SetListener(this);
//         m_pISIPClientConnection->SetErrorListener(this);

//         if (m_pISIPClientConnection->Send() == IMS_FAILURE)
//         {
//             //TODO : Fail Handling
//         }
//     }
//     else
//     {
//         m_pISIPClientConnection->Close();
//         m_pISIPClientConnection = IMS_NULL;
//         //TODO : Fail Handling
//     }
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PROTECTED VIRTUAL
// void USSIConfirmedSession::LoadConfig()
// {
//     return ConfirmedSession::LoadConfig();
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE VIRTUAL
// void USSIConfirmedSession::SendTransactionResponse(IN IMS_UINT32 nResponseCode,
//         IN const AString& strPhrase /*= AString::ConstEmpty()*/)
// {
//     if (m_pISIPServerConnection == IMS_NULL)
//     {
//         return;
//     }

//     if (strPhrase.GetLength() > 0)
//     {
//         m_pISIPServerConnection->SetReasonPhrase(strPhrase);
//     }

//     m_pISIPServerConnection->InitResponse(nResponseCode);
//     m_pISIPServerConnection->Send();
//     m_pISIPServerConnection->Close();
//     m_pISIPServerConnection = IMS_NULL;
// }

// /* ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------ */
// PRIVATE VIRTUAL
// IMS_BOOL USSIConfirmedSession::SetUSSIBody(IN ISIPMessage* pISIPMessage,
//         IN const AString& strUSSDStr, IN IMS_UINT32 nErrorCode)
// {
//     ISIPMessageBodyPart *piBodyPart = pISIPMessage->CreateBodyPart();
//     if (piBodyPart == IMS_NULL)
//     {
//         IMS_TRACE_E(0, "piBodyPart is NULL", 0, 0, 0);
//         return IMS_FALSE;
//     }

//     piBodyPart->SetHeader(ISIPMessageBodyPart::CONTENT_TYPE,
//                 USSDConstants::HEADER_APPLICATION_USSDXML);

//     AStringBuffer objXML(USSDConstants::XML_BUFFER_SIZE);
//     USSDDataCreator::GetXMLBody(strUSSDStr, objXML, m_nSlotID, m_nUSSType, nErrorCode);
//     piBodyPart->SetContent(objXML.GetString());

//     IMS_TRACE_I("SetUSSIBody", 0, 0, 0);
//     return IMS_TRUE;
// }
