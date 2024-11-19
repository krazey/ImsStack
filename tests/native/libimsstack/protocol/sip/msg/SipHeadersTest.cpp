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
#include <gtest/gtest.h>

#include "msg/SipHeaders.h"
#include "msg/SipMsgUtil.h"
#include "platform/SipString.h"

namespace android
{

class SipHeadersTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override { SipMsgUtil::Init(); }

    virtual void TearDown() override {}
};

TEST_F(SipHeadersTest, createCoreHdrObject)
{
    SipHeaderBase* pAllowHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::ALLOW);
    ASSERT_TRUE(pAllowHdr != nullptr);

    SipHeaderBase* pInvalid = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TYPE_END);
    EXPECT_TRUE(pInvalid == nullptr);

    SipHeaderBase* pExpiresHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::EXPIRES_ANY);
    ASSERT_TRUE(pExpiresHdr != nullptr);

    SipHeaderBase* pContactHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTACT_ANY);
    ASSERT_TRUE(pContactHdr != nullptr);

    SipHeaderBase* pRetryHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::RETRY_AFTER_ANY);
    ASSERT_TRUE(pRetryHdr != nullptr);

    pInvalid = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TYPE_INVALID);
    EXPECT_TRUE(pInvalid == nullptr);

    pAllowHdr->SipDelete();
    pExpiresHdr->SipDelete();
    pContactHdr->SipDelete();
    pRetryHdr->SipDelete();
}

TEST_F(SipHeadersTest, CopyHdrs)
{
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
                             branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pViaHdr));
    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();

    SipHeaders* pNewHdrs = new SipHeaders();
    ASSERT_TRUE(pNewHdrs != nullptr);
    EXPECT_EQ(SIP_TRUE, pNewHdrs->CopyHdrs(pHdrs));

    SipIntegerHeader* pMaxForwardsNewHdr = reinterpret_cast<SipIntegerHeader*>(
            pNewHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsNewHdr != nullptr);
    EXPECT_EQ(70, pMaxForwardsNewHdr->GetValueInt());
    pMaxForwardsNewHdr->SipDelete();

    SipViaHeader* pViaNewHdr =
            reinterpret_cast<SipViaHeader*>(pNewHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaNewHdr != nullptr);
    const SIP_CHAR* pViaTransport = pViaNewHdr->GetTransport();
    EXPECT_STREQ("TCP", pViaTransport);
    pViaNewHdr->SipDelete();

    delete pHdrs;
    delete pNewHdrs;
}

TEST_F(SipHeadersTest, CloneHdrObject)
{
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
                      branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    SipHeaderBase* pMaxFwdCloneHdr = SipHeaders::CloneHdrObj(pMaxForwardsHdr);
    SipHeaderBase* pViaCloneHdr = SipHeaders::CloneHdrObj(pViaHdr);
    SipHeaderBase* pInvalidHdr = SipHeaders::CloneHdrObj(nullptr);

    ASSERT_TRUE(pMaxFwdCloneHdr != nullptr);
    ASSERT_TRUE(pViaCloneHdr != nullptr);
    EXPECT_TRUE(pInvalidHdr == nullptr);

    EXPECT_EQ(70, reinterpret_cast<SipIntegerHeader*>(pMaxFwdCloneHdr)->GetValueInt());
    const SIP_CHAR* pViaTransport = reinterpret_cast<SipViaHeader*>(pViaCloneHdr)->GetTransport();
    EXPECT_STREQ("TCP", pViaTransport);

    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();
    pMaxFwdCloneHdr->SipDelete();
    pViaCloneHdr->SipDelete();
}

TEST_F(SipHeadersTest, GetHdrObj)
{
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
                      branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pViaHdr));
    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();

    pMaxForwardsHdr = pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0);
    EXPECT_TRUE(pMaxForwardsHdr != nullptr);
    pMaxForwardsHdr->SipDelete();

    SipHeaderBase* pUserAgentHdr = pHdrs->GetHdrObj(SipHeaderBase::USER_AGENT, 0);
    EXPECT_TRUE(pUserAgentHdr == nullptr);

    pViaHdr = pHdrs->GetHdrObj(SipHeaderBase::VIA, 0);
    ASSERT_TRUE(pViaHdr != nullptr);
    pViaHdr->SipDelete();

    pViaHdr = pHdrs->GetHdrObj(SipHeaderBase::VIA, 5);
    EXPECT_TRUE(pViaHdr == nullptr);

    SipHeaderBase* pSecurityClientHdr = pHdrs->GetHdrObj(SipHeaderBase::SECURITY_CLIENT, 0);
    EXPECT_TRUE(pSecurityClientHdr == nullptr);

    EXPECT_TRUE(nullptr == pHdrs->GetHdrObj(SipHeaderBase::TYPE_END));
    EXPECT_TRUE(nullptr == pHdrs->GetHdrObj(SipHeaderBase::TYPE_INVALID));

    delete pHdrs;
}

