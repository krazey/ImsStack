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
#include "SipDebug.h"
#include "msg/SipAuthBase.h"
#include "msg/SipMsgUtil.h"
#include "msg/SipParameters.h"
#include "platform/SipString.h"

SipAuthBase::SipAuthBase(SIP_INT32 eHdrType) :
        SipHeaderBase(eHdrType),
        m_objAuthList(SipVector<SipNameValue*>())
{
}

SipAuthBase::SipAuthBase(const SipAuthBase& objHeader) :
        SipHeaderBase(objHeader),
        m_objAuthList(SipVector<SipNameValue*>())
{
    SIP_UINT32 nSize = objHeader.m_objAuthList.GetSize();

    for (SIP_UINT32 nCount = SIP_ZERO; nCount < nSize; nCount++)
    {
        SipNameValue* pTempNmVl = objHeader.m_objAuthList.GetAt(nCount);

        if (pTempNmVl != SIP_NULL)
        {
            SipNameValue* pNmVl = new SipNameValue(*pTempNmVl);
            m_objAuthList.Add(pNmVl);
        }
    }
}

SipAuthBase::~SipAuthBase()
{
    while (m_objAuthList.IsEmpty() != SIP_TRUE)
    {
        m_objAuthList.Top()->SipDelete();
        m_objAuthList.Pop();
    }
}

SIP_BOOL SipAuthBase::Encode(AStringBuffer& objBuffer, SIP_BOOL /*bParams*/) const
{
    if (IsValidHeader() == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();
    objBuffer += pszValue;
    objBuffer += SPACE;

    SIP_UINT32 nIndex = SIP_ZERO;
    SIP_UINT32 nSize = m_objAuthList.GetSize();

    while (nIndex < nSize)
    {
        SipNameValue* pNameValue = m_objAuthList.GetAt(nIndex);

        if (nIndex > 0)
        {
            objBuffer += COMMA;
        }

        if (pNameValue->Encode(objBuffer) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        nIndex++;
    }

    return SIP_TRUE;
}

SIP_BOOL SipAuthBase::EncodeHdr(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    const SIP_CHAR* pszValue = GetValue();
    SipPf_Strcpy(*ppCurrPos, pszValue);
    SipEnc_UpdateCurrPos(ppCurrPos);

    /*Encode space*/
    **ppCurrPos = SPACE;
    (*ppCurrPos)++;

    SIP_UINT32 nIndex = SIP_ZERO;
    SIP_UINT32 nCount = m_objAuthList.GetSize();

    while (nIndex < nCount)
    {
        SipNameValue* pParamNamValue = m_objAuthList.GetAt(nIndex);

        if (nIndex > 0)
        {
            *(*ppCurrPos) = COMMA;
            (*ppCurrPos)++;
        }

        if (pParamNamValue->Encode(ppCurrPos) == SIP_FALSE)
        {
            return SIP_FALSE;
        }

        nIndex++;
    }

    return SIP_TRUE;
}

SIP_BOOL SipAuthBase::SetParams(
        const SIP_CHAR* pszName, const SIP_CHAR* pszVal, SIP_BOOL bIsFeatureParam)
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
        pNV->SipDelete();
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempVal = SipPf_Strdup(pszVal);
    if (pszTempVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        pNV->SipDelete();
        return SIP_FALSE;
    }

    pNV->m_valueList.Add(pszTempVal);
    m_objAuthList.Add(pNV);

    return SIP_TRUE;
}

SIP_BOOL SipAuthBase::FindElement(const SIP_CHAR* pszName, SipNameValue*& pNmvl, SIP_UINT32& nPos)
{
    SIP_UINT32 nSize = m_objAuthList.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
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

    const SipVector<SIP_CHAR*>& valueList = pNmVl->m_valueList;
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

SIP_BOOL SipAuthBase::DecodeHdr(SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    SIP_CHAR* pTempPre = SIP_NULL;

    if (SipFindLWS(pStartPt, pEndPt, &pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "LWS not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszScheme = SipCreateString(pStartPt, pTempPre);
    if (SetValue(pszScheme) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation failed", SIP_ZERO, SIP_ZERO);
        if (pszScheme != SIP_NULL)
        {
            delete[] pszScheme;
        }
        return SIP_FALSE;
    }

    if (pszScheme != SIP_NULL)
    {
        delete[] pszScheme;
    }

    /*Update the temp to start of LWS*/
    pTempPre = pTempPre + SIP_ONE;
    /*Skip the LWS*/
    pStartPt = SipSkipFwLWS(pTempPre, pEndPt);
    pTempPre = SIP_NULL;

    while (pStartPt < pEndPt)
    {
        SIP_CHAR* pTempNext = SIP_NULL;

        if (SipFindActualPos(pStartPt, pEndPt, &pTempPre, &pTempNext, COMMA) == SIP_FALSE)
        {
            pTempPre = pEndPt;
            pTempNext = pEndPt;
        }

        SipNameValue* pNmVl = new SipNameValue();
        if (pNmVl == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory Allocation Fail", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNmVl->Decode(pStartPt, pTempPre) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Name Value decode Fail", SIP_ZERO, SIP_ZERO);
            pNmVl->SipDelete();
            return SIP_FALSE;
        }
        if (m_objAuthList.Add(pNmVl) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Adding in list fail", SIP_ZERO, SIP_ZERO);
            pNmVl->SipDelete();
            return SIP_FALSE;
        }
        /*Update the Start point to the start of next Name Value Pair*/
        pStartPt = pTempNext;
        pTempPre = SIP_NULL;
    }

    return SIP_TRUE;
}

SIP_BOOL SipAuthBase::IsValidHeader() const
{
    if ((GetValue() == SIP_NULL) || (m_objAuthList.IsEmpty() == SIP_TRUE))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Invalid header", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
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
