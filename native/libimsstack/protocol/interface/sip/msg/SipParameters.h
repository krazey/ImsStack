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

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
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

    SIP_BOOL Decode(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt, const SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL);

    SIP_BOOL Add(const SIP_CHAR* pszName, const SIP_CHAR* pszValue = SIP_NULL);

    SIP_VOID RemoveParam(const SIP_CHAR* pszName);

    SIP_BOOL FindElement(const SIP_CHAR* pszName, SipNameValue*& pNameValue, SIP_INT32& nPos) const;

    SIP_BOOL IsParamPresent(const SIP_CHAR* pszName) const;

    SIP_INT32 GetParamIndex(const SIP_CHAR* pszName) const;

    SIP_BOOL SetParam(
            const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos = SIP_ZERO);

    inline SipVector<SipNameValue*>& GetList() { return m_objPrmList; }

    inline SIP_UINT32 GetCount() const { return m_objPrmList.GetSize(); }

    inline SipNameValue* GetParam(SIP_UINT32 nPos) const
    {
        return (nPos < m_objPrmList.GetSize()) ? m_objPrmList.GetAt(nPos) : SIP_NULL;
    }

    SIP_CHAR* GetParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO) const;
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

    inline SIP_BOOL Encode(AStringBuffer& objBuffer, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL) const
    {
        return m_objParameterList.Encode(objBuffer, cDelimiter, pParameterComponent);
    }

    inline SIP_BOOL Encode(SIP_CHAR** ppCurrPos, SIP_CHAR cDelimiter,
            IParameterComponent* pParameterComponent = SIP_NULL) const
    {
        return m_objParameterList.Encode(ppCurrPos, cDelimiter, pParameterComponent);
    }

    inline SIP_BOOL Decode(const SIP_CHAR* pStartPt, const SIP_CHAR* pEndPt,
            const SIP_CHAR cDelimiter, IParameterComponent* pParameterComponent = SIP_NULL)
    {
        return m_objParameterList.Decode(pStartPt, pEndPt, cDelimiter, pParameterComponent);
    }

    SIP_BOOL AddParam(const SIP_CHAR* pszName, const SIP_CHAR* pszValue = SIP_NULL);

    SIP_VOID RemoveParam(const SIP_CHAR* pszName);

    SIP_BOOL IsParamPresent(const SIP_CHAR* pszName) const;

    inline SIP_INT32 GetParamIndex(const SIP_CHAR* pszName) const
    {
        return m_objParameterList.GetParamIndex(pszName);
    }

    SIP_BOOL SetParam(
            const SIP_CHAR* pszName, const SIP_CHAR* pszValue, SIP_UINT32 nPos = SIP_ZERO);

    inline SIP_UINT32 GetParamCount() const { return m_objParameterList.GetCount(); }

    SipParameterList& GetParameterList();

    SIP_CHAR* GetParamValue(const SIP_CHAR* pszName, SIP_UINT32 nPos = SIP_ZERO) const;

    inline SipNameValue* GetParam(SIP_UINT32 nPos) const
    {
        return m_objParameterList.GetParam(nPos);
    }
};

#endif  //__SIP_PARAMETERS_H__
