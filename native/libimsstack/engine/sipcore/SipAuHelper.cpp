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
#include "Credential.h"
#include "ImsDigest.h"
#include "ServiceMemory.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"

#include "ISipGenericChallenge.h"
#include "ISipHeader.h"
#include "SipAuHelper.h"
#include "SipDebug.h"
#include "SipPrivate.h"
#include "SipStack.h"

__IMS_TRACE_TAG_SIP_CORE__;

class SipGenericChallenge : public ISipGenericChallenge
{
public:
    explicit SipGenericChallenge(IN IMS_SINT32 nType = ISipHeader::WWW_AUTHENTICATE);
    SipGenericChallenge(IN const SipGenericChallenge& other);
    ~SipGenericChallenge() override;

public:
    SipGenericChallenge& operator=(IN const SipGenericChallenge& other);

public:
    // ISipGenericChallenge class
    inline ISipGenericChallenge* Clone() const override { return new SipGenericChallenge(*this); }
    inline void Destroy() override { delete this; }
    inline const AString& GetAlgorithm() const override { return m_strAlgorithm; }
    inline const AString& GetNonce() const override { return m_strNonce; }
    inline IMS_UINT32 GetNonceCount() const override { return m_nNonceCount; }
    inline const AString& GetQop() const override { return m_strQop; }
    inline const AString& GetRealm() const override { return m_strRealm; }
    inline const AString& GetScheme() const override { return m_strScheme; }
    inline IMS_SINT32 GetType() const override { return m_nType; }
    inline void IncreaseNonceCount() override { m_nNonceCount++; }
    inline void SetNonce(IN const AString& strNonce) override { m_strNonce = strNonce; }

    inline const AString& GetOpaque() const { return m_strOpaque; }
    inline void SetAlgorithm(IN const AString& strAlgorithm) { m_strAlgorithm = strAlgorithm; }
    inline void SetRealm(IN const AString& strRealm) { m_strRealm = strRealm; }
    inline void SetScheme(IN const AString& strScheme) { m_strScheme = strScheme; }
    inline void SetType(IN IMS_SINT32 nType) { m_nType = nType; }
    inline void SetDomain(IN const AString& strDomain) { m_strDomain = strDomain; }
    inline void SetOpaque(IN const AString& strOpaque) { m_strOpaque = strOpaque; }
    inline void SetQop(IN const AString& strQop) { m_strQop = strQop; }
    inline void SetStale(IN const AString& strStale) { m_strStale = strStale; }

private:
    // Type of header (WWW-Authenticate/Proxy-Authenticate)
    // in which this authentication challenge was received.
    // Used at the UAC end to decide whether an Authorization / Proxy-Authorization
    // is to be formed.
    IMS_SINT32 m_nType;
    // The scheme for authentication, eg. Basic/Digest
    AString m_strScheme;
    // Realm field for authentication
    AString m_strRealm;
    // Domain field for authentication
    AString m_strDomain;
    // Opaque value, if any
    AString m_strOpaque;
    // "auth" or "auth-int" if quality of protection is desired
    AString m_strQop;
    // Algorithm : MD5 / MD5-sess / AKAv1-MD5
    AString m_strAlgorithm;
    // Nonce field for authentication
    AString m_strNonce;
    // Stale value, if any
    AString m_strStale;
    // Nonce count
    IMS_UINT32 m_nNonceCount;
};

PUBLIC
SipGenericChallenge::SipGenericChallenge(IN IMS_SINT32 nType /*= ISipHeader::WWW_AUTHENTICATE*/) :
        m_nType(nType),
        m_strScheme(AString::ConstNull()),
        m_strRealm(AString::ConstNull()),
        m_strDomain(AString::ConstNull()),
        m_strOpaque(AString::ConstNull()),
        m_strQop(AString::ConstNull()),
        m_strAlgorithm(AString::ConstNull()),
        m_strNonce(AString::ConstNull()),
        m_strStale(AString::ConstNull()),
        m_nNonceCount(1)
{
}

