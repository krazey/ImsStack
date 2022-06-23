
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
#ifndef IPSEC_SA_H_
#define IPSEC_SA_H_

#include "ImsTypeDef.h"

class SAInfoP;

class IPSecSA
{
public:
    IPSecSA();
    ~IPSecSA();

private:
    IPSecSA(IN const IPSecSA& objRHS);
    IPSecSA& operator=(IN const IPSecSA& objRHS);

public:
    // Upper layer Protocol
    enum
    {
        TRANS_PROTOCOL_UDP = 0,
        TRANS_PROTOCOL_TCP,
        TRANS_PROTOCOL_BOTH
    };

    void SetSA(IN IMS_UINT32 nTransProtocol, IN IMS_UINT32 nUEPort, IN IMS_UINT32 nServerPort,
            IN IMS_UINT32 nSPI, IN IMS_UINT32 nDirection);

    // specific purpose data related IPSec Lib
    void SetContext(IN IMS_PVOID pContext);

    IMS_UINT32 GetTransProtocol();
    IMS_UINT32 GetUePort();
    IMS_UINT32 GetServerPort();
    IMS_UINT32 GetSPI();
    IMS_UINT32 GetDirection();

    void GetSAInfo(OUT IMS_UINT32& nTransProtocol, OUT IMS_UINT32& nUEPort,
            OUT IMS_UINT32& nServerPort, OUT IMS_UINT32& nSPI, OUT IMS_UINT32& nDirection);

    // specific purpose data related IPSec Lib
    IMS_PVOID GetContext();

private:
    SAInfoP* pSAInfoP;
};

#endif  // IPSEC_SA_H_
