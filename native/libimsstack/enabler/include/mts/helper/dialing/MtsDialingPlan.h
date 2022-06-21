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

#ifndef MTS_DIALING_PLAN_H_
#define MTS_DIALING_PLAN_H_

#include "AStringBuffer.h"
#include "ImsAccessNetworkInfoType.h"

class MtsDialingPlan final
{
public:
    MtsDialingPlan(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nNumberFormat, IN const AString& strScheme);
    ~MtsDialingPlan();

private:
    MtsDialingPlan();
    MtsDialingPlan(IN const MtsDialingPlan& objRHS);
    MtsDialingPlan& operator=(IN const MtsDialingPlan& objRHS);

public:
    AString Translate(IN const AString& strNumber, IN IMS_BOOL bAquot = IMS_TRUE,
            IN IMS_BOOL bUssi = IMS_FALSE);
    AString Translate(IN const AString& strNumber, IN const AString& strScheme,
            IN IMS_BOOL bAquot = IMS_TRUE);

    // To translate the dialed number based on the emergency flag
    AString TranslateEx(IN const AString& strNumber, IN IMS_SINT32 nFlags = FLAG_NONE,
            IN IMS_BOOL bAquot = IMS_TRUE, IN IMS_BOOL bUssi = IMS_FALSE);
    AString TranslateEx(IN const AString& strNumber, IN const AString& strScheme,
            IN IMS_SINT32 nFlags = FLAG_NONE, IN IMS_BOOL bAquot = IMS_TRUE);

    IMS_SINT32 GetDialingPolicy() const;
    IMS_SINT32 GetNumberFormat() const;
    const AString& GetNetworkProfile() const;
    const AString& GetScheme() const;

    void SetDialingPolicy(IN IMS_SINT32 nPolicy);
    void SetNumberFormat(IN IMS_SINT32 nNumberFormat);
    void SetNetworkProfile(IN const AString& strNetworkProfile);
    void SetScheme(IN const AString& strScheme);

protected:
    // For geo-local number format
    AccessNetworkInfo* GetAccessNetworkInfo(IN_OUT AccessNetworkInfo& objANI);
    IMS_BOOL TranslateAsGlobal(IN const AString& strNumber, OUT AStringBuffer& objURI);
    IMS_BOOL TranslateAsLocal(IN const AString& strNumber, OUT AStringBuffer& objURI);

    IMS_BOOL FormNonTelUri(IN const AString& strNumber, IN IMS_BOOL bAquot,
            OUT AStringBuffer& objURI, IN const AString& strScheme = AString::ConstNull());
    IMS_BOOL FormUssiNonTelUri(IN const AString& strNumber, OUT AStringBuffer& objURI,
            IN const AString& strScheme = AString::ConstNull());

    IMS_BOOL TranslateAsServiceUrn(IN const AString& strNumber, OUT AStringBuffer& objURI);
    IMS_SINT32 TranslateScheme(IN const AString& strNumber) const;

    static IMS_SINT32 GetDialedNumberFormat(IN const AString& strDial);
    static IMS_BOOL IsVisualSeparator(IN IMS_CHAR ch);

public:
    enum
    {
        FLAG_NONE = 0x0000,
        FLAG_EMERGENCY = 0x0001
    };

    enum
    {
        // Default format is local number format
        NUMBER_FORMAT_LOCAL = 1,
        NUMBER_FORMAT_GLOBAL = 2,

        // Internal usage; In case the dialed number is not local/global number digits
        NUMBER_FORMAT_NON_TEL = 10
    };

protected:
    IMS_SINT32 m_nSlotId;

private:
    // local / global
    IMS_SINT32 m_nNumberFormat;
    AString m_strScheme;

    // For policy to consist of phone-context URI parameter;
    // Refer to ImsIdentity::DIALING_POLICY_XXX
    IMS_SINT32 m_nDialingPolicy;
    // For geo-local number format, it is used to get the access network information
    AString m_strNetworkProfile;
};

#endif
