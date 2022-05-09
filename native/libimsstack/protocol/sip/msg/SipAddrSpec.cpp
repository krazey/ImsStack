#include "IPAddress.h"
#include "msg/SipAddrSpec.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_error.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "msg/SipHeaderBase.h"
#include "msg/SipMessage.h"
#include "msg/sip_msgutil.h"

#define SIP_SIP_ENC  "sip:"
#define SIP_SIPS_ENC "sips:"

/******************************************************************************
 * Function name      : SipUri::SipUri
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUri::SipUri() :
        m_pszUser(SIP_NULL),
        m_pszPassword(SIP_NULL),
        m_pszHost(SIP_NULL),
        m_nPort(SIP_ZERO),
        m_eHostType(SipAddrSpec::HOST_NAME),
        m_pUriParamList(SIP_NULL),
        m_pUriHdrParamList(SIP_NULL),
        m_pParameterComponent(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipUri::SipUri
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUri::SipUri(const SipUri& objSipUri) :
        m_pszUser(SipPf_Strdup(objSipUri.m_pszUser)),
        m_pszPassword(SipPf_Strdup(objSipUri.m_pszPassword)),
        m_pszHost(SipPf_Strdup(objSipUri.m_pszHost)),
        m_nPort(objSipUri.m_nPort),
        m_eHostType(objSipUri.m_eHostType),
        m_pUriParamList(SIP_NULL),
        m_pUriHdrParamList(SIP_NULL),
        m_pParameterComponent(objSipUri.m_pParameterComponent)
{
    if (objSipUri.m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList = new SipParameterList(*(objSipUri.m_pUriParamList));
    }

    if (objSipUri.m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList = new SipParameterList(*(objSipUri.m_pUriHdrParamList));
    }
}

/******************************************************************************
 * Function name      : SipUri::~SipUri
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipUri::~SipUri()
{
    if (m_pszUser != SIP_NULL)
    {
        delete[] m_pszUser;
    }
    if (m_pszPassword != SIP_NULL)
    {
        delete[] m_pszPassword;
    }
    /*Host Port contains host(can be domain name or IP) and port*/
    if (m_pszHost != SIP_NULL)
    {
        delete[] m_pszHost;
    }
    /*List of obj of CSipNameValue*/
    if (m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList->SipDelete();
    }
    /*List of obj of CSipNameValue*/
    if (m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList->SipDelete();
    }
}

/******************************************************************************
 * Function name      : SipUri::SetUser
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::SetUser(const SIP_CHAR* pszUser)
{
    return SetCharVar(pszUser, m_pszUser);
}

/******************************************************************************
 * Function name      : SipUri::SetPassword
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::SetPassword(const SIP_CHAR* pszPass)
{
    return SetCharVar(pszPass, m_pszPassword);
}

/******************************************************************************
 * Function name      : SipUri::SetHost
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::SetHost(const SIP_CHAR* pszHost)
{
    return SetCharVar(pszHost, m_pszHost);
}

/******************************************************************************
 * Function name      : SipUri::SetParameterComponent
 *
 * Description    :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_VOID SipUri::SetParameterComponent(IParameterComponent* pParameterComponent)
{
    m_pParameterComponent = pParameterComponent;
}

/******************************************************************************
 * Function name      : SipUri::AddUriParam
 *
 * Description       Add only name
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::AddUriParam(const SIP_CHAR* pszName)
{
    if (m_pUriParamList == SIP_NULL)
    {
        m_pUriParamList = new SipParameterList();
    }
    if (m_pUriParamList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "AddUriParam: Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return m_pUriParamList->Add(pszName);
}

/******************************************************************************
 * Function name      : SipUri::AddUriParam
 *
 * Description       Add name, val
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::AddUriParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    if (m_pUriParamList == SIP_NULL)
    {
        m_pUriParamList = new SipParameterList();
    }
    if (m_pUriParamList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "AddUriParam: Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return m_pUriParamList->Add(pszName, pszValue);
}

/******************************************************************************
 * Function name      : SipUri::AddUriParam
 *
 * Description       Add name, val, param type
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::AddUriParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_INT32 ePrmType)
{
    if (m_pUriParamList == SIP_NULL)
    {
        m_pUriParamList = new SipParameterList();
    }
    if (m_pUriParamList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "AddUriParam: Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return m_pUriParamList->Add(pszName, pszValue, ePrmType);
}
/******************************************************************************
 * Function name      : SipUri::AddHeader
 *
 * Description       Add a hdr to uri
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::AddHeader(const SIP_CHAR* pszName)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "AddHeader: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pUriHdrParamList == SIP_NULL)
    {
        m_pUriHdrParamList = new SipParameterList();
    }
    if (m_pUriHdrParamList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "AddHeader: Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pUriHdrParamList->Add(pszName);
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUri::AddHeader
 *
 * Description       Add a hdr to uri
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::AddHeader(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    if (m_pUriHdrParamList == SIP_NULL)
    {
        m_pUriHdrParamList = new SipParameterList();
    }
    if (m_pUriHdrParamList == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pUriHdrParamList->Add(pszName, pszValue);

    return SIP_TRUE;
}
/******************************************************************************
 * Function name      : SipUri::GetUriType
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipParameterList* SipUri::GetUriParamList()
{
    if (m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList->increment();
    }
    return m_pUriParamList;
}

/******************************************************************************
 * Function name      : SipUri::GetHdrParamList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipParameterList* SipUri::GetHdrParamList()
{
    if (m_pUriHdrParamList != SIP_NULL)
    {
        m_pUriHdrParamList->increment();
    }
    return m_pUriHdrParamList;
}

/******************************************************************************
 * Function name      : SipUri::RemoveUriParam
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::RemoveUriParam(const SIP_CHAR* pszName)
{
    if (m_pUriParamList != SIP_NULL)
    {
        return m_pUriParamList->Remove(pszName);
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipUri::RemoveHdrParam
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::RemoveHdrParam(const SIP_CHAR* pszName)
{
    if (m_pUriHdrParamList != SIP_NULL)
    {
        return m_pUriHdrParamList->Remove(pszName);
    }
    return SIP_FALSE;
}

/********************************************************************************
  Encode a sip uri
 ********************************************************************************/
