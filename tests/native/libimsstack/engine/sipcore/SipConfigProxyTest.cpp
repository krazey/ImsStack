/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include <gtest/gtest.h>

#include "private/SipConfig.h"

#include "SipConfigProxy.h"

namespace android
{

class SipConfigProxyTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipConfigProxyTest, GetAndIsConfiguredMethods)
{
    // without SipProfile
    EXPECT_EQ(SipConfigProxy::GetDeviceId(IMS_SLOT_0), ISipConfig::DEVICE_ID_UUID_IMEI_SHA1);
    EXPECT_EQ(SipConfigProxy::GetPredefinedDeviceId(IMS_SLOT_0), AString::ConstNull());

    IMS_SINT32 nDefaultChannel = 5060;

    EXPECT_EQ(SipConfigProxy::GetPort(IMS_SLOT_0), nDefaultChannel);

    EXPECT_EQ(SipConfigProxy::GetSipFeatureCaps(IMS_SLOT_0),
            ISipConfig::SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT);
    EXPECT_EQ(SipConfigProxy::GetTagPrefix(IMS_SLOT_0), AString("9009"));

    IMS_SINT32 nTcpCriterionLength = 1300;

    EXPECT_EQ(SipConfigProxy::GetTcpCriterionLength(IMS_SLOT_0), nTcpCriterionLength);

    EXPECT_EQ(SipConfigProxy::GetTransportType(IMS_SLOT_0), SipConfig::TRANSPORT_TYPE_UDP);
    EXPECT_EQ(SipConfigProxy::GetUaString(IMS_SLOT_0), AString::ConstNull());

    const AStringArray& regAllowMethods = SipConfigProxy::GetRegAllowMethods(IMS_SLOT_0);
    EXPECT_TRUE(regAllowMethods.IsEmpty());

    EXPECT_EQ(SipConfigProxy::GetRegContactUserInfoPart(IMS_SLOT_0),
            CarrierConfig::Ims::REGISTRATION_CONTACT_USER_INFO_PART_UUID);
    EXPECT_EQ(SipConfigProxy::GetRegExpires(IMS_SLOT_0), SipConfig::INVALID_EXPIRATION);
    EXPECT_EQ(SipConfigProxy::GetRegSubExpires(IMS_SLOT_0), SipConfig::INVALID_EXPIRATION);
    EXPECT_EQ(SipConfigProxy::GetRegUaString(IMS_SLOT_0), AString::ConstNull());