PUBLIC
SipGenericChallenge::SipGenericChallenge(IN const SipGenericChallenge& other) :
        m_nType(other.m_nType),
        m_strScheme(other.m_strScheme),
        m_strRealm(other.m_strRealm),
        m_strDomain(other.m_strDomain),
        m_strOpaque(other.m_strOpaque),
        m_strQop(other.m_strQop),
        m_strAlgorithm(other.m_strAlgorithm),
        m_strNonce(other.m_strNonce),
        m_strStale(other.m_strStale),
        m_nNonceCount(other.m_nNonceCount)
{
}

PUBLIC
SipGenericChallenge::~SipGenericChallenge() {}

PUBLIC
SipGenericChallenge& SipGenericChallenge::operator=(IN const SipGenericChallenge& other)
{
    if (this != &other)
    {
        m_nType = other.m_nType;
        m_strScheme = other.m_strScheme;
        m_strRealm = other.m_strRealm;
        m_strDomain = other.m_strDomain;
        m_strOpaque = other.m_strOpaque;
        m_strQop = other.m_strQop;
        m_strAlgorithm = other.m_strAlgorithm;
        m_strNonce = other.m_strNonce;
        m_strStale = other.m_strStale;
        m_nNonceCount = other.m_nNonceCount;
    }

    return (*this);
}

class SipGenericResponse
{
public:
    SipGenericResponse();
    SipGenericResponse(IN const SipGenericResponse& other);
    ~SipGenericResponse();

public:
    SipGenericResponse& operator=(IN const SipGenericResponse& other);

private:
    friend class SipAuHelperPrivate;

    // Authorization or Proxy-Authorization
    IMS_SINT32 m_nType;
    // Information from the challenge
    AString m_strScheme;
    AString m_strNonce;
    AString m_strAlgorithm;
    AString m_strOpaque;
    // Home domain name
    AString m_strRealm;
    // Username : IMPI (Private User Identity)
    AString m_strUserName;
    // Password for authentication
    //  AKA MD5
    //    1) The resulting AKA RES parameter is treated as a "password" when
    //      calculating the response directive of RFC 2617.
    //    2) When the AUTS is present, the included "response" parameter is calculated
    //      using an empty password (password of ""), instead of a RES.
    AString m_strPassword;
    // "auth" or "auth-int" if quality of protection is desired
    AString m_strQop;
    // Client nonce value if QoP is desired
    AString m_strCNonce;
    // Nonce count if QoP is desired
    AString m_strNonceCount;
    // Extensions for AKA authentication
    ImsAkaParam m_objAkaParam;
};

PUBLIC
SipGenericResponse::SipGenericResponse() :
        m_nType(ISipHeader::AUTHORIZATION),
        m_strScheme(AString::ConstNull()),
        m_strNonce(AString::ConstNull()),
        m_strAlgorithm(AString::ConstNull()),
        m_strOpaque(AString::ConstNull()),
        m_strRealm(AString::ConstNull()),
        m_strUserName(AString::ConstNull()),
        m_strPassword(AString::ConstNull()),
        m_strQop(AString::ConstNull()),
        m_strCNonce(AString::ConstNull()),
        m_strNonceCount(AString::ConstNull())
{
}

PUBLIC
SipGenericResponse::SipGenericResponse(IN const SipGenericResponse& other) :
        m_nType(other.m_nType),
        m_strScheme(other.m_strScheme),
        m_strNonce(other.m_strNonce),
        m_strAlgorithm(other.m_strAlgorithm),
        m_strOpaque(other.m_strOpaque),
        m_strRealm(other.m_strRealm),
        m_strUserName(other.m_strUserName),
        m_strPassword(other.m_strPassword),
        m_strQop(other.m_strQop),
        m_strCNonce(other.m_strCNonce),
        m_strNonceCount(other.m_strNonceCount),
        m_objAkaParam(other.m_objAkaParam)
{
}

PUBLIC
SipGenericResponse::~SipGenericResponse() {}

