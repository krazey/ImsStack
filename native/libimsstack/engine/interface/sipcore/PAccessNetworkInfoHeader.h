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
#ifndef P_ACCESS_NETWORK_INFO_HEADER_H_
#define P_ACCESS_NETWORK_INFO_HEADER_H_

#include "ImsAccessNetworkInfoType.h"
#include "IpAddress.h"
#include "SipMethod.h"

class INetworkConnection;
class ISipMessage;
class SipProfile;

/**
 * @brief This class provides an interface to form P-Access-Network-Info header.
 */
class PAccessNetworkInfoHeader
{
public:
    PAccessNetworkInfoHeader() = delete;

public:
    /**
     * @brief Forms P-Access-Network-Info header field.
     *
     * @param nSlotId Slot id for this header
     * @param piConnection Current network connection which the device is attached
     * @param objMethod SIP method for this header
     * @param pSipProfile SIP profile for this header
     * @param strHeader Formed header string
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL FormHeader(IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection,
            IN const SipMethod& objMethod, IN const SipProfile* pSipProfile,
            OUT AString& strHeader);
    /**
     * @brief Forms P-Access-Network-Info header field.
     *
     * @param nSlotId Slot id for this header
     * @param objIpAddr Local IP address
     * @param objMethod SIP method for this header
     * @param pSipProfile SIP profile for this header
     * @param strHeader Formed header string
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL FormHeader(IN IMS_SINT32 nSlotId, IN const IpAddress& objIpAddr,
            IN const SipMethod& objMethod, IN const SipProfile* pSipProfile,
            OUT AString& strHeader);

    /**
     * @brief Forms P-Access-Network-Info header and sets it to SIP message.
     *
     * @param nSlotId Slot id for this header
     * @param objIpAddr Local IP address
     * @param pSipProfile SIP profile for this header
     * @param piSipMsg SIP message object
     */
    static void SetHeader(IN IMS_SINT32 nSlotId, IN const IpAddress& objIpAddr,
            IN const SipProfile* pSipProfile, IN_OUT ISipMessage*& piSipMsg);

private:
    /**
     * @brief Forms P-Access-Network-Info header field.
     *
     * @param nSlotId Slot id for this header
     * @param objAnInfo Current access network information which the device is attached
     * @param strHeader Formed header string
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL FormHeader(
            IN IMS_SINT32 nSlotId, IN const AccessNetworkInfo& objAni, OUT AString& strHeader);
    static void RefineMacAddressAsInvalid(IN_OUT AccessNetworkInfo& objAni);
    static void AddLocalTimezone(IN_OUT AString& strHeader);
    static void AddCountryParameter(IN IMS_SINT32 nSlotId, IN_OUT AString& strHeader);
    static void SetPrivateHeaderForPlani(IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection,
            IN_OUT ISipMessage*& piSipMsg);
    static void SetPrivateHeaderForPlci(IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection,
            IN_OUT ISipMessage*& piSipMsg);
    static void SetCniHeader(IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection,
            IN const SipProfile* pSipProfile, IN_OUT ISipMessage*& piSipMsg);
    static IMS_BOOL IsAccessNetworkTypeWiFi(IN const AccessNetworkInfo& objAni);
};

#endif