TEST_F(SipHeadersTest, OverWriteHdrObj)
{
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);
    SipHeaderBase* pSecurityClientHdr =
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::SECURITY_CLIENT);
    ASSERT_TRUE(pSecurityClientHdr != nullptr);
    SipUnknownHeader* pUnknownHdr = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pSecurityClientValue = "ipsec-3gpp;alg=hmac-md5-96;prot=esp;mod=trans;\
            ealg=des-ede3-cbc;spi-c=768058;spi-s=768059;port-c=38003;\
            port-s=39002, ipsec-3gpp;alg=hmac-md5-96;prot=esp;\
            mod=trans;ealg=aes-cbc;spi-c=768058;spi-s=768059;\
            port-c=38003;port-s=39002, ipsec-3gpp;alg=hmac-md5-96;\
            prot=esp;mod=trans;ealg=null;spi-c=768058;spi-s=768059;\
            port-c=38003;port-s=39002, ipsec-3gpp;alg=hmac-sha-1-96;\
            prot=esp;mod=trans;ealg=des-ede3-cbc;spi-c=768058;\
            spi-s=768059;port-c=38003;port-s=39002, ipsec-3gpp;\
            alg=hmac-sha-1-96;prot=esp;mod=trans;ealg=aes-cbc;\
            spi-c=768058;spi-s=768059;port-c=38003;port-s=39002, \
            ipsec-3gpp;alg=hmac-sha-1-96;prot=esp;mod=trans;\
            ealg=null;spi-c=768058;spi-s=768059;port-c=38003;\
            port-s=39002";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE,
            pSecurityClientHdr->Decode(pSecurityClientValue, SipPf_Strlen(pSecurityClientValue)));

    pUnknownHdr->SetHeaderName("UnknownHeaderName");
    pUnknownHdr->SetHeaderValue("UnknownHeaderValue");

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pSecurityClientHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pUnknownHdr));
    pMaxForwardsHdr->SipDelete();
    pSecurityClientHdr->SipDelete();
    pUnknownHdr->SipDelete();

    SipHeaderBase* pMaxForwardsNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsNewHdr != nullptr);
    SipHeaderBase* pSecurityClientNewHdr =
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::SECURITY_CLIENT);
    ASSERT_TRUE(pSecurityClientNewHdr != nullptr);
    SipUnknownHeader* pUnknownNewHdr = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownNewHdr != nullptr);

    const SIP_CHAR* pMaxForwardsNewValue = "95";
    const SIP_CHAR* pSecurityClientNewValue = "ipsec-3gpp;alg=hmac-md5-96;prot=esp;mod=trans;\
            ealg=des-ede3-cbc;spi-c=768058;spi-s=768059;port-c=38003;\
            port-s=39002, ipsec-3gpp;alg=hmac-md5-96;prot=esp;mod=trans;\
            ealg=aes-cbc;spi-c=768058;spi-s=768059;port-c=38003;\
            port-s=39002";

    EXPECT_EQ(SIP_TRUE,
            pMaxForwardsNewHdr->Decode(pMaxForwardsNewValue, SipPf_Strlen(pMaxForwardsNewValue)));
    EXPECT_EQ(SIP_TRUE,
            pSecurityClientNewHdr->Decode(
                    pSecurityClientNewValue, SipPf_Strlen(pSecurityClientNewValue)));

    pUnknownNewHdr->SetHeaderName("UnknownHeaderNewName");
    pUnknownNewHdr->SetHeaderValue("UnknownHeaderNewValue");

    SipHeaders* pNewHdrs = new SipHeaders();
    ASSERT_TRUE(pNewHdrs != nullptr);

    EXPECT_EQ(SIP_TRUE, pNewHdrs->SetHdr(pMaxForwardsNewHdr));
    EXPECT_EQ(SIP_TRUE, pNewHdrs->SetHdr(pSecurityClientNewHdr));
    EXPECT_EQ(SIP_TRUE, pNewHdrs->SetHdr(pUnknownNewHdr));
    pMaxForwardsNewHdr->SipDelete();
    pSecurityClientNewHdr->SipDelete();
    pUnknownNewHdr->SipDelete();

    pHdrs->OverWriteHdrObj(pNewHdrs, SIP_FALSE);

    SipUnknownHeader* pOverwriteHdr =
            reinterpret_cast<SipUnknownHeader*>(pHdrs->GetHdrObj(SipHeaderBase::UNKNOWN, 0));
    ASSERT_TRUE(pOverwriteHdr != nullptr);
    EXPECT_STREQ("UnknownHeaderNewName", pOverwriteHdr->GetHeaderName());
    EXPECT_STREQ("UnknownHeaderNewValue", pOverwriteHdr->GetHeaderValue());
    pOverwriteHdr->SipDelete();

    SipUnknownHeader* pUnknownUpdatedHdr = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownUpdatedHdr != nullptr);
    pUnknownUpdatedHdr->SetHeaderName("UnknownHeaderUpdatedName");
    pUnknownUpdatedHdr->SetHeaderValue("UnknownHeaderUpdatedValue");
    EXPECT_EQ(SIP_TRUE, pNewHdrs->SetHdr(pUnknownUpdatedHdr));
    pUnknownUpdatedHdr->SipDelete();

    pHdrs->OverWriteHdrObj(pNewHdrs, SIP_FALSE);

    pOverwriteHdr =
            reinterpret_cast<SipUnknownHeader*>(pHdrs->GetHdrObj(SipHeaderBase::UNKNOWN, 0));
    ASSERT_TRUE(pOverwriteHdr != nullptr);
    EXPECT_STREQ("UnknownHeaderUpdatedName", pOverwriteHdr->GetHeaderName());
    EXPECT_STREQ("UnknownHeaderUpdatedValue", pOverwriteHdr->GetHeaderValue());
    pOverwriteHdr->SipDelete();

    delete pHdrs;
    delete pNewHdrs;
}

