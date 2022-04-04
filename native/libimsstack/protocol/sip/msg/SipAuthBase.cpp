/******************************************************************************
 * Project Name   : SIP_RTP
 * Group    : IP-CS [MSG-2]
 * Security   : Confidential
 *****************************************************************************/

/******************************************************************************

 * Filename      : SipAuthBase.h
 * Purpose     :
 * Platform      : Windows OR Android
 * Author(s)     : Saurabh Srivastava
 * E-mail id.    : saurabh31.srivastava
 * Creation date   : may. 09 ,2011
 *
 * Edit HisAlertry   Modification description(s)
 * Date      Name    Version    Bug-ID    Description
 * ----------    ----------    -------    ------    -------------
 * Month. Date,10    Saurabh      0.0a    Initial creation
 *****************************************************************************/

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipHeaderBase.h"
#include "msg/SipAuthBase.h"
#include "msg/SipParameterList.h"
#include "sip_error.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "SipConfiguration.h"
#include "msg/sip_msgutil.h"
#include "platform/sip_pf_string.h"
/****************************************************************************
  Macro Definitions
 *****************************************************************************/


/****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
  Class Declaration Starts
 *****************************************************************************/
/*constructor*/
SipAuthBase::SipAuthBase(SIP_INT32 eHdrType)
    : SipHeaderBase(eHdrType)
    , m_objAuthList(SipVector<SipNameValue*>())
{
}

SipAuthBase::SipAuthBase(const SipAuthBase& objHeader)
    : SipHeaderBase(objHeader)
    , m_objAuthList(SipVector<SipNameValue*>())
{
    SIP_UINT32 nSize = objHeader.m_objAuthList.GetSize();

    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize ; nCount++)
    {
        SipNameValue* pTempNmVl = objHeader.m_objAuthList.GetAt(nCount);

        if (pTempNmVl != SIP_NULL)
        {
            SipNameValue* pNmVl = new SipNameValue(*pTempNmVl);
            m_objAuthList.Add(pNmVl);
        }
    }
}


/*destructor*/
SipAuthBase::~SipAuthBase()
{
    while(m_objAuthList.IsEmpty() != SIP_TRUE)
    {
        delete m_objAuthList.Top();
        m_objAuthList.Pop();
    }
}

