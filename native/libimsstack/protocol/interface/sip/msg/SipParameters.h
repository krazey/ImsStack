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
    SipVector<SIP_CHAR*> m_valueList;
    SIP_INT32 m_ePrmType;
    SIP_CHAR m_Sep;

    SipNameValue();
    explicit SipNameValue(SIP_INT32 eHdrType);
    SipNameValue(const SipNameValue& objNmVl);

    SIP_BOOL Encode(
            AStringBuffer& objBuffer, IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Encode(
            SIP_CHAR** ppCurrPos, IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Decode(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt,
            IParameterComponent* pParameterComponent = SIP_NULL);

private:
    virtual ~SipNameValue();
};

class SipParameterList : public SipRefBase
{
private:
    SipVector<SipNameValue*> m_objPrmList;

public:
    SipParameterList();
    explicit SipParameterList(SIP_INT32 eHdrType);
    SipParameterList(const SipParameterList& objPrmList);
    ~SipParameterList();

    SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL) const;

    SIP_BOOL Decode(SIP_CHAR* pStartPt, SIP_CHAR* pEndPt, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL);

    SIP_BOOL Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue = SIP_NULL);

    SIP_BOOL Remove(const SIP_CHAR* pszName);

    SIP_BOOL FindElement(const SIP_CHAR* pszName, SipNameValue*& pNameValue, SIP_UINT32& nPos);

    SIP_BOOL IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pPos);

    SIP_BOOL SetParamValue(
            const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos = SIP_ZERO);

    inline SipVector<SipNameValue*>& GetList() { return m_objPrmList; }

    inline SIP_UINT32 GetCount() const { return m_objPrmList.GetSize(); }

    inline SipNameValue* GetNameValNode(SIP_UINT32 iIndex) const
    {
        return (iIndex < m_objPrmList.GetSize()) ? m_objPrmList.GetAt(iIndex) : SIP_NULL;
    }

    SIP_CHAR* GetParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO);
    SipNameValue* GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos);
};

class SipParameters
{
public:
    /*Enumeration for Prm Type*/
    enum
    {
        GENERIC,
        FEATURE,
        INVALID = SIP_INVALID
    };

    SipParameterList m_objParameterList;

    SipParameters();
    SipParameters(const SipParameters& objParameters);
    ~SipParameters();

    SIP_BOOL AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue = SIP_NULL);

    SIP_BOOL RemoveParam(const SIP_CHAR* pszName);

    SIP_BOOL IsParamExists(const SIP_CHAR* pszName, SIP_UINT32* pnPos);

    SIP_BOOL SetParamValue(
            const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos = SIP_ZERO);

    inline SIP_UINT32 GetParamCount() const { return m_objParameterList.GetCount(); }

    SipParameterList& GetParameterList();

    SIP_CHAR* GetParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO);

    SipNameValue* GetParamNode(const SIP_CHAR* pszName, SIP_UINT32* pnPos);
};

#endif  //__SIP_PARAMETERS_H__