TEST_F(SipHeadersTest, GetHdrObj_Index)
{
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
            branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pViaHdr));
    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();

    pMaxForwardsHdr = pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);
    pMaxForwardsHdr->SipDelete();

    SipHeaderBase* pUserAgentHdr = pHdrs->GetHdrObj(SipHeaderBase::USER_AGENT);
    EXPECT_TRUE(pUserAgentHdr == nullptr);

    pViaHdr = pHdrs->GetHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    pViaHdr->SipDelete();

    EXPECT_TRUE(nullptr == pHdrs->GetHdrObj(SipHeaderBase::TYPE_END));
    EXPECT_TRUE(nullptr == pHdrs->GetHdrObj(SipHeaderBase::TYPE_INVALID));

    delete pHdrs;
}

TEST_F(SipHeadersTest, RemoveHdr)
{
    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_FALSE, pHdrs->RemoveHdr(SipHeaderBase::TYPE_END));
    EXPECT_EQ(SIP_FALSE, pHdrs->RemoveHdr(SipHeaderBase::TYPE_INVALID));

    SipIntegerHeader* pMaxForwardsHdr = reinterpret_cast<SipIntegerHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS));
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsHdr));
    pMaxForwardsHdr->SipDelete();

    EXPECT_EQ(SIP_TRUE, pHdrs->RemoveHdr(SipHeaderBase::MAX_FORWARDS));
    EXPECT_TRUE(nullptr == pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS));

    delete pHdrs;
}

TEST_F(SipHeadersTest, SetHdr)
{
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
            branch=z9hG4bK1422bd448-755bfe94";

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_FALSE, pHdrs->SetHdr(pViaHdr));

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    EXPECT_EQ(SIP_FALSE, pHdrs->SetHdr(nullptr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pViaHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsHdr));
    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();

    SipIntegerHeader* pMaxForwardsCheckHdr =
            reinterpret_cast<SipIntegerHeader*>(pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsCheckHdr != nullptr);
    EXPECT_EQ(70, pMaxForwardsCheckHdr->GetValueInt());
    pMaxForwardsCheckHdr->SipDelete();

    SipViaHeader* pViaCheckHdr =
            reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    const SIP_CHAR* pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("TCP", pBuff);
    pViaCheckHdr->SipDelete();

    SipHeaderBase* pMaxForwardsNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsNewHdr != nullptr);
    const SIP_CHAR* pMaxForwardsNewValue = "90";
    EXPECT_EQ(SIP_TRUE,
            pMaxForwardsNewHdr->Decode(pMaxForwardsNewValue, SipPf_Strlen(pMaxForwardsNewValue)));

    SipHeaderBase* pViaNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaNewHdr != nullptr);
    const SIP_CHAR* pViaNewValue = "SIP/2.0/UDP [2449:4431:241d:5ff5:b54d:c29a:ecea:88b8]:36007;\
            branch=z9hG4bK1422bd448-755bfe95";
    EXPECT_EQ(SIP_TRUE, pViaNewHdr->Decode(pViaNewValue, SipPf_Strlen(pViaNewValue)));

    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pMaxForwardsNewHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pViaNewHdr));
    pMaxForwardsNewHdr->SipDelete();
    pViaNewHdr->SipDelete();

    pMaxForwardsCheckHdr =
            reinterpret_cast<SipIntegerHeader*>(pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsCheckHdr != nullptr);
    EXPECT_EQ(90, pMaxForwardsCheckHdr->GetValueInt());
    pMaxForwardsCheckHdr->SipDelete();

    pViaCheckHdr = reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("UDP", pBuff);
    pViaCheckHdr->SipDelete();

    delete pHdrs;
}

