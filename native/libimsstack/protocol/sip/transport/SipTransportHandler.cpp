/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "SipDebug.h"
#include "SipStackError.h"
#include "platform/SipMemory.h"
#include "platform/SipString.h"
#include "transport/SipTransportHandler.h"
#include "transport/SipTransportInfo.h"
#include "transport/SipTransportParameter.h"

/****************************************************************************
  Member Function Implementations
 *****************************************************************************/

/*!
 * @brief This API encode SIP message and fill transport params
 *
 * @param[in,out] pSipMsg        : SIP message object used for forming raw SIP message
 * @param[in]     pTranspParam   : For Request message it contains transport information where
 *    request to be send. For response, remote transport information is fetched from the Via header
 * @param[out]     ppTranspInfo    : Contains Encoder SIP message and final transport details.
 * @param[out]     pnError        : Appropriate error code as defined in SipEn_ErrorTypes in case
 * of failure
 *
 * @return Status indicator
 * @retval SIP_TRUE If successful
 * @retval SIP_FALSE If function processing failed.
 * @retval Appropriate error code as defined in SipEn_ErrorTypes in case of failure
 *
 */
SIP_BOOL SipTransportHandler::OnSendTransp(IN SipMessage* pSipMsg,
        IN SipTransportParameter* pTranspParam, IN const SIP_CHAR* pSipBuffer,
        IN SIP_UINT32 nSipBufferLen, OUT SipTransportInfo** ppTranspInfo, OUT SIP_UINT16* pnError)
{
    (void)pnError;

    /* Input parameter validation */
    if ((pSipMsg == SIP_NULL) || (pTranspParam == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "SendMsg invalid params", 0, 0);
        return SIP_FALSE;
    }

    SIP_CHAR* pcNewBuffer = new SIP_CHAR[nSipBufferLen + 1];
    SipPf_Memcpy(pcNewBuffer, pSipBuffer, nSipBufferLen);
    pcNewBuffer[nSipBufferLen] = '\0';

    SipTransportBuffer* pTranspBuffer = new SipTransportBuffer(pcNewBuffer, nSipBufferLen);

    if (pTranspBuffer == SIP_NULL)
    {
        delete[] pcNewBuffer;
        return SIP_FALSE;
    }

    SipTransportInfo* pTranspInfo = new SipTransportInfo(pTranspParam, pTranspBuffer);

    if (pTranspInfo == SIP_NULL)
    {
        delete pTranspBuffer;
        return SIP_FALSE;
    }

    SipTransportParameter* pActualDestParam = new SipTransportParameter(pTranspParam);
    pTranspInfo->SetMsgSentTranspParam(pActualDestParam);

    /* store sip message for retransmit purpose. deleted when transinfo is deleted */
    /* storing can be done outside of this api call also. for failure ack and 100 response txn
    gives sip message */
    SipMessage* pStoredSipMsg = new SipMessage(*pSipMsg);
    pTranspInfo->SetSentSipMsg(pStoredSipMsg);

    *ppTranspInfo = pTranspInfo;

    return SIP_TRUE;
}

/*!
 * @brief This API validates via header, txn mandatory headers and check the validiaty of the
 * received message as per the txn state
 *
 * @param[in,out] pSipMsg        : Parsed SIP message object of the received SIP RAW message
 * @param[in]    pTranspParam   : Transport details from where SIP message is received
 * @param[out]    peTranspStatus   : return the status of transport layer after processing.
 * Caller shall take necessary actions based on this status
 * @param[out]    ppTxnKey   :  New instance key of txn to which the message belongs
 * @param[out]     ppTranspInfo    : transport details of the existing transaction, obj of same
 * ref as in txn obj
 * @param[out]     ppUserData    : It contains user data as given by the user. this data is
 * retrieve from the txn obj for txn not existing it contains NULL.
 * NOTE: User data that is return is of same ref as in txn obj
 * @param[out]     pnError        : Appropriate error code as defined in SipEn_ErrorTypes in case
 * of failure
 *
 * @return Status indicator
 * @retval SIP_TRUE If successful
 * @retval SIP_FALSE If function processing failed.
 * @retval Appropriate error code as defined in SipEn_ErrorTypes in case of failure
 *
 */