    EXPECT_FALSE(SipConfigProxy::IsRegExpiresConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsRegSubExpiresConfigured(IMS_SLOT_0));
    EXPECT_TRUE(SipConfigProxy::IsRegSubscriptionConfigured(IMS_SLOT_0));
    EXPECT_TRUE(SipConfigProxy::GetSipConfigV(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsAuthenticationAlgorithmRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsCellularNetworkInfoHeaderRequired(IMS_SLOT_0));
    EXPECT_TRUE(SipConfigProxy::IsCompactFormConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsContactInAll1xxRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsCountryParameterSupportedInPaniHeader(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsDisplayNameDquotRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsExpiresHeaderInRegRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsIpSecConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsGruuConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsKeepAliveConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsMultipleRegConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsRegIdParameterConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsNoAcceptContactHeaderInBye(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsPanInfoInInitialRegRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsPPreferredIdInRegSubRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsRouteHeaderInRegRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsRportConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsTransportErrorReportOnTxnRequired(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsTrustDomainConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsUdpFallbackConfigured(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsUserAgentConfigured(IMS_SLOT_0));
    EXPECT_TRUE(SipConfigProxy::IsUserAgentSetByContext(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsSdpNegotiationRequiredForNonRpr(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(IMS_SLOT_0));
    EXPECT_FALSE(
            SipConfigProxy::IsSipInstanceParamRequiredInContactForNonRegisterRequest(IMS_SLOT_0));
    EXPECT_FALSE(SipConfigProxy::IsSessionIdHeaderSupported(IMS_SLOT_0));
    EXPECT_EQ(
            SipConfigProxy::GetHideMacInPaniHeaderPolicy(IMS_SLOT_0), ISipConfig::HIDE_MAC_IN_PANI);
    EXPECT_FALSE(SipConfigProxy::IsLocalTimezoneParameterSupportedInPaniHeader(IMS_SLOT_0));

    EXPECT_EQ(SipConfigProxy::GetTimerValue100Trying(IMS_SLOT_0), 200);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT1(IMS_SLOT_0), 2000);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT2(IMS_SLOT_0), 16000);

    // without SipProfile and SipConfigV
    EXPECT_EQ(SipConfigProxy::GetTimerValueT1(IMS_SLOT_0, IMS_NULL, IMS_NULL), 2000);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT2(IMS_SLOT_0, IMS_NULL, IMS_NULL), 16000);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT4(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueA(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueB(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueD(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueE(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueF(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueG(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueH(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueI(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueJ(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueK(IMS_SLOT_0, IMS_NULL, IMS_NULL, IMS_FALSE), -1);

    SipConfigV objSipConfigV(IMS_SLOT_0);
    IConfigurable* piConfigurable =
            static_cast<const ISipConfigV*>(&objSipConfigV)->GetConfigurable();

    IMS_SINT32 nIntValue = 10;
    AString strIntValue;

    strIntValue.SetNumber(nIntValue);

    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T1, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T2, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_T4, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_A, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_B, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_C, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_D, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_E, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_F, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_G, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_H, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_I, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_J, strIntValue));
    EXPECT_TRUE(piConfigurable->Update(IConfigurable::CP_I_TIMER_K, strIntValue));

    // with SipConfigV
    EXPECT_EQ(SipConfigProxy::GetTimerValueT1(IMS_SLOT_0, IMS_NULL, &objSipConfigV), nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT2(IMS_SLOT_0, IMS_NULL, &objSipConfigV), nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT4(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueA(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueB(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueD(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueE(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueF(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueG(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueH(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueI(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueJ(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);
    EXPECT_EQ(SipConfigProxy::GetTimerValueK(IMS_SLOT_0, IMS_NULL, &objSipConfigV, IMS_FALSE),
            nIntValue);

    // with SipProfile
    SipProfile objSipProfile;
    IMS_SINT32 nDeviceId = 101;

    objSipProfile.SetDeviceId(nDeviceId);

    EXPECT_EQ(SipConfigProxy::GetDeviceId(IMS_SLOT_0, &objSipProfile), nDeviceId);

    AString strDeviceId("DeviceId");

    objSipProfile.SetPredefinedDeviceId(strDeviceId);

    EXPECT_EQ(SipConfigProxy::GetPredefinedDeviceId(IMS_SLOT_0, &objSipProfile), strDeviceId);

    nDefaultChannel = 999;

    objSipProfile.SetPort(nDefaultChannel);

    EXPECT_EQ(SipConfigProxy::GetPort(IMS_SLOT_0, &objSipProfile), nDefaultChannel);

    IMS_SINT32 nFeatures = 0xFFFFFFFF;

    objSipProfile.SetSipFeatureCaps(nFeatures);

    EXPECT_EQ(SipConfigProxy::GetSipFeatureCaps(IMS_SLOT_0, &objSipProfile), nFeatures);

    AString strTagPrefix("TagPrefix");

    objSipProfile.SetTagPrefix(strTagPrefix);

    EXPECT_EQ(SipConfigProxy::GetTagPrefix(IMS_SLOT_0, &objSipProfile), strTagPrefix);

    nTcpCriterionLength = 1024;

    objSipProfile.SetTcpCriterionLength(nTcpCriterionLength);

    EXPECT_EQ(
            SipConfigProxy::GetTcpCriterionLength(IMS_SLOT_0, &objSipProfile), nTcpCriterionLength);

    AString strUaString("UaString");

    objSipProfile.SetUaString(strUaString);
    objSipProfile.SetRegUaString(strUaString);

    EXPECT_EQ(SipConfigProxy::GetUaString(IMS_SLOT_0, &objSipProfile), strUaString);
    EXPECT_EQ(SipConfigProxy::GetRegUaString(IMS_SLOT_0, &objSipProfile), strUaString);

    AStringArray objAllowMethods;

    objAllowMethods.AddElement("INVITE");
    objAllowMethods.AddElement("BYE");
    objAllowMethods.AddElement("CANCEL");
    objAllowMethods.AddElement("ACK");
    objAllowMethods.AddElement("NOTIFY");
    objAllowMethods.AddElement("UPDATE");
    objAllowMethods.AddElement("REFER");
    objAllowMethods.AddElement("PRACK");
    objAllowMethods.AddElement("INFO");
    objAllowMethods.AddElement("MESSAGE");
    objAllowMethods.AddElement("OPTIONS");

    objSipProfile.SetRegAllowMethods(objAllowMethods);

    const AStringArray& regAllowedMethods =
            SipConfigProxy::GetRegAllowMethods(IMS_SLOT_0, &objSipProfile);
    EXPECT_EQ(regAllowedMethods.GetCount(), objAllowMethods.GetCount());

    IMS_SINT32 nExpires = 360;

    objSipProfile.SetRegExpires(nExpires);
    objSipProfile.SetRegSubExpires(nExpires);

    EXPECT_EQ(SipConfigProxy::GetRegExpires(IMS_SLOT_0, &objSipProfile), nExpires);
    EXPECT_EQ(SipConfigProxy::GetRegSubExpires(IMS_SLOT_0, &objSipProfile), nExpires);

    IMS_SINT32 nRegSub = 1;

    objSipProfile.SetRegSubscription(nRegSub);
    objSipProfile.SetHideMacInPaniHeaderPolicy(ISipConfig::HIDE_MAC_IN_PANI);

    EXPECT_TRUE(SipConfigProxy::IsRegExpiresConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsRegSubExpiresConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsRegSubscriptionConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsAuthenticationAlgorithmRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsCellularNetworkInfoHeaderRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsCompactFormConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsContactInAll1xxRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(
            SipConfigProxy::IsCountryParameterSupportedInPaniHeader(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsDisplayNameDquotRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsExpiresHeaderInRegRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsIpSecConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsGruuConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsKeepAliveConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsNoAcceptContactHeaderInBye(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsPanInfoInInitialRegRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsPPreferredIdInRegSubRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsRouteHeaderInRegRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsRportConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsTransportErrorReportOnTxnRequired(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsTrustDomainConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsUdpFallbackConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsUserAgentConfigured(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsUserAgentSetByContext(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsSdpNegotiationRequiredForNonRpr(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(
            SipConfigProxy::IsRequestUriValidationRequiredInMidDialog(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsSessionTimerUpdateRequiredByReInvite(IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsSipInstanceParamRequiredInContactForNonRegisterRequest(
            IMS_SLOT_0, &objSipProfile));
    EXPECT_TRUE(SipConfigProxy::IsSessionIdHeaderSupported(IMS_SLOT_0, &objSipProfile));
    EXPECT_EQ(SipConfigProxy::GetHideMacInPaniHeaderPolicy(IMS_SLOT_0, &objSipProfile),
            ISipConfig::HIDE_MAC_IN_PANI);
    EXPECT_TRUE(SipConfigProxy::IsLocalTimezoneParameterSupportedInPaniHeader(
            IMS_SLOT_0, &objSipProfile));

    IMS_SINT32 nT1 = 1;
    IMS_SINT32 nT2 = 1;

    SipTimerValues objSipTimerValues = SipTimerValues::CreateTimerValues(nT1, nT2);

    objSipProfile.SetTimerValues(objSipTimerValues);

    EXPECT_EQ(SipConfigProxy::GetTimerValueT1(IMS_SLOT_0, &objSipProfile, &objSipConfigV), nT1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueT2(IMS_SLOT_0, &objSipProfile, &objSipConfigV), nT2);
    EXPECT_EQ(
            SipConfigProxy::GetTimerValueT4(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT2 + 1000);
    EXPECT_EQ(SipConfigProxy::GetTimerValueA(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueB(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1 * 64);
    EXPECT_EQ(SipConfigProxy::GetTimerValueD(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1 * 64);
    EXPECT_EQ(SipConfigProxy::GetTimerValueE(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueF(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1 * 64);
    EXPECT_EQ(SipConfigProxy::GetTimerValueG(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1);
    EXPECT_EQ(SipConfigProxy::GetTimerValueH(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1 * 64);
    EXPECT_EQ(SipConfigProxy::GetTimerValueI(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT2 + 1000);
    EXPECT_EQ(SipConfigProxy::GetTimerValueJ(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT1 * 64);
    EXPECT_EQ(SipConfigProxy::GetTimerValueK(IMS_SLOT_0, &objSipProfile, &objSipConfigV, IMS_FALSE),
            nT2 + 1000);
}

}  // namespace android