TEST_F(SipHeadersTest, AppendHdr)
{
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
            branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_FALSE, pHdrs->AppendHdr(nullptr));
    EXPECT_EQ(SIP_TRUE, pHdrs->AppendHdr(pViaHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->AppendHdr(pMaxForwardsHdr));
    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();

    SipIntegerHeader* pMaxForwardsCheckHdr =
            reinterpret_cast<SipIntegerHeader*>(pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsCheckHdr != nullptr);
    EXPECT_EQ(70, pMaxForwardsCheckHdr->GetValueInt());
    pMaxForwardsCheckHdr->SipDelete();

    SipViaHeader* pViaCheckHdr =
            reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    const SIP_CHAR* pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("TCP", pBuff);
    pViaCheckHdr->SipDelete();

    SipHeaderBase* pMaxForwardsNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsNewHdr != nullptr);
    const SIP_CHAR* pMaxForwardsNewValue = "90";
    EXPECT_EQ(SIP_TRUE,
            pMaxForwardsNewHdr->Decode(pMaxForwardsNewValue, SipPf_Strlen(pMaxForwardsNewValue)));

    SipHeaderBase* pViaNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaNewHdr != nullptr);
    const SIP_CHAR* pViaNewValue = "SIP/2.0/UDP [2449:4431:241d:5ff5:b54d:c29a:ecea:88b8]:36007;\
            branch=z9hG4bK1422bd448-755bfe95";
    EXPECT_EQ(SIP_TRUE, pViaNewHdr->Decode(pViaNewValue, SipPf_Strlen(pViaNewValue)));

    EXPECT_EQ(SIP_TRUE, pHdrs->AppendHdr(pMaxForwardsNewHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->AppendHdr(pViaNewHdr));
    pMaxForwardsNewHdr->SipDelete();
    pViaNewHdr->SipDelete();

    pMaxForwardsCheckHdr =
            reinterpret_cast<SipIntegerHeader*>(pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsCheckHdr != nullptr);
    EXPECT_EQ(90, pMaxForwardsCheckHdr->GetValueInt());
    pMaxForwardsCheckHdr->SipDelete();

    pViaCheckHdr = reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("TCP", pBuff);
    pViaCheckHdr->SipDelete();

    pViaCheckHdr = reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 1));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("UDP", pBuff);
    pViaCheckHdr->SipDelete();

    delete pHdrs;
}

TEST_F(SipHeadersTest, InsertHdr)
{
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    SipHeaderBase* pMaxForwardsHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsHdr != nullptr);

    const SIP_CHAR* pMaxForwardsValue = "70";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
             branch=z9hG4bK1422bd448-755bfe94";

    EXPECT_EQ(
            SIP_TRUE, pMaxForwardsHdr->Decode(pMaxForwardsValue, SipPf_Strlen(pMaxForwardsValue)));
    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));

    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    EXPECT_EQ(SIP_FALSE, pHdrs->InsertHdr(nullptr, 0));
    EXPECT_EQ(SIP_TRUE, pHdrs->InsertHdr(pViaHdr, 0));
    EXPECT_EQ(SIP_TRUE, pHdrs->InsertHdr(pMaxForwardsHdr, 0));
    pMaxForwardsHdr->SipDelete();
    pViaHdr->SipDelete();

    SipIntegerHeader* pMaxForwardsCheckHdr =
            reinterpret_cast<SipIntegerHeader*>(pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsCheckHdr != nullptr);
    EXPECT_EQ(70, pMaxForwardsCheckHdr->GetValueInt());
    pMaxForwardsCheckHdr->SipDelete();

    SipViaHeader* pViaCheckHdr =
            reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    const SIP_CHAR* pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("TCP", pBuff);
    pViaCheckHdr->SipDelete();

    SipHeaderBase* pMaxForwardsNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::MAX_FORWARDS);
    ASSERT_TRUE(pMaxForwardsNewHdr != nullptr);
    const SIP_CHAR* pMaxForwardsNewValue = "90";
    EXPECT_EQ(SIP_TRUE,
            pMaxForwardsNewHdr->Decode(pMaxForwardsNewValue, SipPf_Strlen(pMaxForwardsNewValue)));

    SipHeaderBase* pViaNewHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaNewHdr != nullptr);
    const SIP_CHAR* pViaNewValue = "SIP/2.0/UDP [2449:4431:241d:5ff5:b54d:c29a:ecea:88b8]:36007;\
            branch=z9hG4bK1422bd448-755bfe95";
    EXPECT_EQ(SIP_TRUE, pViaNewHdr->Decode(pViaNewValue, SipPf_Strlen(pViaNewValue)));

    EXPECT_EQ(SIP_TRUE, pHdrs->InsertHdr(pMaxForwardsNewHdr, 0));
    EXPECT_EQ(SIP_TRUE, pHdrs->InsertHdr(pViaNewHdr, 0));
    pMaxForwardsNewHdr->SipDelete();
    pViaNewHdr->SipDelete();

    pMaxForwardsCheckHdr =
            reinterpret_cast<SipIntegerHeader*>(pHdrs->GetHdrObj(SipHeaderBase::MAX_FORWARDS, 0));
    ASSERT_TRUE(pMaxForwardsCheckHdr != nullptr);
    EXPECT_EQ(90, pMaxForwardsCheckHdr->GetValueInt());
    pMaxForwardsCheckHdr->SipDelete();

    pViaCheckHdr = reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 0));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("UDP", pBuff);
    pViaCheckHdr->SipDelete();

    pViaCheckHdr = reinterpret_cast<SipViaHeader*>(pHdrs->GetHdrObj(SipHeaderBase::VIA, 1));
    ASSERT_TRUE(pViaCheckHdr != nullptr);
    pBuff = pViaCheckHdr->GetTransport();
    EXPECT_STREQ("TCP", pBuff);
    pViaCheckHdr->SipDelete();

    delete pHdrs;
}

