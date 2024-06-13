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
#include "msg/SipMsgUtil.h"
#include "msg/SipParameters.h"
#include "platform/SipString.h"

SipParameterList::SipParameterList() :
        m_objPrmList(SipVector<SipNameValue*>())
{
}

SipParameterList::SipParameterList(const SipParameterList& objPrmList) :
        m_objPrmList(SipVector<SipNameValue*>())
{
    SIP_UINT32 nSize = objPrmList.m_objPrmList.GetSize();

    for (SIP_UINT32 unCount = SIP_ZERO; unCount < nSize; unCount++)
    {
        SipNameValue* pElement = objPrmList.m_objPrmList.GetAt(unCount);

        if (pElement != SIP_NULL)
        {
            SipNameValue* pNameValue = new SipNameValue(*pElement);

            if (pNameValue != SIP_NULL)
            {
                m_objPrmList.Add(pNameValue);
            }
        }
    }
}

SipParameterList::~SipParameterList()
{
    while (m_objPrmList.IsEmpty() != SIP_TRUE)
    {
        m_objPrmList.Top()->SipDelete();
        m_objPrmList.Pop();
    }
}

SIP_BOOL SipParameterList::Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objPrmList.GetSize();
    SipNameValue* pNameValue = SIP_NULL;
    SIP_BOOL bFound = SIP_FALSE;
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nSize)
    {
        pNameValue = m_objPrmList.GetAt(nIndex);
        if (SipPf_Strcmp(pNameValue->m_pszName, pszName) == SIP_ZERO)
        {
            bFound = SIP_TRUE;
            break;
        }
        nIndex++;
    }

    if (bFound == SIP_FALSE)
    {
        pNameValue = new SipNameValue();
        if (pNameValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Add: Malloc Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
        pNameValue->m_pszName = SipPf_Strdup(pszName);
    }

    if (pszValue != SIP_NULL)
    {
        pNameValue->m_valueList.Add(SipPf_Strdup(pszValue));
    }

    if (bFound == SIP_FALSE)
    {
        m_objPrmList.Add(pNameValue);
    }

    return SIP_TRUE;
}

SIP_BOOL SipParameterList::Remove(const SIP_CHAR* pszName)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objPrmList.GetSize();

    SIP_BOOL bFound = SIP_FALSE;
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nSize)
    {
        SipNameValue* pNameValue = m_objPrmList.GetAt(nIndex);
        if (SipPf_Strcmp(pNameValue->m_pszName, pszName) == 0)
        {
            bFound = SIP_TRUE;
            break;
        }
        nIndex++;
    }

    if (bFound == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Remove: Param Not found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    m_objPrmList.GetAt(nIndex)->SipDelete();
    m_objPrmList.RemoveAt(nIndex);

    return SIP_TRUE;
}

SIP_BOOL SipParameterList::FindElement(
        const SIP_CHAR* pszName, SipNameValue*& pNameValue, SIP_UINT32& nPos)
{
    SIP_UINT32 nSize = m_objPrmList.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        SipNameValue* pElement = m_objPrmList.GetAt(nIndex);
        if (SipPf_Stricmp(pszName, pElement->m_pszName) == 0)
        {
            nPos = nIndex;
            pNameValue = pElement;
            return SIP_TRUE;
        }
    }
    return SIP_FALSE;
}

SIP_BOOL SipParameterList::SetParamValue(
        const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_UINT32 nTempPos = SIP_ZERO;

    /*Check whether parameter already exists or not*/
    SIP_BOOL bStatus = FindElement(pszName, pNameValue, nTempPos);

    /*If parameter not found add new entry*/
    if ((bStatus == SIP_FALSE) || (pNameValue == SIP_NULL))
    {
        return Add(pszName, pszValue);
    }

    /*If parameter already exists, update value*/
    if (pNameValue->m_valueList.IsEmpty() != SIP_TRUE)
    {
        if (pNameValue->m_valueList.GetSize() <= nPos)
        {
            return SIP_FALSE;
        }

        /*If the new value is NULL, remove the existing value*/
        if ((pszValue == SIP_NULL) || (SipPf_Strlen(pszValue) == SIP_ZERO))
        {
            delete pNameValue->m_valueList.GetAt(nPos);
            pNameValue->m_valueList.RemoveAt(nPos);
        }
        else
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszValue);
            delete pNameValue->m_valueList.GetAt(nPos);
            pNameValue->m_valueList.RemoveAt(nPos);
            pNameValue->m_valueList.InsertAt(pszVal, nPos);
        }
    }
    else
    {
        /*If parameter already not exists, add value*/
        if ((pszValue != SIP_NULL) && (SipPf_Strlen(pszValue) != SIP_ZERO))
        {
            SIP_CHAR* pszVal = SipPf_Strdup(pszValue);
            pNameValue->m_valueList.InsertAt(pszVal, SIP_ZERO);
        }
    }

    return SIP_TRUE;
}

