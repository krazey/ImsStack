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
#ifndef IMS_IDENTITY_H_
#define IMS_IDENTITY_H_

#include "AString.h"

class AccessNetworkInfo;

// Static class for identities of IMS services
class ImsIdentity
{
public:
    ImsIdentity() = delete;

public:
    /**
     * @brief Returns the public user identity with SIP URI from MSISDN or MDN.
     *
     * @param nSlotId The slot-id for this identity
     * @param bUserPhoneParam The flag to indicate if "user=phone" is included or not
     * @return The public user identity as an SIP URI.
     */
    static AString CreateSipUserId(IN IMS_SINT32 nSlotId, IN IMS_BOOL bUserPhoneParam = IMS_FALSE);

    /**
     * @brief Returns the SIP URI from the specified phone number and phone-context parameter.
     *
     * @param strDialString The phone number digits\n
     *                      If null or empty string, the result of this method is same to
     *                      CreateSipUserId().
     * @param nSlotId The slot-id for this identity
     * @param bUserPhoneParam The flag to indicate if "user=phone" is included or not
     * @param strPhoneContext The domain name for phone-context parameter
     * @return The public user identity as an SIP URI.
     */
    static AString CreateSipUserId(IN const AString& strDialString, IN IMS_SINT32 nSlotId,
            IN IMS_BOOL bUserPhoneParam = IMS_FALSE,
            IN const AString& strPhoneContext = AString::ConstNull());

    /**
     * @brief Returns the SIP URI from the specified dial string and phone-context parameter.
     *        It includes "user=dialstring" URI parameter.
     *
     * @param strDialString The dialed string
     * @param nSlotId The slot-id for this identity
     * @param strPhoneContext The domain name for phone-context parameter
     * @return The public user identity as an SIP URI.
     */
    static AString CreateSipUserIdWithDialString(IN const AString& strDialString,
            IN IMS_SINT32 nSlotId, IN const AString& strPhoneContext = AString::ConstNull());

    /**
     * @brief Returns the SIP URI from the specified dial string and phone-context parameter.
     *
     * It includes the embedded tel URI format in user-info field and "user=phone" URI parameter.
     *
     * @param strDialString The dialed string
     * @param nSlotId The slot-id for this identity
     * @param strPhoneContext The domain name for phone-context parameter
     * @return The public user identity as an SIP URI.
     */
    static AString CreateSipUserIdWithPhone(IN const AString& strDialString, IN IMS_SINT32 nSlotId,
            IN const AString& strPhoneContext = AString::ConstNull());

    /**
     * @brief Returns the public user identity with TEL URI from MSISDN or MDN of this device.
     *
     * @param strPhoneContext The domain name for phone-context parameter
     * @param nSlotId The slot-id for this identity
     * @return The public user identity as a Tel URI.
     */
    static AString CreateTelUserId(IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the TEL URI from the specified phone number and phone-context parameter.
     *
     * It returns the TEL URI without 'phone-context' parameter if the strNumberDigits is
     * a global number digits.
     *
     * @param strDialString The dialed string
     * @param strPhoneContext The domain name for phone-context parameter
     * @return The public user identity as a Tel URI.
     */
    static AString CreateTelUserId(IN const AString& strDialString,
            IN const AString& strPhoneContext, IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the temporary home domain name.
     *
     * @param nSlotId The slot-id for this identity
     * @return The temporary home domain name.
     */
    static AString CreateTemporaryHomeDomainName(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the temporary private user identity according to the 3GPP.
     *
     * @param nSlotId The slot-id for this identity
     * @return The temporary private user identity.
     */
    static AString CreateTemporaryPrivateUserId(IN IMS_SINT32 nSlotId);

    /**  // _IMS_IDENTITY_H_
     * @brief Returns the temporary public user identity according to the 3GPP.
     *
     * @param nSlotId The slot-id for this identity
     * @return The temporary public user identity.
     */
    static AString CreateTemporaryPublicUserId(IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the anonymous SIP URI according to RFC 3261.
     *
     * @return The anonymous SIP URI.
     */
    static const AString& GetAnonymousUserId();

    /**
     * @brief Returns the home domain name.
     *
     * @param nSlotId The slot-id for this identity
     * @param strSubscriberId The subscriber id to identify the subscriber's config
     * @return The home domain name.
     */
    static const AString& GetHomeDomainName(
            IN IMS_SINT32 nSlotId, IN const AString& strSubscriberId = AString::ConstNull());

    /**
     * @brief Returns the MCC / MNC from the specified PLMN.
     *
     * If the PLMN is not specified (length is zero), it will retrieve the valid MCC/MNC
     * from the platform.
     *
     * @param strPlmn The PLMN (MCC + MNC) (5 or 6 length)
     * @param nMncDigits The digits of MNC
     * @param strMcc The MCC string
     * @param strMnc The MNC string
     * @param nSlotId The slot-id for this identity
     * @return IMS_TRUE if the MCC and MNC string are not null or empty string, IMS_FALSE otherwise.
     */
    static IMS_BOOL GetMccMnc(IN const AString& strPlmn, IN IMS_SINT32 nMncDigits,
            OUT AString& strMcc, OUT AString& strMnc, IN IMS_SINT32 nSlotId);

    /**
     * @brief Returns the phone-context string for tel URI.
     *
     * @param nDialingPolicy The dialing policy to consist of phone-context\n
     *                       #DIALING_POLICY_HOME_LOCAL\n
     *                       #DIALING_POLICY_GEO_LOCAL\n
     *                       #DIALING_POLICY_OTHER
     * @param nSlotId The slot-id for this identity
     * @param pAni The access network information\n
     *             If the format is geo-local number, it MUST be specified.
     * @param strSubscriberId The subscriber id to identify the subscriber's config
     * @return The phone-context string.
     */
    static const AString GetPhoneContext(IN IMS_SINT32 nDialingPolicy, IN IMS_SINT32 nSlotId,
            IN AccessNetworkInfo* pAni = IMS_NULL,
            IN const AString& strSubscriberId = AString::ConstNull());

    /**
     * @brief Returns the unavailable SIP URI according to 3GPP TS 23.003.
     *
     * @return The unavailable SIP URI string.
     */
    static const AString& GetUnavailableUserId();

public:
    /// Dialing Policy
    /// For type of "phone-context" parameter in tel URI when dialing
    enum
    {
        /// home-local number is a default
        DIALING_POLICY_HOME_LOCAL = 0,
        DIALING_POLICY_GEO_LOCAL,
        DIALING_POLICY_OTHER
    };
};

#endif