PUBLIC
SipGenericResponse& SipGenericResponse::operator=(IN const SipGenericResponse& other)
{
    if (this != &other)
    {
        m_nType = other.m_nType;
        m_strScheme = other.m_strScheme;
        m_strNonce = other.m_strNonce;
        m_strAlgorithm = other.m_strAlgorithm;
        m_strOpaque = other.m_strOpaque;
        m_strRealm = other.m_strRealm;
        m_strUserName = other.m_strUserName;
        m_strPassword = other.m_strPassword;
        m_strQop = other.m_strQop;
        m_strCNonce = other.m_strCNonce;
        m_strNonceCount = other.m_strNonceCount;
        m_objAkaParam = other.m_objAkaParam;
    }

    return (*this);
}

class SipAuHelperPrivate
{
public:
    SipAuHelperPrivate();
    ~SipAuHelperPrivate();

public:
    IMS_BOOL AddChallenge(IN ISipGenericChallenge* piChallenge);
    IMS_BOOL AddChallenge(IN IMS_SINT32 nType, IN SipHeaderBase* pSipHdr);
    IMS_BOOL AddCredential(IN const Credential& objCredential);
    IMS_BOOL AddHeader(IN_OUT ::SipMessage*& pSipMsg);
    IMS_BOOL CalculateResponse();
    void Clear();
    IMS_BOOL FormCredentials(IN const SipMethod& objMethod, IN const AString& strUri,
            IN const AString& strEntityBody, IN const SipGenericResponse& objResponse,
            OUT SipHeaderBase*& pSipHdr);
    ISipGenericChallenge* GetChallenge(IN IMS_SINT32 nIndex) const;
    inline IMS_BOOL IsChallengePresent() const { return !m_objChallenges.IsEmpty(); }
    inline IMS_BOOL IsCredentialPresent() const { return !m_objCredentials.IsEmpty(); }
    const Credential* LookupCredential(IN const AString& strRealm) const;
    inline void RemoveAllCredentials() { m_objCredentials.Clear(); }

private:
    static const IMS_CHAR STR_ALGORITHM[];
    static const IMS_CHAR STR_AUTS[];
    static const IMS_CHAR STR_BASIC[];
    static const IMS_CHAR STR_CNONCE[];
    static const IMS_CHAR STR_DIGEST[];
    static const IMS_CHAR STR_DOMAIN[];
    static const IMS_CHAR STR_MD5_SESS[];
    static const IMS_CHAR STR_NONCE[];
    static const IMS_CHAR STR_NONCECOUNT[];
    static const IMS_CHAR STR_OPAQUE[];
    static const IMS_CHAR STR_QOP[];
    static const IMS_CHAR STR_QOP_AUTH[];
    static const IMS_CHAR STR_QOP_AUTH_INT[];
    static const IMS_CHAR STR_REALM[];
    static const IMS_CHAR STR_RESPONSE[];
    static const IMS_CHAR STR_STALE[];
    static const IMS_CHAR STR_URI[];
    static const IMS_CHAR STR_USERNAME[];

    ImsList<Credential> m_objCredentials;
    ImsList<SipGenericChallenge*> m_objChallenges;
    ImsList<SipGenericResponse*> m_objResponses;
};

PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_ALGORITHM[] = "algorithm";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_AUTS[] = "auts";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_BASIC[] = "Basic";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_CNONCE[] = "cnonce";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_DIGEST[] = "Digest";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_DOMAIN[] = "domain";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_MD5_SESS[] = "MD5-sess";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_NONCE[] = "nonce";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_NONCECOUNT[] = "nc";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_OPAQUE[] = "opaque";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_QOP[] = "qop";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_QOP_AUTH[] = "auth";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_QOP_AUTH_INT[] = "auth-int";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_REALM[] = "realm";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_RESPONSE[] = "response";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_STALE[] = "stale";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_URI[] = "uri";
PRIVATE GLOBAL const IMS_CHAR SipAuHelperPrivate::STR_USERNAME[] = "username";

PUBLIC
SipAuHelperPrivate::SipAuHelperPrivate() {}

PUBLIC
SipAuHelperPrivate::~SipAuHelperPrivate()
{
    m_objCredentials.Clear();
    Clear();
}

