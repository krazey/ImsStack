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
#ifndef AOS_IPSEC_HELPER_H_
#define AOS_IPSEC_HELPER_H_

#include "registration/AosIpsec.h"

class IRegContact;
class IRegParameter;
class IAosAppContext;

/**
 * @brief This class manages related ipsec information from AosRegistration.
 */
class AosIpsecHelper : public IAosIpsecListener
{
public:
    AosIpsecHelper(IN IRegContact* piRegContact, IN IRegParameter* piRegParameter,
            IN IAosAppContext* piAppContext, IN AString& strRegId);
    inline explicit AosIpsecHelper(){};
    ~AosIpsecHelper() override;

private:
    AosIpsecHelper(IN const AosIpsecHelper& objRhs);
    AosIpsecHelper& operator=(IN const AosIpsecHelper& objRhs);

public:
    enum class UePortType
    {
        CLIENT_PORT,
        SERVER_PORT
    };

    virtual IMS_BOOL Create(IN IMS_BOOL bInitial);
    virtual void CreateOnChallenging();

    virtual IMS_BOOL SetPcscfPortnSpi();
    virtual IMS_BOOL IsPcscfServerPortDifferent();
    virtual IMS_BOOL UpdatePreloadedRoute(IN const AString& strPcscf);

    virtual IMS_BOOL MakeSas(IN const AString& strPcscf, IN const IpAddress& objIpa,
            IN const ByteArray& objIk, IN const ByteArray& objCk);

    virtual IMS_BOOL ProcessAuthChallenged(IN IMS_SINT32 nAlgorithm);
    virtual void ProcessRegStarted();
    virtual IMS_BOOL ProcessRegUpdated();

    virtual void InitIpsec();
    virtual IMS_BOOL IsEstablished();
    virtual void SetSecurityServerPortInRegContact();

    virtual void IgnoreCurrentPolicyExpired();

protected:
    virtual void SetSecurityServerPortInRegistration();
    virtual IMS_UINT32 FindAvailableUePort(IN UePortType ePortType, OUT IMS_UINT32& nLastPort);
    virtual void SetUePortnSpi(IN IMS_BOOL bInitial);
    virtual IMS_BOOL SetSecurityClientHeader();
    virtual IMS_BOOL CheckSecurityServerHeader();
    virtual void ProcessPolicyExpired(IN AosIpsec* pIpsec);

    // IAosIpsecListener
    void IPSecPolicyExpired(IN AosIpsec* pIpsec) override;

    void Destroy();

private:
    void CloseUnsecureTCPSocket();
    void CloseSecureTCPSocket(IN AosIpsec* pIpsec);
    IMS_UINT32 GetValidUePort(
            IN IMS_UINT32 nLowerPort, IN IMS_UINT32 nUpperPort, OUT IMS_UINT32& nUePort);
    void DeleteSamePolicy();

protected:
    IRegContact* m_piRegContact;
    IRegParameter* m_piRegParameter;
    IAosAppContext* m_piContext;
    AString m_strRegId;

    AosIpsec* m_pOldIpsec;
    AosIpsec* m_pCurrIpsec;
    AosIpsec* m_pNewIpsec;
    UeIpsecInfo* m_pUeIpsecInfo;

    AString m_strTag;

    static const IMS_UINT32 IPSEC_UPDATE_GUARD_LIFE_TIME_MILLIS = 30000;
    static const IMS_UINT32 IPSEC_PORT_RANGE_MAX_COUNT = 20;
};
#endif  // AOS_IPSEC_HELPER_H_
