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
#ifndef SIP_STACK_H_
#define SIP_STACK_H_

#include "ByteArray.h"
#include "SipMethod.h"
#include "SipParameter.h"
#include "SipStatusCode.h"
#include "msg/SipMessage.h"
#include "txn/SipTxnKey.h"
#include "sip_error.h"
#include "txn/SipTxn.h"
#include "ISipTimerUtil.h"
#include "SipStackManager.h"

#include "SipContextUtils.h"

class ISipConfigV;
class SipEventContext;
class SipProfile;
class SipTimeoutData;
class SipTimerValues;
class SIPTxnContextData;
class SIPTxnKey;

namespace SIPStack
{
// Enumerations
enum
{
    /*
     * The message received was a remote retransmission of a message
     * which has already been responded to. The transaction layer resends
     * the previous response again. Application can ignore this message
     * if this return type is given.
     */
    MESSAGE_RETRANSMISSION = 0,

    /*
     * This value is returned when validity checks in the function fails.
     * For example, error was unallocated or an invalid parameter was passed to the function.
     */
    MESSAGE_FAILURE,

    /*
     * The received message was a stray SIP message.
     * For example, a PRACK without an associated RPR, an ACK without a final response,
     * or a final response without having sent a request, etc.
     */
    MESSAGE_STRAY,

