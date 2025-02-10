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

#include "AString.h"
#include "CarrierConfig.h"
#include "IMtcContext.h"
#include "IPhoneInfoSubscriber.h"
#include "ImsAccessNetworkInfoType.h"
#include "ImsIdentity.h"
#include "ImsLib.h"
#include "ServiceNetwork.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "Sip.h"
#include "SipAddress.h"
#include "call/IMtcCall.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/ImsIdentityProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "util/TextParser.h"
#include <memory>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcDialingPlan::MtcDialingPlan(IN IMtcContext& objContext, IN ISubscriberInfo& objSubscriberInfo) :
        m_pIdentityProxy(new ImsIdentityProxy()),
        m_objContext(objContext),
        m_pTemporaryServiceUrn(nullptr),
        m_objSubscriberInfo(objSubscriberInfo)
{
    IMS_TRACE_I("+MtcDialingPlan", 0, 0, 0);
}

PUBLIC
MtcDialingPlan::~MtcDialingPlan()
{
    IMS_TRACE_I("~MtcDialingPlan", 0, 0, 0);
    delete m_pIdentityProxy;
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

    if (objCallInfo.IsEmergency())
    {
        IMS_TRACE_D("GetToUri Emergency URN will be obtained using SuppType::TARGET_URI", 0, 0, 0);
        return strUri;
    }

    if (objCallInfo.bConference)
    {
        return GetConferenceFactoryUri();
    }

    if (objCallInfo.bUssi)
    {
        return NormalDialingPlan::GetTranslatedUriForDialString(
                m_objContext, strUri, *m_pIdentityProxy);
    }

    return NormalDialingPlan::GetTranslatedUri(m_objContext, strUri, eScheme, *m_pIdentityProxy);
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

PRIVATE
AString MtcDialingPlan::GetConferenceFactoryUri() const
{
    AString strUri = m_objContext.GetConfigurationProxy().GetString(
            ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING);

    IMS_TRACE_D("GetConferenceFactoryUri uri from config[%s]", strUri.GetStr(), 0, 0);

    if (strUri.GetLength() <= 0)
    {
        strUri = "sip:mmtel@conf-factory.ims.mnc[MNC].mcc[MCC].3gppnetwork.org";
    }

    if (strUri.Contains("[MNC") || strUri.Contains("[MCC"))
    {
        IMS_TRACE_D("GetConferenceFactoryUri MNC/MCC converting", 0, 0, 0);
        strUri = strUri.Replace("[MCC]", GetMcc())
                         .Replace("[MNC]", GetMnc(3))
                         .Replace("[MNC2]", GetMnc(2));
    }

    if (strUri.Contains("[DOMAIN"))
    {
        IMS_TRACE_D("GetConferenceFactoryUri DOMAIN converting", 0, 0, 0);
        strUri = strUri.Replace(
                "[DOMAIN]", m_pIdentityProxy->GetHomeDomainName(m_objContext.GetSlotId()));
    }

    if (IsUriForm(strUri) == IMS_FALSE)
    {
        strUri = strUri.Prepend("sip:");
    }

    IMS_TRACE_I("GetConferenceFactoryUri [%s]", strUri.GetStr(), 0, 0);
    return strUri;
}

PRIVATE
AString MtcDialingPlan::GetMcc() const
{
    AString strMcc;
    m_objSubscriberInfo.GetSimMcc(strMcc);
    return strMcc;
}

PRIVATE
AString MtcDialingPlan::GetMnc(IN IMS_UINT32 nLength) const
{
    AString strMnc;
    m_objSubscriberInfo.GetSimMnc(strMnc);
    if (nLength == 3 && strMnc.GetLength() == 2)
    {
        strMnc.Prepend("0");
    }
    return strMnc;
}
