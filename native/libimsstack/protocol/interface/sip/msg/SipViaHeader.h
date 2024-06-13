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
#ifndef __SIP_VIA_HEADER_H__
#define __SIP_VIA_HEADER_H__

#include "msg/SipHeaderBase.h"

class SipViaHeader : public SipHeaderBase
{
private:
    /*Sent Protocol*/
    /*Protocol Name*/
    SIP_CHAR* m_pszProtocolName;
    /*Protocol Version*/
    SIP_CHAR* m_pszProtocolVer;
    /*Transport*/
    SIP_CHAR* m_pszTransport;

    /*Sent By*/
    /*Host*/
    SIP_CHAR* m_pszHost;
    /*Port*/
    SIP_UINT16 m_nPort;

    SIP_INT32 m_eHostType;

    SIP_BOOL DecHostPort(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt);

public:
    /*constructor*/
    SipViaHeader();
    SipViaHeader(const SipViaHeader& objHeader);

    static SipHeaderBase* GetNewObj(SIP_INT32 eHeaderType, SipHeaderBase* pHeader);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const override;
    SIP_BOOL EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL bParams = SIP_TRUE) override;

    SIP_BOOL DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen) override;

    /*set methods*/
    SIP_BOOL SetProtocolName(const SIP_CHAR* pszProtocolNm);

    SIP_BOOL SetProtocolVer(const SIP_CHAR* pszProtocolVer);

    SIP_BOOL SetTransport(const SIP_CHAR* pszTransport);

    SIP_BOOL SetHost(const SIP_CHAR* pszHost);
    SIP_BOOL SetPortNum(SIP_UINT16 nPort);

    /*Get methods*/

    inline const SIP_CHAR* GetProtocolName() const { return m_pszProtocolName; }

    inline const SIP_CHAR* GetProtocolVer() const { return m_pszProtocolVer; }

    inline const SIP_CHAR* GetTransport() const { return m_pszTransport; }

    inline const SIP_CHAR* GetHost() const { return m_pszHost; }

    inline SIP_UINT16 GetPort() const { return m_nPort; }

    const SIP_CHAR* GetBranch() const;

    SIP_BOOL SetBranchParam(const SIP_CHAR* pszBranch);
    SIP_BOOL IsValidHeader() const override;

private:
    ~SipViaHeader();
};

#endif  //__SIP_VIA_HEADER_H__
