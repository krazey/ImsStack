/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceSystemTime.h"
#include "IMSDigest.h"
#include "SIPPrivate.h"
#include "ISIPHeader.h"
#include "SIPDebug.h"
#include "SIPAuHelper.h"

__IMS_TRACE_TAG_SIP__;



class SIPGenericChallenge
    : public ISIPGenericChallenge
{
public:
    SIPGenericChallenge(IN IMS_SINT32 nType_ = ISIPHeader::WWW_AUTHENTICATE);
    SIPGenericChallenge(IN CONST SIPGenericChallenge &objRHS);
    virtual ~SIPGenericChallenge();

public:
    SIPGenericChallenge& operator=(IN CONST SIPGenericChallenge &objRHS);

public:
    // ISIPGenericChallenge class
    inline virtual ISIPGenericChallenge* Clone() const
    {
        return new SIPGenericChallenge(*this);
    }

    inline virtual void Destroy()
    {
        delete this;
    }

    inline virtual const AString& GetAlgorithm() const
    {
        return strAlgorithm;
    }
    inline virtual const AString& GetNonce() const
    {
        return strNonce;
    }
    inline virtual IMS_UINT32 GetNonceCount() const
    {
        return nNonceCount;
    }
    inline virtual const AString& GetQop() const
    {
        return strQop;
    }
    inline virtual const AString& GetRealm() const
    {
        return strRealm;
    }
    inline virtual const AString& GetScheme() const
    {
        return strScheme;
    }
    inline virtual IMS_SINT32 GetType() const
    {
        return nType;
    }
    inline virtual void IncreaseNonceCount()
    {
        nNonceCount++;
    }
    inline virtual void SetNonce(IN CONST AString &strNonce)
    {
        this->strNonce = strNonce;
    }
    inline virtual void SetNonceCount(IN IMS_UINT32 nNonceCount)
    {
        if (this->nNonceCount >= nNonceCount)
            return;

        this->nNonceCount = nNonceCount;
    }

    inline const AString& GetDomain() const
    {
        return strDomain;
    }
    inline const AString& GetOpaque() const
    {
        return strOpaque;
    }
    inline const AString& GetStale() const
    {
        return strStale;
    }

    inline void SetAlgorithm(IN CONST AString &strAlgorithm)
    {
        this->strAlgorithm = strAlgorithm;
    }
    inline void SetRealm(IN CONST AString &strRealm)
    {
        this->strRealm = strRealm;
    }
    inline void SetScheme(IN CONST AString &strScheme)
    {
        this->strScheme = strScheme;
    }
    inline void SetType(IN IMS_SINT32 nType)
    {
        this->nType = nType;
    }
    inline void SetDomain(IN CONST AString &strDomain)
    {
        this->strDomain = strDomain;
    }
    inline void SetOpaque(IN CONST AString &strOpaque)
    {
        this->strOpaque = strOpaque;
    }
    inline void SetQop(IN CONST AString &strQop)
    {
        this->strQop = strQop;
    }
    inline void SetStale(IN CONST AString &strStale)
    {
        this->strStale = strStale;
    }

private:
    // Type of header (WWW-Authenticate/Proxy-Authenticate)
    // in which this authentication challenge was received.
    // Used at the UAC end to decide whether an Authorization / Proxy-Authorization
    // is to be formed.
    IMS_SINT32 nType;

    // The scheme for authentication, eg. Basic/Digest
    AString strScheme;

    // Realm field for authentication
    AString strRealm;

    // Domain field for authentication
    AString strDomain;

    // Opaque value, if any
    AString strOpaque;

    // "auth" or "auth-int" if quality of protection is desired
    AString strQop;

    // Algorithm : MD5 / MD5-sess / AKAv1-MD5
    AString strAlgorithm;

    // Nonce field for authentication
    AString strNonce;

    // Stale value, if any
    AString strStale;

    // Nonce count
    IMS_UINT32 nNonceCount;
};



