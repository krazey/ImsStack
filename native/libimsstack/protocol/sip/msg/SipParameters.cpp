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

SipNameValue::SipNameValue() :
        m_pszName(SIP_NULL),
        m_valueList(SipVector<SIP_CHAR*>()),
        m_eParamType(SipParameters::GENERIC),
        m_Separator(',')
{
}

SipNameValue::SipNameValue(const SipNameValue& objNameValue) :
        m_pszName(SipPf_Strdup(objNameValue.m_pszName)),
        m_valueList(SipVector<SIP_CHAR*>()),
        m_eParamType(objNameValue.m_eParamType),
        m_Separator(objNameValue.m_Separator)
{
    SIP_UINT32 nSize = objNameValue.m_valueList.GetSize();

    for (SIP_UINT32 unCount = SIP_ZERO; unCount < nSize; unCount++)
    {
        SIP_CHAR* pszElement = objNameValue.m_valueList.GetAt(unCount);

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
            SIP_CHAR* pszTempValue =
                    SipPercentEncoding::DoPercentEncoding_Param(m_pszName, pszValue);
            objBuffer += pszTempValue;
            delete[] pszTempValue;
        }
        else
        {
            if (m_eParamType == SipParameters::FEATURE)
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
                    objBuffer += m_Separator;
                }

                nIndex++;
            }

            if (m_eParamType == SipParameters::FEATURE)
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
            SIP_CHAR* pszTempValue =
                    SipPercentEncoding::DoPercentEncoding_Param(m_pszName, pszValue);
            SipPf_Strcpy(*ppCurrPos, pszTempValue);
            SipEnc_UpdateCurrPos(ppCurrPos);
            delete[] pszTempValue;
        }
        else
        {
            if (m_eParamType == SipParameters::FEATURE)
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
                    **ppCurrPos = m_Separator;
                    (*ppCurrPos)++;
                }
                sLocalCount++;
            }

            if (m_eParamType == SipParameters::FEATURE)
            {
                **ppCurrPos = DQUOTE;
                (*ppCurrPos)++;
            }
        }
    }
    return SIP_TRUE;
}