SIP_BOOL SipAuthBase::EncodeAuthList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter)
{
    SIP_UINT32 nIndex = SIP_ZERO;
    SIP_UINT32 nCount = m_objAuthList.GetSize();

    //  enc for *( ";" uri-parameter )
    while(nIndex < nCount)
    {
        SipNameValue* pParamNamValue = m_objAuthList.GetAt(nIndex);

        if (nIndex > 0)
        {
            *(*ppCurrPos) = cDelimiter;
            (*ppCurrPos)++;
        }

        if (pParamNamValue->EncodeFromList(ppCurrPos) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        nIndex++;
    }
    return SIP_TRUE;
}

/*virtual methods*/
/*Function for encoding of headers*/
SIP_BOOL SipAuthBase::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    const SIP_CHAR* pszValue = GetValue();
    if (pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Missing Scheme", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    /*Encode space*/
    **ppCurrPos = SPACE;
    (*ppCurrPos)++;
    EncodeAuthList(ppCurrPos, COMMA);

    return SIP_TRUE;
}


SIP_BOOL SipAuthBase::SetParams(const SIP_CHAR* pszName, const SIP_CHAR* pszVal,
        SIP_BOOL bIsFeatureParam)
{
    SipNameValue* pNV = new SipNameValue();

    if (pNV == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (bIsFeatureParam == SIP_TRUE)
    {
        pNV->m_ePrmType = SipParameters::FEATURE;
    }

    pNV->m_pszName = SipPf_Strdup(pszName);
    if (pNV->m_pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        delete pNV;
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempVal = SipPf_Strdup(pszVal);
    if (pszTempVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        delete pNV;
        return SIP_FALSE;
    }

    pNV->m_valueList.Add(pszTempVal);
    m_objAuthList.Add(pNV);

    return SIP_TRUE;
}

SIP_BOOL SipAuthBase::FindElement(const SIP_CHAR* pszName, SipNameValue *&pNmvl,
        SIP_UINT32& nPos)
{
    SIP_UINT32 nSize = m_objAuthList.GetSize();

    for(SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        SipNameValue* pNmVl = m_objAuthList.GetAt(nIndex);
        if (SipPf_Stricmp(pszName, pNmVl->m_pszName) == 0)
        {
            nPos = nIndex;
            pNmvl = pNmVl;
            return SIP_TRUE;
        }
    }
    return SIP_FALSE;
}

SIP_CHAR* SipAuthBase::GetAuthValue(const SIP_CHAR* pszName)
{
    if (pszName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipNameValue* pNmVl = SIP_NULL;
    SIP_UINT32 nPos = SIP_ZERO;
    SIP_BOOL bStatus = FindElement(pszName, pNmVl, nPos);
    if ((bStatus == SIP_FALSE) || (pNmVl == SIP_NULL))
    {
        return SIP_NULL;
    }

    SipVector<SIP_CHAR*>& valueList = pNmVl->m_valueList;
    if (valueList.IsEmpty() == SIP_TRUE)
    {
        return SIP_NULL;
    }

    SIP_CHAR* pszVal = SIP_NULL;
    SIP_CHAR* pszElement = valueList.GetAt(SIP_ZERO);
    if (pNmVl->m_ePrmType == SipParameters::FEATURE)
    {
        int nLen = SIP_ZERO;
        if (pszElement != SIP_NULL)
        {
            nLen = SipPf_Strlen(pszElement);
        }
        pszVal = new SIP_CHAR[nLen + SIP_THREE];
        pszVal[0] = DQUOTE;
        SipPf_Strcpy(pszVal + SIP_ONE, pszElement);
        pszVal[nLen + SIP_ONE] = DQUOTE;
        pszVal[nLen + SIP_TWO] = SIP_NULL;
    }
    else
    {
        pszVal = SipPf_Strdup(pszElement);
    }
    return pszVal;
}


/******************************************************************************
 * Function name      : SipAuthBase::DecodeHdr
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipAuthBase::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;

    if (sipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "LWS not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszScheme = sipCreateString(pStartPt, pTempPre);
    if (SetValue(pszScheme) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        if (pszScheme != SIP_NULL)
        {
            delete[] pszScheme;
        }
        return SIP_FALSE;
    }
    delete[] pszScheme;

    /*Update the temp to start of LWS*/
    pTempPre = pTempPre + SIP_ONE;
    /*Skip the LWS*/
    pStartPt = sipSkipFwLWS(pTempPre, pEndPt);
    pTempPre = SIP_NULL;

    while(pStartPt < pEndPt)
    {
        SIP_CHAR* pTempNext = SIP_NULL;

        if (sipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, COMMA) == SIP_FALSE)
        {
            pTempPre = pEndPt;
            pTempNext = pEndPt;
        }

        SipNameValue* pNmVl = new SipNameValue(GetHdrType());
        if (pNmVl == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNmVl->DecHdrNameVal(pStartPt, pTempPre) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Name Value decode Fail", SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }
        if (m_objAuthList.Add(pNmVl) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Adding in list fail", SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }
        /*Update the Start point to the start of next Name Value Pair*/
        pStartPt = pTempNext;
        pTempPre = SIP_NULL;
    }

    return SIP_TRUE;
}

SipHeaderBase* SipAuthBase::GetNewObj(SIP_INT32 eHdr, SipHeaderBase* pHeader)
{
    if (pHeader != SIP_NULL)
    {
        return new SipAuthBase(*reinterpret_cast<SipAuthBase*>(pHeader));
    }
    return new SipAuthBase(eHdr);
}
