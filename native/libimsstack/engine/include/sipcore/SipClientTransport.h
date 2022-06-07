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
#ifndef SIP_CLIENT_TRANSPORT_H_
#define SIP_CLIENT_TRANSPORT_H_

#include "SipTransport.h"

class SipClientTransport : public SipTransport
{
public:
    explicit SipClientTransport(IN IMS_SINT32 nSlotId);
    virtual ~SipClientTransport();

    SipClientTransport(IN const SipClientTransport&) = delete;
    SipClientTransport& operator=(IN const SipClientTransport&) = delete;

public:
    IMS_BOOL FormViaHeader(
            IN_OUT ::SipMessage*& pSipMsg, IN const SipProfile* pProfile = IMS_NULL) override;
    IMS_BOOL ReserveResource(IN const SipProfile* pProfile = IMS_NULL) override;
    IMS_BOOL UpdateDestinationInfo(IN ::SipMessage* pSipMsg, IN IMS_BOOL bRoutingLr = IMS_TRUE,
            IN SipAddrSpec* pImplicitRoute = IMS_NULL) override;
    IMS_SINT32 ValidateViaHeader(IN ::SipMessage* pSipMsg) override;

    inline void SetExtensionTokenForViaBranch(IN const AString& strToken)
    {
        m_strExtensionTokenForViaBranch = strToken;
    }

protected:
    // ISipSocketListener interface
    void Socket_NotifyError(IN SipSocket* pSocket, IN IMS_SINT32 nErrorCode) override;

private:
    static IMS_BOOL IsSameHostAndPort(IN SipAddrSpec* pAddrSpec1, IN SipAddrSpec* pAddrSpec2);

private:
    SipSocket* m_pServerSocket;
    // Extension for branch parameter in the Via header
    AString m_strExtensionTokenForViaBranch;
};

#endif
