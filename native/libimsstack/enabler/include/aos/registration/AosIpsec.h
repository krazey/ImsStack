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
#ifndef AOS_IPSEC_H_
#define AOS_IPSEC_H_

#include "IpAddress.h"
#include "ByteArray.h"
#include "SipSecurityHeader.h"
#include "IIpSecPolicyListener.h"

class INetworkIpSec;
class IIpSecPolicy;
class UeIpsecInfo;
class PcscfIpsecInfo;
class IAosIpsecListener;

/**
 * @brief This class provides ipsec information of UE
 */
class UeIpsecInfo
{
public:
    UeIpsecInfo() :
            nSpiC(0),
            nSpiS(0),
            nPortC(0),
            nPortS(0)
    {
    }
    ~UeIpsecInfo() {}

public:
    IMS_UINT32 nSpiC;
    IMS_UINT32 nSpiS;
    IMS_UINT32 nPortC;
    IMS_UINT32 nPortS;
    IpAddress objIpa;
    ByteArray objIk;
    ByteArray objCk;
};

/**
 * @brief This class provides ipsec information of pcscf
 */
class PcscfIpsecInfo
{
public:
    PcscfIpsecInfo() :
            nSpiC(0),
            nSpiS(0),
            nPortC(0),
            nPortS(0)
    {
    }
    ~PcscfIpsecInfo() {}

public:
    IMS_UINT32 nSpiC;
    IMS_UINT32 nSpiS;
    IMS_UINT32 nPortC;
    IMS_UINT32 nPortS;
    IpAddress objIpa;
};

/**
 * @brief This class manages the ipsec related data for making Security Associations(SAs)
 *        and Security Policies(SPs) in the kernel layer.
 */
class AosIpsec : public IIpSecPolicyListener
{
public:
    AosIpsec(IN IAosIpsecListener* piListener, IN IMS_SINT32 nSlotId);
    ~AosIpsec() override;

private:
    AosIpsec(IN const AosIpsec& objRhs);
    AosIpsec& operator=(IN const AosIpsec& objRhs);

public:
    enum
    {
        TYPE_CLIENT = 0,
        TYPE_SERVER
    };

    enum
    {
        DIRECTION_OUTBOUND = 0,
        DIRECTION_INBOUND
    };

    enum
    {
        TCP_CLIENT_OUTBOUND = 0,
        TCP_CLIENT_INBOUND,
        TCP_SERVER_OUTBOUND,
        TCP_SERVER_INBOUND
    };

    enum
    {
        SA_DIR_UC_PS = 0,
        SA_DIR_US_PC,
        SA_DIR_PC_US,
        SA_DIR_PS_UC,
    };

    /// IIpSecPolicyListener Interface
    void IpSecPolicy_OnSecurityAssociationExpired(IN IIpSecPolicy* piPolicy) override;

    /// Create UE Transport Port and SPI Identity
    IMS_UINT32 CreateUeSpi();

    /// Create SPs for TCP or UDP
    void CreateSps(IN IMS_UINT32 nType);
    /// Create SAs
    void CreateSas();

    /// Add Policy to IPSEC Libs
    IMS_BOOL AddPolicy();

    /// Set Sa Establishment
    void SetSaEstablished();

    /// Get Sa Established Info
    IMS_BOOL IsSaEstablished();

    /// Set Local and P-CSCF IP Address
    void SetIps(IN const IpAddress& objLocalIpa, IN const IpAddress& objPcscfIpa);

    /// Set Securtity Keys - IK, CK
    void SetKeys(IN const ByteArray& objAuthKey, IN const ByteArray& objEncrKey);

    /// Set Integrity Algorithm
    void SetSecurityAlgorithm(
            IN IMS_UINT32 nSecuAlog, IN IMS_UINT32 nAuthAlgo, IN IMS_UINT32 nEncrAlgo);

    /// Set UE Ports(C,S) and SPIs(C,S)
    void SetUePortsAndSpis(
            IN IMS_UINT32 nPortC, IN IMS_UINT32 nPortS, IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS);

    /// Set PCSCF Ports(C,S) and SPIs(C,S)
    void SetPcscfPortsAndSpis(
            IN IMS_UINT32 nPortC, IN IMS_UINT32 nPortS, IN IMS_UINT32 nSpiC, IN IMS_UINT32 nSpiS);

