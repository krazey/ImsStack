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
#ifndef INTERFACE_AOS_PCSCF_H_
#define INTERFACE_AOS_PCSCF_H_

#include "ImsTypeDef.h"
#include "ImsList.h"
#include "IpAddress.h"
#include "AStringArray.h"

class IAosPcscfListener;

class IAosPcscf
{
public:
    virtual ~IAosPcscf(){};

    virtual void Configure(IN IMS_UINT32 nIpVersion = IpAddress::UNKNOWN) = 0;
    virtual IMS_BOOL IsConfigured() const = 0;

    virtual IMS_BOOL IsAsyncDnsDiscovery() const = 0;
    virtual IMS_BOOL IsSinglePcoScheme() = 0;

    virtual const AStringArray& GetPcscfs() = 0;
    virtual const ImsList<IMS_SINT32>& GetPcscfsPorts() = 0;
    virtual void UpdatePcscfs(IN const AStringArray& objPcscfs,
            IN ImsList<IMS_SINT32> objPorts = ImsList<IMS_SINT32>()) = 0;

    virtual IMS_BOOL HasPcscf(IN IMS_SINT32 nIndex) = 0;
    virtual IMS_UINT32 GetPcscfCount() = 0;

    virtual void SetCurrentPcscfInvalid(
            IN IMS_BOOL bIsTimer = IMS_FALSE, IN IMS_UINT32 nSeconds = 0) = 0;
    virtual void RemoveCurrentPcscf() = 0;
    virtual void SetAllPcscfValid() = 0;

    virtual IMS_BOOL IsAllPcscfTried() = 0;
    virtual void SetCurrentPcscfTried() = 0;
    virtual void ResetAllPcscfTried() = 0;

    virtual IMS_UINT32 GetCurrentPcscfTriedCount() = 0;
    virtual void IncreaseCurrentPcscfTriedCount() = 0;
    virtual void ResetCurrentPcscfTriedCount() = 0;
    virtual void ResetAllPcscfTriedCount() = 0;

    virtual IMS_BOOL GetCurrentPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort) = 0;
    virtual IMS_UINT32 GetCurrentIndex() const = 0;

    virtual IMS_BOOL IsFirstPcscf() = 0;
    virtual IMS_BOOL GetFirstPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort) = 0;

    virtual IMS_BOOL HasNextPcscf() = 0;
    virtual IMS_SINT32 GetNextPcscfIndex() = 0;
    virtual IMS_BOOL GetNextPcscf(OUT AString& objPcscf, OUT IMS_UINT32& nPort) = 0;

    virtual void SetFirstPcscfIndex() = 0;

    virtual IMS_BOOL CheckAndProcessChangeFromPco() = 0;
    virtual IMS_UINT32 GetChangedType() = 0;

    virtual void SetListener(IN IAosPcscfListener* piListener) = 0;

    enum
    {
        TYPE_CONFIG = 0,
        TYPE_NORMAL_REGISTRATION,
        TYPE_CONFIG_EX
    };

    enum
    {
        TYPE_CHANGED_SAME = 0,
        TYPE_CHANGED_DIFFERENT,
        TYPE_CHANGED_REORDER
    };

protected:
    friend class AosBuildDirector;
    friend class AosAppContext;
    virtual void Init() = 0;
    virtual void CleanUp() = 0;
};

class IAosPcscfListener
{
public:
    virtual ~IAosPcscfListener(){};

    virtual void Pcscf_NotifyResult(IN IMS_BOOL bResult) = 0;
};

#endif  // INTERFACE_AOS_PCSCF_H_
