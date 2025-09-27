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
#include <gtest/gtest.h>

#include "ISipHeader.h"
#include "SipParsingHelper.h"
#include "SipSecurityHeader.h"

namespace android
{

class SipSecurityHeaderTest : public ::testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}

    inline SipSecurityHeader CreateDefaultHeader()
    {
        SipSecurityHeader objHeader;

        objHeader.SetAlgorithm(SipSecurityHeader::ALG_HMAC_SHA_1_96);
        objHeader.SetEncryptionAlgorithm(SipSecurityHeader::EALG_NULL);
        objHeader.SetPreference(PREFERENCE);
        objHeader.SetMode(SipSecurityHeader::MODE_TRANSPORT);
        objHeader.SetProtocol(SipSecurityHeader::PROTOCOL_ESP);
        objHeader.SetPort(PORT_C, PORT_S);
        objHeader.SetSpi(SPI_C, SPI_S);
        objHeader.SetExtensionParameter(EXTENSION_KEY, EXTENSION_VALUE);
        objHeader.SetUnknownParameterValue(SipSecurityHeader::SEC_P_ALG, UNKNOWN_ALGORITHM);

        return objHeader;
    }

protected:
    static constexpr IMS_SINT32 PORT_C = 40001;
    static constexpr IMS_SINT32 PORT_S = 50001;
    static constexpr IMS_SINT32 SPI_C = 100;
    static constexpr IMS_SINT32 SPI_S = 200;
    static constexpr const IMS_CHAR* PREFERENCE = "0.5";
    static constexpr const IMS_CHAR* UNKNOWN_ALGORITHM = "test-alg";
    static constexpr const IMS_CHAR* EXTENSION_KEY = "test-key";
    static constexpr const IMS_CHAR* EXTENSION_VALUE = "test-value";
};

TEST_F(SipSecurityHeaderTest, Constructor)
{
    SipSecurityHeader objHeader;

    EXPECT_EQ(objHeader.GetMechanism(), SipSecurityHeader::MECHANISM_IPSEC_3GPP);
    EXPECT_EQ(objHeader.GetAlgorithm(), SipSecurityHeader::ALG_HMAC_SHA_1_96);
    EXPECT_EQ(objHeader.GetEncryptionAlgorithm(), SipSecurityHeader::EALG_NULL);
    EXPECT_EQ(objHeader.GetProtocol(), SipSecurityHeader::PROTOCOL_ESP);
    EXPECT_EQ(objHeader.GetMode(), SipSecurityHeader::MODE_TRANSPORT);

    SipSecurityHeader objDigestHeader(SipSecurityHeader::MECHANISM_DIGEST);

    EXPECT_EQ(objDigestHeader.GetMechanism(), SipSecurityHeader::MECHANISM_DIGEST);

    const AString strUnknownMechanism("test");
    SipSecurityHeader objUnknownHeader(SipSecurityHeader::MECHANISM_UNKNOWN, strUnknownMechanism);

    EXPECT_EQ(objUnknownHeader.GetMechanism(), SipSecurityHeader::MECHANISM_UNKNOWN);
    EXPECT_EQ(objUnknownHeader.GetUnknownMechanism(), strUnknownMechanism);
}

