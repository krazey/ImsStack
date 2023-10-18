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
#ifndef OS_IPSEC_SP_H_
#define OS_IPSEC_SP_H_

#include "IIpSecSp.h"
#include "IpSecSaParameter.h"

class OsIpSecSpPrivate;

class OsIpSecSp : public IIpSecSp
{
public:
    OsIpSecSp();
    virtual ~OsIpSecSp();

    OsIpSecSp(IN const OsIpSecSp&) = delete;
    OsIpSecSp& operator=(IN const OsIpSecSp&) = delete;

public:
    // IIpSecSp class
    void SetTransportInfo(IN const IpAddress& objSrcIp, IN IMS_UINT32 nSrcPort,
            IN const IpAddress& objDstIp, IN IMS_UINT32 nDstPort, IN IMS_UINT32 nTransportProtocol,
            IN IMS_UINT32 nAction, IN IMS_UINT32 nDirection, IN IMS_UINT32 nSpi,
            IN IMS_UINT32 nMode) override;
    void SetSecurityAlgorithmInfo(IN IMS_UINT32 nSecurityProtocol, IN IMS_UINT32 nAuthAlgorithm,
            IN IMS_UINT32 nEncryptionAlgorithm) override;
    void DoneSp() override;

    IMS_UINT32 GetSpi() const;
    void DisplayInfo();

    IpSecSaParameter::Policy CreateSaPolicy() const;

public:
    enum
    {
        IP_VERSION_NO = 0,
        IP_VERSION_V4,
        IP_VERSION_V6,
    };

private:
    OsIpSecSpPrivate* m_pIpSecSpP;

    // CK : 128 bit (16 bytes)
    static const IMS_UINT32 ENCRALG_AES_KEY_LEN = 16;
    static const IMS_UINT32 ENCRALG_DES_KEY_LEN = 8;
};

#endif