    /// Make Security Client Header
    void MakeSecurityClientH(IN SipSecurityHeader& objSecuH, IN IMS_BOOL bSpi3gpp = IMS_TRUE);

    /// Set lifetime of SAs
    void IgnorePolicyLifetime();
    void ManagePolicyLifetime(IN IMS_UINT32 nDuration);

    /// Get Policy Interface
    IIpSecPolicy* GetPolicy();

    /// Get Integrity & Encryption Algorithm
    IMS_UINT32 GetIntegrityAlgorithm();
    IMS_UINT32 GetAuthAlgoForIpsec(IN IMS_UINT32 nAlgo) const;
    IMS_UINT32 GetEncrAlgoForIpsec(IN IMS_UINT32 nAlgo) const;
    IMS_SINT32 GetAuthAlgoForSecurityHeader(IN IMS_SINT32 nAlgo) const;
    IMS_SINT32 GetEncrAlgoForSecurityHeader(IN IMS_SINT32 nAlgo) const;

    // Get Security Mode & Protocol Algorithm
    IMS_SINT32 GetModeForSecurityHeader(IN IMS_UINT32 nMode) const;
    IMS_UINT32 GetProtocolForIpsec(IN IMS_UINT32 nProtocol) const;
    IMS_SINT32 GetProtocolForSecurityHeader(IN IMS_UINT32 nProtocol) const;

    /// Get UE Port & Spi
    const IpAddress& GetUeIpa() const;
    IMS_UINT32 GetUePort(IN IMS_UINT32 nType);
    IMS_UINT32 GetUeSpi(IN IMS_UINT32 nType);

    /// Get P-CSCF Port & Spi
    const IpAddress& GetPcscfIpa() const;
    IMS_UINT32 GetPcscfPort(IN IMS_UINT32 nType);
    IMS_UINT32 GetPcscfSpi(IN IMS_UINT32 nType);

    /// Display SAs Dump
    void DumpSas();

private:
    /// Create SPs in Both Protocols
    void CreateSpforUdp(IN IMS_UINT32 nDir);
    void CreateSpforTcp(IN IMS_UINT32 nType);
    /// Create SAs
    void CreateSa(IN IMS_UINT32 nType);

public:
    // ePDG requires a certain range of UE ports. So here we made a change from 58001 to 38001.
    // Plain TCP can be transmitted from a port 40000 to 50000. Avoid this range for Ipsec.
    static const IMS_UINT32 UE_PORT_LOWER = 38001;
    static const IMS_UINT32 UE_PORT_UPPER = 39000;
    // UE Server Port 39000 ~ 39999
    static const IMS_UINT32 PCSCF_PORT_LOWER = 10001;
    static const IMS_UINT32 PCSCF_PORT_UPPER = 11000;
    // Pcscf Server Port 11001 ~ 12000

    // Server Port + 1000
    static const IMS_UINT32 PORTS_INTERVAL = 1000;

    /// Set the SPI to 10 digits.
    static const IMS_UINT32 SPI_MIN = 1000000000;
    /// Increase the SPI for sending REGISTER doing authentication
    static const IMS_UINT32 SPI_VALUE_TO_BE_INCREASED = 2;

protected:
    INetworkIpSec* m_piNetIpsec;
    IIpSecPolicy* m_piPolicy;
    PcscfIpsecInfo* m_pPcscfInfo;
    IMS_BOOL m_bSaEstablished;
    IMS_BOOL m_bIgnorePolicyExpired;

private:
    IAosIpsecListener* m_piListener;
    UeIpsecInfo* m_pUeInfo;
    IMS_UINT32 m_nSecuProto;
    IMS_UINT32 m_nAuthAlgo;
    IMS_UINT32 m_nEncrAlgo;
    IMS_UINT32 m_nMode;
    IMS_BOOL m_bAddPolicy;
    IMS_SINT32 m_nSlotId;
    AString m_strTag;

private:
    friend class AosIpsecTest;
};

/**
 * @brief This class provides related ipsec information to AosIpsecHelper.
 */
class IAosIpsecListener
{
public:
    virtual ~IAosIpsecListener(){};

    /**
     * @brief Notify to AosIpsecHelper when the duration of the ipsec policy is expired.
     *
     * @param pAosIpsec Expired AosIpsec.
     */
    virtual void IPSecPolicyExpired(IN AosIpsec* pAosIpsec) = 0;
};

#endif  // AOS_IPSEC_H_