TEST_F(SipHeadersTest, Encode)
{
    SipHeaders* pHdrs = new SipHeaders();
    ASSERT_TRUE(pHdrs != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    /* Empty buffer */
    EXPECT_EQ(SIP_FALSE, pHdrs->Encode(&pBuff, SipConfiguration::MSG_OPT_ENCODE_NONE));

    SipUnknownHeader* pUnknownHdr = reinterpret_cast<SipUnknownHeader*>(
            SipHeaders::CreateCoreHdrObj(SipHeaderBase::UNKNOWN));
    ASSERT_TRUE(pUnknownHdr != nullptr);
    SipHeaderBase* pContentLengthHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTENT_LENGTH);
    ASSERT_TRUE(pContentLengthHdr != nullptr);

    SipHeaderBase* pSupportedHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::SUPPORTED);
    ASSERT_TRUE(pSupportedHdr != nullptr);
    SipHeaderBase* pToHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::TO);
    ASSERT_TRUE(pToHdr != nullptr);
    SipHeaderBase* pCallIDHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CALL_ID);
    ASSERT_TRUE(pCallIDHdr != nullptr);
    SipHeaderBase* pCSeqHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CSEQ);
    ASSERT_TRUE(pCSeqHdr != nullptr);
    SipHeaderBase* pContentTypeHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::CONTENT_TYPE);
    ASSERT_TRUE(pContentTypeHdr != nullptr);
    SipHeaderBase* pViaHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::VIA);
    ASSERT_TRUE(pViaHdr != nullptr);
    SipHeaderBase* pFromHdr = SipHeaders::CreateCoreHdrObj(SipHeaderBase::FROM);
    ASSERT_TRUE(pFromHdr != nullptr);

    const SIP_CHAR* pCallIdValue = "13217132a-3c0d31f9@2409:4031:241d:5ff5:b54d:c29a:ecea:88b8";
    const SIP_CHAR* pCSeqValue = "3 REGISTER";
    const SIP_CHAR* pContentTypeValue = "application/sdp";
    const SIP_CHAR* pSupportedValue = "path, eventlist, sec-agree";
    const SIP_CHAR* pViaValue = "SIP/2.0/TCP [2409:4031:241d:5ff5:b54d:c29a:ecea:88b8]:39002;\
            branch=z9hG4bK1422bd448-755bfe94";
    const SIP_CHAR* pFromValue = "<sip:405861079851317@ims.mnc861.mcc405.3gppnetwork.org>;\
                       tag=544671422bd42c-2899e679";
    const SIP_CHAR* pToValue = "<sip:405861079851317@ims.mnc861.mcc405.3gppnetwork.org>";
    const SIP_CHAR* pContentLengthValue = "120";

    pUnknownHdr->SetHeaderName("UnknownHeaderName");
    pUnknownHdr->SetHeaderValue("UnknownHeaderValue");

    EXPECT_EQ(SIP_TRUE, pViaHdr->Decode(pViaValue, SipPf_Strlen(pViaValue)));
    EXPECT_EQ(SIP_TRUE, pFromHdr->Decode(pFromValue, SipPf_Strlen(pFromValue)));
    EXPECT_EQ(SIP_TRUE, pToHdr->Decode(pToValue, SipPf_Strlen(pToValue)));
    EXPECT_EQ(SIP_TRUE, pCallIDHdr->Decode(pCallIdValue, SipPf_Strlen(pCallIdValue)));
    EXPECT_EQ(SIP_TRUE, pCSeqHdr->Decode(pCSeqValue, SipPf_Strlen(pCSeqValue)));
    EXPECT_EQ(
            SIP_TRUE, pContentTypeHdr->Decode(pContentTypeValue, SipPf_Strlen(pContentTypeValue)));
    EXPECT_EQ(SIP_TRUE, pSupportedHdr->Decode(pSupportedValue, SipPf_Strlen(pSupportedValue)));
    EXPECT_EQ(SIP_TRUE,
            pContentLengthHdr->Decode(pContentLengthValue, SipPf_Strlen(pContentLengthValue)));

    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pCallIDHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pCSeqHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pContentTypeHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pSupportedHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pToHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pUnknownHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pViaHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pContentLengthHdr));
    EXPECT_EQ(SIP_TRUE, pHdrs->SetHdr(pFromHdr));

    pViaHdr->SipDelete();
    pFromHdr->SipDelete();
    pToHdr->SipDelete();
    pCallIDHdr->SipDelete();
    pCSeqHdr->SipDelete();
    pContentTypeHdr->SipDelete();
    pSupportedHdr->SipDelete();
    pUnknownHdr->SipDelete();
    pContentLengthHdr->SipDelete();

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);

    SIP_CHAR aShortFormBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pShortFormBuff = &(aShortFormBuffer[0]);

    /* Encode headers */
    EXPECT_EQ(SIP_TRUE, pHdrs->Encode(&pBuff, SipConfiguration::MSG_OPT_ENCODE_NONE));
    EXPECT_EQ(
            SIP_TRUE, pHdrs->Encode(&pShortFormBuff, SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM));

    SIP_UINT32 uiLength = pBuff - &(aBuffer[0]);

    const SIP_CHAR* pucStartPt = &(aBuffer[0]);
    const SIP_CHAR* pucEndPt = pucStartPt + uiLength;
    const SIP_CHAR* pucTempPos = SIP_NULL;

    SIP_BOOL bHdrEnd = SIP_FALSE;

    const SIP_INT32 NUM_OF_HEADERS = 9;

    const SIP_INT32 arHeadersOrder[NUM_OF_HEADERS] = {
            SipHeaderBase::VIA,
            SipHeaderBase::FROM,
            SipHeaderBase::TO,
            SipHeaderBase::CALL_ID,
            SipHeaderBase::CSEQ,
            SipHeaderBase::SUPPORTED,
            SipHeaderBase::CONTENT_TYPE,
            SipHeaderBase::UNKNOWN,
            SipHeaderBase::CONTENT_LENGTH,
    };

    SIP_INT32 iHeaderCount = 0;

    /* Check encoded buffer has headers as per the order in arHeadersOrder */
    while ((pucStartPt < pucEndPt) && (bHdrEnd == SIP_FALSE))
    {
        SIP_UINT32 uiDecLen = 0;

        SipAbnfUtil::FindTerminatingCrlf(pucStartPt, pucEndPt, pucTempPos, bHdrEnd);

        uiDecLen = pucTempPos - pucStartPt + SIP_ONE;

        SIP_CHAR* pucHdrName = SIP_NULL;
        SIP_CHAR* pucHdrBody = SIP_NULL;

        pHdrs->Decode(pucStartPt, uiDecLen, &pucHdrName, &pucHdrBody);

        SIP_INT32 eHdrType = SipMsgUtil::GetHeaderType(pucHdrName);

        EXPECT_EQ(arHeadersOrder[iHeaderCount], eHdrType);
        iHeaderCount++;

        pucStartPt = pucTempPos + SIP_THREE;
        pucTempPos = SIP_NULL;
    }

    EXPECT_EQ(iHeaderCount + 1, NUM_OF_HEADERS);

    iHeaderCount = 0;
    bHdrEnd = SIP_FALSE;
    uiLength = pShortFormBuff - &(aShortFormBuffer[0]);
    pucStartPt = &(aShortFormBuffer[0]);
    pucEndPt = pucStartPt + uiLength;
    pucTempPos = SIP_NULL;

    /* Check encoded short form buffer has headers as per the order in arHeadersOrder */
    while ((pucStartPt < pucEndPt) && (bHdrEnd == SIP_FALSE))
    {
        SIP_UINT32 uiDecLen = 0;

        SipAbnfUtil::FindTerminatingCrlf(pucStartPt, pucEndPt, pucTempPos, bHdrEnd);

        uiDecLen = pucTempPos - pucStartPt + SIP_ONE;

        SIP_CHAR* pucHdrName = SIP_NULL;
        SIP_CHAR* pucHdrBody = SIP_NULL;

        pHdrs->Decode(pucStartPt, uiDecLen, &pucHdrName, &pucHdrBody);

        SIP_INT32 eHdrType = SipMsgUtil::GetHeaderType(pucHdrName);

        EXPECT_EQ(arHeadersOrder[iHeaderCount], eHdrType);
        iHeaderCount++;

        pucStartPt = pucTempPos + SIP_THREE;
        pucTempPos = SIP_NULL;
    }

    EXPECT_EQ(iHeaderCount + 1, NUM_OF_HEADERS);
}