SIP_CHAR* SipParameterList::GetParamValue(
        const SIP_CHAR* pszName, SIP_UINT32 nPos /*default value is zero*/)
{
    if (pszName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_UINT32 nPos1 = SIP_ZERO;

    SIP_BOOL bStatus = FindElement(pszName, pNameValue, nPos1);
    if ((bStatus == SIP_FALSE) || (pNameValue == SIP_NULL))
    {
        return SIP_NULL;
    }

    if ((pNameValue->m_valueList.IsEmpty() == SIP_TRUE) ||
            (pNameValue->m_valueList.GetSize() < nPos))
    {
        return SIP_NULL;
    }

    SIP_CHAR* pszElement = pNameValue->m_valueList.GetAt(nPos);

    return (pszElement != SIP_NULL) ? SipPf_Strdup(pszElement) : SIP_NULL;
}

SIP_BOOL SipParameterList::IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipNameValue* pNameValue = SIP_NULL;

    return FindElement(pszName, pNameValue, *pnPos);
}

SipNameValue* SipParameterList::GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_BOOL bStatus = FindElement(pszName, pNameValue, *pnPos);
    if ((bStatus == SIP_FALSE) || (pNameValue == SIP_NULL))
    {
        return SIP_NULL;
    }

    return pNameValue;
}

