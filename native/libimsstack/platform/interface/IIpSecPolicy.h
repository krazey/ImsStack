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
#ifndef INTERFACE_IPSEC_POLICY_H_
#define INTERFACE_IPSEC_POLICY_H_

#include "ImsTypeDef.h"
#include "IMSList.h"

class IIPSecSA;
class IIPSecSP;
class IIPSecPolicyListener;

class IIPSecPolicy
{
public:
    /*
     Returns an Identifier of this IIPSecPolicy.
    */
    virtual IMS_SINT32 GetId() const = 0;

    /*

    Creates SP Configuration

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IIPSecSAD*              Pointer to IIPSecSAD
    </table>

    */
    virtual IIPSecSP* CreateSP() = 0;

    /*

    Destroy SP Configuration

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>
    IIPSecSP*               Pointer to IIPSecSP

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DestroySP(IN IIPSecSP* piSP) = 0;

    /*

    Creates SA Configuration

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>

    Returns
    <table>
    return                  description
    ----------              ----------
    IIPSecSA*               Pointer to IIPSecSA
    </table>

    */
    virtual IIPSecSA* CreateSA() = 0;

    /*

    Destroy SA Configuration

    Parameters
    <table>
    parameter               description
    ----------              ----------
    </table>
    IIPSecSA*               Pointer to IIPSecSA

    Returns
    <table>
    return                  description
    ----------              ----------

    </table>

    */
    virtual void DestroySA(IN IIPSecSA* piSA) = 0;

    /*

    Manage SAs lifetime

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
    virtual void ManageLifetime(IN IMS_UINT32 nDuration) = 0;

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
    virtual void SetListener(IN IIPSecPolicyListener* piListener) = 0;

};

#endif // INTERFACE_IPSEC_POLICY_H_