PUBLIC
SIPGenericChallenge::SIPGenericChallenge(IN IMS_SINT32 nType_ /* = ISIPHeader::WWW_AUTHENTICATE */)
    : nType(nType_)
    , strScheme(AString::ConstNull())
    , strRealm(AString::ConstNull())
    , strDomain(AString::ConstNull())
    , strOpaque(AString::ConstNull())
    , strQop(AString::ConstNull())
    , strAlgorithm(AString::ConstNull())
    , strNonce(AString::ConstNull())
    , strStale(AString::ConstNull())
    , nNonceCount(1)
{
}

PUBLIC
SIPGenericChallenge::SIPGenericChallenge(IN CONST SIPGenericChallenge &objRHS)
    : nType(objRHS.nType)
    , strScheme(objRHS.strScheme)
    , strRealm(objRHS.strRealm)
    , strDomain(objRHS.strDomain)
    , strOpaque(objRHS.strOpaque)
    , strQop(objRHS.strQop)
    , strAlgorithm(objRHS.strAlgorithm)
    , strNonce(objRHS.strNonce)
    , strStale(objRHS.strStale)
    , nNonceCount(objRHS.nNonceCount)
{
}

PUBLIC
SIPGenericChallenge::~SIPGenericChallenge()
{
}

PUBLIC
SIPGenericChallenge& SIPGenericChallenge::operator=(IN CONST SIPGenericChallenge &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nType = objRHS.nType;
        strScheme = objRHS.strScheme;
        strRealm = objRHS.strRealm;
        strDomain = objRHS.strDomain;
        strOpaque = objRHS.strOpaque;
        strQop = objRHS.strQop;
        strAlgorithm = objRHS.strAlgorithm;
        strNonce = objRHS.strNonce;
        strStale = objRHS.strStale;
        nNonceCount = objRHS.nNonceCount;
    }

    return (*this);
}



class SIPGenericResponse
{
public:
    SIPGenericResponse();
    SIPGenericResponse(IN CONST SIPGenericResponse &objRHS);
    ~SIPGenericResponse();

public:
    SIPGenericResponse& operator=(IN CONST SIPGenericResponse &objRHS);

private:
    friend class SIPAuHelperPrivate;

    // Authorization or Proxy-Authorization
    IMS_SINT32 nType;

    // Information from the challenge
    AString strScheme;
    AString strNonce;
    AString strAlgorithm;
    AString strOpaque;

    // Home domain name
    AString strRealm;

    // Username : IMPI (Private User Identity)
    AString strUserName;

    // Password for authentication
    //  AKA MD5
    //    1) The resulting AKA RES parameter is treated as a "password" when
    //      calculating the response directive of RFC 2617.
    //    2) When the AUTS is present, the included "response" parameter is calculated
    //      using an empty password (password of ""), instead of a RES.
    AString strPassword;

    // "auth" or "auth-int" if quality of protection is desired
    AString strQop;

    // Client nonce value if QoP is desired
    AString strCNonce;

    // Nonce count if QoP is desired
    AString strNonceCount;

    // Extensions for AKA authentication
    IMS_AKA stAKAParam;
};



PUBLIC
SIPGenericResponse::SIPGenericResponse()
    : nType(ISIPHeader::AUTHORIZATION)
    , strScheme(AString::ConstNull())
    , strNonce(AString::ConstNull())
    , strAlgorithm(AString::ConstNull())
    , strOpaque(AString::ConstNull())
    , strRealm(AString::ConstNull())
    , strUserName(AString::ConstNull())
    , strPassword(AString::ConstNull())
    , strQop(AString::ConstNull())
    , strCNonce(AString::ConstNull())
    , strNonceCount(AString::ConstNull())
{
}

PUBLIC
SIPGenericResponse::SIPGenericResponse(IN CONST SIPGenericResponse &objRHS)
    : nType(objRHS.nType)
    , strScheme(objRHS.strScheme)
    , strNonce(objRHS.strNonce)
    , strAlgorithm(objRHS.strAlgorithm)
    , strOpaque(objRHS.strOpaque)
    , strRealm(objRHS.strRealm)
    , strUserName(objRHS.strUserName)
    , strPassword(objRHS.strPassword)
    , strQop(objRHS.strQop)
    , strCNonce(objRHS.strCNonce)
    , strNonceCount(objRHS.strNonceCount)
    , stAKAParam(objRHS.stAKAParam)
{
}

