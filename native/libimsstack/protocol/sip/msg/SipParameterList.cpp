/***********************************************************
 * Project Name : SIP_RTP
 * Group        : MSG-2
 * Security     : Confidential
 ***********************************************************/

/**********************************************************
 * Filename          : SipParameterList.cpp
 * Purpose           :
 * Platform          : Windows XP
 * Author(s)         : Saurabh Srivastava
 * E-mail id.        : saurabh31.srivastava@
 * Creation date     : July 26, 2010
 *
 * Modifications:
 * 1. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0a
 *
 * 2. Modified by    : <Name>
 *    Date           : <mmm. dd, yyyy> (E.g. Apr. 21, 2006)
 *    Description    :
 *    Version Number : 0.0b
 **********************************************************/

/*****************************************************************************
  Header Inclusions
 *****************************************************************************/
#include "msg/SipParameterList.h"
#include "msg/SipAddrSpec.h"
#include "msg/SipRequestLine.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"
#include "sip_debug.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "msg/sip_msgutil.h"

SIP_CHAR gaszAuthInfo[SIP_AINFO_CNT][SIP_AINFO_LEN]={"nextnonce","qop","rspauth","cnonce","nc"};

/****************************************************************************
  Macro Definitions
 *****************************************************************************/

/****************************************************************************
  Class Member Function Implementations
 *****************************************************************************/

/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipParameterList::~SipParameterList()
{
    while(m_objPrmList.IsEmpty() != SIP_TRUE)
    {
        delete m_objPrmList.Top();
        m_objPrmList.Pop();
    }
}

/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipParameterList::SipParameterList()
    : m_objPrmList(SipVector<SipNameValue*>())
    , m_eHdrType(SipHeaderBase::TYPE_INVALID)
{
}

/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipParameterList::SipParameterList(SIP_INT32 eHdrType)
    : m_objPrmList(SipVector<SipNameValue*>())
    , m_eHdrType(eHdrType)
{
}

/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipParameterList::SipParameterList(const SipParameterList& objPrmList)
    : m_objPrmList(SipVector<SipNameValue*>())
    , m_eHdrType(objPrmList.m_eHdrType)
{
    SIP_UINT32 nSize = objPrmList.m_objPrmList.GetSize();

    for (SIP_UINT32 unCount = SIP_ZERO; unCount < nSize; unCount++)
    {
        SipNameValue* pElement = objPrmList.m_objPrmList.GetAt(unCount);

        if (pElement == SIP_NULL)
        {
            continue;
        }

        SipNameValue* pNmVl = new SipNameValue(*pElement);

        if (pNmVl == SIP_NULL)
        {
            continue;
        }

        m_objPrmList.Add(pNmVl);
    }
}


/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::Add(const SIP_CHAR* pszName)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    SipNameValue* pNV = new SipNameValue();
    if (pNV == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipEnc_SipMsg: Malloc Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    pNV->m_pszName = SipPf_Strdup(pszName);
    m_objPrmList.Add(pNV);
    return SIP_TRUE;


}