#define MAX_PORT_LEN 20
SIP_BOOL SipUri::EncodeSipUri(SIP_CHAR** ppCurrPos)
{
    /* encoding of user info
       userinfo = ( user / telephone-subscriber ) [ ":" password ] "@"  */
    if (m_pszUser != SIP_NULL)
    {
        /*Do Percent Encoding if Required*/
        if ((m_pParameterComponent != SIP_NULL) &&
                m_pParameterComponent->IsValidComponent(SIP_USER))
        {
            SIP_CHAR* pszTempUser =
                    SipPercentEncoding::DoPerEnc_UserAndHeader(m_pszUser, (SIP_CHAR*)SIP_USER);
            SipPf_Strcpy(*ppCurrPos, pszTempUser);
            delete[] pszTempUser;
        }
        else
        {
            SipPf_Strcpy(*ppCurrPos, m_pszUser);
        }

        SipEnc_UpdateCurrPos(ppCurrPos);

        if (m_pszPassword != SIP_NULL)
        {
            /*encode the password*/
            SIP_ENC_COLON(*ppCurrPos);

            /*Do Percent Encoding if Required*/
            if ((m_pParameterComponent != SIP_NULL) &&
                    m_pParameterComponent->IsValidComponent(SIP_PASSWORD))
            {
                SIP_CHAR* pszTempPassword = SipPercentEncoding::DoPerEnc_Password(m_pszPassword);
                SipPf_Strcpy(*ppCurrPos, pszTempPassword);
                delete[] pszTempPassword;
            }
            else
            {
                SipPf_Strcpy(*ppCurrPos, m_pszPassword);
            }

            SipEnc_UpdateCurrPos(ppCurrPos);
        }

        SIP_ENC_ATTHERATE(*ppCurrPos);
    }

    /* encoding of host port
       hostport = host [ ":" port ] */
    if (m_pszHost != SIP_NULL)
    {
        /*Check for the IPV6 and IPV4 */
        if (m_eHostType == SipAddrSpec::HOST_IPV6)
        {
            **ppCurrPos = LEFT_SQUARE;
            (*ppCurrPos)++;

            SipPf_Strcpy(*ppCurrPos, m_pszHost);
            SipEnc_UpdateCurrPos(ppCurrPos);

            **ppCurrPos = RIGHT_SQUARE;
            (*ppCurrPos)++;
        }
        /*Do Percent Encoding if Required*/
        else if ((m_pParameterComponent != SIP_NULL) &&
                (m_pParameterComponent->IsValidComponent(SIP_HOST)))
        {
            SIP_CHAR* pszTempHost = SipPercentEncoding::DoPerEnc_Host(m_pszHost);
            SipPf_Strcpy(*ppCurrPos, pszTempHost);
            SipEnc_UpdateCurrPos(ppCurrPos);
            delete[] pszTempHost;
        }
        else
        {
            SipPf_Strcpy(*ppCurrPos, m_pszHost);
            SipEnc_UpdateCurrPos(ppCurrPos);
        }

        /*Encoding of Port*/
        if ((m_nPort != SIP_ZERO) && (m_nPort != SIP_UNSPECIFIED_PORT))
        {
            // TODO Percent Encoding
            SIP_CHAR szTmp[MAX_PORT_LEN];
            memset(szTmp, 0x0, sizeof(szTmp));
            SipPf_Sprintf(szTmp, (SIP_CHAR*)"%u", m_nPort);

            SIP_ENC_COLON(*ppCurrPos);

            SipPf_Strcpy(*ppCurrPos, szTmp);
            SipEnc_UpdateCurrPos(ppCurrPos);
        }
    }
    else
    {
        SIP_DEBUG_WARNING(
                ESIPTRACE_MODENCODER, "EncodeSipUri: Host value Missing", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*encoding of URI params*/
    if (m_pUriParamList != SIP_NULL)
    {
        m_pUriParamList->EncodeUriParamList(ppCurrPos, SIP_SEMI, m_pParameterComponent);
    }

    /*encoding of Hdr params*/
    // "?"   header   *( "&"   header )
    if (m_pUriHdrParamList != SIP_NULL)
    {
        SipVector<SipNameValue*>& sipList = m_pUriHdrParamList->GetList();

        if (sipList.IsEmpty() == SIP_TRUE)
        {
            // ADD LOG
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeSipUri: Empty list", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /*Put a semicolon*/
        SIP_ENC_QMARK(*ppCurrPos);

        SIP_UINT32 nSize = sipList.GetSize();

        SIP_TRACE_NORMAL(ESIPTRACE_MODENCODER, "List Size %d", nSize, SIP_ZERO);

        for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
        {
            SipNameValue* pParamNamValue = sipList.GetAt(nCount);

            if (pParamNamValue != SIP_NULL)
            {
                if (nCount != SIP_ZERO)
                {
                    SIP_ENC_AMPERSAND(*ppCurrPos);
                }

                pParamNamValue->EncodeFromUriHdrList(ppCurrPos, m_pParameterComponent);
            }
        }
    }

    return SIP_TRUE;
}

/*SipAddrSpec class implementation*/
/******************************************************************************
 * Function name      : SipAddrSpec::SipAddrSpec
 *
 * Description       :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipAddrSpec::SipAddrSpec() :
        m_eUriType(SipUri::SCHEME_SIP),
        m_pSipUri(SIP_NULL),
        m_pszAbsUri(SIP_NULL),
        m_pParameterComponent(SIP_NULL)
{
}

/******************************************************************************
 * Function name      : SipAddrSpec::SipAddrSpec
 *
 * Description       :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipAddrSpec::SipAddrSpec(const SipAddrSpec& objAddressSpec) :
        m_eUriType(objAddressSpec.m_eUriType),
        m_pSipUri(SIP_NULL),
        m_pszAbsUri(SipPf_Strdup(objAddressSpec.m_pszAbsUri)),
        m_pParameterComponent(objAddressSpec.m_pParameterComponent)
{
    if (objAddressSpec.m_pSipUri != SIP_NULL)
    {
        m_pSipUri = new SipUri(*(objAddressSpec.m_pSipUri));
    }
}

/******************************************************************************
 * Function name      : SipAddrSpec::~SipAddrSpec
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipAddrSpec::~SipAddrSpec()
{
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->SipDelete();
    }
    if (m_pszAbsUri != SIP_NULL)
    {
        delete[] m_pszAbsUri;
    }
}

/******************************************************************************
 * Function name      : SipAddrSpec::SetAbsUri
 *
 * Description       :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAddrSpec::SetAbsUri(const SIP_CHAR* pszSipUri)
{
    return SetCharVar(pszSipUri, m_pszAbsUri);
}

/******************************************************************************
 * Function name      : SipAddrSpec::EncodeAddrSpec
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAddrSpec::EncodeAddrSpec(SIP_CHAR** ppCurrPos) const
{
    if (m_pSipUri != SIP_NULL)
    {
        // implement this in cpp
        /*encoding of uri name*/
        if (m_eUriType == SipUri::SCHEME_SIP)
        {
            SipPf_Strcpy(*ppCurrPos, SIP_SIP_ENC);
        }
        else if (m_eUriType == SipUri::SCHEME_SIPS)
        {
            SipPf_Strcpy(*ppCurrPos, SIP_SIPS_ENC);
        }

        SipEnc_UpdateCurrPos(ppCurrPos);

        m_pSipUri->SetParameterComponent(m_pParameterComponent);

        if (m_pSipUri->EncodeSipUri(ppCurrPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Uri Encoding error", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (m_pszAbsUri != SIP_NULL)
    {
        SipPf_Strcpy(*ppCurrPos, m_pszAbsUri);
        SipEnc_UpdateCurrPos(ppCurrPos);
    }
    else
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No Uri set for encoding", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

/*SipNameAddr Class implementation*/
SipNameAddr::SipNameAddr() :
        m_pszDispName(SIP_NULL),
        m_pAddrSpec(SIP_NULL),
        m_pParameterComponent(SIP_NULL)
{
}

SipNameAddr::SipNameAddr(const SipNameAddr& objNameAddr) :
        m_pszDispName(SipPf_Strdup(objNameAddr.m_pszDispName)),
        m_pAddrSpec(SIP_NULL),
        m_pParameterComponent(objNameAddr.m_pParameterComponent)
{
    if (objNameAddr.m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec = new SipAddrSpec(*(objNameAddr.m_pAddrSpec));
    }
}

SipNameAddr::~SipNameAddr()
{
    if (m_pszDispName != SIP_NULL)
    {
        delete[] m_pszDispName;
    }
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->SipDelete();
    }
}

SipAddrSpec* SipNameAddr::GetAddrSpec()
{
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->increment();
    }
    return m_pAddrSpec;
}
SIP_BOOL SipNameAddr::SetAddrSpec(SipAddrSpec* pSipAddrSpec)
{
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->SipDelete();
    }
    m_pAddrSpec = pSipAddrSpec;
    return SIP_TRUE;
}

/*set methods*/
SIP_BOOL SipAddrSpec::SetSipUri(SipUri* pSipUri, SIP_INT32 eUriType)
{
    (void)eUriType;

    if (m_pSipUri == SIP_NULL)
    {
        m_pSipUri = pSipUri;
        m_pSipUri->increment();

        return SIP_TRUE;
    }
    return SIP_FALSE;
}

SIP_VOID SipAddrSpec::SetParameterComponent(IParameterComponent* pParameterComponent)
{
    m_pParameterComponent = pParameterComponent;
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->SetParameterComponent(pParameterComponent);
    }
}

SipUri* SipAddrSpec::GetSipUri()
{
    if (m_pSipUri != SIP_NULL)
    {
        m_pSipUri->increment();
    }
    return m_pSipUri;
}
/******************************************************************************
 * Function name      : SipNameAddr::SetName
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameAddr::SetDisplayName(const SIP_CHAR* pszDisplayName)
{
    return SetCharVar(pszDisplayName, m_pszDispName);
}

SIP_VOID SipNameAddr::SetParameterComponent(IParameterComponent* pParameterComponent)
{
    m_pParameterComponent = pParameterComponent;
    if (m_pAddrSpec != SIP_NULL)
    {
        m_pAddrSpec->SetParameterComponent(pParameterComponent);
    }
}

/******************************************************************************
 * Function name      : SipNameAddr::GetName
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameAddr::EncodeNameAddr(SIP_CHAR** ppCurrPos)
{
    if (m_pAddrSpec == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "No Addr Spec", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_pszDispName != SIP_NULL)
    {
        SipPf_Strcpy(*ppCurrPos, m_pszDispName);
        SipEnc_UpdateCurrPos(ppCurrPos);

        // FIX_MESSAGE_ENCODING_OPERATION
        //  Add LWS between the display name and left angle quote ('<').
        //  First, check the display name if it has a double quotation.
        //  If present, just do normal procedure. Else, add the display name and space.
        //  But, we will always add the space after the display name.
        SIP_ENC_SP(*ppCurrPos);
    }

    SIP_ENC_LAQUOT(*ppCurrPos);

    m_pAddrSpec->SetParameterComponent(m_pParameterComponent);

    if (m_pAddrSpec->EncodeAddrSpec(ppCurrPos) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Addr Spec failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_ENC_RAQUOT(*ppCurrPos);

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipAddrSpec::DecodeAddrSpec
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAddrSpec::DecodeAddrSpec(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /*Validate the input prm*/
    if ((nDecLen == SIP_ZERO) || (pStartPt == SIP_NULL))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalide Input prm", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPos = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "No URI scheme", SIP_ZERO, SIP_ZERO);
        m_eUriType = SipUri::SCHEME_ABS;
    }
    else
    {
        m_eUriType = sipGetUriType(pStartPt, pTempPos);
    }

    /*CAse of Sip or Sips URI*/
    if ((m_eUriType == SipUri::SCHEME_SIP) || (m_eUriType == SipUri::SCHEME_SIPS))
    {
        /*Set the Start point after COLON*/
        pStartPt = pTempPos + SIP_TWO;
        /*Update the length of buffer*/
        nDecLen = pEndPt - pStartPt + SIP_ONE;

        SipUri* pSipUri = new SipUri();
        if (pSipUri == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipAddrSpec::DecodeAddrSpec: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*Set PercentEncoding Value*/
        pSipUri->SetParameterComponent(m_pParameterComponent);

        /*Decode the sip uri*/
        if (pSipUri->DecodeSipUri(pStartPt, nDecLen) == SIP_FALSE)
        {
            pSipUri->SipDelete();
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipAddrSpec::DecodeAddrSpec: Sip Uri Decode Failed", SIP_ZERO, SIP_ZERO);

            return SIP_FALSE;
        }
        m_pSipUri = pSipUri;
    }
    else
    {
        m_pszAbsUri = sipCreateString(pStartPt, pEndPt);
        if (m_pszAbsUri == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipNameAddr::DecodeNameAddr
 *
 * Description   :Function for decoding
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameAddr::DecodeNameAddr(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    SIP_CHAR* pTempPre = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, LEFT_ANGLE) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Left Angle Not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    /*Case of display Name present*/
    if (pStartPt <= pTempPre)
    {
        m_pszDispName = sipCreateString(pStartPt, pTempPre);
        if (m_pszDispName == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    /*Now Decode the Addr Spec*/
    m_pAddrSpec = new SipAddrSpec();
    if (m_pAddrSpec == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_pAddrSpec->SetParameterComponent(m_pParameterComponent);

    /*Get the length of address spec*/
    SIP_UINT32 nDecLen = pEndPt - pTempNext + SIP_ONE;

    if (m_pAddrSpec->DecodeAddrSpec(pTempNext, nDecLen) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Addr Spec decoding failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}

/*****************************************************************************
 * Function name      : SipUri::DecUserInfo
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_BOOL SipUri::DecUserInfo(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    /* check for userinfo = ( user / telephone-subscriber ) [ ":" password ] "@" */

    SIP_CHAR* pTempPos = SIP_NULL;
    /* Decode password part in userinfo */
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_TRUE)
    {
        SIP_CHAR* pPasswordStart = pTempPos + SIP_TWO;
        SIP_CHAR* pszPassword = sipCreateString(pPasswordStart, pEndPt);
        if (pszPassword == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        /*Do percentage Encoding*/
        if ((m_pParameterComponent != SIP_NULL) &&
                (m_pParameterComponent->IsValidComponent(SIP_PASSWORD)))
        {
            m_pszPassword = SipPercentEncoding::DoPercentDecoding(pszPassword);
        }
        else
        {
            m_pszPassword = pszPassword;
        }

        pEndPt = pTempPos;
    }
    /* Decode ( user   /   telephone-subscriber ) part in userinfo */
    SIP_CHAR* pszUser = sipCreateString(pStartPt, pEndPt);
    if (pszUser == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    /*Do percentage Encoding*/
    if ((m_pParameterComponent != SIP_NULL) && (m_pParameterComponent->IsValidComponent(SIP_USER)))
    {
        m_pszUser = SipPercentEncoding::DoPercentDecoding(pszUser);
    }
    else
    {
        m_pszUser = pszUser;
    }

    return SIP_TRUE;
}

/*****************************************************************************
 * Function name      : SipUri::DecHostPort
 *
 * Description        :
 *
 * Preconditions      :
 *
 * Side Effects          : none
 *****************************************************************************/
SIP_BOOL SipUri::DecHostPort(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    /*hostport = host [ ":" port ]
      host = hostname   /   IPv4address   /   IPv6reference
      hostname = *( domainlabel   "." )   toplabel   [ "." ]
      domainlabel = alphanum   /   alphanum   *( alphanum   /   "-" )   alphanum
      toplabel = ALPHA   /   ALPHA   *( alphanum   /   "-" )   alphanum
      IPv4address = 1*3DIGIT   "."   1*3DIGIT   "."   1*3DIGIT   "."   1*3DIGIT
      IPv6reference = "["   IPv6address   "]"
      IPv6address = hexpart   [ ":"   IPv4address ]
      hexpart = hexseq   /   hexseq   "::"   [ hexseq ]   /   "::"   [ hexseq ]
      hexseq = hex4   *( ":"   hex4 )
      hex4 = 1*4HEXDIG
      port = 1*DIGIT */

    SIP_CHAR* pTempPos = SIP_NULL;

    /* IPV6 is enclosed in between '[' and ']', get start and end point of Ipv6 address*/
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, LEFT_SQUARE) == SIP_TRUE)
    {
        m_eHostType = SipAddrSpec::HOST_IPV6;
        pStartPt = pTempPos + SIP_TWO;
        pTempPos = SIP_NULL;
        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, RIGHT_SQUARE) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Invalid Host[IPV6]", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }
    /* Host : Hostname or IPv4 or IPv6 */
    m_pszHost = sipCreateString(pStartPt, pTempPos);
    if (m_pszHost == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (m_eHostType != SipAddrSpec::HOST_IPV6)
    {
        IPAddress pbjIpAddr;
        if (pbjIpAddr.Parse(AString(m_pszHost)) == SIP_TRUE)
        {
            m_eHostType = SipAddrSpec::HOST_IPV4;
        }
    }

    if ((m_pParameterComponent != SIP_NULL) &&
            (m_pParameterComponent->IsValidComponent(SIP_HOST)) &&
            (m_eHostType == SipAddrSpec::HOST_NAME))
    {
        m_pszHost = SipPercentEncoding::DoPercentDecoding(m_pszHost);
    }

    pStartPt = pTempPos;
    /* Port number */
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, COLON) == SIP_TRUE)
    {
        pTempPos = pTempPos + SIP_TWO;
        SIP_CHAR* pszPort = sipCreateString(pTempPos, pEndPt);
        if (pszPort == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_CHAR* pszPortTemp = SIP_NULL;

        if ((m_pParameterComponent != SIP_NULL) &&
                (m_pParameterComponent->IsValidComponent(SIP_PORT)))
        {
            pszPortTemp = SipPercentEncoding::DoPercentDecoding(pszPort);
        }
        else
        {
            pszPortTemp = pszPort;
        }
        m_nPort = SipPf_Atoi(pszPortTemp);
        delete[] pszPortTemp;
    }
    else
    {
        m_nPort = SIP_UNSPECIFIED_PORT;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipUri::DecodeSipUri
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipUri::DecodeSipUri(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    /* SIP URI : sip(s):user:password@host:port;uri-parameters?headers
     * uri-parameters (parameter-name "=" parameter-value pairs) are separated by semi-colons and
     * headers (hname = hvalue pairs) are separated by Ampersand
     */
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPos = SIP_NULL;

    /* Decode user:password part in SIP URI */
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, ATRATE) == SIP_TRUE)
    {
        if (DecUserInfo(pStartPt, pTempPos) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "User Info Decode Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pStartPt = pTempPos + SIP_TWO;
    }
    /* Decode headers part in SIP URI */
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, QMARK) == SIP_TRUE)
    {
        SIP_CHAR* pHeaderStart = pTempPos + SIP_TWO;

        m_pUriHdrParamList = new SipParameterList();
        if (m_pUriHdrParamList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pUriHdrParamList->DecUriHdrSipParameterList(
                    pHeaderStart, pEndPt, AMPERSEND, m_pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Hdr prm Decode Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        pEndPt = pTempPos;
        pTempPos = SIP_NULL;
    }
    /* Decode uri-parameters part in SIP URI */
    if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, SIP_SEMI) == SIP_TRUE)
    {
        SIP_CHAR* pUriParamStart = pTempPos + SIP_TWO;

        m_pUriParamList = new SipParameterList();
        if (m_pUriParamList == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (m_pUriParamList->DecUriSipParameterList(
                    pUriParamStart, pEndPt, SIP_SEMI, m_pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Uri Prm Decode Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        pEndPt = pTempPos;
        pTempPos = SIP_NULL;
    }
    /* Decode host:port part in SIP URI */
    if (DecHostPort(pStartPt, pEndPt) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Host port Decode Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    return SIP_TRUE;
}