TEST_F(SipSecurityHeaderTest, CopyConstructor)
{
    SipSecurityHeader objHeader = CreateDefaultHeader();
    SipSecurityHeader objNewHeader(objHeader);

    EXPECT_EQ(objNewHeader.GetMechanism(), SipSecurityHeader::MECHANISM_IPSEC_3GPP);
    EXPECT_EQ(objNewHeader.GetAlgorithm(), SipSecurityHeader::ALG_HMAC_SHA_1_96);
    EXPECT_EQ(objNewHeader.GetEncryptionAlgorithm(), SipSecurityHeader::EALG_NULL);
    EXPECT_STREQ(objNewHeader.GetPreference().GetStr(), PREFERENCE);
    EXPECT_EQ(objNewHeader.GetMode(), SipSecurityHeader::MODE_TRANSPORT);
    EXPECT_EQ(objNewHeader.GetProtocol(), SipSecurityHeader::PROTOCOL_ESP);
    EXPECT_EQ(objNewHeader.GetPortC(), PORT_C);
    EXPECT_EQ(objNewHeader.GetPortS(), PORT_S);
    EXPECT_EQ(objNewHeader.GetSpiC(), SPI_C);
    EXPECT_EQ(objNewHeader.GetSpiS(), SPI_S);
    EXPECT_STREQ(objNewHeader.GetUnknownParameterValue(SipSecurityHeader::SEC_P_ALG).GetStr(),
            UNKNOWN_ALGORITHM);
    EXPECT_TRUE(objNewHeader.IsParameterPresent(SipSecurityHeader::PORT_C));
    EXPECT_TRUE(objNewHeader.IsParameterPresent(SipSecurityHeader::PORT_S));
    EXPECT_TRUE(objNewHeader.IsParameterPresent(SipSecurityHeader::SPI_C));
    EXPECT_TRUE(objNewHeader.IsParameterPresent(SipSecurityHeader::SPI_S));

    const ImsMap<AString, AString>& objExtensionParams = objNewHeader.GetExtensionParameters();
    EXPECT_STREQ(objExtensionParams.GetValue(EXTENSION_KEY).GetStr(), EXTENSION_VALUE);

    const ImsMap<IMS_SINT32, AString>& objUnknownParams = objNewHeader.GetUnknownParameterValues();
    EXPECT_STREQ(
            objUnknownParams.GetValue(SipSecurityHeader::SEC_P_ALG).GetStr(), UNKNOWN_ALGORITHM);
}

TEST_F(SipSecurityHeaderTest, ToString)
{
    SipSecurityHeader objHeader = CreateDefaultHeader();
    AString strHeader = objHeader.ToString();

    EXPECT_TRUE(strHeader.Contains(SipSecurityHeader::P_VALUE_MECHANISM_IPSEC_3GPP));
    EXPECT_TRUE(strHeader.Contains(SipSecurityHeader::P_VALUE_ALG_HMAC_SHA_1_96));
    EXPECT_TRUE(strHeader.Contains(SipSecurityHeader::P_VALUE_EALG_NULL));
    EXPECT_TRUE(strHeader.Contains(PREFERENCE));
    EXPECT_TRUE(strHeader.Contains(SipSecurityHeader::P_VALUE_MOD_TRANS));
    EXPECT_TRUE(strHeader.Contains(SipSecurityHeader::P_VALUE_PROT_ESP));

    AString strIntValue;
    strIntValue.SetNumber(PORT_C);
    EXPECT_TRUE(strHeader.Contains(strIntValue));

    strIntValue.SetNumber(PORT_S);
    EXPECT_TRUE(strHeader.Contains(strIntValue));

    strIntValue.Sprintf("%010u", SPI_C);
    EXPECT_TRUE(strHeader.Contains(strIntValue));

    strIntValue.Sprintf("%010u", SPI_S);
    EXPECT_TRUE(strHeader.Contains(strIntValue));

    EXPECT_TRUE(strHeader.Contains(EXTENSION_KEY));
    EXPECT_TRUE(strHeader.Contains(EXTENSION_VALUE));
}

TEST_F(SipSecurityHeaderTest, FromSipHeader)
{
    AString strHeaderValue;
    strHeaderValue.Sprintf(
            "%s;q=%s;alg=%s;prot=%s;mod=%s;ealg=%s;spi-c=%010u;spi-s=%010u;port-c=%d;port-s=%d",
            SipSecurityHeader::P_VALUE_MECHANISM_IPSEC_3GPP, PREFERENCE,
            SipSecurityHeader::P_VALUE_ALG_HMAC_SHA_1_96, SipSecurityHeader::P_VALUE_PROT_ESP,
            SipSecurityHeader::P_VALUE_MOD_TRANS, SipSecurityHeader::P_VALUE_EALG_AES_CBC, SPI_C,
            SPI_S, PORT_C, PORT_S);
    ISipHeader* piSipHeader =
            SipParsingHelper::CreateHeader(ISipHeader::SECURITY_CLIENT, strHeaderValue);

    ASSERT_TRUE(piSipHeader != nullptr);

    SipSecurityHeader* pHeader = SipSecurityHeader::FromSipHeader(piSipHeader);
    piSipHeader->Destroy();

    ASSERT_TRUE(pHeader != nullptr);

    EXPECT_EQ(pHeader->GetMechanism(), SipSecurityHeader::MECHANISM_IPSEC_3GPP);
    EXPECT_EQ(pHeader->GetAlgorithm(), SipSecurityHeader::ALG_HMAC_SHA_1_96);
    EXPECT_EQ(pHeader->GetEncryptionAlgorithm(), SipSecurityHeader::EALG_AES_CBC);
    EXPECT_STREQ(pHeader->GetPreference().GetStr(), PREFERENCE);
    EXPECT_EQ(pHeader->GetMode(), SipSecurityHeader::MODE_TRANSPORT);
    EXPECT_EQ(pHeader->GetProtocol(), SipSecurityHeader::PROTOCOL_ESP);
    EXPECT_EQ(pHeader->GetPortC(), PORT_C);
    EXPECT_EQ(pHeader->GetPortS(), PORT_S);
    EXPECT_EQ(pHeader->GetSpiC(), SPI_C);
    EXPECT_EQ(pHeader->GetSpiS(), SPI_S);
    EXPECT_TRUE(pHeader->IsParameterPresent(SipSecurityHeader::PORT_C));
    EXPECT_TRUE(pHeader->IsParameterPresent(SipSecurityHeader::PORT_S));
    EXPECT_TRUE(pHeader->IsParameterPresent(SipSecurityHeader::SPI_C));
    EXPECT_TRUE(pHeader->IsParameterPresent(SipSecurityHeader::SPI_S));

    delete pHeader;
}