    /*
     * If the message was decoded successfully and is not a remote retransmission,
     * then this value is returned.
     */
    MESSAGE_SUCCESS
};

// Options for the selective SIP message encoding
enum
{
    OPT_START_LINE = 0x01,
    OPT_HEADER_PART = 0x02,
    OPT_BODY_PART = 0x03,
    OPT_ALL = (OPT_START_LINE | OPT_HEADER_PART | OPT_BODY_PART)
};

// SIP message types
enum
{
    SIP_MESSAGE_REQUEST = ESIP_REQTYPE,
    SIP_MESSAGE_RESPONSE = ESIP_RESPTYPE,
    SIP_MESSAGE_ANY = ESIP_INVALIDTYPE
};

// SIP transaction API called types
enum
{
    SIP_TXN_MSG_SENT = 0,
    SIP_TXN_MSG_RECEIVED = 1
};

GLOBAL void Initialize();
GLOBAL void SetTransactionTimerValues(
        IN CONST SipProfile* pSIPProfile, IN CONST ISipConfigV* piSipConfigV);

GLOBAL SipEn_ErrorTypes GetLastError();
inline IMS_BOOL IsLastErrorNoExist()
{
    return (GetLastError() == EERR_NOEXISTS);
}

template <typename T>
inline void AddReference(IN_OUT T& pObject)
{
    if (pObject != IMS_NULL)
        pObject->increment();  //(pObject->m_usRefCount)++;
}
template <typename T>
inline void RemoveReference(IN_OUT T& pObject)
{
    if (pObject != IMS_NULL)
        pObject->decrement();  //(pObject->m_usRefCount)--;
}
template <typename T>
inline void Free(IN T& pMemBlock);

inline IMS_BOOL IsValidHeader(IN CONST SipHeaderBase* pstHeader)
{
    return ((pstHeader != IMS_NULL) && (pstHeader->GetHdrType() != ESIPHDR_INVALID));
}
inline IMS_SINT32 GetAnyHeaderType()
{
    return ESIPHDR_END;
}
inline IMS_SINT32 GetHeaderType(IN CONST SipHeaderBase* pstHeader)
{
    return static_cast<IMS_SINT32>(pstHeader->GetHdrType());
}
inline void SetHeaderType(IN IMS_SINT32 nType, IN_OUT SipHeaderBase*& pstHeader)
{
    pstHeader->SetHdrType(static_cast<SipEn_HdrType>(nType));
}

inline IMS_BOOL IsUriSchemeSIP(IN CONST SipAddrSpec* pstAddrSpec)
{
    return const_cast<SipAddrSpec*>(pstAddrSpec)->GetUriScheme() == SipUri::SCHEME_SIP;
}
inline IMS_BOOL IsUriSchemeSIPS(IN CONST SipAddrSpec* pstAddrSpec)
{
    return const_cast<SipAddrSpec*>(pstAddrSpec)->GetUriScheme() == SipUri::SCHEME_SIPS;
}
inline const IMS_CHAR* AddrSpec_GetUser(IN CONST SipAddrSpec* pstAddrSpec)
{
    return pstAddrSpec->GetSipUriAsRef()->GetUser();
}
inline const IMS_CHAR* AddrSpec_GetPassword(IN CONST SipAddrSpec* pstAddrSpec)
{
    return pstAddrSpec->GetSipUriAsRef()->GetPassword();
}
inline const IMS_CHAR* AddrSpec_GetHost(IN CONST SipAddrSpec* pstAddrSpec)
{
    return pstAddrSpec->GetSipUriAsRef()->GetHost();
}
inline IMS_SINT32 AddrSpec_GetPort(IN CONST SipAddrSpec* pstAddrSpec)
{
    return pstAddrSpec->GetSipUriAsRef()->GetPort();
}

inline IMS_BOOL IsACKTransmissionRequiredForNon2XX()
{
    return IMS_FALSE;
}

GLOBAL IMS_BOOL AppendHeader(IN SipHeaderBase* pstHeader, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL AppendMessageBody(IN SipMsgBody* pstMsgBody, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL PrependHeader(IN SipHeaderBase* pstHeader, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL PrependUnknownHeader(
        IN CONST AString& strName, IN CONST AString& strValue, IN_OUT SipMessage*& pstMessage);

GLOBAL IMS_BOOL CheckMandatoryHeaders(IN CONST SipMessage* pstMessage);

GLOBAL SipHeaderBase* CloneHeader(IN SipHeaderBase* pstHeader);
GLOBAL SipMessage* CloneMessage(IN SipMessage* pstMessage);
GLOBAL SipMsgBody* CloneMessageBody(IN SipMsgBody* pstMsgBody);
GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase* pstHeader);
GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase* pstDest, IN SipHeaderBase* pstSrc);

GLOBAL void CorrectCSeq(IN CONST AString& strMethod, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL CorrectMessageBody(IN_OUT SipMessage*& pstMessage);

GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType);
GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType, IN SipAddrSpec* pstAddrSpec);
GLOBAL SipMessage* CreateMessage(IN IMS_SINT32 nType);
GLOBAL SipMsgBody* CreateMessageBody();
GLOBAL IMS_BOOL CreateMIMEHeader(IN_OUT SipMsgBody* pstMsgBody);
GLOBAL SipHeaderBase* CreateViaHeader(IN CONST AString& strSentProtocol,
        IN CONST AString& strSentBy, IN CONST AString& strBranch);

GLOBAL SipAddrSpec* DecodeAddrSpec(IN CONST AString& strAddress);
GLOBAL SipHeaderBase* DecodeHeader(
        IN IMS_SINT32 nType, IN CONST AString& strName, IN CONST AString& strBody);
inline SipHeaderBase* DecodeHeader(IN IMS_SINT32 nType, IN CONST AString& strBody)
{
    return DecodeHeader(nType, AString::ConstNull(), strBody);
}
#if 0
#ifdef REQUIRED_ONLY_FOR_HSS
    GLOBAL IMS_SINT32 DecodeMessage(IN CONST SIPTransportBuffer *pBuffer, IN SipOptions stOptions,
            IN_OUT SipEventContext *pstEventContext, OUT SipMessage *&pstMessage,
            OUT SipTxnKey *&pstTxnKey, OUT IMS_SINT8 *&pNextBuffer);
#endif
#endif
GLOBAL IMS_BOOL DecodeMessage(IN CONST IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        IN IMS_SINT32 nOptions, OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL DecodeMessageBody(IN SipMessage* pstMessage);

GLOBAL IMS_BOOL EncodeAddrSpec(
        IN CONST SipAddrSpec* pstAddrSpec, IN IMS_BOOL bParams, OUT AString& strAddrSpec);
GLOBAL IMS_BOOL EncodeHeaderBody(
        IN CONST SipHeaderBase* pstHeader, IN IMS_BOOL bParams, OUT AString& strHeaderBody);
GLOBAL IMS_BOOL EncodeMessage(IN SipMessage* pstMessage, IN IMS_SINT32 nOptions,
        OUT IMS_BYTE*& pBuffer, OUT IMS_SINT32& nBuffLen);
GLOBAL IMS_BOOL EncodePartialMessage(
        IN SipMessage* pstMessage, IN IMS_SINT32 nOptions, OUT ByteArray& objMessage);
GLOBAL IMSList<SipParameter*> ExtractParameters(IN SipHeaderBase* pstHeader);
GLOBAL IMSList<SipParameter*> ExtractParameters(IN SipAddrSpec* pstAddrSpec);
GLOBAL IMSList<SipParameter*> ExtractParameters(IN CONST AString& strParams, IN IMS_CHAR cSep);

GLOBAL void FreeMemBlock(IN void*& pvMemBlock);
GLOBAL void FreeAddrSpec(IN SipAddrSpec*& pstAddrSpec);
GLOBAL void FreeHeader(IN SipHeaderBase* pstHeader);
GLOBAL void FreeHeaderEx(IN SipHeaderBase*& pstHeader);
GLOBAL void FreeMessage(IN SipMessage*& pstMessage);
GLOBAL void FreeMessageBody(IN SipMsgBody*& pstMsgBody);

GLOBAL IMS_CHAR GetCompactHeaderName(
        IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull());
GLOBAL const IMS_CHAR* GetHeaderName(
        IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull());
GLOBAL const IMS_CHAR* GetHeaderNameFromType(IN IMS_SINT32 nType);
GLOBAL IMS_SINT32 GetHeaderTypeFromName(IN CONST AString& strName);

GLOBAL SipAddrSpec* GetAddrSpec(IN SipHeaderBase* pstHeader);
GLOBAL SipAddrSpec* GetAddrSpec(
        IN SipMessage* pstMessage, IN IMS_SINT32 nType, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetChallengeScheme(IN SipHeaderBase* pstHeader);
GLOBAL IMS_BOOL GetContent(
        IN SipMsgBody* pstMsgBody, OUT IMS_BYTE*& pContent, OUT IMS_SINT32& nContentLength);
GLOBAL IMS_UINT32 GetCSeqNumber(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL GetRAckHeader(IN SipMessage* pstMessage, OUT IMS_UINT32& nResponseNum,
        OUT IMS_UINT32& nCSeqNum, OUT SipMethod& objMethod);
GLOBAL IMS_SINT32 GetDestinationTransport(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL GetEventHeader(
        IN SipMessage* pstMessage, OUT AString& strEvent, OUT AString& strEventId);
GLOBAL SipHeaderBase* GetHeader(
        IN SipMessage* pstMessage, IN IMS_SINT32 nType, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetHeaderAsString(IN SipMessage* pstMessage, IN IMS_SINT32 nType,
        IN IMS_BOOL bParams = IMS_FALSE, IN IMS_UINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetHeaderCount(IN SipMessage* pstMessage, IN IMS_SINT32 nType);
GLOBAL IMS_BOOL GetHostAndPort(
        IN SipAddrSpec* pstAddrSpec, OUT AString& strHost, OUT IMS_UINT32& nPort);
GLOBAL IMS_BOOL GetHostNPortFromViaHeader(
        IN SipMessage* pstMessage, OUT AString& strHost, OUT IMS_SINT32& nPort);
GLOBAL SipMethod GetMethod(IN SipMessage* pstMessage);
GLOBAL SipMsgBody* GetMessageBody(IN SipMessage* pstMessage, IN IMS_SINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetMessageBodyCount(IN SipMessage* pstMessage);
GLOBAL AString GetMIMEHeader(
        IN SipMsgBody* pstMsgBody, IN IMS_SINT32 nType, IN IMS_SINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetMIMEHeaderCount(IN SipMsgBody* pstMsgBody, IN IMS_SINT32 nType);
GLOBAL AString GetParameter(
        IN SipAddrSpec* pstAddrSpec, IN CONST AString& strName, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetParameter(
        IN SipHeaderBase* pstHeader, IN CONST AString& strName, IN IMS_UINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetParameterCount(IN SipAddrSpec* pstAddrSpec);
GLOBAL IMS_SINT32 GetParameterCount(IN SipHeaderBase* pstHeader);
GLOBAL SipAddrSpec* GetRequestUri(IN SipMessage* pstMessage);
GLOBAL AString GetSentByFromVia(IN SipHeaderBase* pstHeader);
GLOBAL AString GetSentProtocolFromVia(IN SipHeaderBase* pstHeader);
GLOBAL AString GetSIPVersion(IN SipMessage* pstMessage);
GLOBAL IMS_SINT32 GetStatusCode(IN SipMessage* pstMessage);
GLOBAL SipStatusCode GetStatusCodeEx(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL GetSubscriptionStateHeader(
        IN SipMessage* pstMessage, OUT AString& strSubsState, OUT IMS_SINT32* pnExpires = IMS_NULL);
GLOBAL SipHeaderBase* GetUnknownHeader(
        IN SipMessage* pstMessage, IN CONST AString& strName, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetUnknownHeaderName(IN SipHeaderBase* pstHeader);
GLOBAL AString GetUnknownHeaderBody(IN SipHeaderBase* pstHeader);
GLOBAL AString GetViaBranchParameter(IN SipMessage* pstMessage);
GLOBAL SipEn_HdrType GetHdrEnumType(IN SipEn_HdrType nType);

GLOBAL IMS_BOOL HasParameter(IN SipHeaderBase* pstHeader, IN CONST AString& strName);
GLOBAL IMS_BOOL HasParameter(IN SipAddrSpec* pstAddrSpec, IN CONST AString& strName);
GLOBAL IMS_BOOL HasMIMEMessageBody(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL HasSDPMessageBody(IN SipMessage* pstMessage);

GLOBAL IMS_BOOL InsertHeader(
        IN SipHeaderBase* pstHeader, IN IMS_UINT32 nIndex, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL IsCompactHeaderNameSupported(
        IN IMS_SINT32 nType, IN CONST AString& strName = AString::ConstNull());
GLOBAL IMS_BOOL IsHeaderPresent(IN SipMessage* pstMessage, IN IMS_SINT32 nType);
GLOBAL IMS_BOOL IsMessageBodyCompressed(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL UncompressMessageBody(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL IsMessageBodySDP(IN SipMsgBody* pstMsgBody);
GLOBAL IMS_BOOL IsMessageRPR(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL IsOptionRequired(IN SipMessage* pstMessage, IN CONST AString& strOption);
GLOBAL IMS_BOOL IsOptionSupported(IN SipMessage* pstMessage, IN CONST AString& strOption);
GLOBAL IMS_BOOL IsRequestMessage(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL IsSingleHeader(IN IMS_SINT32 nType);
GLOBAL IMS_BOOL IsAddressFormatHeader(IN IMS_SINT32 nType, IN CONST AString& strName);
GLOBAL IMS_BOOL IsAQUOTRequiredForAddressFormat(IN IMS_SINT32 nType, IN CONST AString& strName);
GLOBAL IMS_BOOL IsUnknownHeader(
        IN_OUT IMS_SINT32& nType, IN CONST AString& strName = AString::ConstNull());
GLOBAL IMS_BOOL IsUriSchemeAllowed(IN SipHeaderBase* pstHeader);
GLOBAL IMS_BOOL IsUriSchemeSupported(IN SipHeaderBase* pstHeader);

GLOBAL IMS_BOOL OverwriteHeaders(IN SipMessage* pstSrcMessage, IN_OUT SipMessage*& pstDestMessage);

GLOBAL void ParseHostNPort(
        IN CONST AString& strHostNPort, OUT AString& strHost, OUT IMS_SINT32& nPort);

GLOBAL IMS_BOOL RemoveAllMessageBodies(IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL RemoveHeader(IN IMS_SINT32 nType, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL RemoveParameter(IN CONST AString& strName, IN_OUT SipHeaderBase*& pstHeader);
GLOBAL IMS_BOOL RemoveParameter(IN CONST AString& strName, IN_OUT SipAddrSpec*& pstAddrSpec);
GLOBAL void RemoveUserAndPassword(IN_OUT SipAddrSpec*& pstAddrSpec);

GLOBAL IMS_BOOL SetChallengeScheme(IN CONST AString& strScheme, IN_OUT SipHeaderBase*& pstHeader);
GLOBAL IMS_BOOL SetContent(
        IN CONST IMS_BYTE* pContent, IN IMS_SINT32 nContentLength, IN_OUT SipMsgBody*& pstMsgBody);
GLOBAL IMS_BOOL SetHeader(IN SipHeaderBase* pstHeader, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL SetMethod(IN CONST SipMethod& objMethod, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL SetMIMEHeader(
        IN IMS_SINT32 nType, IN SipHeaderBase* pstHeader, IN_OUT SipMsgBody*& pstMsgBody);
GLOBAL IMS_BOOL SetMIMEHeader(IN IMS_SINT32 nType, IN CONST AString& strName,
        IN CONST AString& strBody, IN_OUT SipMsgBody*& pstMsgBody);
GLOBAL IMS_BOOL SetParameter(
        IN SipHeaderBase* pstHeader, IN CONST AString& strName, IN CONST AString& strValue);
GLOBAL IMS_BOOL SetRequestLine(
        IN CONST AString& strMethod, IN CONST AString& strURI, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL SetRequestUri(IN SipAddrSpec* pstAddrSpec, IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL SetStatusLine(IN IMS_SINT32 nStatusCode, IN CONST AString& strReasonPhrase,
        IN_OUT SipMessage*& pstMessage);
GLOBAL IMS_BOOL SetUnknownHeader(
        IN SipHeaderBase* pstHeader, IN CONST AString& strName, IN_OUT SipMessage*& pstMessage);

GLOBAL IMS_BOOL UpdateSentProtocol(IN SipMessage* pstMessage, IN CONST AString& strSentProtocol);

// APIs for Bad header control
GLOBAL void DisplayBadHeaders(IN SipMessage* pstMessage);
GLOBAL IMS_SINT32 GetBadHeaderCount(IN SipMessage* pstMessage);
GLOBAL IMS_BOOL HasMandatoryHeaders(IN SipMessage* pstMessage);

// APIs for SIP authentication
GLOBAL IMS_BOOL GetEntityBody(IN SipMessage* pstMessage, OUT AString& strEntityBody);

// APIs for SIP transaction layer
inline SipEventContext* TxnBuffer_GetEventContext(IN SipTxn* pTxn)
{
    (void)pTxn;
    return IMS_NULL;
}
inline IMS_BOOL TxnBuffer_IsClientTransaction(IN SipTxn* pTxn)
{
    SipEn_TxnType enTxnType = static_cast<SipEn_TxnType>(pTxn->GetTxnType());
    return (enTxnType == ETXN_INVCLITXN) || (enTxnType == ETXN_NONINVCLITXN);
}
inline const IMS_CHAR* TxnKey_GetViaBranch(IN SipTxnKey* pstTxnKey)
{
    return pstTxnKey->GetViaBranchParam();
}
inline const IMS_CHAR* TxnKey_GetCallId(IN SipTxnKey* pstTxnKey)
{
    return pstTxnKey->GetCallId();
}
inline const IMS_CHAR* TxnKey_GetMethod(IN SipTxnKey* pstTxnKey)
{
    return pstTxnKey->GetMethod();
}
inline IMS_SINT32 TxnKey_GetStatusCode(IN SipTxnKey* pstTxnKey)
{
    return pstTxnKey->GetRespCode();
}

GLOBAL SIPTxnKey* CreateTxnKey(IN SipMessage* pstMessage);
GLOBAL SipTxnKey* CreateTxnKey(IN SipMessage* pstMessage, IN IMS_SINT32 nAPICalled);
GLOBAL SIPTxnKey* CreateTxnKeyFromKey(IN SipTxnKey* pstTxnKey);
GLOBAL IMS_BOOL CompareTxnKeys(IN SipTxnKey* pstTxnKey1, IN SipTxnKey* pstTxnKey2);
GLOBAL IMS_BOOL CompareTxnKeysForAck(IN SipTxnKey* pstTxnKey1, IN SipTxnKey* pstTxnKey2);
GLOBAL IMS_BOOL CompareTxnKeysForCancel(IN SipTxnKey* pstCancelKey, IN SipTxnKey* pstTxnKey);

GLOBAL IMS_BOOL AbortTransaction(IN SipTxnKey* pstTxnKey, IN SipTxnContext* pstTxnContext);
GLOBAL SipEventContext* CreateEventContext();
GLOBAL SipTxnContext* CreateTxnContext();
GLOBAL void DestroyEventContext(IN SipEventContext* pstContext);
GLOBAL void DestroyTxnContext(IN SipTxnContext* pstContext);
GLOBAL void DisplayTxnKey(IN CONST SipTxnKey* pstTxnKey);
GLOBAL void FreeTxnKey(IN SipTxnKey*& pstTxnKey);
GLOBAL void FreeTxn(IN SipTxn*& pTxn);
GLOBAL SIPTxnContextData* GetTxnContextData(IN SipEventContext* pstContext);
#if 0
    GLOBAL IMS_SINT32 HandleTxn(IN SipMessage *pstMessage, IN SipTxnContext *pstTxnContext,
            OUT SipTxnKey *&pstTxnKey);
    GLOBAL IMS_BOOL SendToTxnLayer(IN CONST ByteArray &objBuffer, IN SipMessage *pstMessage,
            IN SipTranspAddr *pstAddr, IN IMS_BYTE byTranspType, IN SipTxnContext *pstTxnContext,
            OUT SipTxnKey *&pstTxnKey);
#endif
GLOBAL void TerminateTransaction(IN SipTxnKey* pstTxnKey);

// APIs for SIP transaction timer
GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN SipEn_TimerType enTimerType);
GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN SipTimeoutData* pData);
GLOBAL void InvokeTimerCallback(
        IN SipTimerCallback pfnCallback, IN SipTimeoutData* pData, IN IMS_PVOID pvExtraParam);
GLOBAL void SetTimerValues(IN SipTimerValues* pTV, IN_OUT SipTxnContext*& pstTxnContext);

// APIs for trace or debugging
GLOBAL void DisplayUnknownHeaders(IN SipMessage* pstMessage);
GLOBAL const IMS_CHAR* GetLogString(IN CONST IMS_CHAR* pszInput, IN_OUT IMS_CHAR* pszOutput,
        IN IMS_SINT32 nOutSize /* > 3, excluding null char */,
        IN CONST IMS_CHAR cDelimiter = 0 /* no delimiter */);
}  // namespace SIPStack

template <typename T>
inline void SIPStack::Free(IN T& pMemBlock)
{
    SIPStack::FreeMemBlock(reinterpret_cast<void*&>(pMemBlock));
}

#endif