/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    if (pszName == SIP_NULL || pszValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
            "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objPrmList.GetSize();
    SipNameValue* pNV = SIP_NULL;
    SIP_BOOL bFound = SIP_FALSE;
    SIP_UINT32 nIndex = SIP_ZERO;

    while ((nIndex < nSize) && (bFound != SIP_TRUE))
    {
        pNV = m_objPrmList.GetAt(nIndex);
        if (SipPf_Strcmp(pNV->m_pszName, pszName) == SIP_ZERO)
        {
            //found the required node
            bFound = SIP_TRUE;
            break;
        }
        nIndex++;
    }

    if (bFound == SIP_FALSE)
    {
        pNV = new SipNameValue();
        if (pNV == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipEnc_SipMsg: Malloc Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pNV->m_pszName = SipPf_Strdup(pszName);
    }

    pNV->m_valueList.Add(SipPf_Strdup(pszValue));

    // the name value obj to the list
    if (bFound == SIP_FALSE)
    {
        m_objPrmList.Add(pNV);
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipParameterList::Add
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL  SipParameterList::Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue,
        SIP_INT32 ePrmType)
{
    (void)pszName;
    (void)pszValue;
    (void)ePrmType;

    return SIP_TRUE;
}


/******************************************************************************
 * Function name      : SipParameterList::Remove
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::Remove(const SIP_CHAR* pszName)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
            "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objPrmList.GetSize();

    SIP_BOOL bFound = SIP_FALSE;
    SIP_UINT32 nIndex = SIP_ZERO;
    while (nIndex < nSize && bFound != SIP_TRUE)
    {
        SipNameValue* pNV = m_objPrmList.GetAt(nIndex);
        if (SipPf_Strcmp(pNV->m_pszName,
                    pszName) == 0)
        {
            //found the required node
            bFound = SIP_TRUE;
            break;
        }
        nIndex++;
    }

    if (bFound == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                "SipEnc_SipMsg: Param Not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    delete m_objPrmList.GetAt(nIndex);
    m_objPrmList.RemoveAt(nIndex);

    return SIP_TRUE;
}

SIP_BOOL SipParameterList::FindElement(const SIP_CHAR* pszName, SipNameValue*& pNmvl,
        SIP_UINT32& nPos)
{
    SIP_UINT32 nSize = m_objPrmList.GetSize();

    for(SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        SipNameValue* pNmVl = m_objPrmList.GetAt(nIndex);
        if (SipPf_Stricmp(pszName, pNmVl->m_pszName) == 0)
        {
            nPos = nIndex;
            pNmvl = pNmVl;
            return SIP_TRUE;
        }
    }
    return SIP_FALSE;
}

/******************************************************************************
 * Function name      : SipParameterList::SetParamValue
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::SetParamValue(const SIP_CHAR* pszName, const SIP_CHAR* pszValue,
        SIP_UINT32 nPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipNameValue* pNmVl = SIP_NULL;
    SIP_UINT32 nTempPos = SIP_ZERO;

    /*Check whether parameter already exists or not*/
    SIP_BOOL bStatus = FindElement(pszName, pNmVl, nTempPos);

    /*If parameter not found add new entry*/
    if ((bStatus == SIP_FALSE) || (pNmVl == SIP_NULL))
    {
        if ((pszValue != SIP_NULL) && (SipPf_Strlen(pszValue) != SIP_ZERO))
        {
            return Add(pszName, pszValue);
        }
        return Add(pszName);
    }

    /*If parameter already exists, update value*/
    if (pNmVl->m_valueList.IsEmpty() != SIP_TRUE)
    {

        if (pNmVl->m_valueList.GetSize() < nPos)
        {
            return SIP_FALSE;
        }

        /*If the new value is NULL, remove the existing value*/
        if ((pszValue == SIP_NULL) || (SipPf_Strlen(pszValue) == SIP_ZERO))
        {
            delete pNmVl->m_valueList.GetAt(nPos);
            pNmVl->m_valueList.RemoveAt(nPos);
        }
        else
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszValue);
            delete pNmVl->m_valueList.GetAt(nPos);
            pNmVl->m_valueList.RemoveAt(nPos);
            pNmVl->m_valueList.InsertAt(pszVal, nPos);
        }
    }
    else
    {
        /*If parameter already not exists, add value*/
        if ((pszValue != SIP_NULL) && (SipPf_Strlen(pszValue) != SIP_ZERO))
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszValue);
            pNmVl->m_valueList.InsertAt(pszVal, SIP_ZERO);
        }
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipParameterList::GetParamValue
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_CHAR* SipParameterList::GetParamValue(const SIP_CHAR* pszName,
        SIP_UINT32 nPos /*default value is zero*/)
{
    if (pszName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipNameValue* pNmVl = SIP_NULL;
    SIP_UINT32 nPos1 = SIP_ZERO;

    SIP_BOOL bStatus = FindElement(pszName, pNmVl, nPos1);
    if ((bStatus == SIP_FALSE) || (pNmVl == SIP_NULL))
    {
        return SIP_NULL;
    }

    if ((pNmVl->m_valueList.IsEmpty() == SIP_TRUE) ||
        (pNmVl->m_valueList.GetSize() < nPos))
    {
        return SIP_NULL;
    }

    SIP_CHAR* pszElement = pNmVl->m_valueList.GetAt(nPos);

    return (pszElement != SIP_NULL) ? SipPf_Strdup(pszElement) : SIP_NULL;
}

/******************************************************************************
 * Function name      : SipParameterList::GetParamNode
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipNameValue* pNmVl = SIP_NULL;

    return FindElement(pszName, pNmVl, *pnPos);
}

/******************************************************************************
 * Function name      : SipParameterList::GetParamNode
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipNameValue* SipParameterList::GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipNameValue* pNmVl = SIP_NULL;
    SIP_BOOL bStatus = FindElement(pszName, pNmVl, *pnPos);
    if ((bStatus == SIP_FALSE) || (pNmVl == SIP_NULL))
    {
        return SIP_NULL;
    }

    return pNmVl;
}

SIP_BOOL SipParameterList::EncodeList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter)
{
    SIP_UINT32 nCount = m_objPrmList.GetSize();

    SIP_UINT32 nIndex = SIP_ZERO;
    //  enc for *( ";" uri-parameter )
    while (nIndex < nCount)
    {
        SipNameValue* pstParamNamValue = m_objPrmList.GetAt(nIndex);

        *(*ppCurrPos) = cDelimiter;
        (*ppCurrPos)++;
        pstParamNamValue->EncodeFromList(ppCurrPos);
        nIndex++;
    }
    return SIP_TRUE;
}

SIP_BOOL SipParameterList::EncodeUriParamList(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter,
        IParameterComponent* m_pParameterComponent)
{
    SIP_UINT32 nCount = m_objPrmList.GetSize();

    SIP_UINT32 nIndex = SIP_ZERO;
    //  enc for *( ";" uri-parameter )
    while (nIndex < nCount)
    {
        SipNameValue* pstParamNamValue = m_objPrmList.GetAt(nIndex);

        *(*ppCurrPos) = cDelimiter;
        (*ppCurrPos)++;
        pstParamNamValue->EncodeFromUriList(ppCurrPos, m_pParameterComponent);
        nIndex++;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipNameValue::SipNameValue
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipNameValue::SipNameValue()
    : m_pszName(SIP_NULL)
    , m_valueList(SipVector<SIP_CHAR*>())
    , m_ePrmType(SipParameters::GENERIC)
    , m_Sep (',')
    , m_eHdrType(SipHeaderBase::TYPE_INVALID)
{

}

/******************************************************************************
 * Function name      : SipNameValue::SipNameValue
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipNameValue::SipNameValue(SIP_INT32 eHdrType)
    : m_pszName(SIP_NULL)
    , m_valueList(SipVector<SIP_CHAR*>())
    , m_ePrmType(SipParameters::GENERIC)
    , m_Sep (',')
    , m_eHdrType(eHdrType)
{

}

/******************************************************************************
 * Function name      : SipNameValue::SipNameValue
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipNameValue::SipNameValue(const SipNameValue& objNmVl)
    : m_pszName(SipPf_Strdup(objNmVl.m_pszName))
    , m_valueList(SipVector<SIP_CHAR*>())
    , m_ePrmType(objNmVl.m_ePrmType)
    , m_Sep (objNmVl.m_Sep)
    , m_eHdrType(SipHeaderBase::TYPE_INVALID)
{
    SIP_UINT32 nSize = objNmVl.m_valueList.GetSize();

    for (SIP_UINT32 unCount = SIP_ZERO; unCount < nSize; unCount++)
    {
        SIP_CHAR* pszElement = objNmVl.m_valueList.GetAt(unCount);

        if (pszElement == SIP_NULL)
        {
            continue;
        }

        SIP_CHAR* pszValue = SipPf_Strdup(pszElement);
        m_valueList.Add(pszValue);
    }
}

/******************************************************************************
 * Function name      : SipNameValue::~SipNameValue
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SipNameValue::~SipNameValue()
{
    if (m_pszName != SIP_NULL)
    {
        delete[] m_pszName;
    }
    while(m_valueList.IsEmpty() != SIP_TRUE)
    {
        delete m_valueList.Top();
        m_valueList.Pop();
    }
}


/******************************************************************************
 * Function name      : SipNameValue::SetSeparator
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameValue::SetSeparator(SIP_CHAR cSeparator)
{
    m_Sep = cSeparator;
    return SIP_TRUE;
}


SIP_BOOL SipNameValue::EncodeFromList(SIP_CHAR** ppCurrPos)
{
    if (m_pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_valueList.IsEmpty() != SIP_TRUE)
    {
        SIP_UINT32 nCount = m_valueList.GetSize();

        **ppCurrPos = EQUAL;
        (*ppCurrPos)++;

        if (m_ePrmType == SipParameters::FEATURE)
        {
            **ppCurrPos = DQUOTE;
            (*ppCurrPos)++;
        }

        SIP_UINT32 sLocalCount = SIP_ZERO;

        while (sLocalCount < nCount)
        {
            SIP_CHAR* pszVal = m_valueList.GetAt(sLocalCount);
            if (pszVal == SIP_NULL)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                        "EncodeFromList: null value", SIP_ZERO, SIP_ZERO);
                return SIP_FALSE;
            }

            SipPf_Strcpy(*ppCurrPos, pszVal);
            SipEnc_UpdateCurrPos(ppCurrPos);

            /*Condition to prevent last put of separator*/
            if (sLocalCount < (nCount - SIP_ONE))
            {
                **ppCurrPos = m_Sep;
                (*ppCurrPos)++;
            }
            sLocalCount++;
        }

        if (m_ePrmType == SipParameters::FEATURE)
        {
            **ppCurrPos = DQUOTE;
            (*ppCurrPos)++;
        }
    }
    else
    {
        if (m_ePrmType == SipParameters::FEATURE)
        {
            **ppCurrPos = EQUAL;
            (*ppCurrPos)++;

            **ppCurrPos = DQUOTE;
            (*ppCurrPos)++;

            **ppCurrPos = DQUOTE;
            (*ppCurrPos)++;
        }
    }

    return SIP_TRUE;
}