PUBLIC
IMS_BOOL SipAuHelperPrivate::AddChallenge(IN ISipGenericChallenge* piChallenge)
{
    SipGenericChallenge* pChallenge = DYNAMIC_CAST(SipGenericChallenge*, piChallenge);

    if (pChallenge == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SipGenericChallenge* pNewChallenge = new SipGenericChallenge(*pChallenge);

    if (pNewChallenge == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!m_objChallenges.Append(pNewChallenge))
    {
        delete pNewChallenge;
        return IMS_FALSE;
    }

    IMS_TRACE_D("AuHelper: AddChallenge - nonce(%s), nonce_count(%d), algorithm(%s)",
            pChallenge->GetNonce().GetStr(), pChallenge->GetNonceCount(),
            pChallenge->GetAlgorithm().GetStr());

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipAuHelperPrivate::AddChallenge(IN IMS_SINT32 nType, IN SipHeaderBase* pSipHdr)
{
    SipGenericChallenge* pChallenge = new SipGenericChallenge(nType);

    if (pChallenge == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pChallenge->SetScheme(SipStack::GetChallengeScheme(pSipHdr));
    pChallenge->SetRealm(SipStack::GetParameter(pSipHdr, STR_REALM));

    const AString& strRealm = pChallenge->GetRealm();

    if (strRealm.StartsWith(TextParser::CHAR_DQUOT))
    {
        // Unquote the realm information
        pChallenge->SetRealm(strRealm.Mid(1, strRealm.GetLength() - 2));
    }

    pChallenge->SetDomain(SipStack::GetParameter(pSipHdr, STR_DOMAIN));
    pChallenge->SetOpaque(SipStack::GetParameter(pSipHdr, STR_OPAQUE));
    pChallenge->SetQop(SipStack::GetParameter(pSipHdr, STR_QOP));

    const AString& strQop = pChallenge->GetQop();

    if (strQop.StartsWith(TextParser::CHAR_DQUOT))
    {
        // Unquote the qop information
        pChallenge->SetQop(strQop.Mid(1, strQop.GetLength() - 2));
    }

    pChallenge->SetAlgorithm(SipStack::GetParameter(pSipHdr, STR_ALGORITHM));
    pChallenge->SetNonce(SipStack::GetParameter(pSipHdr, STR_NONCE));

    const AString& strNonce = pChallenge->GetNonce();

    if (strNonce.StartsWith(TextParser::CHAR_DQUOT))
    {
        // Unquote the nonce information
        pChallenge->SetNonce(strNonce.Mid(1, strNonce.GetLength() - 2));
    }

    pChallenge->SetStale(SipStack::GetParameter(pSipHdr, STR_STALE));

    // We assume that a default authentication algorithm is an MD5.
    if (pChallenge->GetAlgorithm().GetLength() == 0)
    {
        pChallenge->SetAlgorithm(Credential::STR_MD5);
    }

    if (!m_objChallenges.Append(pChallenge))
    {
        delete pChallenge;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipAuHelperPrivate::AddCredential(IN const Credential& objCredential)
{
    const Credential* pCredential = LookupCredential(objCredential.GetRealm());

    if (pCredential != IMS_NULL)
    {
        IMS_TRACE_D("Credential already exists: (%s|***|%s)",
                SipDebug::GetCharA2(pCredential->GetUsername().GetStr(), 6),
                SipDebug::GetCharA1(pCredential->GetRealm().GetStr(), 4), 0);
        return IMS_TRUE;
    }

    if (!m_objCredentials.Append(objCredential))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipAuHelperPrivate::AddHeader(IN_OUT ::SipMessage*& pSipMsg)
{
    // Get the URI from the Request-Line to be used in the uri parameter of the response
    // in case of Digest scheme.
    AString strUri;
    SipMethod objMethod = SipStack::GetMethod(pSipMsg);
    AString strEntityBody("");
    IMS_BOOL bQopAuthInt = IMS_FALSE;
    SipAddrSpec* pAddrSpec = SipStack::GetRequestUri(pSipMsg);

    SipStack::EncodeAddrSpec(pAddrSpec, IMS_FALSE, strUri);
    SipStack::FreeAddrSpec(pAddrSpec);

    // Checks if the integrity of the authentication required
    for (IMS_UINT32 i = 0; i < m_objResponses.GetSize(); ++i)
    {
        const SipGenericResponse* pResponse = m_objResponses.GetAt(i);

        if (pResponse == IMS_NULL)
            continue;

        if (pResponse->m_strQop.EqualsIgnoreCase(STR_QOP_AUTH_INT))
        {
            bQopAuthInt = IMS_TRUE;
            break;
        }
    }

    // Form an entity-body field for MD5-sess
    if (bQopAuthInt)
    {
        if (!SipStack::GetEntityBody(pSipMsg, strEntityBody))
        {
            return IMS_FALSE;
        }
    }

    for (IMS_UINT32 i = 0; i < m_objResponses.GetSize(); ++i)
    {
        const SipGenericResponse* pResponse = m_objResponses.GetAt(i);

        if (pResponse == IMS_NULL)
        {
            return IMS_FALSE;
        }

        SipHeaderBase* pSipHdr = SipStack::CreateHeader(pResponse->m_nType);

        if (pSipHdr == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!FormCredentials(objMethod, strUri, strEntityBody, *pResponse, pSipHdr))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        if (!SipStack::AppendHeader(pSipHdr, pSipMsg))
        {
            SipStack::FreeHeaderEx(pSipHdr);
            return IMS_FALSE;
        }

        SipStack::FreeHeaderEx(pSipHdr);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SipAuHelperPrivate::CalculateResponse()
{
    for (IMS_UINT32 i = 0; i < m_objChallenges.GetSize(); ++i)
    {
        const SipGenericChallenge* pChallenge = m_objChallenges.GetAt(i);

        if (pChallenge == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pChallenge->GetScheme().EqualsIgnoreCase(STR_BASIC) ||
                pChallenge->GetScheme().EqualsIgnoreCase(STR_DIGEST))
        {
            const Credential* pCredential = LookupCredential(pChallenge->GetRealm());

            if (pCredential == IMS_NULL)
            {
                continue;
            }

            SipGenericResponse* pResponse = new SipGenericResponse();

            if (pResponse == IMS_NULL)
            {
                continue;
            }

            // Copy the challenge parameters to the response parameters
            if (pChallenge->GetType() == ISipHeader::WWW_AUTHENTICATE)
            {
                pResponse->m_nType = ISipHeader::AUTHORIZATION;
            }
            else
            {
                pResponse->m_nType = ISipHeader::PROXY_AUTHORIZATION;
            }

            pResponse->m_strScheme = pChallenge->GetScheme();
            pResponse->m_strNonce = pChallenge->GetNonce();
            pResponse->m_strAlgorithm = pChallenge->GetAlgorithm();
            pResponse->m_strOpaque = pChallenge->GetOpaque();
            pResponse->m_strRealm = pCredential->GetRealm();
            pResponse->m_strUserName = pCredential->GetUsername();
            // If IMS AKA is used to authenticate the user,
            // sets the AKA-related parameters in here.
            pResponse->m_strPassword = pCredential->GetPassword();

            if (pCredential->GetType() != Credential::TYPE_MD5)
            {
                pResponse->m_objAkaParam = pCredential->GetAkaResponse();

                // Sets the auts field
                // if (pResponse->m_objAkaParam.nStatus == ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED)
                // pResponse->m_objAkaParam.strAuts = pResponse->m_objAkaParam.strAuts.ToBase64();
            }

            // Extract the string "auth" / "auth-int" from the received qop parameter
            if (pChallenge->GetQop().GetLength() > 0)
            {
                AStringArray objTokens = pChallenge->GetQop().Split(TextParser::CHAR_COMMA);

                for (IMS_SINT32 j = 0; j < objTokens.GetCount(); ++j)
                {
                    const AString& strToken = objTokens.GetElementAt(j);

                    // Digest (QoP) : auth
                    if (strToken.EqualsIgnoreCase(STR_QOP_AUTH))
                    {
                        pResponse->m_strQop = STR_QOP_AUTH;
                    }
                    // Digest (QoP) : auth-int
                    else if (strToken.EqualsIgnoreCase(STR_QOP_AUTH_INT))
                    {
                        if (pResponse->m_strQop.GetLength() == 0)
                        {
                            pResponse->m_strQop = STR_QOP_AUTH_INT;
                        }
                    }
                }

                // Nonce count : '0' prefix needs
                // LHEX format (8 hexadecimal)
                pResponse->m_strNonceCount.Sprintf("%08x", pChallenge->GetNonceCount());

                // Client nonce value : Encodes the client-nonce by using base64 encoding scheme
                pResponse->m_strCNonce.SetNumber(IMS_SYS_GetSRandom0(), 16);
                pResponse->m_strCNonce = pResponse->m_strCNonce.ToBase64();
            }

            if (!m_objResponses.Append(pResponse))
            {
                delete pResponse;
                return IMS_FALSE;
            }
        }
        else
        {
            IMS_TRACE_D("___ AUTHENTICATION SCHEME IS NOT SUPPORTED BY SIP ___", 0, 0, 0);
        }
    }

    return IMS_TRUE;
}

PUBLIC
void SipAuHelperPrivate::Clear()
{
    for (IMS_UINT32 i = 0; i < m_objChallenges.GetSize(); ++i)
    {
        SipGenericChallenge* pChallenge = m_objChallenges.GetAt(i);

        if (pChallenge != IMS_NULL)
        {
            delete pChallenge;
        }
    }

    m_objChallenges.Clear();

    for (IMS_UINT32 i = 0; i < m_objResponses.GetSize(); ++i)
    {
        SipGenericResponse* pResponse = m_objResponses.GetAt(i);

        if (pResponse != IMS_NULL)
        {
            delete pResponse;
        }
    }

    m_objResponses.Clear();
}

PUBLIC
IMS_BOOL SipAuHelperPrivate::FormCredentials(IN const SipMethod& objMethod,
        IN const AString& strUri, IN const AString& strEntityBody,
        IN const SipGenericResponse& objResponse, OUT SipHeaderBase*& pSipHdr)
{
    if (!SipStack::SetChallengeScheme(objResponse.m_strScheme, pSipHdr))
    {
        return IMS_FALSE;
    }

    // Basic
    if (objResponse.m_strScheme.EqualsIgnoreCase(STR_BASIC))
    {
        // Server has requested Basic Authentication Scheme.
        AString strBasicRes;

        if ((objResponse.m_strUserName.GetLength() != 0) &&
                (objResponse.m_strPassword.GetLength() != 0))
        {
            strBasicRes += objResponse.m_strUserName;
            strBasicRes += TextParser::CHAR_COLON;
            strBasicRes += objResponse.m_strPassword;
        }

        // Encode the "username:password" string using the base64 algorithm
        strBasicRes = strBasicRes.ToBase64();

        if (!SipStack::SetParameter(pSipHdr, strBasicRes, AString::ConstNull()))
        {
            return IMS_FALSE;
        }
    }
    // Digest
    else if (objResponse.m_strScheme.EqualsIgnoreCase(STR_DIGEST))
    {
        // Add DQUOTE in here
        AString strDigestRes;

        // If the MAC is invalid,
        // the value of "response" parameter in Authorization header field
        // will be set to an empty("") value.
        if (objResponse.m_objAkaParam.m_nStatus == ImsAkaParam::RESULT_NOK_MAC_INVALID)
        {
            strDigestRes.Append(TextParser::CHAR_DQUOT);
            strDigestRes.Append(TextParser::CHAR_DQUOT);
        }
        else
        {
            // ImsAkaParam::AUTH_SUCCESS
            // ImsAkaParam::AUTH_SQN_SYNCHRONIZATION_FAILURE

            // Server has requested Digest Authentication Scheme.
            HASHHEX hEntity = {
                    0,
            };
            HASHHEX hA1 = {
                    0,
            };
            IMS_CHAR acResponse[HASHHEX_SIZE + 1] = {
                    0,
            };

            // Calculate H(Entity-Body) required for the response
            if (objResponse.m_strQop.EqualsIgnoreCase(STR_QOP_AUTH_INT))
            {
                ImsDigest_CalculateEntity(strEntityBody, hEntity);
            }

            // Calculate the H(A1)
            ImsDigest_CalculateA1(objResponse.m_strAlgorithm, objResponse.m_strUserName,
                    objResponse.m_strRealm, objResponse.m_strPassword, objResponse.m_strNonce,
                    objResponse.m_strCNonce, hA1);

            // Calculate the response digest
            ImsDigest_CalculateResponse(hA1, objResponse.m_strNonce, objResponse.m_strNonceCount,
                    objResponse.m_strCNonce, objResponse.m_strQop, objMethod.ToString(), strUri,
                    hEntity, acResponse);

            strDigestRes.Append(TextParser::CHAR_DQUOT);
            strDigestRes.Append(acResponse);
            strDigestRes.Append(TextParser::CHAR_DQUOT);
        }

        // "username" field in the generic challenge
        if (objResponse.m_strUserName.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.m_strUserName.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = objResponse.m_strUserName;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.m_strUserName;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_USERNAME, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }

        // "realm" field
        if (objResponse.m_strRealm.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.m_strRealm.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = objResponse.m_strRealm;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.m_strRealm;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_REALM, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }

        // "nonce" field
        if (objResponse.m_strNonce.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.m_strNonce.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = objResponse.m_strNonce;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.m_strNonce;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_NONCE, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }

        // "uri" field (digest uri)
        if (strUri.GetLength() > 0)
        {
            AString strQuotedValue;

            if (strUri.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = strUri;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += strUri;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_URI, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }

        // "response digest" field
        if (!SipStack::SetParameter(pSipHdr, STR_RESPONSE, strDigestRes))
        {
            return IMS_FALSE;
        }

        // "algorithm" field
        if (objResponse.m_strAlgorithm.GetLength() > 0)
        {
            if (!SipStack::SetParameter(pSipHdr, STR_ALGORITHM, objResponse.m_strAlgorithm))
            {
                return IMS_FALSE;
            }
        }

        // "cnonce" field
        if (objResponse.m_strCNonce.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.m_strCNonce.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = objResponse.m_strCNonce;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.m_strCNonce;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_CNONCE, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }

        // "opaque" field
        if (objResponse.m_strOpaque.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.m_strOpaque.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = objResponse.m_strOpaque;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.m_strOpaque;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_OPAQUE, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }

        // "qop" field
        if (objResponse.m_strQop.GetLength() > 0)
        {
            if (!SipStack::SetParameter(pSipHdr, STR_QOP, objResponse.m_strQop))
            {
                return IMS_FALSE;
            }
        }

        // "nonce-count" field
        if (objResponse.m_strNonceCount.GetLength() > 0)
        {
            if (!SipStack::SetParameter(pSipHdr, STR_NONCECOUNT, objResponse.m_strNonceCount))
            {
                return IMS_FALSE;
            }
        }

        ///// Additional parameters for IMS AKA authentication

        // "auts" field
        if (objResponse.m_objAkaParam.m_nStatus == ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED)
        {
            AString strQuotedValue;

            if (objResponse.m_objAkaParam.m_strAuts.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
            {
                strQuotedValue = objResponse.m_objAkaParam.m_strAuts;
            }
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.m_objAkaParam.m_strAuts;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SipStack::SetParameter(pSipHdr, STR_AUTS, strQuotedValue))
            {
                return IMS_FALSE;
            }
        }
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
ISipGenericChallenge* SipAuHelperPrivate::GetChallenge(IN IMS_SINT32 nIndex) const
{
    if (m_objChallenges.IsEmpty())
    {
        return IMS_NULL;
    }

    if ((nIndex < 0) || (nIndex >= static_cast<IMS_SINT32>(m_objChallenges.GetSize())))
    {
        return IMS_NULL;
    }

    return m_objChallenges.GetAt(nIndex);
}

PUBLIC
const Credential* SipAuHelperPrivate::LookupCredential(IN const AString& strRealm) const
{
    for (IMS_UINT32 i = 0; i < m_objCredentials.GetSize(); ++i)
    {
        const Credential& objCredential = m_objCredentials.GetAt(i);

        if (objCredential.IsSameRealm(strRealm))
        {
            return &objCredential;
        }
    }

    return IMS_NULL;
}

PUBLIC
SipAuHelper::SipAuHelper() :
        m_pAuHelperPrivate(new SipAuHelperPrivate())
{
}

PUBLIC
SipAuHelper::~SipAuHelper()
{
    if (m_pAuHelperPrivate != IMS_NULL)
    {
        delete m_pAuHelperPrivate;
    }
}

PUBLIC
IMS_BOOL SipAuHelper::AddChallenge(IN ISipGenericChallenge* piChallenge)
{
    return m_pAuHelperPrivate->AddChallenge(piChallenge);
}

PUBLIC
IMS_BOOL SipAuHelper::AddCredential(IN const Credential& objCredential)
{
    return m_pAuHelperPrivate->AddCredential(objCredential);
}

PUBLIC
IMS_BOOL SipAuHelper::FormCredentials(IN_OUT ::SipMessage*& pSipMsg)
{
    if (!m_pAuHelperPrivate->IsChallengePresent())
    {
        IMS_TRACE_I("No challenges", 0, 0, 0);
        return IMS_TRUE;
    }

    // Removes all Authorization/Proxy-Authorization headers if present.
    IMS_SINT32 nHCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::AUTHORIZATION);

    while (nHCount > 0)
    {
        SipStack::RemoveHeader(ISipHeader::AUTHORIZATION, pSipMsg);
        --nHCount;
    }

    nHCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::PROXY_AUTHORIZATION);

    while (nHCount > 0)
    {
        SipStack::RemoveHeader(ISipHeader::PROXY_AUTHORIZATION, pSipMsg);
        --nHCount;
    }

    if (!m_pAuHelperPrivate->CalculateResponse())
    {
        IMS_TRACE_E(0, "Calculating the digest-response failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!m_pAuHelperPrivate->AddHeader(pSipMsg))
    {
        IMS_TRACE_E(0, "Adding Authorization / Proxy-Authorization header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
ISipGenericChallenge* SipAuHelper::GetChallenge(IN IMS_SINT32 nIndex /* = 0 */) const
{
    return m_pAuHelperPrivate->GetChallenge(nIndex);
}

PUBLIC
IMS_BOOL SipAuHelper::IsChallengePresent() const
{
    return m_pAuHelperPrivate->IsChallengePresent();
}

PUBLIC
IMS_BOOL SipAuHelper::IsCredentialPresent() const
{
    return m_pAuHelperPrivate->IsCredentialPresent();
}

PUBLIC
void SipAuHelper::RemoveAllChallenges()
{
    return m_pAuHelperPrivate->Clear();
}

PUBLIC
void SipAuHelper::RemoveAllCredentials()
{
    return m_pAuHelperPrivate->RemoveAllCredentials();
}

PUBLIC
IMS_BOOL SipAuHelper::SetChallenges(IN ::SipMessage* pSipMsg)
{
    IMS_SINT32 nHCount = 0;

    m_pAuHelperPrivate->Clear();

    // In case of 401 response, the header type to be extracted is a WWW-Authenticate.
    nHCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::WWW_AUTHENTICATE);

    if (nHCount != 0)
    {
        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase* pSipHdr = SipStack::GetHeader(pSipMsg, ISipHeader::WWW_AUTHENTICATE, i);

            if (SipStack::IsValidHeader(pSipHdr))
            {
                if (!m_pAuHelperPrivate->AddChallenge(ISipHeader::WWW_AUTHENTICATE, pSipHdr))
                {
                    SipStack::FreeHeaderEx(pSipHdr);
                    return IMS_FALSE;
                }
            }

            SipStack::FreeHeaderEx(pSipHdr);
        }
    }

    // In case of 407 response, the header type to be extracted is a Proxy-Authenticate.
    nHCount = SipStack::GetHeaderCount(pSipMsg, ISipHeader::PROXY_AUTHENTICATE);

    if (nHCount != 0)
    {
        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase* pSipHdr =
                    SipStack::GetHeader(pSipMsg, ISipHeader::PROXY_AUTHENTICATE, i);

            if (SipStack::IsValidHeader(pSipHdr))
            {
                if (!m_pAuHelperPrivate->AddChallenge(ISipHeader::PROXY_AUTHENTICATE, pSipHdr))
                {
                    SipStack::FreeHeaderEx(pSipHdr);
                    return IMS_FALSE;
                }
            }

            SipStack::FreeHeaderEx(pSipHdr);
        }
    }

    return IMS_TRUE;
}
