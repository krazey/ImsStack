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

#include "ISipTimerUtil.h"
#include "SipContextUtils.h"
#include "SipMethod.h"
#include "SipParameter.h"
#include "SipStackManager.h"
#include "SipStatusCode.h"
#include "SipTxnKey.h"
#include "SipStackError.h"
#include "msg/SipMessage.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnKey.h"

class ISipConfigV;
class ISipHeader;
class SipProfile;
class SipTimeoutData;
class SipTimerValues;

namespace SipStack
{

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

/// Options for the selective SIP message encoding
enum
{
    OPT_START_LINE = 0x01,
    OPT_HEADER_PART = 0x02,
    OPT_BODY_PART = 0x03,
    OPT_ALL = (OPT_START_LINE | OPT_HEADER_PART | OPT_BODY_PART)
};

/// SIP message types
enum
{
    SIP_MESSAGE_REQUEST = ::SipMessage::REQ_TYPE,
    SIP_MESSAGE_RESPONSE = ::SipMessage::RESP_TYPE,
    SIP_MESSAGE_ANY = ::SipMessage::TYPE_INVALID
};

/// SIP transaction API called types
enum
{
    SIP_TXN_MSG_SENT = 0,
    SIP_TXN_MSG_RECEIVED = 1
};

GLOBAL void Initialize();
GLOBAL void SetTransactionTimerValues(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile, IN const ISipConfigV* piSipConfigV);
GLOBAL SipEn_ErrorTypes GetLastError();
inline IMS_BOOL IsLastErrorNoExist()
{
    return (GetLastError() == EERR_NOEXISTS);
}

template <typename T>
inline void AddReference(IN_OUT T& pObject)
{
    if (pObject != IMS_NULL)
    {
        pObject->Increment();
    }
}
template <typename T>
inline void RemoveReference(IN_OUT T& pObject)
{
    if (pObject != IMS_NULL)
    {
        pObject->Decrement();
    }
}
template <typename T>
inline void Free(IN T& pMemBlock);

inline IMS_BOOL IsValidHeader(IN const SipHeaderBase* pHeader)
{
    return (pHeader != IMS_NULL) && (pHeader->GetHdrType() != SipHeaderBase::TYPE_INVALID);
}
inline IMS_SINT32 GetAnyHeaderType()
{
    return SipHeaderBase::TYPE_END;
}
inline IMS_SINT32 GetHeaderType(IN const SipHeaderBase* pHeader)
{
    return pHeader->GetHdrType();
}
inline void SetHeaderType(IN IMS_SINT32 nType, IN_OUT SipHeaderBase*& pHeader)
{
    pHeader->SetHdrType(nType);
}

inline IMS_BOOL IsUriSchemeSip(IN const SipAddrSpec* pAddrSpec)
{
    return pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIP;
}
inline IMS_BOOL IsUriSchemeSips(IN const SipAddrSpec* pAddrSpec)
{
    return pAddrSpec->GetUriScheme() == SipUri::SCHEME_SIPS;
}
inline const IMS_CHAR* AddrSpec_GetUser(IN const SipAddrSpec* pAddrSpec)
{
    return pAddrSpec->GetSipUriAsRef()->GetUser();
}
inline const IMS_CHAR* AddrSpec_GetPassword(IN const SipAddrSpec* pAddrSpec)
{
    return pAddrSpec->GetSipUriAsRef()->GetPassword();
}
inline const IMS_CHAR* AddrSpec_GetHost(IN const SipAddrSpec* pAddrSpec)
{
    return pAddrSpec->GetSipUriAsRef()->GetHost();
}
inline IMS_SINT32 AddrSpec_GetPort(IN const SipAddrSpec* pAddrSpec)
{
    return pAddrSpec->GetSipUriAsRef()->GetPort();
}

GLOBAL IMS_BOOL AppendHeader(IN SipHeaderBase* pHeader, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL AppendMessageBody(IN SipMsgBody* pMsgBody, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL PrependHeader(IN SipHeaderBase* pHeader, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL PrependUnknownHeader(
        IN const AString& strName, IN const AString& strValue, IN_OUT ::SipMessage*& pMessage);

GLOBAL IMS_BOOL CheckMandatoryHeaders(IN const ::SipMessage* pMessage);

GLOBAL SipHeaderBase* CloneHeader(IN SipHeaderBase* pHeader);
GLOBAL ::SipMessage* CloneMessage(IN ::SipMessage* pMessage);
GLOBAL SipMsgBody* CloneMessageBody(IN SipMsgBody* pMsgBody);
GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase* pHeader);
GLOBAL SipHeaderBase* CopyHeader(IN SipHeaderBase* pDstHeader, IN SipHeaderBase* pSrcHeader);

GLOBAL IMS_BOOL CorrectMessageBody(IN_OUT ::SipMessage*& pMessage);

GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType);
GLOBAL SipHeaderBase* CreateHeader(IN IMS_SINT32 nType, IN SipAddrSpec* pAddrSpec);
GLOBAL ::SipMessage* CreateMessage(IN IMS_SINT32 nType);
GLOBAL SipMsgBody* CreateMessageBody();
GLOBAL SipHeaderBase* CreateViaHeader(IN const AString& strSentProtocol,
        IN const AString& strSentBy, IN const AString& strBranch);

GLOBAL SipAddrSpec* DecodeAddrSpec(IN const AString& strAddress);
GLOBAL SipHeaderBase* DecodeHeader(
        IN IMS_SINT32 nType, IN const AString& strName, IN const AString& strBody);
inline SipHeaderBase* DecodeHeader(IN IMS_SINT32 nType, IN const AString& strBody)
{
    return DecodeHeader(nType, AString::ConstNull(), strBody);
}
GLOBAL IMS_BOOL DecodeMessage(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen,
        IN IMS_SINT32 nOptions, OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL DecodeMessageBody(IN ::SipMessage* pMessage);

GLOBAL IMS_BOOL EncodeAddrSpec(
        IN const SipAddrSpec* pAddrSpec, IN IMS_BOOL bParams, OUT AString& strAddrSpec);
GLOBAL IMS_BOOL EncodeHeaderBody(
        IN const SipHeaderBase* pHeader, IN IMS_BOOL bParams, OUT AString& strHeaderBody);
GLOBAL IMS_BOOL EncodeMessage(IN ::SipMessage* pMessage, IN IMS_SINT32 nOptions,
        OUT IMS_BYTE*& pBuffer, OUT IMS_SINT32& nBuffLen);
GLOBAL IMS_BOOL EncodePartialMessage(
        IN ::SipMessage* pMessage, IN IMS_SINT32 nOptions, OUT ByteArray& objMessage);
GLOBAL ImsList<SipParameter*> ExtractParameters(IN const SipHeaderBase* pHeader);
GLOBAL ImsList<SipParameter*> ExtractParameters(IN SipAddrSpec* pAddrSpec);
GLOBAL ImsList<SipParameter*> ExtractParameters(IN const AString& strParams, IN IMS_CHAR cSep);

GLOBAL void FreeAddrSpec(IN SipAddrSpec*& pAddrSpec);
GLOBAL void FreeHeader(IN SipHeaderBase* pHeader);
GLOBAL void FreeHeaderEx(IN SipHeaderBase*& pHeader);
GLOBAL void FreeMessage(IN ::SipMessage*& pMessage);
GLOBAL void FreeMessageBody(IN SipMsgBody*& pMsgBody);

GLOBAL IMS_CHAR GetCompactHeaderName(
        IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull());
GLOBAL const IMS_CHAR* GetHeaderName(
        IN IMS_SINT32 nType, IN const AString& strName = AString::ConstNull());
GLOBAL IMS_SINT32 GetHeaderTypeFromName(IN const AString& strName);

GLOBAL SipAddrSpec* GetAddrSpec(IN SipHeaderBase* pHeader);
GLOBAL SipAddrSpec* GetAddrSpec(
        IN ::SipMessage* pMessage, IN IMS_SINT32 nType, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetChallengeScheme(IN SipHeaderBase* pHeader);
GLOBAL IMS_BOOL GetContent(
        IN SipMsgBody* pMsgBody, OUT IMS_BYTE*& pContent, OUT IMS_SINT32& nContentLength);
GLOBAL IMS_UINT32 GetCSeqNumber(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL GetEventHeader(
        IN ::SipMessage* pMessage, OUT AString& strEvent, OUT AString& strEventId);
GLOBAL SipHeaderBase* GetHeader(
        IN ::SipMessage* pMessage, IN IMS_SINT32 nType, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetHeaderAsString(IN ::SipMessage* pMessage, IN IMS_SINT32 nType,
        IN IMS_BOOL bParams = IMS_FALSE, IN IMS_UINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetHeaderCount(IN ::SipMessage* pMessage, IN IMS_SINT32 nType);
GLOBAL IMS_BOOL GetHostAndPort(
        IN SipAddrSpec* pAddrSpec, OUT AString& strHost, OUT IMS_UINT32& nPort);
GLOBAL IMS_BOOL GetHostNPortFromViaHeader(
        IN ::SipMessage* pMessage, OUT AString& strHost, OUT IMS_SINT32& nPort);
GLOBAL SipMethod GetMethod(IN ::SipMessage* pMessage);
GLOBAL SipMsgBody* GetMessageBody(IN ::SipMessage* pMessage, IN IMS_SINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetMessageBodyCount(IN ::SipMessage* pMessage);
GLOBAL AString GetMimeHeader(
        IN SipMsgBody* pMsgBody, IN IMS_SINT32 nType, IN IMS_SINT32 nIndex = 0);
GLOBAL IMS_SINT32 GetMimeHeaderCount(IN SipMsgBody* pMsgBody, IN IMS_SINT32 nType);
GLOBAL AString GetParameter(
        IN SipAddrSpec* pAddrSpec, IN const AString& strName, IN IMS_UINT32 nIndex = 0);
GLOBAL AString GetParameter(
        IN SipHeaderBase* pHeader, IN const AString& strName, IN IMS_UINT32 nIndex = 0);
GLOBAL SipAddrSpec* GetRequestUri(IN ::SipMessage* pMessage);
GLOBAL AString GetSentByFromVia(IN SipHeaderBase* pHeader);
GLOBAL AString GetSentProtocolFromVia(IN SipHeaderBase* pHeader);
GLOBAL AString GetSipVersion(IN ::SipMessage* pMessage);
GLOBAL IMS_SINT32 GetStatusCode(IN ::SipMessage* pMessage);
GLOBAL SipStatusCode GetStatusCodeEx(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL GetSubscriptionStateHeader(
        IN ::SipMessage* pMessage, OUT AString& strSubsState, OUT IMS_SINT32* pnExpires = IMS_NULL);
GLOBAL AString GetUnknownHeaderName(IN const SipHeaderBase* pHeader);
GLOBAL AString GetUnknownHeaderBody(IN const SipHeaderBase* pHeader);
GLOBAL AString GetViaBranchParameter(IN ::SipMessage* pMessage);
GLOBAL IMS_SINT32 GetHdrEnumType(IN IMS_SINT32 nType);

GLOBAL IMS_BOOL HasParameter(IN SipHeaderBase* pHeader, IN const AString& strName);
GLOBAL IMS_BOOL HasParameter(IN SipAddrSpec* pAddrSpec, IN const AString& strName);
GLOBAL IMS_BOOL HasMimeMessageBody(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL HasSdpMessageBody(IN ::SipMessage* pMessage);

GLOBAL IMS_BOOL InsertHeader(
        IN SipHeaderBase* pHeader, IN IMS_UINT32 nIndex, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL IsHeaderPresent(IN ::SipMessage* pMessage, IN IMS_SINT32 nType);
GLOBAL IMS_BOOL IsMessageBodyCompressed(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL UncompressMessageBody(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL IsMessageBodySdp(IN SipMsgBody* pMsgBody);
GLOBAL IMS_BOOL IsMessageRpr(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL IsOptionRequired(IN ::SipMessage* pMessage, IN const AString& strOption);
GLOBAL IMS_BOOL IsOptionSupported(IN ::SipMessage* pMessage, IN const AString& strOption);
GLOBAL IMS_BOOL IsRequestMessage(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL IsSingleHeader(IN IMS_SINT32 nType);
GLOBAL IMS_BOOL IsAddressFormatHeader(IN IMS_SINT32 nType, IN const AString& strName);
GLOBAL IMS_BOOL IsAquotRequiredForAddressFormat(IN IMS_SINT32 nType, IN const AString& strName);
GLOBAL IMS_BOOL IsUnknownHeader(
        IN_OUT IMS_SINT32& nType, IN const AString& strName = AString::ConstNull());
GLOBAL IMS_BOOL OverwriteHeaders(IN ::SipMessage* pSrcMessage, IN_OUT ::SipMessage*& pDstMessage);

GLOBAL void ParseHostNPort(
        IN const AString& strHostNPort, OUT AString& strHost, OUT IMS_SINT32& nPort);

GLOBAL IMS_BOOL RemoveAllMessageBodies(IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL RemoveHeader(IN IMS_SINT32 nType, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL RemoveParameter(IN const AString& strName, IN_OUT SipHeaderBase*& pHeader);
GLOBAL IMS_BOOL RemoveParameter(IN const AString& strName, IN_OUT SipAddrSpec*& pAddrSpec);
GLOBAL void RemoveUserAndPassword(IN_OUT SipAddrSpec*& pAddrSpec);

GLOBAL IMS_BOOL SetChallengeScheme(IN const AString& strScheme, IN_OUT SipHeaderBase*& pHeader);
GLOBAL IMS_BOOL SetContent(
        IN const IMS_BYTE* pContent, IN IMS_SINT32 nContentLength, IN_OUT SipMsgBody*& pMsgBody);
GLOBAL IMS_BOOL SetHeader(IN SipHeaderBase* pHeader, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL SetMethod(IN const SipMethod& objMethod, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL SetMimeHeader(
        IN IMS_SINT32 nType, IN SipHeaderBase* pHeader, IN_OUT SipMsgBody*& pMsgBody);
GLOBAL IMS_BOOL SetMimeHeader(IN IMS_SINT32 nType, IN const AString& strName,
        IN const AString& strBody, IN_OUT SipMsgBody*& pMsgBody);
GLOBAL IMS_BOOL SetParameter(
        IN SipHeaderBase* pHeader, IN const AString& strName, IN const AString& strValue);
GLOBAL IMS_BOOL SetRequestLine(
        IN const AString& strMethod, IN const AString& strUri, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL SetRequestUri(IN SipAddrSpec* pAddrSpec, IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL SetStatusLine(IN IMS_SINT32 nStatusCode, IN const AString& strReasonPhrase,
        IN_OUT ::SipMessage*& pMessage);
GLOBAL IMS_BOOL UpdateSentProtocol(IN ::SipMessage* pMessage, IN const AString& strSentProtocol);

// APIs for bad header control
GLOBAL void DisplayBadHeaders(IN ::SipMessage* pMessage);
GLOBAL IMS_SINT32 GetBadHeaderCount(IN ::SipMessage* pMessage);
GLOBAL IMS_BOOL HasMandatoryHeaders(IN ::SipMessage* pMessage);

// APIs for SIP authentication
GLOBAL IMS_BOOL GetEntityBody(IN ::SipMessage* pMessage, OUT AString& strEntityBody);

inline const IMS_CHAR* TxnKey_GetViaBranch(IN ::SipTxnKey* pTxnKey)
{
    return pTxnKey->GetViaBranchParam();
}
inline const IMS_CHAR* TxnKey_GetCallId(IN ::SipTxnKey* pTxnKey)
{
    return pTxnKey->GetCallId();
}
inline const IMS_CHAR* TxnKey_GetMethod(IN ::SipTxnKey* pTxnKey)
{
    return pTxnKey->GetMethod();
}
inline IMS_SINT32 TxnKey_GetStatusCode(IN ::SipTxnKey* pTxnKey)
{
    return pTxnKey->GetRespCode();
}

GLOBAL sipcore::SipTxnKey* CreateTxnKey(IN ::SipMessage* pMessage);
GLOBAL ::SipTxnKey* CreateTxnKey(IN ::SipMessage* pMessage, IN IMS_SINT32 nApiCalled);
GLOBAL sipcore::SipTxnKey* CreateTxnKeyFromKey(IN ::SipTxnKey* pTxnKey);
GLOBAL IMS_BOOL CompareTxnKeys(IN ::SipTxnKey* pTxnKey1, IN ::SipTxnKey* pTxnKey2);
GLOBAL IMS_BOOL CompareTxnKeysForAck(IN ::SipTxnKey* pTxnKey1, IN ::SipTxnKey* pTxnKey2);
GLOBAL IMS_BOOL CompareTxnKeysForCancel(IN ::SipTxnKey* pCancelKey, IN ::SipTxnKey* pTxnKey);

GLOBAL IMS_BOOL AbortTransaction(IN ::SipTxnKey* pTxnKey, IN SipTxnContext* pTxnContext);
GLOBAL SipTxnContext* CreateTxnContext();
GLOBAL void DestroyTxnContext(IN SipTxnContext* pContext);
GLOBAL void DisplayTxnKey(IN const ::SipTxnKey* pTxnKey);
GLOBAL void FreeTxnKey(IN ::SipTxnKey*& pTxnKey);
GLOBAL void FreeTxn(IN SipTxn*& pTxn);
GLOBAL void TerminateTransaction(IN ::SipTxnKey* pTxnKey);

// APIs for SIP transaction timer
GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN IMS_SINT32 eTimerType);
GLOBAL const IMS_CHAR* GetTimerTypeAsString(IN SipTimeoutData* pData);
GLOBAL void InvokeTimerCallback(
        IN SipTimerCallback pfnCallback, IN SipTimeoutData* pData, IN IMS_PVOID pvExtraParam);
GLOBAL void SetTimerValues(IN SipTimerValues* pTv, IN_OUT SipTxnContext*& pTxnContext);

// APIs for trace or debugging
GLOBAL void DisplayUnknownHeaders(IN ::SipMessage* pMessage);
GLOBAL const IMS_CHAR* GetLogString(IN const IMS_CHAR* pszInput, IN_OUT IMS_CHAR* pszOutput,
        IN IMS_SINT32 nOutSize /* > 3, excluding null char*/,
        IN const IMS_CHAR cDelimiter = 0 /*no delimiter*/);

GLOBAL IMS_BOOL DecodeHeaderComponent(
        IN const SipAddrSpec* pAddrSpec, OUT ImsList<ISipHeader*>& objHeaders);
GLOBAL IMS_BOOL DecodeHeaderComponent(
        IN const AString& strHeaders, OUT ImsList<ISipHeader*>& objHeaders);
}  // namespace SipStack

#endif