TEST_F(SipHeadersTest, Decode)
{
    SIP_CHAR* pszHeaderName = nullptr;
    SIP_CHAR* pszHeaderBody = nullptr;
    SipHeaders* pHdrs = new SipHeaders();

    const SIP_CHAR* pHeaderBuffer = "Content-Type: application/sdp";
    EXPECT_EQ(SIP_TRUE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));
    ASSERT_TRUE(nullptr != pszHeaderName);
    EXPECT_STREQ("Content-Type", pszHeaderName);
    ASSERT_TRUE(nullptr != pszHeaderBody);
    EXPECT_STREQ("application/sdp", pszHeaderBody);
    SipHeaderBase* pContentTypeHdr = pHdrs->GetHdrObj(SipHeaderBase::CONTENT_TYPE);
    ASSERT_TRUE(nullptr != pContentTypeHdr);
    pContentTypeHdr->SipDelete();
    delete[] pszHeaderName;
    delete[] pszHeaderBody;
    pszHeaderName = nullptr;
    pszHeaderBody = nullptr;

    pHeaderBuffer = "UnknownHeaderName: UnknownHeaderBody";
    EXPECT_EQ(SIP_TRUE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));
    ASSERT_TRUE(nullptr != pszHeaderName);
    EXPECT_STREQ("UnknownHeaderName", pszHeaderName);
    ASSERT_TRUE(nullptr != pszHeaderBody);
    EXPECT_STREQ("UnknownHeaderBody", pszHeaderBody);
    SipHeaderBase* pUnknownHdr = pHdrs->GetHdrObj(SipHeaderBase::UNKNOWN);
    ASSERT_TRUE(nullptr != pUnknownHdr);
    pUnknownHdr->SipDelete();
    delete[] pszHeaderName;
    delete[] pszHeaderBody;
    pszHeaderName = nullptr;
    pszHeaderBody = nullptr;

    pHeaderBuffer = "UnknownHeaderName    :       UnknownHeaderBody";
    EXPECT_EQ(SIP_TRUE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));
    ASSERT_TRUE(nullptr != pszHeaderName);
    EXPECT_STREQ("UnknownHeaderName", pszHeaderName);
    ASSERT_TRUE(nullptr != pszHeaderBody);
    EXPECT_STREQ("UnknownHeaderBody", pszHeaderBody);
    pUnknownHdr = pHdrs->GetHdrObj(SipHeaderBase::UNKNOWN);
    ASSERT_TRUE(nullptr != pUnknownHdr);
    pUnknownHdr->SipDelete();
    delete[] pszHeaderName;
    delete[] pszHeaderBody;
    pszHeaderName = nullptr;
    pszHeaderBody = nullptr;

    pHeaderBuffer = "cseq: 1 REGISTER";
    ASSERT_EQ(SIP_TRUE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));
    delete[] pszHeaderName;
    delete[] pszHeaderBody;
    pszHeaderName = nullptr;
    pszHeaderBody = nullptr;

    // Decoding again single header cseq fails.
    EXPECT_EQ(SIP_FALSE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));
    ASSERT_TRUE(nullptr != pszHeaderName);
    delete[] pszHeaderName;
    ASSERT_TRUE(nullptr != pszHeaderBody);
    delete[] pszHeaderBody;
    pszHeaderName = nullptr;
    pszHeaderBody = nullptr;

    pHeaderBuffer = "ThisIsNotHeader";
    EXPECT_EQ(SIP_FALSE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));

    pHeaderBuffer = "UnknownHeaderWithNoValue:";
    EXPECT_EQ(SIP_TRUE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));

    pHeaderBuffer = ": HeaderValueWithoutName";
    EXPECT_EQ(SIP_FALSE,
            pHdrs->Decode(
                    pHeaderBuffer, SipPf_Strlen(pHeaderBuffer), &pszHeaderName, &pszHeaderBody));

    EXPECT_EQ(SIP_FALSE, pHdrs->Decode(nullptr, 0, &pszHeaderName, &pszHeaderBody));
    EXPECT_EQ(SIP_FALSE, pHdrs->Decode("", 0, &pszHeaderName, &pszHeaderBody));

    delete pHdrs;
}

