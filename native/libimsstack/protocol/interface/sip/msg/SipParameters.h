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
#ifndef __SIP_PARAMETERS_H__
#define __SIP_PARAMETERS_H__

#include "AStringBuffer.h"
#include "SipPercentEncoding.h"
#include "SipRefBase.h"
#include "SipVector.h"
#include "msg/IParameterComponent.h"

class SipNameValue : public SipRefBase
{
public:
    SIP_CHAR* m_pszName;
    SipVector<SIP_CHAR*> m_objValueList;
    SIP_INT32 m_eParamType;
    SIP_CHAR m_Separator;

    SipNameValue();
    explicit SipNameValue(SIP_INT32 eHdrType);
    SipNameValue(const SipNameValue& objNameValue);

    SIP_BOOL Encode(
            AStringBuffer& objBuffer, IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Encode(
            SIP_CHAR** ppCurrPos, IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
            IParameterComponent* pParameterComponent = SIP_NULL);

private:
    virtual ~SipNameValue();
};

class SipParameters
{
public:
    /* Enumeration for parameter type */
    enum
    {
        GENERIC,
        FEATURE,
        INVALID = SIP_INVALID
    };

    SipVector<SipNameValue*> m_objNameValueList;

    SipParameters();
    SipParameters(const SipParameters& objParameters);
    ~SipParameters();

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL);

    SIP_BOOL AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue = SIP_NULL);

    SIP_VOID RemoveParam(const SIP_CHAR* pszName);

    SIP_BOOL FindElement(const SIP_CHAR* pszName, SipNameValue*& pNameValue, SIP_INT32& nPos) const;

    SIP_BOOL IsParamPresent(const SIP_CHAR* pszName) const;

    SIP_INT32 GetParamIndex(const SIP_CHAR* pszName) const;

    SIP_BOOL SetParam(
            const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos = SIP_ZERO);

    inline SIP_UINT32 GetParamCount() const { return m_objNameValueList.GetSize(); }

    SIP_CHAR* GetParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO) const;

    inline SipNameValue* GetParam(SIP_UINT32 nPos) const
    {
        return (nPos < m_objNameValueList.GetSize()) ? m_objNameValueList.GetAt(nPos) : SIP_NULL;
    }
};

#endif  //__SIP_PARAMETERS_H__
