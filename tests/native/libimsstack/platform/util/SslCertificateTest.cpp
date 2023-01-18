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

#include "SslCertificate.h"

namespace android
{

class SslCertificateTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SslCertificateTest, Constructor)
{
    SslCertificate objSslCert;

    EXPECT_EQ(SslCertificate::FILETYPE_PEM, objSslCert.GetKeyFileType());
    ASSERT_TRUE(objSslCert.GetKeyFile().IsNull());
    ASSERT_TRUE(objSslCert.GetCaFile().IsNull());
    ASSERT_TRUE(objSslCert.GetCaPath().IsNull());
    ASSERT_TRUE(objSslCert.GetCiphers().IsNull());
    ASSERT_TRUE(objSslCert.GetPassword().IsNull());

    AString strKeyFilePem("/data/certificate.pem");
    SslCertificate objSslCert1(strKeyFilePem);

    EXPECT_EQ(SslCertificate::FILETYPE_PEM, objSslCert1.GetKeyFileType());
    EXPECT_STREQ(strKeyFilePem.GetStr(), objSslCert1.GetKeyFile().GetStr());
    ASSERT_TRUE(objSslCert1.GetCaFile().IsNull());
    ASSERT_TRUE(objSslCert1.GetCaPath().IsNull());
    ASSERT_TRUE(objSslCert1.GetCiphers().IsNull());
    ASSERT_TRUE(objSslCert1.GetPassword().IsNull());

    AString strKeyFileAsn1("/data/certificate.asn1");
    SslCertificate objTempSslCert(strKeyFileAsn1, SslCertificate::FILETYPE_ASN1);
    // This is a test to verify the copy constructor.
    // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
    SslCertificate objSslCert2(objTempSslCert);

    EXPECT_EQ(SslCertificate::FILETYPE_ASN1, objSslCert2.GetKeyFileType());
    EXPECT_STREQ(strKeyFileAsn1.GetStr(), objSslCert2.GetKeyFile().GetStr());
    ASSERT_TRUE(objSslCert2.GetCaFile().IsNull());
    ASSERT_TRUE(objSslCert2.GetCaPath().IsNull());
    ASSERT_TRUE(objSslCert2.GetCiphers().IsNull());
    ASSERT_TRUE(objSslCert2.GetPassword().IsNull());
}

TEST_F(SslCertificateTest, OperatorAssignment)
{
    AString strKeyFilePem("/data/certificate.pem");
    AString strCaFile = "sslcert.ca";
    AString strCaPath = "/data/ca";
    AString strCiphers = "abcdedfhig1234567890";
    AString strPassword = "1122334455";

    SslCertificate objSslCert(strKeyFilePem);

    objSslCert.SetCaFile(strCaFile);
    objSslCert.SetCaPath(strCaPath);
    objSslCert.SetCiphers(strCiphers);
    objSslCert.SetPassword(strPassword);

    SslCertificate objOtherSslCert = objSslCert;

    EXPECT_EQ(SslCertificate::FILETYPE_PEM, objOtherSslCert.GetKeyFileType());
    EXPECT_STREQ(strKeyFilePem.GetStr(), objOtherSslCert.GetKeyFile().GetStr());
    EXPECT_STREQ(strCaFile.GetStr(), objOtherSslCert.GetCaFile().GetStr());
    EXPECT_STREQ(strCaPath.GetStr(), objOtherSslCert.GetCaPath().GetStr());
    EXPECT_STREQ(strCiphers.GetStr(), objOtherSslCert.GetCiphers().GetStr());
    EXPECT_STREQ(strPassword.GetStr(), objOtherSslCert.GetPassword().GetStr());

    AString strKeyFileAsn1("/data/certificate.asn1");

    objSslCert.SetKeyFile(strKeyFileAsn1);
    objSslCert.SetKeyFileType(SslCertificate::FILETYPE_ASN1);

    objOtherSslCert = objSslCert;

    EXPECT_EQ(SslCertificate::FILETYPE_ASN1, objOtherSslCert.GetKeyFileType());
    EXPECT_STREQ(strKeyFileAsn1.GetStr(), objOtherSslCert.GetKeyFile().GetStr());
    EXPECT_STREQ(strCaFile.GetStr(), objOtherSslCert.GetCaFile().GetStr());
    EXPECT_STREQ(strCaPath.GetStr(), objOtherSslCert.GetCaPath().GetStr());
    EXPECT_STREQ(strCiphers.GetStr(), objOtherSslCert.GetCiphers().GetStr());
    EXPECT_STREQ(strPassword.GetStr(), objOtherSslCert.GetPassword().GetStr());
}

TEST_F(SslCertificateTest, Setter)
{
    AString strKeyFilePem("/data/certificate.pem");
    AString strCaFile = "sslcert.ca";
    AString strCaPath = "/data/ca";
    AString strCiphers = "abcdedfhig1234567890";
    AString strPassword = "1122334455";

    SslCertificate objSslCert(strKeyFilePem);

    objSslCert.SetCaFile(strCaFile);
    objSslCert.SetCaPath(strCaPath);
    objSslCert.SetCiphers(strCiphers);
    objSslCert.SetPassword(strPassword);

    EXPECT_EQ(SslCertificate::FILETYPE_PEM, objSslCert.GetKeyFileType());
    EXPECT_STREQ(strKeyFilePem.GetStr(), objSslCert.GetKeyFile().GetStr());
    EXPECT_STREQ(strCaFile.GetStr(), objSslCert.GetCaFile().GetStr());
    EXPECT_STREQ(strCaPath.GetStr(), objSslCert.GetCaPath().GetStr());
    EXPECT_STREQ(strCiphers.GetStr(), objSslCert.GetCiphers().GetStr());
    EXPECT_STREQ(strPassword.GetStr(), objSslCert.GetPassword().GetStr());

    AString strKeyFileAsn1("/data/certificate.asn1");

    objSslCert.SetKeyFile(strKeyFileAsn1);
    objSslCert.SetKeyFileType(SslCertificate::FILETYPE_ASN1);

    EXPECT_EQ(SslCertificate::FILETYPE_ASN1, objSslCert.GetKeyFileType());
    EXPECT_STREQ(strKeyFileAsn1.GetStr(), objSslCert.GetKeyFile().GetStr());
}

}  // namespace android