PUBLIC
SIPGenericResponse::~SIPGenericResponse()
{
}

PUBLIC
SIPGenericResponse& SIPGenericResponse::operator=(IN CONST SIPGenericResponse &objRHS)
{
    //---------------------------------------------------------------------------------------------

    if (this != &objRHS)
    {
        nType = objRHS.nType;
        strScheme = objRHS.strScheme;
        strNonce = objRHS.strNonce;
        strAlgorithm = objRHS.strAlgorithm;
        strOpaque = objRHS.strOpaque;
        strRealm = objRHS.strRealm;
        strUserName = objRHS.strUserName;
        strPassword = objRHS.strPassword;
        strQop = objRHS.strQop;
        strCNonce = objRHS.strCNonce;
        strNonceCount = objRHS.strNonceCount;
        stAKAParam = objRHS.stAKAParam;
    }

    return (*this);
}



class SIPAuHelperPrivate
{
public:
    SIPAuHelperPrivate();
    ~SIPAuHelperPrivate();

public:
    IMS_BOOL AddChallenge(IN ISIPGenericChallenge *piChallenge);
    IMS_BOOL AddChallenge(IN IMS_SINT32 nType, IN SipHeaderBase *pstHeader);
    IMS_BOOL AddCredential(IN CONST Credential &objCredential);
    IMS_BOOL AddHeader(IN_OUT SipMessage *&pstMessage);
    IMS_BOOL CalculateResponse();
    void Clear();
    IMS_BOOL FormCredentials(IN CONST SIPMethod &objMethod, IN CONST AString &strURI,
            IN CONST AString &strEntityBody, IN_OUT SIPGenericResponse &objResponse,
            OUT SipHeaderBase *&pstHeader);
    ISIPGenericChallenge* GetChallenge(IN IMS_SINT32 nIndex) const;
    IMS_BOOL IsChallengePresent() const;
    IMS_BOOL IsCredentialPresent() const;
    const Credential* LookupCredential(IN CONST AString &strRealm) const;
    void RemoveAllCredentials();

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

    IMSList<Credential> objCredentials;
    IMSList<SIPGenericChallenge*> objChallenges;
    IMSList<SIPGenericResponse*> objResponses;
};



PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_ALGORITHM[] = "algorithm";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_AUTS[] = "auts";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_BASIC[] = "Basic";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_CNONCE[] = "cnonce";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_DIGEST[] = "Digest";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_DOMAIN[] = "domain";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_MD5_SESS[] = "MD5-sess";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_NONCE[] = "nonce";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_NONCECOUNT[] = "nc";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_OPAQUE[] = "opaque";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_QOP[] = "qop";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_QOP_AUTH[] = "auth";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_QOP_AUTH_INT[] = "auth-int";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_REALM[] = "realm";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_RESPONSE[] = "response";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_STALE[] = "stale";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_URI[] = "uri";
PRIVATE GLOBAL
const IMS_CHAR SIPAuHelperPrivate::STR_USERNAME[] = "username";



PUBLIC
SIPAuHelperPrivate::SIPAuHelperPrivate()
{
}

