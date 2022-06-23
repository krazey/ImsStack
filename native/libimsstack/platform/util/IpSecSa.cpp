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
#include "ServiceMemory.h"
#include "ImsStrLib.h"
#include "IpSecSa.h"

class SAInfoP
{
public:
    inline SAInfoP() :
            nTransProtocol(0),
            nUEPort(0),
            nServerPort(0),
            nSPI(0),
            nDirection(0),
            pContext(IMS_NULL)
    {
    }
    inline ~SAInfoP() {}

public:
    IMS_UINT32 nTransProtocol;
    IMS_UINT32 nUEPort;
    IMS_UINT32 nServerPort;
    IMS_UINT32 nSPI;
    IMS_UINT32 nDirection;
    void* pContext;
};

PUBLIC
IPSecSA::IPSecSA() :
        pSAInfoP(new SAInfoP())
{
}

PUBLIC
IPSecSA::~IPSecSA()
{
    if (pSAInfoP != IMS_NULL)
    {
        delete pSAInfoP;
        pSAInfoP = IMS_NULL;
    }
}

/*

Remarks

*/
PUBLIC
void IPSecSA::SetSA(IN IMS_UINT32 nTransProtocol, IN IMS_UINT32 nUEPort, IN IMS_UINT32 nServerPort,
        IN IMS_UINT32 nSPI, IN IMS_UINT32 nDirection)
{
    pSAInfoP->nTransProtocol = nTransProtocol;
    pSAInfoP->nUEPort = nUEPort;
    pSAInfoP->nServerPort = nServerPort;
    pSAInfoP->nSPI = nSPI;
    pSAInfoP->nDirection = nDirection;
}

/*

Remarks

*/
PUBLIC
void IPSecSA::SetContext(IN IMS_PVOID pContext)
{
    pSAInfoP->pContext = pContext;
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecSA::GetTransProtocol()
{
    return pSAInfoP->nTransProtocol;
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecSA::GetUePort()
{
    return pSAInfoP->nUEPort;
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecSA::GetServerPort()
{
    return pSAInfoP->nServerPort;
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecSA::GetSPI()
{
    return pSAInfoP->nSPI;
}

/*

Remarks

*/
PUBLIC
IMS_UINT32 IPSecSA::GetDirection()
{
    return pSAInfoP->nDirection;
}

/*

Remarks

*/
PUBLIC
void IPSecSA::GetSAInfo(OUT IMS_UINT32& nTransProtocol, OUT IMS_UINT32& nUEPort,
        OUT IMS_UINT32& nServerPort, OUT IMS_UINT32& nSPI, OUT IMS_UINT32& nDirection)
{
    nTransProtocol = pSAInfoP->nTransProtocol;
    nUEPort = pSAInfoP->nUEPort;
    nServerPort = pSAInfoP->nServerPort;
    nSPI = pSAInfoP->nSPI;
    nDirection = pSAInfoP->nDirection;
}

/*

Remarks

*/
PUBLIC
IMS_PVOID IPSecSA::GetContext()
{
    return pSAInfoP->pContext;
}
