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

#include "ImsIdentity.h"
#include "ImsAccessNetworkInfoType.h"
#include "IMSLib.h"
#include "ServiceNetwork.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "AString.h"
#include "Sip.h"
#include "SipAddress.h"
#include "IMtcContext.h"
#include "call/IMtcCall.h"
#include "dialingplan/MtcDialingPlan.h"
#include "dialingplan/EmergencyDialingPlan.h"
#include "util/TextParser.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcDialingPlan::MtcDialingPlan(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_pTemporaryServiceUrn(nullptr)
{
    IMS_TRACE_I("+MtcDialingPlan", 0, 0, 0);
}

PUBLIC
MtcDialingPlan::~MtcDialingPlan()
{
    IMS_TRACE_I("~MtcDialingPlan", 0, 0, 0);
}

PUBLIC
AString MtcDialingPlan::GetToUri(IN const AString& strNumber, IN const CallInfo& objCallInfo,
        IN Scheme eScheme /* = Scheme::Unknown*/)
{
    AString strUri = strNumber;

    // this is for the case STK generates URI.
    if (IsUriForm(strUri))
    {
        return strUri;
    }

    if (m_pTemporaryServiceUrn)
    {
        if (m_pTemporaryServiceUrn->GetNumber().Equals(strNumber))
        {
            AString strUrn = m_pTemporaryServiceUrn->GetUrn();
            m_pTemporaryServiceUrn = nullptr;
            return strUrn;
        }
        m_pTemporaryServiceUrn = nullptr;
    }

    if (objCallInfo.bUssi)
    {
        return NormalDialingPlan::GetTranslatedUriForDialString(m_objContext, strUri);
    }

    if (objCallInfo.bConference)
    {
        // TODO: creating confrence factory uri also needs to be moved
    }

    if (objCallInfo.bEmergency)
    {
        return EmergencyDialingPlan::GetTranslatedUri(m_objContext, strUri);
    }

    return NormalDialingPlan::GetTranslatedUri(m_objContext, strUri, eScheme);
}

PUBLIC
void MtcDialingPlan::OnCountrySpecificServiceUrnReceived(
        IN const AString& strNumber, IN const AString& strServiceUrn)
{
    // if already exists, overwrite.
    m_pTemporaryServiceUrn = std::make_unique<TemporaryServiceUrn>(strNumber, strServiceUrn);
}

PRIVATE
IMS_BOOL MtcDialingPlan::IsUriForm(IN const AString& strNumber)
{
    SipAddress objSipAddress;
    objSipAddress.Create(strNumber);

    if (objSipAddress.GetScheme().EqualsIgnoreCase(Sip::STR_SIP) ||
            objSipAddress.GetScheme().EqualsIgnoreCase(Sip::STR_SIPS) ||
            objSipAddress.GetScheme().EqualsIgnoreCase(Sip::STR_TEL))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
