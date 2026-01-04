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
#ifndef SIP_TRANSACTION_STATE_H_
#define SIP_TRANSACTION_STATE_H_

#include "RcObject.h"

#include "msg/SipMessage.h"
#include "txn/SipTxnKey.h"

#include "Sip.h"
#include "SipDialogEx.h"
#include "SipProfile.h"

class ByteArray;
class IpAddress;

class SipTxnContext;

class ISipTransactionStateListener;
class ISipTransportListener;
class SipMessageInfo;
class SipMethod;
class SipTimerValues;
class SipTransport;

class SipTransactionState : public RcObject
{
public:
    SipTransactionState();
    explicit SipTransactionState(IN SipDialogEx* pDialogEx);
    SipTransactionState(IN const SipTransactionState& other);
    ~SipTransactionState() override;

    SipTransactionState& operator=(IN const SipTransactionState&) = delete;

public:
    virtual void Abort();
    virtual void Terminate();
    virtual IMS_SINT32 CheckMessageValidity();
    inline virtual IMS_BOOL FormMessage() { return IMS_TRUE; }
    virtual IMS_BOOL InitTxnDetails(IN const SipTransactionState* pTState);
    virtual void NotifyTimerExpired();
    virtual void PostProcessMessageSentByStack(
            IN ::SipMessage* pSipMsg, IN const ByteArray& objBuffer);
    virtual void PreProcessMessageSentByStack(IN ::SipMessage* pSipMsg);
    virtual IMS_BOOL Send(IN SipTimerValues* pTimerValues = IMS_NULL);
    virtual IMS_RESULT RetransmitMessage();
    virtual IMS_BOOL UpdateTransportDetails();

    inline IMS_SINT32 GetType() const { return m_nType; }
    inline SipDialogEx* GetDialog() const { return m_pDialogEx.Get(); }
    inline ::SipMessage* GetMessage() const { return m_pSipMsg; }
    inline ::SipMessage* GetLastMessage() const { return m_pLastSipMsg; }
    inline SipProfile* GetSipProfile() const { return m_pSipProfile.Get(); }
    inline SipTransport* GetSipTransport() const { return m_pTransport; }
    inline ::SipTxnKey* GetTxnKey() const { return m_pTxnKey; }
    IMS_SINT32 GetSlotId() const;
    IMS_BOOL IsIpSecRequired() const;
    inline void SetSipProfile(IN SipProfile* pProfile) { m_pSipProfile = pProfile; }
    inline void SetTransactionListener(IN ISipTransactionStateListener* piListener)
    {
        m_piListener = piListener;
    }
    void SetTransportListener(IN ISipTransportListener* piListener);
    void SetTransportTuple(IN const IpAddress& objIp, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFc = Sip::PORT_UNSPECIFIED,
            IN IMS_SINT32 nTransportExt = Sip::TRANSPORT_EXT_ANY);

    IMS_BOOL SendToNetwork(IN const IMS_BYTE* pBuffer, IN IMS_SINT32 nBuffLen);
    void UpdateMessage(IN ::SipMessage* pSipMsg);

protected:
    virtual SipTransactionState* Clone();

    IMS_BOOL Send(IN ::SipMessage* pSipMsg, IN const SipTimerValues* pTimerValues);
    void SetTimerValues(IN const SipTimerValues* pTimerValues, IN_OUT SipTxnContext*& pTxnContext);
    void SetFlowControlOption(IN const SipMethod& objMethod);

    static void LogSipMessageInfo(IN const SipMessageInfo& objMsgInfo);

public:
    enum
    {
        TYPE_CLIENT = 1,
        TYPE_SERVER
    };

    enum
    {
        CLASS_NONE,
        CLASS_INVITE,
        CLASS_REGULAR,
        CLASS_OVERLAP,
        CLASS_MAX
    };

protected:
    IMS_SINT32 m_nType;   // CLIENT or SERVER
    IMS_SINT32 m_nClass;  // Class of transaction (INVITE/non-INVITE/Overlap)
    IMS_UINT32 m_nCSeqNumber;
    RcPtr<SipProfile> m_pSipProfile;
    ISipTransactionStateListener* m_piListener;
    RcPtr<SipDialogEx> m_pDialogEx;
    SipTransport* m_pTransport;

    ::SipMessage* m_pSipMsg;
    ::SipMessage* m_pLastSipMsg;
    ::SipTxnKey* m_pTxnKey;
    ::SipTxnKey* m_pRprTxnKey;
};

#endif
