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
#include "call/IMtcCallContext.h"
#include "call/message/TemplateFormatter.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/ImsIdentityProxy.h"
#include "dialingplan/MtcDialingPlan.h"
#include "util/TextParser.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcDialingPlan::MtcDialingPlan() :
        m_pIdentityProxy(new ImsIdentityProxy())
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
AString MtcDialingPlan::GetToUri(IN const AString& strNumber, IN IMtcCallContext& objContext,
        IN Scheme eScheme /* = Scheme::Unknown*/)
{
    AString strUri = strNumber;

    // this is for the case STK generates URI.
    if (IsUriForm(strUri))
    {
        return strUri;
    }

    if (objContext.GetCallInfo().bConference)
    {
        return GetConferenceFactoryUri(objContext);
    }

    if (objContext.GetCallInfo().bUssi)
    {
        return NormalDialingPlan::GetTranslatedUriForDialString(
                objContext, strUri, *m_pIdentityProxy);
    }

    return NormalDialingPlan::GetTranslatedUri(objContext, strUri, eScheme, *m_pIdentityProxy);
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
AString MtcDialingPlan::GetConferenceFactoryUri(IN IMtcCallContext& objContext) const
{
    AString strUri = objContext.GetConfigurationProxy().GetString(
            ConfigVoice::KEY_CONFERENCE_FACTORY_URI_STRING);

    IMS_TRACE_D("GetConferenceFactoryUri uri from config[%s]", strUri.GetStr(), 0, 0);

    if (strUri.GetLength() <= 0)
    {
        strUri = "sip:mmtel@conf-factory.ims.mnc#MNC#.mcc#MCC#.3gppnetwork.org";
    }

    strUri = TemplateFormatter::Format(strUri, objContext);

    if (IsUriForm(strUri) == IMS_FALSE)
    {
        strUri = strUri.Prepend("sip:");
    }

    IMS_TRACE_I("GetConferenceFactoryUri [%s]", strUri.GetStr(), 0, 0);
    return strUri;
}