PUBLIC
SIPAuHelperPrivate::~SIPAuHelperPrivate()
{
    objCredentials.Clear();
    Clear();
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::AddChallenge(IN ISIPGenericChallenge *piChallenge)
{
    SIPGenericChallenge *pChallenge = DYNAMIC_CAST(SIPGenericChallenge*, piChallenge);

    //---------------------------------------------------------------------------------------------

    if (pChallenge == IMS_NULL)
    {
        return IMS_FALSE;
    }

    SIPGenericChallenge *pNewChallenge = new SIPGenericChallenge(*pChallenge);

    if (pNewChallenge == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (!objChallenges.Append(pNewChallenge))
    {
        delete pNewChallenge;
        return IMS_FALSE;
    }

    IMS_TRACE_D("AuHelper :: AddChallenge - nonce (%s), nonce_count (%d), algorithm (%s)",
            pChallenge->GetNonce().GetStr(),
            pChallenge->GetNonceCount(),
            pChallenge->GetAlgorithm().GetStr());

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::AddChallenge(IN IMS_SINT32 nType, IN SipHeaderBase *pstHeader)
{
    SIPGenericChallenge *pChallenge = new SIPGenericChallenge(nType);

    //---------------------------------------------------------------------------------------------

    if (pChallenge == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pChallenge->SetScheme(SIPStack::GetChallengeScheme(pstHeader));
    pChallenge->SetRealm(SIPStack::GetParameter(pstHeader, STR_REALM));

    const AString &strRealm = pChallenge->GetRealm();

    if (strRealm.StartsWith(TextParser::CHAR_DQUOT))
    {
        // Unquote the realm information
        pChallenge->SetRealm(strRealm.Mid(1, strRealm.GetLength() - 2));
    }

    pChallenge->SetDomain(SIPStack::GetParameter(pstHeader, STR_DOMAIN));
    pChallenge->SetOpaque(SIPStack::GetParameter(pstHeader, STR_OPAQUE));
    pChallenge->SetQop(SIPStack::GetParameter(pstHeader, STR_QOP));

    const AString &strQop = pChallenge->GetQop();

    if (strQop.StartsWith(TextParser::CHAR_DQUOT))
    {
        // Unquote the qop information
        pChallenge->SetQop(strQop.Mid(1, strQop.GetLength() - 2));
    }

    pChallenge->SetAlgorithm(SIPStack::GetParameter(pstHeader, STR_ALGORITHM));
    pChallenge->SetNonce(SIPStack::GetParameter(pstHeader, STR_NONCE));

    const AString &strNonce = pChallenge->GetNonce();

    if (strNonce.StartsWith(TextParser::CHAR_DQUOT))
    {
        // Unquote the nonce information
        pChallenge->SetNonce(strNonce.Mid(1, strNonce.GetLength() - 2));
    }

    pChallenge->SetStale(SIPStack::GetParameter(pstHeader, STR_STALE));

    // We assume that a default authentication algorithm is an MD5.
    if (pChallenge->GetAlgorithm().GetLength() == 0)
    {
        pChallenge->SetAlgorithm(Credential::STR_MD5);
    }

    if (!objChallenges.Append(pChallenge))
    {
        delete pChallenge;
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::AddCredential(IN CONST Credential &objCredential)
{
    const Credential *pExCredential = LookupCredential(objCredential.GetRealm());

    //---------------------------------------------------------------------------------------------

    if (pExCredential != IMS_NULL)
    {
        IMS_TRACE_D("The credential already exists - Realm (%s), UserName (%s), Password (xxx)",
                SIPDebug::GetCharA1(pExCredential->GetRealm().GetStr(), 4),
                SIPDebug::GetCharA2(pExCredential->GetUsername().GetStr(), 6),
                0);
        return IMS_TRUE;
    }

    if (!objCredentials.Append(objCredential))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::AddHeader(IN_OUT SipMessage *&pstMessage)
{
    // Get the URI from the Request-Line to be used in the uri parameter of the response
    // in case of Digest scheme.
    AString strURI;
    SIPMethod objMethod = SIPStack::GetMethod(pstMessage);
    AString strEntityBody("");
    IMS_BOOL bQopAuthInt = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    SipAddrSpec *pstAddrSpec = SIPStack::GetRequestUri(pstMessage);

    SIPStack::EncodeAddrSpec(pstAddrSpec, IMS_FALSE, strURI);
    SIPStack::FreeAddrSpec(pstAddrSpec);

    // Checks if the integrity of the authentication required
    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        const SIPGenericResponse *pResponse = objResponses.GetAt(i);

        if (pResponse == IMS_NULL)
            continue;

        if (pResponse->strQop.EqualsIgnoreCase(STR_QOP_AUTH_INT))
        {
            bQopAuthInt = IMS_TRUE;
            break;
        }
    }

    // Form an entity-body field for MD5-sess
    if (bQopAuthInt)
    {
        if (!SIPStack::GetEntityBody(pstMessage, strEntityBody))
        {
            return IMS_FALSE;
        }
    }

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        SIPGenericResponse *pResponse = objResponses.GetAt(i);

        if (pResponse == IMS_NULL)
        {
            return IMS_FALSE;
        }

        SipHeaderBase *pstHeader = SIPStack::CreateHeader(pResponse->nType);

        if (pstHeader == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (!FormCredentials(objMethod, strURI, strEntityBody, *pResponse, pstHeader))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        if (!SIPStack::AppendHeader(pstHeader, pstMessage))
        {
            SIPStack::FreeHeaderEx(pstHeader);
            return IMS_FALSE;
        }

        SIPStack::FreeHeaderEx(pstHeader);
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::CalculateResponse()
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objChallenges.GetSize(); ++i)
    {
        const SIPGenericChallenge *pChallenge = objChallenges.GetAt(i);

        if (pChallenge == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pChallenge->GetScheme().EqualsIgnoreCase(STR_BASIC)
                || pChallenge->GetScheme().EqualsIgnoreCase(STR_DIGEST))
        {
            const Credential *pCredential = LookupCredential(pChallenge->GetRealm());

            if (pCredential == IMS_NULL)
                continue;

            SIPGenericResponse *pResponse = new SIPGenericResponse();

            if (pResponse == IMS_NULL)
            {
                continue;
            }

            // Copy the challenge parameters to the response parameters
            if (pChallenge->GetType() == ISIPHeader::WWW_AUTHENTICATE)
                pResponse->nType = ISIPHeader::AUTHORIZATION;
            else
                pResponse->nType = ISIPHeader::PROXY_AUTHORIZATION;

            pResponse->strScheme = pChallenge->GetScheme();
            pResponse->strNonce = pChallenge->GetNonce();
            pResponse->strAlgorithm = pChallenge->GetAlgorithm();
            pResponse->strOpaque = pChallenge->GetOpaque();

            pResponse->strRealm = pCredential->GetRealm();
            pResponse->strUserName = pCredential->GetUsername();

            // If IMS AKA is used to authenticate the user,
            // sets the AKA-related parameters in here.
            pResponse->strPassword = pCredential->GetPassword();

            if (pCredential->GetType() != Credential::TYPE_MD5)
            {
                pResponse->stAKAParam = pCredential->GetAKAResponse();

                // Sets the auts field
                //if (pResponse->stAKAParam.nStatus == IMS_AKA::RESULT_NOK_SQN_SYNC_FAILED)
                //    pResponse->stAKAParam.strAUTS = pResponse->stAKAParam.strAUTS.ToBase64();
            }

            // Extract the string "auth" / "auth-int" from the received qop parameter
            if (pChallenge->GetQop().GetLength() > 0)
            {
                AStringArray objTokens = pChallenge->GetQop().Split(TextParser::CHAR_COMMA);

                for (IMS_SINT32 i = 0; i < objTokens.GetCount(); ++i)
                {
                    const AString &strToken = objTokens.GetElementAt(i);

                    // Digest (QoP) : auth
                    if (strToken.EqualsIgnoreCase(STR_QOP_AUTH))
                    {
                        pResponse->strQop = STR_QOP_AUTH;
                    }
                    // Digest (QoP) : auth-int
                    else if (strToken.EqualsIgnoreCase(STR_QOP_AUTH_INT))
                    {
                        if (pResponse->strQop.GetLength() == 0)
                        {
                            pResponse->strQop = STR_QOP_AUTH_INT;
                        }
                    }
                }

                // Nonce count : '0' prefix needs
                // LHEX format (8 hexadecimal)
                pResponse->strNonceCount.Sprintf("%08x", pChallenge->GetNonceCount());

                // Client nonce value : Encodes the client-nonce by using base64 encoding scheme
                pResponse->strCNonce.SetNumber(IMS_SYS_GetSRandom0(), 16);
                pResponse->strCNonce = pResponse->strCNonce.ToBase64();
            }

            if (!objResponses.Append(pResponse))
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
void SIPAuHelperPrivate::Clear()
{
    for (IMS_UINT32 i = 0; i < objChallenges.GetSize(); ++i)
    {
        SIPGenericChallenge *pChallenge = objChallenges.GetAt(i);

        if (pChallenge != IMS_NULL)
            delete pChallenge;
    }

    objChallenges.Clear();

    for (IMS_UINT32 i = 0; i < objResponses.GetSize(); ++i)
    {
        SIPGenericResponse *pResponse = objResponses.GetAt(i);

        if (pResponse != IMS_NULL)
            delete pResponse;
    }

    objResponses.Clear();
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::FormCredentials(IN CONST SIPMethod &objMethod,
        IN CONST AString &strURI, IN CONST AString &strEntityBody,
        IN_OUT SIPGenericResponse &objResponse, OUT SipHeaderBase *&pstHeader)
{
    //---------------------------------------------------------------------------------------------

    if (!SIPStack::SetChallengeScheme(objResponse.strScheme, pstHeader))
        return IMS_FALSE;

    // Basic
    if (objResponse.strScheme.EqualsIgnoreCase(STR_BASIC))
    {
        // Server has requested Basic Authentication Scheme.
        AString strBasicRES;

        if ((objResponse.strUserName.GetLength() != 0)
                && (objResponse.strPassword.GetLength() != 0))
        {
            strBasicRES += objResponse.strUserName;
            strBasicRES += TextParser::CHAR_COLON;
            strBasicRES += objResponse.strPassword;
        }

        // Encode the "username:password" string using the base64 algorithm
        strBasicRES = strBasicRES.ToBase64();

        if (!SIPStack::SetParameter(pstHeader, strBasicRES, AString::ConstNull()))
            return IMS_FALSE;
    }
    // Digest
    else if (objResponse.strScheme.EqualsIgnoreCase(STR_DIGEST))
    {
        // Add DQUOTE in here
        AString strDigestRES;

        // If the MAC is invalid,
        // the value of "response" parameter in Authorization header field
        // will be set to an empty("") value.
        if (objResponse.stAKAParam.nStatus == IMS_AKA::RESULT_NOK_MAC_INVALID)
        {
            strDigestRES.Append(TextParser::CHAR_DQUOT);
            strDigestRES.Append(TextParser::CHAR_DQUOT);
        }
        else
        {
            // IMS_AKA::AUTH_SUCCESS
            // IMS_AKA::AUTH_SQN_SYNCHRONIZATION_FAILURE

            // Server has requested Digest Authentication Scheme.
            HASHHEX HEntity = { 0, };
            HASHHEX HA1 = { 0, };
            IMS_CHAR acResponse[HASHHEX_SIZE + 1] = { 0, };

            // Calculate H(Entity-Body) required for the response
            if (objResponse.strQop.EqualsIgnoreCase(STR_QOP_AUTH_INT))
            {
                IMSDigestEx_CalculateHEntity(strEntityBody, HEntity);
            }

            // Calculate the H(A1)
            IMSDigestEx_CalculateHA1(objResponse.strAlgorithm, objResponse.strUserName,
                    objResponse.strRealm, objResponse.strPassword,
                    objResponse.strNonce, objResponse.strCNonce, HA1);

            // Calculate the response digest
            IMSDigestEx_CalculateResponse(HA1, objResponse.strNonce,
                    objResponse.strNonceCount, objResponse.strCNonce,
                    objResponse.strQop, objMethod.ToString(),
                    strURI, HEntity, acResponse);

            strDigestRES.Append(TextParser::CHAR_DQUOT);
            strDigestRES.Append(acResponse);
            strDigestRES.Append(TextParser::CHAR_DQUOT);
        }

        // "username" field in the generic challenge
        if (objResponse.strUserName.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.strUserName.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = objResponse.strUserName;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.strUserName;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_USERNAME, strQuotedValue))
                return IMS_FALSE;
        }

        // "realm" field
        if (objResponse.strRealm.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.strRealm.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = objResponse.strRealm;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.strRealm;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_REALM, strQuotedValue))
                return IMS_FALSE;
        }

        // "nonce" field
        if (objResponse.strNonce.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.strNonce.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = objResponse.strNonce;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.strNonce;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_NONCE, strQuotedValue))
                return IMS_FALSE;
        }

        // "uri" field (digest uri)
        if (strURI.GetLength() > 0)
        {
            AString strQuotedValue;

            if (strURI.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = strURI;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += strURI;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_URI, strQuotedValue))
                return IMS_FALSE;
        }

        // "response digest" field
        if (!SIPStack::SetParameter(pstHeader, STR_RESPONSE, strDigestRES))
            return IMS_FALSE;

        // "algorithm" field
        if (objResponse.strAlgorithm.GetLength() > 0)
        {
            if (!SIPStack::SetParameter(pstHeader, STR_ALGORITHM, objResponse.strAlgorithm))
                return IMS_FALSE;
        }

        // "cnonce" field
        if (objResponse.strCNonce.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.strCNonce.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = objResponse.strCNonce;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.strCNonce;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_CNONCE, strQuotedValue))
                return IMS_FALSE;
        }

        // "opaque" field
        if (objResponse.strOpaque.GetLength() > 0)
        {
            AString strQuotedValue;

            if (objResponse.strOpaque.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = objResponse.strOpaque;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.strOpaque;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_OPAQUE, strQuotedValue))
                return IMS_FALSE;
        }

        // "qop" field
        if (objResponse.strQop.GetLength() > 0)
        {
            if (!SIPStack::SetParameter(pstHeader, STR_QOP, objResponse.strQop))
                return IMS_FALSE;
        }

        // "nonce-count" field
        if (objResponse.strNonceCount.GetLength() > 0)
        {
            if (!SIPStack::SetParameter(pstHeader, STR_NONCECOUNT, objResponse.strNonceCount))
                return IMS_FALSE;
        }

        ///// Additional parameters for IMS AKA authentication

        // "auts" field
        if (objResponse.stAKAParam.nStatus == IMS_AKA::RESULT_NOK_SQN_SYNC_FAILED)
        {
            AString strQuotedValue;

            if (objResponse.stAKAParam.strAUTS.GetIndexOf(TextParser::CHAR_DQUOT) == 0)
                strQuotedValue = objResponse.stAKAParam.strAUTS;
            else
            {
                strQuotedValue += TextParser::CHAR_DQUOT;
                strQuotedValue += objResponse.stAKAParam.strAUTS;
                strQuotedValue += TextParser::CHAR_DQUOT;
            }

            if (!SIPStack::SetParameter(pstHeader, STR_AUTS, strQuotedValue))
                return IMS_FALSE;
        }
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
ISIPGenericChallenge* SIPAuHelperPrivate::GetChallenge(IN IMS_SINT32 nIndex) const
{
    //---------------------------------------------------------------------------------------------

    if (objChallenges.IsEmpty())
        return IMS_NULL;

    if ((nIndex < 0) || (nIndex >= static_cast<IMS_SINT32>(objChallenges.GetSize())))
        return IMS_NULL;

    return objChallenges.GetAt(nIndex);
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::IsChallengePresent() const
{
    //---------------------------------------------------------------------------------------------

    return !objChallenges.IsEmpty();
}

PUBLIC
IMS_BOOL SIPAuHelperPrivate::IsCredentialPresent() const
{
    //---------------------------------------------------------------------------------------------

    return !objCredentials.IsEmpty();
}

PUBLIC
const Credential* SIPAuHelperPrivate::LookupCredential(IN CONST AString &strRealm) const
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objCredentials.GetSize(); ++i)
    {
        const Credential &objCredential = objCredentials.GetAt(i);

        if (objCredential.IsSameRealm(strRealm))
            return &objCredential;
    }

    return IMS_NULL;
}

PUBLIC
void SIPAuHelperPrivate::RemoveAllCredentials()
{
    //---------------------------------------------------------------------------------------------

    objCredentials.Clear();
}



PUBLIC
SIPAuHelper::SIPAuHelper()
    : pSAHelper(new SIPAuHelperPrivate())
{
}

PUBLIC
SIPAuHelper::~SIPAuHelper()
{
    if (pSAHelper != IMS_NULL)
        delete pSAHelper;
}

PUBLIC
IMS_BOOL SIPAuHelper::AddChallenge(IN ISIPGenericChallenge *piChallenge)
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->AddChallenge(piChallenge);
}