TEST_F(SipHeadersTest, SipEncodeShortHdrName)
{
    const SIP_INT32 BUFFER_SIZE = 64;
    SIP_CHAR aBuff[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_FALSE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::TYPE_INVALID, &pBuff));

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_FALSE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::TYPE_END, &pBuff));

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::VIA, &pBuff));
    EXPECT_STREQ("v: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::TO, &pBuff));
    EXPECT_STREQ("t: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::FROM, &pBuff));
    EXPECT_STREQ("f: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CALL_ID, &pBuff));
    EXPECT_STREQ("i: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CONTACT, &pBuff));
    EXPECT_STREQ("m: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CONTACT_WILD, &pBuff));
    EXPECT_STREQ("m: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CONTACT_ANY, &pBuff));
    EXPECT_STREQ("m: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CONTENT_TYPE, &pBuff));
    EXPECT_STREQ("c: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CONTENT_LENGTH, &pBuff));
    EXPECT_STREQ("l: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::ACCEPT_CONTACT, &pBuff));
    EXPECT_STREQ("a: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::SESSION_EXPIRES, &pBuff));
    EXPECT_STREQ("x: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::SUPPORTED, &pBuff));
    EXPECT_STREQ("k: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE,
            SipHeaders::SipEncodeShortHdrName(SipHeaderBase::REQUEST_DISPOSITION, &pBuff));
    EXPECT_STREQ("d: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::REFERRED_BY, &pBuff));
    EXPECT_STREQ("b: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::REFER_TO, &pBuff));
    EXPECT_STREQ("r: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::CONTENT_ENCODING, &pBuff));
    EXPECT_STREQ("e: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::SUBJECT, &pBuff));
    EXPECT_STREQ("s: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::REJECT_CONTACT, &pBuff));
    EXPECT_STREQ("j: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::EVENT, &pBuff));
    EXPECT_STREQ("o: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::ALLOW_EVENTS, &pBuff));
    EXPECT_STREQ("u: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::IDENTITY, &pBuff));
    EXPECT_STREQ("y: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::IDENTITY_INFO, &pBuff));
    EXPECT_STREQ("n: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE, SipHeaders::SipEncodeShortHdrName(SipHeaderBase::ALLOW, &pBuff));
    EXPECT_STREQ("Allow: ", &aBuff[0]);
    pBuff = &aBuff[0];
}

TEST_F(SipHeadersTest, IsListHdr)
{
    SipHeaders objHeaders;
    EXPECT_EQ(SIP_FALSE, objHeaders.IsListHdr(SipHeaderBase::TYPE_INVALID));
    EXPECT_EQ(SIP_FALSE, objHeaders.IsListHdr(SipHeaderBase::TYPE_END));
    EXPECT_EQ(SIP_FALSE, objHeaders.IsListHdr(SipHeaderBase::TYPE_END + 1));
    EXPECT_EQ(SIP_FALSE, objHeaders.IsListHdr(SipHeaderBase::CALL_ID));
    EXPECT_EQ(SIP_TRUE, objHeaders.IsListHdr(SipHeaderBase::VIA));
}

TEST_F(SipHeadersTest, SipEncodeHdrName)
{
    const SIP_INT32 BUFFER_SIZE = 64;
    SIP_CHAR aBuff[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);

    EXPECT_EQ(SIP_FALSE,
            SipHeaders::SipEncodeHdrName(
                    SipHeaderBase::TYPE_INVALID, &pBuff, SipConfiguration::MSG_OPT_ENCODE_NONE));
    EXPECT_EQ(SIP_FALSE,
            SipHeaders::SipEncodeHdrName(
                    SipHeaderBase::TYPE_END, &pBuff, SipConfiguration::MSG_OPT_ENCODE_NONE));
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE,
            SipHeaders::SipEncodeHdrName(
                    SipHeaderBase::ALLOW, &pBuff, SipConfiguration::MSG_OPT_ENCODE_NONE));
    EXPECT_STREQ("Allow: ", &aBuff[0]);
    pBuff = &aBuff[0];

    memset(pBuff, 0, BUFFER_SIZE);
    EXPECT_EQ(SIP_TRUE,
            SipHeaders::SipEncodeHdrName(
                    SipHeaderBase::EVENT, &pBuff, SipConfiguration::MSG_OPT_ENCODE_SHORT_FORM));
    EXPECT_STREQ("o: ", &aBuff[0]);
    pBuff = &aBuff[0];
}

}  // namespace android
