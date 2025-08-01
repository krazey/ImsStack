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

#ifndef NORMAL_DIALING_PLAN_H_
#define NORMAL_DIALING_PLAN_H_

#include "AString.h"
#include "ImsIdentity.h"

class IMtcContext;
class ImsIdentityProxy;

class NormalDialingPlan
{
public:
    enum class NumberFormat
    {
        // Default format is local number format
        LOCAL_FORMAT = 1,
        GLOBAL_FORMAT = 2,

        // Internal usage; In case the dialed number is not local/global number digits
        NON_NUMBER = 3
    };

    enum class LocalNumberPolicy
    {
        HOME,  // refers. ImsIdentity::DIALING_POLICY_HOME_LOCAL
        GEO,   // refers. ImsIdentity::DIALING_POLICY_GEO_LOCAL
        GEO_LOCAL_ONLY_IN_ROAMING
    };

    enum class Scheme
    {
        UNKNOWN = 0,
        TEL,
        SIP
    };

    // TODO: removing bAquot is for VZW GetEntryUri. should not be required.
    static AString& GetTranslatedUri(IN IMtcContext& objContext, IN_OUT AString& strNumber,
            Scheme eScheme, IN const ImsIdentityProxy& objIdentityProxy);
    static AString& GetTranslatedUriForDialString(IN const IMtcContext& objContext,
            IN_OUT AString& strNumber, IN const ImsIdentityProxy& objIdentityProxy);

private:
    static AString& Translate(IN IMtcContext& objContext, IN_OUT AString& strNumber,
            IN Scheme eScheme, IN const ImsIdentityProxy& objIdentityProxy);

    static void FormSipUri(IN IMtcContext& objContext, IN_OUT AString& strNumber,
            IN const ImsIdentityProxy& objIdentityProxy);
    static void FormTelUri(IN IMtcContext& objContext, IN_OUT AString& strNumber,
            IN const ImsIdentityProxy& objIdentityProxy);

    static IMS_BOOL IsVisualSeparator(IN IMS_CHAR ch);
    static IMS_BOOL IsNameAddress(IN const AString& strNumber);
    static IMS_BOOL IsLocalNumberFormat(IN const AString& strNumber);
    static IMS_BOOL IsAddressSpec(IN const AString& strNumber);
    static void AddAquotIfRequired(IN_OUT AString& strNumber);

    static NumberFormat GetDialedNumberFormat(IN const AString& strNumber);

    // For geo-local number format
    static AccessNetworkInfo& GetAccessNetworkInfo(
            IN IMtcContext& objContext, OUT AccessNetworkInfo& objAni);

    static Scheme GetScheme(IN IMtcContext& objContext);
    static LocalNumberPolicy GetLocalNumberPolicy(IN IMtcContext& objContext);
    static IMS_UINT32 ConvertDialingPolicy(IN LocalNumberPolicy ePolicy);
};

#endif