TEST_F(SipSecurityHeaderTest, IsSecurityMechanismMatched)
{
    SipSecurityHeader objHeader1 = CreateDefaultHeader();
    SipSecurityHeader objHeader2 = CreateDefaultHeader();

    EXPECT_TRUE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    IMS_SINT32 nOldValue = objHeader2.GetAlgorithm();
    objHeader2.SetAlgorithm(SipSecurityHeader::ALG_HMAC_MD5_96);

    // Different integrity algorithm
    EXPECT_FALSE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetAlgorithm(nOldValue);
    nOldValue = objHeader2.GetEncryptionAlgorithm();
    objHeader2.SetEncryptionAlgorithm(SipSecurityHeader::EALG_AES_CBC);

    // Different encryption algorithm
    EXPECT_FALSE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetEncryptionAlgorithm(nOldValue);
    nOldValue = objHeader2.GetProtocol();
    objHeader2.SetProtocol(SipSecurityHeader::PROTOCOL_AH);

    // Different protocol
    EXPECT_FALSE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetProtocol(nOldValue);
    nOldValue = objHeader2.GetMode();
    objHeader2.SetMode(SipSecurityHeader::MODE_TUNNEL);

    // Different mode
    EXPECT_FALSE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetMode(nOldValue);
    nOldValue = objHeader2.GetEncryptionAlgorithm();
    objHeader2.SetEncryptionAlgorithm(SipSecurityHeader::EALG_UNSPECIFIED);

    // Default encryption algorithm: null
    EXPECT_TRUE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetEncryptionAlgorithm(nOldValue);
    nOldValue = objHeader2.GetProtocol();
    objHeader2.SetProtocol(SipSecurityHeader::PROTOCOL_UNSPECIFIED);

    // Default protocol: esp
    EXPECT_TRUE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetProtocol(nOldValue);
    nOldValue = objHeader2.GetMode();
    objHeader2.SetMode(SipSecurityHeader::MODE_UNSPECIFIED);

    // Default mode: transport
    EXPECT_TRUE(objHeader1.IsSecurityMechanismMatched(objHeader2));

    objHeader2.SetMode(nOldValue);

    // Different mechanism
    SipSecurityHeader objHeader3(SipSecurityHeader::MECHANISM_DIGEST);
    EXPECT_FALSE(objHeader1.IsSecurityMechanismMatched(objHeader3));

    // Same unknown mechanism
    SipSecurityHeader objHeader4(SipSecurityHeader::MECHANISM_UNKNOWN, "test");
    SipSecurityHeader objHeader5(SipSecurityHeader::MECHANISM_UNKNOWN, "test");
    EXPECT_TRUE(objHeader4.IsSecurityMechanismMatched(objHeader5));

    // Different unknown mechanism
    SipSecurityHeader objHeader6(SipSecurityHeader::MECHANISM_UNKNOWN, "test1");
    SipSecurityHeader objHeader7(SipSecurityHeader::MECHANISM_UNKNOWN, "test2");
    EXPECT_FALSE(objHeader6.IsSecurityMechanismMatched(objHeader7));
}

}  // namespace android
