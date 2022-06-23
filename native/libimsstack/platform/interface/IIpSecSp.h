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
#ifndef INTERFACE_IPSEC_SP_H_
#define INTERFACE_IPSEC_SP_H_

#include "IpAddress.h"

class IIpSecSp
{
public:
    virtual void SetTransportInfo(IN const IPAddress& objSrcIp, IN IMS_UINT32 nSrcPort,
            IN const IPAddress& objDstIp, IN IMS_UINT32 nDstPort, IN IMS_UINT32 nTransportProtocol,
            IN IMS_UINT32 nAction, IN IMS_UINT32 nDirection, IN IMS_UINT32 nSpi,
            IN IMS_UINT32 nMode) = 0;

    virtual void SetSecurityAlgorithmInfo(IN IMS_UINT32 nSecurityProtocol,
            IN IMS_UINT32 nAuthAlgorithm, IN IMS_UINT32 nEncryptionAlgorithm) = 0;

    virtual void DoneSp() = 0;
};

#endif