SIP_BOOL SipNameValue::Decode(
        const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, IParameterComponent* pParameterComponent)
{
    const SIP_CHAR* pTempPos = SIP_NULL;
    const SIP_CHAR* pTempNext = SIP_NULL;

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
        const SIP_CHAR* pszValuePtr = pTempNext;

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

SipParameters::SipParameters() :
        m_objNameValueList(SipVector<SipNameValue*>())
{
}

SipParameters::SipParameters(const SipParameters& objParameters) :
        m_objNameValueList(SipVector<SipNameValue*>())
{
    SIP_UINT32 nSize = objParameters.m_objNameValueList.GetSize();

    for (SIP_UINT32 unCount = SIP_ZERO; unCount < nSize; unCount++)
    {
        SipNameValue* pElement = objParameters.m_objNameValueList.GetAt(unCount);

        if (pElement != SIP_NULL)
        {
            SipNameValue* pNameValue = new SipNameValue(*pElement);

            if (pNameValue != SIP_NULL)
            {
                m_objNameValueList.Add(pNameValue);
            }
        }
    }
}

SipParameters::~SipParameters() {}

SIP_BOOL SipParameters::Encode(AStringBuffer& objBuffer, SIP_CHAR cDelimiter,
        IParameterComponent* pParameterComponent) const
{
    SIP_UINT32 nSize = m_objNameValueList.GetSize();
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nSize)
    {
        SipNameValue* pNameValue = m_objNameValueList.GetAt(nIndex);

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

SIP_BOOL SipParameters::Encode(
        SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent) const
{
    SIP_UINT32 nCount = m_objNameValueList.GetSize();
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nCount)
    {
        SipNameValue* pNameValue = m_objNameValueList.GetAt(nIndex);

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

SIP_BOOL SipParameters::Decode(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
        const SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent)
{
    if ((pStartPt > pEndPt) || ((pStartPt == pEndPt) && (*pStartPt == '\0')))
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODDECODER, "Decode: No Value Present", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    while (pStartPt <= pEndPt)
    {
        const SIP_CHAR* pTempPos = SIP_NULL;
        const SIP_CHAR* pTempNext = SIP_NULL;

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

        if (m_objNameValueList.Add(pNameValue) < SIP_ZERO)
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

SIP_BOOL SipParameters::AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    SIP_UINT32 nSize = m_objNameValueList.GetSize();
    SipNameValue* pNameValue = SIP_NULL;
    SIP_BOOL bFound = SIP_FALSE;
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nSize)
    {
        pNameValue = m_objNameValueList.GetAt(nIndex);
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
        m_objNameValueList.Add(pNameValue);
    }

    return SIP_TRUE;
}

SIP_VOID SipParameters::RemoveParam(const SIP_CHAR* pszName)
{
    if (pszName == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODENCODER, "Add: NULL param received", SIP_ZERO, SIP_ZERO);
        return;
    }

    SIP_UINT32 nSize = m_objNameValueList.GetSize();
    SIP_UINT32 nIndex = SIP_ZERO;

    while (nIndex < nSize)
    {
        SipNameValue* pNameValue = m_objNameValueList.GetAt(nIndex);
        if (SipPf_Strcmp(pNameValue->m_pszName, pszName) == 0)
        {
            pNameValue->SipDelete();
            m_objNameValueList.RemoveAt(nIndex);
            return;
        }
        nIndex++;
    }
}

SIP_BOOL SipParameters::SetParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos)
{
    if (pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_INT32 nTempPos = -1;

    /*If parameter not found add new entry*/
    if (FindElement(pszName, pNameValue, nTempPos) == SIP_FALSE)
    {
        return AddParam(pszName, pszValue);
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

SIP_BOOL SipParameters::FindElement(
        const SIP_CHAR* pszName, SipNameValue*& pNameValue, SIP_INT32& nPos) const
{
    SIP_UINT32 nSize = m_objNameValueList.GetSize();

    for (SIP_UINT32 nIndex = 0; nIndex < nSize; nIndex++)
    {
        SipNameValue* pElement = m_objNameValueList.GetAt(nIndex);
        if (SipPf_Stricmp(pszName, pElement->m_pszName) == 0)
        {
            nPos = nIndex;
            pNameValue = pElement;
            return SIP_TRUE;
        }
    }
    return SIP_FALSE;
}

SIP_BOOL SipParameters::IsParamPresent(const SIP_CHAR* pszName) const
{
    if (pszName == SIP_NULL)
    {
        return SIP_FALSE;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_INT32 nPos = -1;

    return FindElement(pszName, pNameValue, nPos);
}

SIP_CHAR* SipParameters::GetParamValue(
        const SIP_CHAR* pszName, SIP_UINT32 nPos /*= SIP_ZERO*/) const
{
    if (pszName == SIP_NULL)
    {
        return SIP_NULL;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_INT32 nPos1 = -1;

    if (FindElement(pszName, pNameValue, nPos1) == SIP_FALSE)
    {
        return SIP_NULL;
    }

    if (pNameValue->m_valueList.GetSize() <= nPos)
    {
        return SIP_NULL;
    }

    SIP_CHAR* pszElement = pNameValue->m_valueList.GetAt(nPos);

    return (pszElement != SIP_NULL) ? SipPf_Strdup(pszElement) : SIP_NULL;
}

SIP_INT32 SipParameters::GetParamIndex(const SIP_CHAR* pszName) const
{
    if (pszName == SIP_NULL)
    {
        return -1;
    }

    SipNameValue* pNameValue = SIP_NULL;
    SIP_INT32 nPos = -1;

    return FindElement(pszName, pNameValue, nPos) ? nPos : -1;
}
