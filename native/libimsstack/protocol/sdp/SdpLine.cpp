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
#include "IpAddress.h"
#include "ServiceMemory.h"

#include "Sdp.h"
#include "SdpLine.h"

PUBLIC
SdpLine::SdpLine() {}

PUBLIC
SdpLine::SdpLine(IN const SdpLine& /*other*/) {}

PUBLIC VIRTUAL SdpLine::~SdpLine() {}

PUBLIC
SdpLine& SdpLine::operator=(IN const SdpLine& /*other*/)
{
    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL SdpLine::Decode(IN const AString& /*strValue*/)
{
    return IMS_FALSE;
}

PUBLIC VIRTUAL AString SdpLine::Encode() const
{
    return AString::ConstNull();
}

PUBLIC VIRTUAL AString SdpLine::GetValue() const
{
    return AString::ConstNull();
}

PROTECTED GLOBAL IMS_BOOL SdpLine::CheckValidityForAddress(
        IN const AString& strAddress, IN IMS_SINT32 nAddrType)
{
    IPAddress objIpAddr;

    if (objIpAddr.Parse(strAddress))
    {
        if (objIpAddr.IsIPv4Address() && (nAddrType != Sdp::ADDR_TYPE_IP4))
        {
            return IMS_FALSE;
        }

        if (objIpAddr.IsIPv6Address() && (nAddrType != Sdp::ADDR_TYPE_IP6))
        {
            return IMS_FALSE;
        }

        return IMS_TRUE;
    }

    // Check if the address format is FQDN / extn-addr
    if (!Sdp::IsFqdnString(strAddress) && !Sdp::IsNonWsString(strAddress))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}