SIP_BOOL SipTransportHandler::OnRecvTransp(IN SipMessage* pSipMsg,
        IN SipTransportParameter* pTranspParam, OUT SIP_INT32* peTxnStatus,
        OUT SIP_BOOL* pbTxnExist, OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError)
{
    (void)pTranspParam;

    /* RFC 3261: Section 18.2.1
       Fetch value of the "sent-by" parameter in the top Via header field
       from Request. If host portion differs, then add a "received"
       parameter to that Via header field value.Set parameter of source
       address to address from which the packet was received.
     */
    /* It's handled by the upper layer in the client side. */

    /* returns txn key from Sip message */
    SipTxnKey* pTxnKey = SIP_NULL;
    if (SIP_FALSE == GetTxnKeyFromSipMsg(pSipMsg, &pTxnKey, pnError))
    {
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTRANSP, "OnRecvTransp: GetTxnKeyFromSipMsg fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTxnKey->GetMsgType() == SipMessage::REQ_TYPE)
    {
        if ((SipPf_Strcmp(SipMsgUtil::METHOD_ACK, pTxnKey->GetMethod()) == SIP_EQUALS) ||
                (SipPf_Strcmp(SipMsgUtil::METHOD_INVITE, pTxnKey->GetMethod()) == SIP_EQUALS))
        {
            pTxnKey->SetTxnType(SipTxn::INVITE_SERVER);
        }
        else
        {
            pTxnKey->SetTxnType(SipTxn::NON_INVITE_SERVER);
        }
    }
    else
    {
        if (SipPf_Strcmp(SipMsgUtil::METHOD_INVITE, pTxnKey->GetMethod()) == SIP_EQUALS)
        {
            pTxnKey->SetTxnType(SipTxn::INVITE_CLIENT);
        }
        else
        {
            pTxnKey->SetTxnType(SipTxn::NON_INVITE_CLIENT);
        }
    }

    /* Fetches object from Database[Utility func within txn] */
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_TRUE;
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTRANSP, "OnRecvTransp: GetTxnObjFromDb fail", SIP_ZERO, SIP_ZERO);

        pTxnKey->SipDelete();
        return SIP_FALSE;
    }

    *pbTxnExist = bTxnExist;

    if (bTxnExist == SIP_NO)
    {
        if (pTxnKey->GetMsgType() == SipMessage::RESP_TYPE)
        {
            /* Transaction for the response doesn't exist */

            SIP_UINT16 nStatusCode = pSipMsg->GetStatusCode();

            /* In case of retransmitted 2xx for INVITE, return as valid sip message */
            if ((pSipMsg->GetMethodType() == SipMessage::METHOD_INVITE) &&
                    SipMsgUtil::IsSuccessfulResponse(nStatusCode))
            {
                *peTxnStatus = SipTxn::STATUS_VALID_MESSAGE;
            }
            else
            {
                *peTxnStatus = SipTxn::STATUS_STRAY_RESP;
            }
        }
        else
        {
            /* Transaction for the Request doesn't exist */
            *peTxnStatus = SipTxn::STATUS_NEW_REQ_RECVD;
        }

        *ppTxnKey = pTxnKey;  // new instance
        return SIP_TRUE;
    }

    *ppTxnKey = pTxnKey;  // New instance
    *peTxnStatus = SipTxn::STATUS_VALID_MESSAGE;

    /* For Both Request and Response Notify Transaction Layer */

    return SIP_TRUE;
}

/*****************************************************************************
 * Function name    : OnRecvTanspError
 * Description      : If Error is occurred due to Mesg Constraint , swith back to UDP and transport
 * Preconditions/   :
 * Side Effects     :
 *****************************************************************************/