SIP_BOOL SipParameterList::Encode(AStringBuffer& objBuffer, SIP_CHAR cDelimiter,
        IParameterComponent* pParameterComponent /*= SIP_NULL*/) const
{
    SIP_UINT32 nSize = m_objPrmList.GetSize();
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nSize)
    {
        SipNameValue* pNameValue = m_objPrmList.GetAt(nIndex);

        objBuffer += cDelimiter;

        if (pNameValue->Encode(objBuffer, pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeList: Encoding parameter is failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        nIndex++;
    }

    return SIP_TRUE;
}

SIP_BOOL SipParameterList::Encode(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter,
        IParameterComponent* pParameterComponent /*= SIP_NULL*/) const
{
    SIP_UINT32 nCount = m_objPrmList.GetSize();
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nCount)
    {
        SipNameValue* pNameValue = m_objPrmList.GetAt(nIndex);

        *(*ppCurrPos) = cDelimiter;
        (*ppCurrPos)++;

        if (pNameValue->Encode(ppCurrPos, pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "EncodeList: Encoding parameter is failed",
                    SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        nIndex++;
    }
    return SIP_TRUE;
}

SIP_BOOL SipParameterList::Decode(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR cDelimiter,
        IParameterComponent* pParameterComponent)
{
    if ((pStartPt > pEndPt) || ((pStartPt == pEndPt) && (*pStartPt == '\0')))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode: No Value Present", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    while (pStartPt <= pEndPt)
    {
        SIP_CHAR* pTempPos = SIP_NULL;
        SIP_CHAR* pTempNext = SIP_NULL;

        if (SipFindActualPos(pStartPt, pEndPt, &pTempPos, &pTempNext, cDelimiter) == SIP_FALSE)
        {
            pTempPos = pEndPt;
        }

        SipNameValue* pNameValue = new SipNameValue();
        if (pNameValue == SIP_NULL)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }

        if (pNameValue->Decode(pStartPt, pTempPos, pParameterComponent) == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode: Name Val Decode fail", SIP_ZERO, SIP_ZERO);
            pNameValue->SipDelete();
            return SIP_FALSE;
        }

        if (m_objPrmList.Add(pNameValue) < SIP_ZERO)
        {
            SIP_DEBUG_WARNING(
                    ESIPTRACE_MODDECODER, "Decode: Append in list Failed", SIP_ZERO, SIP_ZERO);
            pNameValue->SipDelete();
            return SIP_FALSE;
        }

        if (pTempPos == pEndPt)
        {
            return SIP_TRUE;
        }

        pStartPt = pTempNext;

        if (pStartPt > pEndPt)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode: No Value Present", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    return SIP_TRUE;
}

SipNameValue::SipNameValue() :
        m_pszName(SIP_NULL),
        m_valueList(SipVector<SIP_CHAR*>()),
        m_ePrmType(SipParameters::GENERIC),
        m_Sep(',')
{
}

SipNameValue::SipNameValue(const SipNameValue& objNmVl) :
        m_pszName(SipPf_Strdup(objNmVl.m_pszName)),
        m_valueList(SipVector<SIP_CHAR*>()),
        m_ePrmType(objNmVl.m_ePrmType),
        m_Sep(objNmVl.m_Sep)
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

SipNameValue::~SipNameValue()
{
    if (m_pszName != SIP_NULL)
    {
        delete[] m_pszName;
    }
    while (m_valueList.IsEmpty() != SIP_TRUE)
    {
        delete m_valueList.Top();
        m_valueList.Pop();
    }
}

SIP_BOOL SipNameValue::Encode(
        AStringBuffer& objBuffer, IParameterComponent* pParameterComponent /*= SIP_NULL*/) const
{
    if (m_pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    objBuffer += m_pszName;

    if (m_valueList.IsEmpty() != SIP_TRUE)
    {
        objBuffer += EQUAL;

        if ((pParameterComponent != SIP_NULL) &&
                ((pParameterComponent->GetComponentType() == IParameterComponent::HEADER) ||
                        ((pParameterComponent->GetComponentType() == IParameterComponent::URI) &&
                                (pParameterComponent->IsValidComponent(m_pszName) == SIP_TRUE))))
        {
            SIP_CHAR* pszValue = m_valueList.GetAt(SIP_ZERO);
            SIP_CHAR* pszTempValue = SipPercentEncoding::DoPerEnc_Param(m_pszName, pszValue);
            objBuffer += pszTempValue;
            delete[] pszTempValue;
        }
        else
        {
            if (m_ePrmType == SipParameters::FEATURE)
            {
                objBuffer += DQUOTE;
            }

            SIP_UINT32 nSize = m_valueList.GetSize();
            SIP_UINT32 nIndex = SIP_ZERO;

            while (nIndex < nSize)
            {
                const SIP_CHAR* pszValue = m_valueList.GetAt(nIndex);

                if (pszValue == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(
                            ESIPTRACE_MODENCODER, "Encode: Value is null", SIP_ZERO, SIP_ZERO);
                    return SIP_FALSE;
                }

                objBuffer += pszValue;

                // Condition to prevent last put of separator
                if (nIndex < (nSize - SIP_ONE))
                {
                    objBuffer += m_Sep;
                }

                nIndex++;
            }

            if (m_ePrmType == SipParameters::FEATURE)
            {
                objBuffer += DQUOTE;
            }
        }
    }
    return SIP_TRUE;
}

SIP_BOOL SipNameValue::Encode(
        SIP_CHAR** ppCurrPos, IParameterComponent* pParameterComponent /*= SIP_NULL*/) const
{
    if (m_pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipPf_Strcpy(*ppCurrPos, m_pszName);
    SipEnc_UpdateCurrPos(ppCurrPos);

    if (m_valueList.IsEmpty() != SIP_TRUE)
    {
        **ppCurrPos = EQUAL;
        (*ppCurrPos)++;

        if ((pParameterComponent != SIP_NULL) &&
                ((pParameterComponent->GetComponentType() == IParameterComponent::HEADER) ||
                        ((pParameterComponent->GetComponentType() == IParameterComponent::URI) &&
                                (pParameterComponent->IsValidComponent(m_pszName) == SIP_TRUE))))
        {
            SIP_CHAR* pszValue = m_valueList.GetAt(SIP_ZERO);
            SIP_CHAR* pszTempValue = SipPercentEncoding::DoPerEnc_Param(m_pszName, pszValue);
            SipPf_Strcpy(*ppCurrPos, pszTempValue);
            SipEnc_UpdateCurrPos(ppCurrPos);
            delete[] pszTempValue;
        }
        else
        {
            if (m_ePrmType == SipParameters::FEATURE)
            {
                **ppCurrPos = DQUOTE;
                (*ppCurrPos)++;
            }

            SIP_UINT32 nCount = m_valueList.GetSize();
            SIP_UINT32 sLocalCount = SIP_ZERO;

            while (sLocalCount < nCount)
            {
                SIP_CHAR* pszVal = m_valueList.GetAt(sLocalCount);
                if (pszVal == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(
                            ESIPTRACE_MODENCODER, "Encode: null value", SIP_ZERO, SIP_ZERO);
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
    }
    return SIP_TRUE;
}

SIP_BOOL SipNameValue::Decode(
        SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, IParameterComponent* pParameterComponent)
{
    SIP_CHAR* pTempPos = SIP_NULL;
    SIP_CHAR* pTempNext = SIP_NULL;

    if (SipFindActualPos(pStartPt, pEndPt, &pTempPos, &pTempNext, EQUAL) == SIP_FALSE)
    {
        pTempPos = pEndPt;
    }

    m_pszName = SipCreateString(pStartPt, pTempPos);
    if (m_pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                "SipNameValue::DecUriNameVal: Memory Allocation Failed", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    if (pTempPos != pEndPt)
    {
        /*Update the pTempPos to the start of Value List*/
        SIP_CHAR* pszValuePtr = pTempNext;

        if ((pParameterComponent != SIP_NULL) &&
                ((pParameterComponent->GetComponentType() == IParameterComponent::HEADER) ||
                        ((pParameterComponent->GetComponentType() == IParameterComponent::URI) &&
                                (pParameterComponent->IsValidComponent(m_pszName) == SIP_TRUE))))
        {
            SIP_CHAR* pszValue = SipCreateString(pszValuePtr, pEndPt);
            if (pszValue == SIP_NULL)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipNameValue::DecUriNameVal: Memory Allocation Failed", SIP_ZERO,
                        SIP_ZERO);
                return SIP_FALSE;
            }

            SIP_CHAR* pszTempValue = SIP_NULL;

            pszTempValue = SipPercentEncoding::DoPercentDecoding(pszValue);

            /*put the value in the value list*/
            if (m_valueList.Add(pszTempValue) < SIP_ZERO)
            {
                SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                        "SipNameValue::DecUriNameVal:Adding in list failed", SIP_ZERO, SIP_ZERO);
                delete[] pszTempValue;
                return SIP_FALSE;
            }
        }
        else
        {
            while (pszValuePtr <= pEndPt)
            {
                SIP_CHAR* pszValue = SIP_NULL;
                pTempPos = SIP_NULL;

                if (SipFindPreDelimiter(pszValuePtr, pEndPt, &pTempPos, COMMA) == SIP_FALSE)
                {
                    pTempPos = pEndPt;
                }

                pszValue = SipCreateString(pszValuePtr, pTempPos);
                if (pszValue == SIP_NULL)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "SipNameValue::DecHdrNameVal: Memory Allocation Failed", SIP_ZERO,
                            SIP_ZERO);
                    return SIP_FALSE;
                }
                /*put the value in the value list*/
                if (m_valueList.Add(pszValue) < SIP_ZERO)
                {
                    SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER,
                            "SipNameValue::DecHdrNameVal:Adding in list failed", SIP_ZERO,
                            SIP_ZERO);
                    delete[] pszValue;
                    return SIP_FALSE;
                }

                if (pTempPos == pEndPt)
                {
                    return SIP_TRUE;
                }

                pszValuePtr = pTempPos + SIP_TWO;
            }
        }
    }

    return SIP_TRUE;
}

SipParameters::SipParameters() {}

SipParameters::SipParameters(const SipParameters& objParameters) :
        m_objParameterList(objParameters.m_objParameterList)
{
}

SipParameters::~SipParameters() {}

SIP_BOOL SipParameters::AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    return m_objParameterList.Add(pszName, pszValue);
}

SIP_BOOL SipParameters::RemoveParam(const SIP_CHAR* pszName)
{
    return m_objParameterList.Remove(pszName);
}

SIP_BOOL SipParameters::SetParamValue(
        const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos)
{
    return m_objParameterList.SetParamValue(pszName, pszValue, nPos);
}

SIP_BOOL SipParameters::IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    return m_objParameterList.IsParamExists(pszName, pnPos);
}

SipParameterList& SipParameters::GetParameterList()
{
    return m_objParameterList;
}

SIP_CHAR* SipParameters::GetParamValue(
        const SIP_CHAR* pszName, SIP_UINT32 nPos /*default value is zero*/)
{
    return m_objParameterList.GetParamValue(pszName, nPos);
}

SipNameValue* SipParameters::GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos)
{
    return m_objParameterList.GetParamNode(pszName, pnPos);
}
