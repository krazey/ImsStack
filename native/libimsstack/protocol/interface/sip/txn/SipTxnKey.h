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
#ifndef __SIP_TXN_KEY_H__
#define __SIP_TXN_KEY_H__

#include "SipDatatypes.h"
#include "msg/SipMessage.h"

/* Transaction Key Used to Match Response to Request vice versa */
class SipTxnKey : public SipRefBase
{
private:
    SIP_UINT32 m_nRSeqNum;

public:
    enum
    {
        RULE_NONE = 0x0000,
        RULE_COMPARE_TO_TAG = 0x0001,
        RULE_COMPARE_VIA_BRANCH = 0x0002,
        RULE_COMPARE_RSEQ = 0x0004
    };

    SIP_INT32 m_eMsgType;

    /* Key Fetched from via */
    SIP_CHAR* m_pszViaBranchParam;
    SIP_CHAR* m_pszViaHost;
    SIP_UINT16 m_nViaHostPort;

    /*
       For Req : Method <== Request Line
       For Resp:Method <==  CSeq
     */
    SIP_CHAR* m_pszMethod;
    SIP_UINT32 m_nCseqNum;

    /* For INVITE at Receiving Side */
    SipAddrSpec* m_pRequestUri;

    SIP_CHAR* m_pszToTag;
    SIP_CHAR* m_pszFromTag;
    SIP_CHAR* m_pszCallId;
    /*Status code store*/
    SIP_UINT16 m_nRespCode;
    SIP_INT32 m_eTxnType;
    SIP_UINT32 m_nRules;

public:
    SipTxnKey();
    SipTxnKey(SipTxnKey* pTxnKey, SIP_UINT16* pnError);
    SipTxnKey(SipMessage* pSipMsg, SIP_UINT16* pnError);

    SIP_VOID Init(SipTxnKey* pTxnKey, SIP_UINT16* pnError);

    inline void AddRule(SIP_UINT32 nRule) { m_nRules |= nRule; }
    inline void RemoveRule(SIP_UINT32 nRule) { m_nRules &= (~nRule); }
    inline SIP_BOOL HasRule(SIP_UINT32 nRule) const
    {
        return ((m_nRules & nRule) != 0) ? SIP_TRUE : SIP_FALSE;
    }
    inline SIP_UINT32 GetRules() const { return m_nRules; }
    inline SIP_VOID SetRules(SIP_UINT32 nRules) { m_nRules = nRules; }

    inline SIP_CHAR* GetCallId() const { return m_pszCallId; }
    inline SIP_CHAR* GetFromTag() const { return m_pszFromTag; }
    inline SIP_CHAR* GetMethod() const { return m_pszMethod; }
    inline SIP_CHAR* GetToTag() const { return m_pszToTag; }
    inline SIP_CHAR* GetViaBranchParam() const { return m_pszViaBranchParam; }
    inline SIP_CHAR* GetViaHost() const { return m_pszViaHost; }
    inline SIP_UINT32 GetCSeqNum() const { return m_nCseqNum; }
    inline SIP_INT32 GetMsgType() const { return m_eMsgType; }
    inline SIP_UINT32 GetRSeq() const { return m_nRSeqNum; }
    inline SIP_UINT16 GetResponseCode() const { return m_nRespCode; }
    inline SIP_INT32 GetTxnType() const { return m_eTxnType; }

    SIP_VOID SetMethod(const SIP_CHAR* pszMethod);
    inline SIP_VOID SetCseqNum(SIP_UINT32 nCseqNum) { m_nCseqNum = nCseqNum; }
    inline SIP_VOID SetRSeq(SIP_UINT32 nRseqNum) { m_nRSeqNum = nRseqNum; }
    inline SIP_VOID SetResponseCode(SIP_UINT16 nRespCode) { m_nRespCode = nRespCode; }
    inline SIP_VOID SetTxnType(SIP_INT32 eTxnType) { m_eTxnType = eTxnType; }

    SIP_INT32 CompareKeys(SipTxnKey* pGeneratedKey);
    SIP_INT32 CompareKeysForRPR(SipTxnKey* pGeneratedKey);

private:
    ~SipTxnKey() override;
    SIP_VOID Clear();
};

#endif  //__SIP_TXN_KEY_H__
