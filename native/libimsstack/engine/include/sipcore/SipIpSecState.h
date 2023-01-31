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
#ifndef SIP_IPSEC_STATE_H_
#define SIP_IPSEC_STATE_H_

#include "EngineActivity.h"
#include "ISipIpSecState.h"
#include "SipTransportAddress.h"
#include "SipTxnKey.h"
#include "msg/SipMessage.h"
#include "txn/SipTxnKey.h"

class SipIpSecState : public EngineActivity, public ISipIpSecState
{
private:
    class SecurityAssociation
    {
    public:
        SecurityAssociation();
        SecurityAssociation(IN const IpAddress& objIpU_, IN IMS_SINT32 nPortUc_,
                IN IMS_SINT32 nPortUs_, IN const IpAddress& objIpP_, IN IMS_SINT32 nPortPc_,
                IN IMS_SINT32 nPortPs_);
        SecurityAssociation(IN const SecurityAssociation& other);
        ~SecurityAssociation();

        SecurityAssociation& operator=(IN const SecurityAssociation&) = delete;

    public:
        IMS_BOOL AddTransaction(IN const sipcore::SipTxnKey* pTxnKey);
        IMS_BOOL CheckIpAddress(IN const SipTransportAddress& objNearEnd,
                IN const SipTransportAddress& objFarEnd) const;
        IMS_SINT32 GetSa(IN const SipTransportAddress& objNearEnd,
                IN const SipTransportAddress& objFarEnd, IN IMS_SINT32 nDirection) const;
        inline IMS_SINT32 GetState() const { return nState; }
        inline IMS_BOOL HasPendingTransaction() const { return !objSipTxnKeys.IsEmpty(); }
        IMS_BOOL RemoveTransaction(IN const sipcore::SipTxnKey* pTxnKey);
        void SetState(IN IMS_SINT32 nState);

    private:
        static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

    public:
        /// Direction of SA
        enum
        {
            DIRECTION_IN = 1,
            DIRECTION_OUT = 2
        };

        /// SA pairs
        enum
        {
            /// src_port, dst_port, transport, direction
            SA_START = 1,

            SA_PUC_PPS_U_OUT = SA_START,
            SA_PUC_PPS_T_OUT,
            SA_PUS_PPC_T_OUT,
            SA_PPC_PUS_U_IN,
            SA_PPC_PUS_T_IN,
            SA_PPS_PUC_T_IN,

            SA_END
        };

        IpAddress objIpU;
        IMS_SINT32 nPortUc;
        IMS_SINT32 nPortUs;
        IpAddress objIpP;
        IMS_SINT32 nPortPc;
        IMS_SINT32 nPortPs;

        IMS_SINT32 nState;
        // For tracking SIP transaction
        IMSList<sipcore::SipTxnKey> objSipTxnKeys;
    };

public:
    SipIpSecState();
    virtual ~SipIpSecState();

    SipIpSecState(IN const SipIpSecState&) = delete;
    SipIpSecState& operator=(IN const SipIpSecState&) = delete;

public:
    inline IMS_BOOL IsIpSecEnabled() const
    {
        return (m_pNewSa != IMS_NULL) || (m_pOldSa != IMS_NULL);
    }
    void NotifyMessageReceived(IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd, IN ::SipMessage* pSipMsg);
    void NotifyMessageSent(IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd, IN ::SipMessage* pSipMsg);
    void NotifyMessageSentFailed(IN ::SipMessage* pSipMsg);
    void NotifyTransactionAborted(IN ::SipTxnKey* pSipTxnKey);

private:
    // EngineActivity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // ISipIpSecState class
    void ClearIpSecSa(IN IMS_SINT32 nSaType) override;
    IMS_SINT32 GetState(IN IMS_SINT32 nSaType) const override;
    IMS_BOOL HasPendingTransaction(IN IMS_SINT32 nSaType) const override;
    void SetIpSecSa(IN IMS_SINT32 nSaType, IN const IpAddress& objIpU, IN IMS_SINT32 nPortUc,
            IN IMS_SINT32 nPortUs, IN const IpAddress& objIpP, IN IMS_SINT32 nPortPc,
            IN IMS_SINT32 nPortPs) override;
    inline void SetListener(IN ISipIpSecStateListener* piListener) override
    {
        m_piListener = piListener;
    }

    void NotifyMessageReceivedInternal(IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd, IN sipcore::SipTxnKey* pTxnKey);
    void NotifyMessageSentInternal(IN const SipTransportAddress& objNearEnd,
            IN const SipTransportAddress& objFarEnd, IN sipcore::SipTxnKey* pTxnKey);
    void NotifyTransactionAbortedInternal(IN sipcore::SipTxnKey* pTxnKey);

private:
    // Event for message processing
    enum
    {
        AMSG_NOTIFY_STATE_CHANGED = AMSG_USER
    };

    SecurityAssociation* m_pNewSa;
    SecurityAssociation* m_pOldSa;

    ISipIpSecStateListener* m_piListener;
};

#endif
