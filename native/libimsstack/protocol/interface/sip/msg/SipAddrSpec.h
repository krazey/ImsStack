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
#ifndef __SIP_ADDR_SPEC_H__
#define __SIP_ADDR_SPEC_H__

#include "AStringBuffer.h"
#include "SipDatatypes.h"
#include "SipRefBase.h"
#include "msg/IParameterComponent.h"
#include "msg/SipParameters.h"

class SipUri : public SipRefBase, public IParameterComponent
{
public:
    /*Enumeration for URI Scheme*/
    enum UriType
    {
        SCHEME_INVALID = SIP_INVALID,
        SCHEME_SIP,
        SCHEME_SIPS,
        SCHEME_ABS
    };

private:
    /*User Info contains username and pwd*/
    SIP_CHAR* m_pszUser;
    SIP_CHAR* m_pszPassword;
    /*Host Port contains host(can be domain name or IP) and port*/
    SIP_CHAR* m_pszHost;
    SIP_UINT16 m_nPort;
    SIP_INT32 m_eHostType;
    SipParameters* m_pUriParams;
    // for storing each header in
    // "?"   header   *( "&"   header )
    // each node consists of a SipNameValue obj for one header
    SipParameters* m_pUriHdrParams;

    SIP_BOOL DecodeUserInfo(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    SIP_BOOL DecodeHostPort(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    ~SipUri();

public:
    /*constructor*/
    SipUri();
    SipUri(const SipUri& objSipUri);

    SIP_BOOL IsValidComponent(const SIP_CHAR* pszComponent) const override;

    SIP_BOOL DecodeSipUri(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);

    SIP_VOID SetUser(const SIP_CHAR* pszUser);

    SIP_VOID SetPassword(const SIP_CHAR* pszPass);

    /*Get methods*/
    inline const SIP_CHAR* GetUser() const { return m_pszUser; }

    inline const SIP_CHAR* GetPassword() const { return m_pszPassword; }

    inline const SIP_CHAR* GetHost() const { return m_pszHost; }

    inline SIP_UINT16 GetPort() const { return m_nPort; }

    inline SipNameValue* GetUriParam(SIP_UINT32 nPos)
    {
        return (m_pUriParams != SIP_NULL) ? m_pUriParams->GetParam(nPos) : SIP_NULL;
    }

    inline SIP_CHAR* GetUriParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO) const
    {
        return m_pUriParams->GetParamValue(pszName, nPos);
    }

    inline SIP_UINT32 GetUriParamCount() const
    {
        return (m_pUriParams != SIP_NULL) ? m_pUriParams->GetParamCount() : SIP_ZERO;
    }
    inline SipNameValue* GetHdrParam(SIP_UINT32 nPos) const
    {
        return (m_pUriHdrParams != SIP_NULL) ? m_pUriHdrParams->GetParam(nPos) : SIP_NULL;
    }

    inline SIP_UINT32 GetHdrParamCount() const
    {
        return (m_pUriHdrParams != SIP_NULL) ? m_pUriHdrParams->GetParamCount() : SIP_ZERO;
    }

    inline SIP_VOID RemoveHdrParam(const SIP_CHAR* pszName)
    {
        if (m_pUriHdrParams != SIP_NULL)
        {
            m_pUriHdrParams->RemoveParam(pszName);
        }
    }

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const;
    SIP_BOOL EncodeSipUri(SIP_CHAR** ppCurrPos);
    SIP_BOOL DecodeSipUri(SIP_CHAR** ppCurrPos);

    static const SIP_CHAR* GetSchemeString(UriType eUriType);
};

class SipAddrSpec : public SipRefBase
{
public:
    /*Enumeration for host type*/
    enum
    {
        HOST_NAME,
        HOST_IPV4,
        HOST_IPV6,
        HOST_INVALID = SIP_INVALID
    };

private:
    SipUri::UriType m_eUriType;

protected:
    SipUri* m_pSipUri;
    SIP_CHAR* m_pszAbsUri;

public:
    SipAddrSpec();

    SipAddrSpec(const SipAddrSpec& objAddressSpec);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const;

    /*Function for encoding*/
    SIP_BOOL EncodeAddrSpec(SIP_CHAR** ppCurrPos) const;

    /*Function for decoding*/
    SIP_BOOL DecodeAddrSpec(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen);
    /*function for getting the header type*/
    inline SipUri::UriType GetUriScheme() const { return m_eUriType; }

    SipUri* GetSipUri();

    SIP_VOID SetAbsUri(const SIP_CHAR* pszSipUri);

    inline const SIP_CHAR* GetAbsUri() const { return m_pszAbsUri; }

    /*get methods*/

    /* Get methods as reference */
    inline const SipUri* GetSipUriAsRef() const { return m_pSipUri; }

private:
    ~SipAddrSpec();
};

class SipNameAddr : public SipRefBase
{
public:
    SIP_CHAR* m_pszDispName;
    SipAddrSpec* m_pAddrSpec;

public:
    SipNameAddr();
    SipNameAddr(const SipNameAddr& objNameAddr);

    SIP_BOOL SetAddrSpec(SipAddrSpec* pSipAddrSpec);

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_BOOL bParams) const;
    SIP_BOOL EncodeNameAddr(SIP_CHAR** ppCurrPos);

    /*Function for decoding*/
    SIP_BOOL DecodeNameAddr(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt);

    SipAddrSpec* GetAddrSpec();

    SIP_VOID SetDisplayName(const SIP_CHAR* pszDisplayName);

    inline const SIP_CHAR* GetDisplayName() const { return m_pszDispName; }

private:
    virtual ~SipNameAddr();
};

#endif  //__SIP_ADDR_SPEC_H__
