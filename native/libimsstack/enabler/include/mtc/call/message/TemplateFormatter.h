/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TEMPLATE_FORMATTER_H_
#define TEMPLATE_FORMATTER_H_

#include "AString.h"
#include <functional>

class IMtcCallContext;

/*
 * Formats template literals using device information.
 *
 * Supported literals: #IMEI#, #IMSI#, #MAC#, #IP#, #PORT#, #PUID#, #AID#
 */
class TemplateFormatter final
{
public:
    TemplateFormatter() = delete;
    ~TemplateFormatter() = delete;
    TemplateFormatter(const TemplateFormatter&) = delete;
    TemplateFormatter& operator=(const TemplateFormatter&) = delete;

    static AString Format(IN const AString& strFormatString, IN IMtcCallContext& objContext);

private:
    static AString GetImei(IN const IMtcCallContext& objContext);
    static AString GetImsi(IN const IMtcCallContext& objContext);
    static AString GetMacAddress(IN const IMtcCallContext& objContext);
    static AString GetIpAddress(IN IMtcCallContext& objContext);
    static AString GetPort(IN IMtcCallContext& objContext);
    static const AString& GetPublicUserId(IN const IMtcCallContext& objContext);
    static AString GetWifiCallingAddressId(IN const IMtcCallContext& objContext);
    static AString GetMsisdn(IN const IMtcCallContext& objContext);
    static AString GetHomeDomain(IN const IMtcCallContext& objContext);
    static AString GetUniqueId();
    static AString GetMcc(IN const IMtcCallContext& objContext);
    static AString GetMnc(IN const IMtcCallContext& objContext, IN IMS_UINT32 nLength);

    static void Replace(IN_OUT AString& strText, IN const AString& strTemplateLiteral,
            IN const std::function<AString()>& objSubstitution);

    static IMS_BOOL IsInUnknownCountry(IN IMS_SINT32 nSlotId);
};

#endif
