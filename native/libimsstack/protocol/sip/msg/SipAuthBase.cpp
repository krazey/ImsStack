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
        SipNameValue* pTempNameValue = objHeader.m_objAuthList.GetAt(nCount);

        if (pTempNameValue != SIP_NULL)
        {
            SipNameValue* pNameValue = new SipNameValue(*pTempNameValue);
            m_objAuthList.Add(pNameValue);
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

SIP_BOOL SipAuthBase::Encode(SIP_CHAR** ppCurrPos, SIP_BOOL /*bParams = SIP_TRUE*/)
{
    if (IsValidHeader() == SIP_FALSE)
    {
        return SIP_FALSE;
    }

    SipAbnfUtil::Append(*ppCurrPos, GetValue());

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
    SipNameValue* pNameValue = new SipNameValue();

    if (pNameValue == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (bIsFeatureParam == SIP_TRUE)
    {
        pNameValue->m_eParamType = SipParameters::FEATURE;
    }

    pNameValue->m_pszName = SipPf_Strdup(pszName);
    if (pNameValue->m_pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        pNameValue->SipDelete();
        return SIP_FALSE;
    }

    SIP_CHAR* pszTempVal = SipPf_Strdup(pszVal);
    if (pszTempVal == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Memory allocation Fail", SIP_ZERO, SIP_ZERO);
        pNameValue->SipDelete();
        return SIP_FALSE;
    }

    pNameValue->m_objValueList.Add(pszTempVal);
    m_objAuthList.Add(pNameValue);

    return SIP_TRUE;
}

SIP_BOOL SipAuthBase::FindElement(
        const SIP_CHAR* pszName, SipNameValue*& pNameValue, SIP_UINT32& nPos)
{
    SIP_UINT32 nSize = m_objAuthList.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        SipNameValue* pTempNameValue = m_objAuthList.GetAt(nIndex);
        if (SipPf_Stricmp(pszName, pTempNameValue->m_pszName) == 0)
        {
            nPos = nIndex;
            pNameValue = pTempNameValue;
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

    SipNameValue* pTempNameValue = SIP_NULL;
    SIP_UINT32 nPos = SIP_ZERO;
    SIP_BOOL bStatus = FindElement(pszName, pTempNameValue, nPos);
    if ((bStatus == SIP_FALSE) || (pTempNameValue == SIP_NULL))
    {
        return SIP_NULL;
    }

    const SipVector<SIP_CHAR*>& valueList = pTempNameValue->m_objValueList;
    if (valueList.IsEmpty() == SIP_TRUE)
    {
        return SIP_NULL;
    }

    SIP_CHAR* pszVal = SIP_NULL;
    SIP_CHAR* pszElement = valueList.GetAt(SIP_ZERO);
    if (pTempNameValue->m_eParamType == SipParameters::FEATURE)
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

SIP_BOOL SipAuthBase::Decode(const SIP_CHAR* pStartPt, SIP_UINT32 nDecLen)
{
    if (nDecLen == SIP_ZERO)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Empty buffer", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    const SIP_CHAR* pEndPt = pStartPt + nDecLen - SIP_ONE;
    const SIP_CHAR* pTempPre = SIP_NULL;

    if (SipAbnfUtil::FindWhiteSpace(pStartPt, pEndPt, pTempPre) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "LWS not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_CHAR* pszScheme = SipAbnfUtil::CreateString(pStartPt, pTempPre);
    if (SetValue(pszScheme) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
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
    pStartPt = SipAbnfUtil::SkipWhiteSpaceFromLeft(pTempPre, pEndPt);
    pTempPre = SIP_NULL;

    while (pStartPt < pEndPt)
    {
        const SIP_CHAR* pTempNext = SIP_NULL;

        if (SipAbnfUtil::FindActualPosition(pStartPt, pEndPt, pTempPre, pTempNext, COMMA) ==
                SIP_FALSE)
        {
            pTempPre = pEndPt;
            pTempNext = pEndPt;
        }

        SipNameValue* pTempNameValue = new SipNameValue();
        if (pTempNameValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Memory allocation failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pTempNameValue->Decode(pStartPt, pTempPre) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Name value decode failed", SIP_ZERO, SIP_ZERO);
            pTempNameValue->SipDelete();
            return SIP_FALSE;
        }
        if (m_objAuthList.Add(pTempNameValue) < 0)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Adding in list failed", SIP_ZERO, SIP_ZERO);
            pTempNameValue->SipDelete();
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
