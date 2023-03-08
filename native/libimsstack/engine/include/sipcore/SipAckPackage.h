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
#ifndef SIP_ACK_PACKAGE_H_
#define SIP_ACK_PACKAGE_H_

#include "AString.h"
#include "ImsList.h"

#include "ISipAckPackage.h"
#include "msg/SipMessage.h"
#include "txn/SipTxnKey.h"

class SipAck;
class SipAckPackagePrivate;
class SipClientTransactionState;

class SipAckPackage : public ISipAckPackage
{
private:
    explicit SipAckPackage(IN const AString& strCallId);

public:
    virtual ~SipAckPackage();

public:
    void AddAck(IN SipClientTransactionState* pCtState, IN IMS_SINT32 nAliveInterval);
    inline IMS_BOOL IsSamePackage(IN const AString& strCallId) const
    {
        return m_strCallId.Equals(strCallId);
    }
    IMS_BOOL NotifyStray2xx(IN ::SipTxnKey* pTxnKey);

private:
    // ISipObject class
    void Destroy() override;

    // ISipAckPackage class
    void RemoveStrayAcks() override;

public:
    static SipAckPackage* CreateAckPackage(IN const AString& strCallId);
    static IMS_BOOL HandleStray2xx(IN ::SipMessage* pSipMsg);
    static void Init();

private:
    AString m_strCallId;
    ImsList<SipAck*> m_objAcks;

    static SipAckPackagePrivate* s_pAckPackagePrivate;
};

#endif
