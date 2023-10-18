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
#ifndef SIP_HEADER_H_
#define SIP_HEADER_H_

#include "ISipHeader.h"
#include "msg/SipHeaderBase.h"

class SipParameter;

/**
 * @brief This class provides generic SIP header parser helper.
 *
 * It can be used to parse base string header values that are read from SIP message
 * using e.g. SipConnection::GetHeader() method.
 * It uses generic format to parse the header value and parameters following the syntax given
 * in RFC 3261.
 *
 * - field-name: field-value *(;parameter-name=parameter-value)
 * - auth-header-name: auth-scheme LWS auth-param *(COMMA auth-param)
 */
class SipHeader : public ISipHeader
{
public:
    SipHeader();
    explicit SipHeader(IN IMS_SINT32 nType);
    explicit SipHeader(IN const AString& strName);
    explicit SipHeader(IN const SipHeaderBase* pSipHdr);
    virtual ~SipHeader();

    SipHeader(IN const SipHeader&) = delete;
    // To ignore an assignment operator of object
    SipHeader& operator=(IN const SipHeader&) = delete;

public:
    // ISipObject interface
    void Destroy() override;
    // ISipHeader interface
    ISipHeader* Clone() const override;
    IMS_BOOL Equals(IN const ISipHeader* piHeader) const override;
    inline const SipAddress* GetSipAddress() const override { return m_pAddress; }
    AString GetHeaderValue() const override;
    inline const AString& GetName() const override { return m_strName; }
    const SipParameter* GetParameter(IN const AString& strName) const override;
    IMS_RESULT GetParameterNames(OUT ImsList<AString>& objParamNames) const override;
    inline const ImsList<SipParameter*>& GetParameters() const override { return m_objParams; }
    inline IMS_SINT32 GetType() const override { return m_nType; }
    inline const AString& GetValue() const override { return m_strBody; }
    IMS_SINT32 GetValueInt() const override;
    void RemoveParameter(IN const AString& strName) override;
    void SetName(IN const AString& strName_) override;
    IMS_RESULT SetParameter(IN const AString& strName, IN const AString& strValue) override;
    inline IMS_RESULT SetHeaderValue(IN const AString& strHeaderValue) override
    {
        return Decode(strHeaderValue) ? IMS_SUCCESS : IMS_FAILURE;
    }
    inline IMS_RESULT SetValue(IN const AString& strValue) override
    {
        return Decode(strValue, IMS_FALSE) ? IMS_SUCCESS : IMS_FAILURE;
    }
    IMS_RESULT SetValueInt(IN IMS_SINT32 nValue) override;
    AString ToString() const override;
    AString ToStringWithoutName() const override;

    static IMS_BOOL IsValidType(IN IMS_SINT32 nType);

private:
    IMS_BOOL Decode(IN const AString& strBody, IN IMS_BOOL bParseParameter = IMS_TRUE);
    IMS_BOOL ParseUnknownBody(IN const AString& strBody);
    static IMS_BOOL IsHeaderBodyDigitFormat(IN IMS_SINT32 nType);

public:
    static const IMS_CHAR* NAME[];

private:
    IMS_SINT32 m_nType;
    AString m_strName;
    AString m_strBody;
    // This field is not NULL if the header type can have an URI format body
    SipAddress* m_pAddress;
    ImsList<SipParameter*> m_objParams;
};

#endif