SIP_BOOL SipTransportHandler::OnRecvTanspError(SIP_INT32 eTranspError, SipTxnKey* pTxnKey,
        SIP_INT32* peTxnStatus, SipTransportInfo** ppTranspInfo, ISipUserData* pUserData,
        SIP_UINT16* pnError)
{
    (void)pUserData;
    (void)eTranspError;

    /*Fetches object from Database[Utility func within txn] */
    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_FALSE;
    if (GetTxnObjFromDb(pTxnKey, &pTxn, &bTxnExist, pnError) == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    if (bTxnExist == SIP_NO)
    {
        /* Transaction for the response doesn't exist */
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTRANSP, "OnRecvTanspError: Transaction Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pTxn->Increment();

    SipTransportInfo* pTranspInfo = pTxn->GetTranspInfo();
    if (pTranspInfo == SIP_NULL)
    {
        pTxn->SipDelete();

        /*  stack error */
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTRANSP, "OnRecvTanspError: sipFetchElement Error", SIP_ZERO, SIP_ZERO);
        *pnError = ETXN_STACKERROR;
        return SIP_FALSE;
    }

    SipTransportParameter* pMsgSentTransParam = pTranspInfo->GetMsgSentTranspParam();
    if (pMsgSentTransParam == SIP_NULL)
    {
        pTxn->SipDelete();

        /*  stack error */
        SIP_DEBUG_STACKBUG(
                ESIPTRACE_MODTRANSP, "OnRecvTanspError: sipFetchElement Error", SIP_ZERO, SIP_ZERO);
        *pnError = ETXN_STACKERROR;
        return SIP_FALSE;
    }

    SIP_INT32 eTransportType = pMsgSentTransParam->GetTranspProtocol();
    if (eTransportType == SipTransportInfo::PROTOCOL_INVALID)
    {
        pTxn->SipDelete();

        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTRANSP,
                "OnRecvTanspError: SipTransportInfo::PROTOCOL_INVALID\n", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Check if Error occurred due to switching to TCP (Message Len Constraint)*/
    if (eTransportType == SipTransportInfo::PROTOCOL_TCP)
    {
        /* Switch to previous Transport Information as initially given by stack user */
        /* Change Protocol in Buffer */
        if (UpdateViaSipMsg(pTranspInfo->GetSentSipMsg(), pTranspInfo->GetTranspSipBuffer(),
                    SipTransportInfo::PROTOCOL_UDP) == SIP_FALSE)
        {
            pTxn->SipDelete();

            SIP_DEBUG_WARNING(ESIPTRACE_MODTRANSP, "OnRecvTanspError: sipUpdateViaSipMsg Fails\n",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /* Change Protocol in Transport Information */
        pMsgSentTransParam->SetTranspProtocol(SipTransportInfo::PROTOCOL_UDP);

        *peTxnStatus = SipTxn::STATUS_RETRANSMISSION;

        *ppTranspInfo = pTranspInfo;

        pTxn->SipDelete();
        return SIP_TRUE;
    }

    pTxn->SipDelete();

    return SIP_TRUE;
}

/*****************************************************************************
 * Function name         : IsInviteTxnPresentForAckTxn
 * Description            : Checks if INVITE server transaction is present which is matched
 *                         with the specified ACK transaction key.
 * Preconditions/        :
 * Side Effects            :
 *****************************************************************************/
SIP_BOOL SipTransportHandler::IsInviteTxnPresentForAckTxn(IN SipTxnKey* pAckTxnKey)
{
    SIP_UINT16 nError = 0;
    SipTxnKey* pInviteTxnKey = new SipTxnKey(pAckTxnKey, &nError);

    pInviteTxnKey->SetMethod(SipMsgUtil::METHOD_INVITE);
    pInviteTxnKey->RemoveRule(SipTxnKey::RULE_COMPARE_VIA_BRANCH);

    SipTxn* pTxn = SIP_NULL;
    SIP_BOOL bTxnExist = SIP_TRUE;
    if (GetTxnObjFromDb(pInviteTxnKey, &pTxn, &bTxnExist, &nError) == SIP_FALSE)
    {
        bTxnExist = SIP_FALSE;
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTRANSP, "IsInviteTxnPresentForAckTxn: GetTxnObjFromDb fail",
                SIP_ZERO, SIP_ZERO);
    }

    pInviteTxnKey->SipDelete();

    return (bTxnExist == SIP_TRUE) ? SIP_TRUE : SIP_FALSE;
}

/*****************************************************************************
 * Function name         : UpdateViaSipMsg
 * Description            : Changing Transport with in VIA Header
 * Preconditions/        :
 * Side Effects            :
 *****************************************************************************/
PRIVATE SIP_BOOL SipTransportHandler::UpdateViaSipMsg(
        SipMessage* pSipMsg, SipTransportBuffer* pSentBuffer, SIP_INT32 eChangeProto)
{
    const SIP_CHAR STR_VIA_ENC_FORMAT[] = "\r\nVia:";
    const SIP_CHAR STR_VIA_COMPACT_ENC_FORMAT[] = "\r\nv:";
    SIP_CHAR* pSipBuffer = pSentBuffer->GetSipBuffer();
    SIP_CHAR* pszTemp = SipPf_Strstr(pSipBuffer, STR_VIA_ENC_FORMAT);

    if (pszTemp == SIP_NULL)
    {
        pszTemp = SipPf_Strstr(pSipBuffer, STR_VIA_COMPACT_ENC_FORMAT);
        if (pszTemp == SIP_NULL)
        {
            return SIP_FALSE;
        }
    }

    const SIP_CHAR STR_VIA_LINE_TCP[] = "/TCP";
    const SIP_CHAR STR_VIA_LINE_UDP[] = "/UDP";

    if (eChangeProto == SipTransportInfo::PROTOCOL_TCP)
    {
        pszTemp = SipPf_Strstr(pszTemp, STR_VIA_LINE_UDP);
        if (pszTemp == SIP_NULL)
        {
            return SIP_FALSE;
        }
        /* Curpos = "/UDP"  */
        pszTemp = pszTemp + SIP_ONE;

        /* Curpos = "UDP"  */
        /* Change UDP to TCP */
        *pszTemp = 'T';
        pszTemp = pszTemp + SIP_ONE;
        *pszTemp = 'C';
    }
    else if (eChangeProto == SipTransportInfo::PROTOCOL_UDP)
    {
        pszTemp = SipPf_Strstr(pszTemp, STR_VIA_LINE_TCP);
        if (pszTemp == SIP_NULL)
        {
            return SIP_FALSE;
        }

        /* Curpos = "/TCP"  */
        pszTemp = pszTemp + SIP_ONE;

        /* Curpos = "TCP"  */
        /* Change TCP to UDP */
        *pszTemp = 'U';
        pszTemp = pszTemp + SIP_ONE;
        *pszTemp = 'D';
    }
    else
    {
        return SIP_FALSE;
    }

    if (pSipMsg != SIP_NULL)
    {
        /* Update Via Hdr of Sip Message from UDP to TCP */
        SipViaHeader* pViaHdr = static_cast<SipViaHeader*>(pSipMsg->GetHdrObj(SipHeaderBase::VIA));
        if (pViaHdr == SIP_NULL)
        {
            SIP_DEBUG_STACKBUG(ESIPTRACE_MODTRANSP, "Memory allocation fail", SIP_ZERO, SIP_ZERO);

            return SIP_FALSE;
        }

        const SIP_CHAR STR_TRANSPORT_TCP[] = "TCP";
        const SIP_CHAR STR_TRANSPORT_UDP[] = "UDP";

        if (eChangeProto == SipTransportInfo::PROTOCOL_TCP)
        {
            pViaHdr->SetProtocolName(STR_TRANSPORT_TCP);
        }
        else
        {
            pViaHdr->SetProtocolName(STR_TRANSPORT_UDP);
        }
        pViaHdr->SipDelete();
    }
    return SIP_TRUE;
}

/*****************************************************************************
 * Function name         : GetTxnKeyFromSipMsg
 * Description            : returns txn key from Sip message
 * Preconditions/        :
 * Side Effects            :
 *****************************************************************************/
PRIVATE SIP_BOOL SipTransportHandler::GetTxnKeyFromSipMsg(
        IN SipMessage* pSipMsg, OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError)
{
    SIP_INT32 eMsgType = SipMessage::TYPE_INVALID;
    SIP_INT32 eMethodType = SipMessage::METHOD_INVALID;

    /* Check if it's proper to start Transaction Ref: RFC 3261 8.1.1.*/
    if (SipMessage::CheckTxnMandatoryParams(pSipMsg, &eMsgType, &eMethodType) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTRANSP,
                "GetTxnKeyFromSipMsg: CheckTxnMandatoryParams fails\n", SIP_ZERO, SIP_ZERO);

        return SIP_FALSE;
    }

    SipTxnKey* pTxnKey = new SipTxnKey(pSipMsg, pnError);
    if (pTxnKey == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTRANSP, "GetTxnKeyFromSipMsg:key Creation Fails", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (*pnError == EERR_MALLOCFAILED)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODTRANSP, "GetTxnKeyFromSipMsg:key Creation Fails", SIP_ZERO, SIP_ZERO);
        pTxnKey->SipDelete();
        return SIP_FALSE;
    }

    *ppTxnKey = pTxnKey;
    return SIP_TRUE;
}

/*****************************************************************************
 * Function name         : GetTxnObjFromDb
 * Description            : Fetches object from Database[Utility func within txn]
 * Preconditions/        :
 * Side Effects            :
 *****************************************************************************/
PRIVATE SIP_BOOL SipTransportHandler::GetTxnObjFromDb(IN SipTxnKey* pTxnKey, OUT SipTxn** ppTxn,
        OUT SIP_BOOL* pbTxnExist, OUT SIP_UINT16* pnError)
{
    SIP_BOOL bTxnExist = Sip_Cbk_FetchTransaction(reinterpret_cast<SIP_VOID*>(pTxnKey),
            SipTxn::OPT_FETCH, SIP_NULL, reinterpret_cast<SIP_VOID**>(ppTxn));

    if ((bTxnExist == SIP_YES) && (*ppTxn == SIP_NULL))
    {
        /*  stack error */
        *pnError = ETXN_STACKERROR;
        SIP_DEBUG_STACKBUG(ESIPTRACE_MODTXN, "GetTxnObjFromDb: Stack Error,Db Status Error\n",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    *pbTxnExist = bTxnExist;
    return SIP_TRUE;
}
