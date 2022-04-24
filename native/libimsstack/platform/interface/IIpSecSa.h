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
#ifndef INTERFACE_IPSEC_SA_H_
#define INTERFACE_IPSEC_SA_H_

#include "IPAddress.h"

class IIpSecSa
{
public:
    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void SetSa
        (
        IN const IPAddress &objSrcIPA,
        IN IMS_UINT32 nSrcPort,
        IN const IPAddress &objDestIPA,
        IN IMS_UINT32 nDestPort,
        IN IMS_UINT32 nSecuProto,
        IN IMS_UINT32 nSPI,
        IN IMS_UINT32 nMode,
        IN IMS_UINT32 nAuthAlgo,
        IN IMS_UINT32 nEncrAlgo,
        IN const ByteArray &objAuthKey,
        IN const ByteArray &objEncrKey
        ) = 0;

    /*

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    </table>

    */
    virtual void DoneSa() = 0;

};

#endif // INTERFACE_IPSEC_SA_H_
