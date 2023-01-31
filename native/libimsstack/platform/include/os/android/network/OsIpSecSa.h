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
#ifndef OS_IPSEC_SA_H_
#define OS_IPSEC_SA_H_

#include "IIpSecSa.h"
#include "IpSecSaParameter.h"

class OsIpSecSaPrivate;

class OsIpSecSa : public IIpSecSa
{
public:
    OsIpSecSa();
    virtual ~OsIpSecSa();

    OsIpSecSa(IN const OsIpSecSa&) = delete;
    OsIpSecSa& operator=(IN const OsIpSecSa&) = delete;

public:
    // IIpSecSa class
    void SetSa(IN const IpAddress& objSrcIp, IN IMS_UINT32 nSrcPort, IN const IpAddress& objDstIp,
            IN IMS_UINT32 nDstPort, IN IMS_UINT32 nSecurityProtocol, IN IMS_UINT32 nSpi,
            IN IMS_UINT32 nMode, IN IMS_UINT32 nAuthAlgorithm, IN IMS_UINT32 nEncryptionAlgorithm,
            IN const ByteArray& objAuthKey, IN const ByteArray& objEncryptionKey) override;
    void DoneSa() override;

    IMS_UINT32 GetSpi() const;
    void DisplayInfo();

    IpSecSaParameter CreateSaParameter(IN IMS_SINT32 nId) const;

private:
    OsIpSecSaPrivate* m_pIpSecSaP;

    // IK : 128 bit (16 bytes)
    static const IMS_UINT32 AUTHALG_MD5_KEY_LEN = 16;
    static const IMS_UINT32 AUTHALG_SHA1_KEY_LEN = 20;
    // CK : 128 bit (16 bytes)
    static const IMS_UINT32 ENCRALG_AES_KEY_LEN = 16;
    static const IMS_UINT32 ENCRALG_3DES_KEY_LEN = 24;
};

#endif