PUBLIC
IMS_BOOL SIPAuHelper::AddCredential(IN CONST Credential &objCredential)
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->AddCredential(objCredential);
}

PUBLIC
IMS_BOOL SIPAuHelper::FormCredentials(IN_OUT SipMessage *&pstMessage)
{
    //---------------------------------------------------------------------------------------------

    if (!pSAHelper->IsChallengePresent())
    {
        IMS_TRACE_I("No challenges", 0, 0, 0);
        return IMS_TRUE;
    }

    // Removes all Authorization/Proxy-Authorization headers if present.
    IMS_SINT32 nHCount = SIPStack::GetHeaderCount(pstMessage, ISIPHeader::AUTHORIZATION);

    while (nHCount > 0)
    {
        SIPStack::RemoveHeader(ISIPHeader::AUTHORIZATION, pstMessage);
        --nHCount;
    }

    nHCount = SIPStack::GetHeaderCount(pstMessage, ISIPHeader::PROXY_AUTHORIZATION);

    while (nHCount > 0)
    {
        SIPStack::RemoveHeader(ISIPHeader::PROXY_AUTHORIZATION, pstMessage);
        --nHCount;
    }

    if (!pSAHelper->CalculateResponse())
    {
        IMS_TRACE_E(0, "Calculating the digest-response failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (!pSAHelper->AddHeader(pstMessage))
    {
        IMS_TRACE_E(0, "Adding Authorization / Proxy-Authorization header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
ISIPGenericChallenge* SIPAuHelper::GetChallenge(IN IMS_SINT32 nIndex /* = 0 */) const
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->GetChallenge(nIndex);
}

PUBLIC
IMS_BOOL SIPAuHelper::IsChallengePresent() const
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->IsChallengePresent();
}

PUBLIC
IMS_BOOL SIPAuHelper::IsCredentialPresent() const
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->IsCredentialPresent();
}

PUBLIC
void SIPAuHelper::RemoveAllChallenges()
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->Clear();
}

PUBLIC
void SIPAuHelper::RemoveAllCredentials()
{
    //---------------------------------------------------------------------------------------------

    return pSAHelper->RemoveAllCredentials();
}

PUBLIC
IMS_BOOL SIPAuHelper::SetChallenges(IN SipMessage *pstMessage)
{
    IMS_SINT32 nHCount = 0;

    //---------------------------------------------------------------------------------------------

    pSAHelper->Clear();

    // In case of 401 response, the header type to be extracted is a SipHdrTypeWwwAuthenticate.
    nHCount = SIPStack::GetHeaderCount(pstMessage, ISIPHeader::WWW_AUTHENTICATE);

    if (nHCount != 0)
    {
        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase *pstHeader = SIPStack::GetHeader(
                    pstMessage, ISIPHeader::WWW_AUTHENTICATE, i);

            if (SIPStack::IsValidHeader(pstHeader))
            {
                if (!pSAHelper->AddChallenge(ISIPHeader::WWW_AUTHENTICATE, pstHeader))
                {
                    SIPStack::FreeHeaderEx(pstHeader);
                    return IMS_FALSE;
                }
            }

            SIPStack::FreeHeaderEx(pstHeader);
        }
    }

    // In case of 407 response, the header type to be extracted is a Proxy-Authenticate.
    nHCount = SIPStack::GetHeaderCount(pstMessage, ISIPHeader::PROXY_AUTHENTICATE);

    if (nHCount != 0)
    {
        for (IMS_SINT32 i = 0; i < nHCount; ++i)
        {
            SipHeaderBase *pstHeader = SIPStack::GetHeader(
                    pstMessage, ISIPHeader::PROXY_AUTHENTICATE, i);

            if (SIPStack::IsValidHeader(pstHeader))
            {
                if (!pSAHelper->AddChallenge(ISIPHeader::PROXY_AUTHENTICATE, pstHeader))
                {
                    SIPStack::FreeHeaderEx(pstHeader);
                    return IMS_FALSE;
                }
            }

            SIPStack::FreeHeaderEx(pstHeader);
        }
    }

    return IMS_TRUE;
}