SIP_BOOL SipNameValue::EncodeFromUriList(SIP_CHAR** ppCurrPos,
        IParameterComponent* pParameterComponent)
{
    if (m_pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_valueList.IsEmpty() != SIP_TRUE)
    {
        //encode as name =  val
        SIP_CHAR* pszVal = m_valueList.GetAt(SIP_ZERO);
        if (pszVal == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                    "SipNameValue::EncodeFromList: List err", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        **ppCurrPos = EQUAL;
        (*ppCurrPos)++;

        /*Do percentage Encoding of Value if Required*/
        if ((pParameterComponent != SIP_NULL) &&
            (pParameterComponent->IsValidComponent(m_pszName) == SIP_TRUE))
        {
            SIP_CHAR* pszValTemp = SipPercentEncoding::DoPerEnc_Param(m_pszName, pszVal);
            SipPf_Strcpy(*ppCurrPos, pszValTemp);
            delete[] pszValTemp;
        }
        else
        {
            SipPf_Strcpy(*ppCurrPos, pszVal);
        }
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return SIP_TRUE;
}


SIP_BOOL SipNameValue::EncodeFromUriHdrList(SIP_CHAR** ppCurrPos,
        IParameterComponent* pParameterComponent)
{
    if (m_pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_valueList.IsEmpty() != SIP_TRUE)
    {
        //encode as name =  val
        SIP_CHAR* pszVal = m_valueList.GetAt(SIP_ZERO);
        if (pszVal == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER,
                    "SipNameValue::EncodeFromList: List err", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        **ppCurrPos = EQUAL;
        (*ppCurrPos)++;

        /*Do percentage Encoding of Value if Required*/
        if ((pParameterComponent != SIP_NULL) &&
            (pParameterComponent->IsValidComponent(SIP_HEADERS) == SIP_TRUE))
        {
            SIP_CHAR* pszValTemp =
                SipPercentEncoding::DoPerEnc_Param((SIP_CHAR *)SIP_HEADERS, pszVal);
            SipPf_Strcpy(*ppCurrPos, pszValTemp);
            delete[] pszValTemp;
        }
        else
        {
            SipPf_Strcpy(*ppCurrPos, pszVal);
        }
        SipEnc_UpdateCurrPos(ppCurrPos);
    }

    return SIP_TRUE;
}


/******************************************************************************
 * Function name      : SipNameValue::DecUriNameVal
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameValue::DecUriNameVal(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
        IParameterComponent* pParameterComponent)
{
    SIP_CHAR* pNameEnd = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pNameEnd, EQUAL) == SIP_FALSE)
    {
        pNameEnd = pEndPt;
    }

    m_pszName = sipCreateString(pStartPt, pNameEnd);
    if (m_pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
            "SipNameValue::DecUriNameVal: Memory Allocation Failed",
            SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pNameEnd != pEndPt)
    {
        /*Update the pNameEnd to the start of Value List*/
        SIP_CHAR* pszValuePtr = pNameEnd + SIP_TWO;
        SIP_CHAR* pszValue = sipCreateString(pszValuePtr, pEndPt);
        if (pszValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipNameValue::DecUriNameVal: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_CHAR* pszTempValue = SIP_NULL;
        /*Do percent Decoding of value if applicable*/
        if ((pParameterComponent != SIP_NULL) &&
            (pParameterComponent->IsValidComponent(m_pszName) == SIP_TRUE))
        {
            pszTempValue = SipPercentEncoding::DoPercentDecoding(pszValue);
        }
        else
        {
            pszTempValue = pszValue;
        }
        /*put the value in the value list*/
        if (m_valueList.Add(pszTempValue) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipNameValue::DecUriNameVal:Adding in list failed",
                    SIP_ZERO, SIP_ZERO);
            delete[] pszTempValue;
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}


/******************************************************************************
 * Function name      : SipNameValue::DecUriHdrNameVal
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameValue::DecUriHdrNameVal(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
        IParameterComponent* pParameterComponent)
{
    SIP_CHAR* pNameEnd = SIP_NULL;

    if (sipFindPreDelimiter(pStartPt, pEndPt, &pNameEnd, EQUAL) == SIP_FALSE)
    {
        pNameEnd = pEndPt;
    }

    m_pszName = sipCreateString(pStartPt,  pNameEnd);
    if (m_pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
            "DecUriHdrNameVal: Memory Allocation Failed",
            SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pNameEnd != pEndPt)
    {
        /*Update the pNameEnd to the start of Value List*/
        SIP_CHAR* pszValuePtr = pNameEnd + SIP_TWO;
        SIP_CHAR* pszValue = sipCreateString(pszValuePtr, pEndPt);
        if (pszValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecUriHdrNameVal: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_CHAR* pszTempValue = SIP_NULL;
        /*Do percent Decoding of value if applicable*/
        if ((pParameterComponent != SIP_NULL) &&
            (pParameterComponent->IsValidComponent(SIP_HEADERS) == SIP_TRUE))
        {
            pszTempValue = SipPercentEncoding::DoPercentDecoding(pszValue);
        }
        else
        {
            pszTempValue = pszValue;
        }
        /*put the value in the value list*/
        if (m_valueList.Add(pszTempValue) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecUriHdrNameVal:Adding in list failed",
                    SIP_ZERO, SIP_ZERO);
            delete[] pszTempValue;
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipNameValue::DecHdrNameVal
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipNameValue::DecHdrNameVal(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt)
{
    if (pStartPt > pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipNameValue::DecHdrNameVal: No Parameter Present",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pTempNext = SIP_NULL;
    SIP_CHAR* pNameEnd = SIP_NULL;

    /*Case of name and Value Both*/
    if (sipFindActualPos(pStartPt, pEndPt, &pNameEnd, &pTempNext, EQUAL) == SIP_FALSE)
    {
        /*in case No equal present => only name present*/
        m_pszName = sipCreateString(pStartPt,  pEndPt);
        if (m_pszName == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipNameValue::DecHdrNameVal: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        return SIP_TRUE;
    }

    /*Check For Double Equal Condition*/
    if (*pTempNext == EQUAL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipNameValue::DecHdrNameVal: Double Equal Present",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;

    }

    /*Update the value pointer*/
    SIP_CHAR* pszValuePtr = pTempNext;

    /*Case of no value*/
    if (pszValuePtr > pEndPt)
    {
        m_pszName = sipCreateString(pStartPt,  pNameEnd);
        if (m_pszName == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipNameValue::DecHdrNameVal: Name Allocation Failed",
                SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        return SIP_TRUE;
    }

    m_pszName = sipCreateString(pStartPt,  pNameEnd);
    if (m_pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipNameValue::DecHdrNameVal: Name Allocation Failed",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /*Case of multiple and single values*/
    /*TempPos will point to the end of current value */
    while (pszValuePtr <= pEndPt)
    {
        SIP_CHAR* pszValue = SIP_NULL;
        pTempNext = SIP_NULL;

        if (sipFindPreDelimiter(pszValuePtr, pEndPt, &pTempNext, COMMA) == SIP_FALSE)
        {
            pTempNext = pEndPt;
        }

        pszValue = sipCreateString(pszValuePtr, pTempNext);
        if (pszValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipNameValue::DecHdrNameVal: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        /*put the value in the value list*/
        if (m_valueList.Add(pszValue) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipNameValue::DecHdrNameVal:Adding in list failed",
                    SIP_ZERO, SIP_ZERO);
            delete[] pszValue;
            return SIP_FALSE;
        }

        pszValuePtr = pTempNext + SIP_TWO;

        if (pTempNext == pEndPt)
        {
            return SIP_TRUE;
        }
    }
    return SIP_TRUE;
}


/******************************************************************************
 * Function name      : SipParameterList::DecUriSipParameterList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::DecUriSipParameterList(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
        SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent)
{
    if (pStartPt > pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "DecUriSipParameterList: No Value Present",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    while (pStartPt <= pEndPt)
    {
        SIP_CHAR* pTempPos = SIP_NULL;

        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, cDelimiter) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        SipNameValue* pNmVl = new SipNameValue();
        if (pNmVl == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecUriSipParameterList: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNmVl->DecUriNameVal(pStartPt, pTempPos, pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecUriSipParameterList: Name Val Decode fail",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }

        if (m_objPrmList.Add(pNmVl) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecUriSipParameterList: Append in list Failed",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            return SIP_TRUE;
        }

        pStartPt = pTempPos + SIP_TWO;

        if (pStartPt > pEndPt)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecUriSipParameterList: No Value Present",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}



/******************************************************************************
 * Function name      : SipParameterList::DecUriHdrSipParameterList
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::DecUriHdrSipParameterList(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
        SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent)
{
    if (pStartPt > pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipParameterList::DecUriHdrSipParameterList: No Value Present",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    while (pStartPt <= pEndPt)
    {
        SIP_CHAR* pTempPos = SIP_NULL;

        if (sipFindPreDelimiter(pStartPt, pEndPt, &pTempPos, cDelimiter) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        SipNameValue* pNmVl = new SipNameValue();
        if (pNmVl == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipParameterList::DecUriSipParameterList: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNmVl->DecUriHdrNameVal(pStartPt, pTempPos, pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipParameterList::DecUriSipParameterList: Name Val Decode fail",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }

        if (m_objPrmList.Add(pNmVl) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipParameterList::DecUriSipParameterList: Append in list Failed",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            return SIP_TRUE;
        }

        pStartPt = pTempPos + SIP_TWO;

        if (pStartPt > pEndPt)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "SipParameterList::DecUriSipParameterList: No Value Present",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;
}

/******************************************************************************
 * Function name      : SipParameterList::DecHdrSipPrm
 *
 * Description     :
 *
 * Preconditions      :
 *
 * Side Effects      : none
 *****************************************************************************/
SIP_BOOL SipParameterList::DecHdrSipParameterList(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
        SIP_CHAR cDelimiter)
{
    if (pStartPt > pEndPt)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "DecHdrSipParameterList: No Value Present",
                SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    while (pStartPt <= pEndPt)
    {
        SIP_CHAR* pTempPre = SIP_NULL;
        SIP_CHAR* pTempNext = SIP_NULL;

        if (sipFindActualPos(pStartPt, pEndPt,
                &pTempPre, &pTempNext, cDelimiter) == SIP_FALSE)
        {
            pTempPre = pEndPt;
            pTempNext = pEndPt;
        }

        SipNameValue* pNmVl = new SipNameValue(m_eHdrType);
        if (pNmVl == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecHdrSipParameterList: Memory Allocation Failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        SIP_CHAR* pTempPt = SIP_NULL;

        if (sipFindPreDelimiter(pStartPt, pTempPre, &pTempPt, EQUAL) == SIP_TRUE)
        {
            SIP_CHAR* pTempPrm = sipCreateString(pStartPt, pTempPt);

            if (SipPf_Stricmp(pTempPrm, "method")== SIP_ZERO)
            {
                SipNameValue* pNmVlMethod = SIP_NULL;
                pNmVlMethod = new SipNameValue(m_eHdrType);
                if (pNmVlMethod == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "DecHdrSipParameterList: Memory Allocation Failed",
                        SIP_ZERO, SIP_ZERO);
                    delete[] pTempPrm;
                    delete pNmVl;
                    return SIP_FALSE;
                }

                SIP_CHAR* pQtnMark = SIP_NULL;
                if (sipFindPreDelimiter(pTempPt+2, pTempPre, &pQtnMark, QMARK) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "DecHdrSipParameterList: Question mark not Found",
                            SIP_ZERO, SIP_ZERO);
                    delete pNmVlMethod;
                    delete pNmVl;
                    delete[] pTempPrm;
                    return SIP_FALSE;
                }
                if (pNmVlMethod->DecHdrNameVal(pStartPt, pQtnMark) == SIP_FALSE)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "DecHdrSipParameterList:Name Value decode Failed",
                            SIP_ZERO, SIP_ZERO);
                    delete pNmVlMethod;
                    delete pNmVl;
                    delete[] pTempPrm;
                    return SIP_FALSE;
                }
                if (m_objPrmList.Add(pNmVlMethod) < 0)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "DecHdrSipParameterList: Append in list Failed",
                            SIP_ZERO, SIP_ZERO);
                    delete pNmVlMethod;
                    delete pNmVl;
                    delete[] pTempPrm;
                    return SIP_FALSE;
                }
                pStartPt = pQtnMark +2;
            }
            delete[] pTempPrm;
        }

        if (pNmVl->DecHdrNameVal(pStartPt, pTempPre) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecHdrSipParameterList:Name Value decode Failed",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }
        if (m_objPrmList.Add(pNmVl) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecHdrSipParameterList: Append in list Failed",
                    SIP_ZERO, SIP_ZERO);
            delete pNmVl;
            return SIP_FALSE;
        }

        if (pTempNext == pEndPt)
        {
            return SIP_TRUE;
        }
        /*Update the Start point to the start of next Name Value Pair*/
        pStartPt = pTempNext;
        if (pStartPt > pEndPt)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                    "DecHdrSipParameterList: No Parameter Present",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }

    return SIP_TRUE;

}

/* To Print all header parameters & their values*/
/* Enable SIP_TRACE_ENABLE to log */
SIP_VOID SipParameterList::PrintParams()
{
    //first search the list if the name exists
    SIP_UINT32 nSize = m_objPrmList.GetSize();
    if (nSize <= SIP_ZERO)
    {
        SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER,
                "PrintParams: Param List Empty", SIP_ZERO, SIP_ZERO);
        return;
    }

    SIP_UINT32 nIndex = SIP_ZERO;
    while (nIndex < nSize)
    {
        SipNameValue* pNV = m_objPrmList.GetAt(nIndex);

        if (pNV != SIP_NULL)
        {
            SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "PrintParams: Name of Param : %s",
                            pNV->m_pszName, SIP_ZERO);

            {
                SIP_UINT32 nValSize = pNV->m_valueList.GetSize();
                for (SIP_UINT32 unCount = SIP_ZERO; unCount < nValSize; unCount++)
                {
                    SIP_CHAR* pElement = pNV->m_valueList.GetAt(unCount);
                    if (pElement == SIP_NULL)
                    {
                        break;
                    }

                    SIP_TRACE_NORMAL(ESIPTRACE_MODDECODER, "PrintParams: Value of param : %s",
                            pElement, SIP_ZERO);
                }
             }
        }
        nIndex++;
    }
}

/*****************************************************************************
  SipParameters - Class Member Function Implementations
*****************************************************************************/
SipParameters::SipParameters() : pParameterList(new SipParameterList())
{
}

SipParameters::SipParameters(const SipParameters& objParameters)
{
    if (objParameters.pParameterList != SIP_NULL)
    {
        pParameterList = new SipParameterList(*(objParameters.pParameterList));
    }
    else
    {
        pParameterList = new SipParameterList();
    }
}

SipParameters::SipParameters(SipParameterList* pParameterList_)
{
    if (pParameterList_ != SIP_NULL)
    {
        pParameterList = new SipParameterList(*pParameterList_);
    }
    else
    {
        pParameterList = new SipParameterList();
    }
}

SipParameters::~SipParameters()
{
    if (pParameterList != SIP_NULL)
    {
        pParameterList->SipDelete();
    }
}

SIP_BOOL SipParameters::AddParam(const SIP_CHAR* pszName)
{
    if (pParameterList == SIP_NULL)
    {
        pParameterList = new SipParameterList();
    }

    pParameterList->Add(pszName);

    return SIP_TRUE;
}

SIP_BOOL SipParameters::AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    if (pParameterList == SIP_NULL)
    {
        pParameterList = new SipParameterList();
    }

    return pParameterList->Add(pszName, pszValue);
}

SIP_BOOL SipParameters::AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue,
        SIP_INT32 ePrmType)
{
    if (pParameterList == SIP_NULL)
    {
        pParameterList = new SipParameterList();
    }

    return pParameterList->Add(pszName, pszValue, ePrmType);
}

SIP_BOOL SipParameters::RemoveParam(const SIP_CHAR* pszName)
{
    if (pParameterList == SIP_NULL)
    {
        return SIP_TRUE;
    }

    return pParameterList->Remove(pszName);
}

SIP_VOID SipParameters::SetParameterList(SipParameterList* pSipPrm)
{
    if (pSipPrm == SIP_NULL)
    {
        return;
    }

    if (pParameterList != SIP_NULL)
    {
        pParameterList->SipDelete();
    }

    pParameterList = pSipPrm;
}

SIP_BOOL SipParameters::SetParamValue(const SIP_CHAR* pszName, const SIP_CHAR* pszValue,
        SIP_UINT32 nPos)
{
    if (pParameterList == SIP_NULL)
    {
        pParameterList = new SipParameterList();
    }

    return pParameterList->SetParamValue(pszName, pszValue, nPos);
}

SIP_BOOL SipParameters::IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    return (pParameterList != SIP_NULL) ?
        pParameterList->IsParamExists(pszName, pnPos) : SIP_FALSE;
}

SipParameterList* SipParameters::GetParameterList()
{
    return pParameterList;
}

SIP_CHAR* SipParameters::GetParamValue(const SIP_CHAR* pszName)
{
    if (pParameterList != SIP_NULL)
    {
        return pParameterList->GetParamValue(pszName);
    }
    return SIP_NULL;
}

SipNameValue* SipParameters::GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    if (pParameterList != SIP_NULL)
    {
        return pParameterList->GetParamNode(pszName, pnPos);
    }

    return SIP_NULL;
}
