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

#include "SipAbnfUtil.h"
#include "msg/SipAddrSpec.h"

namespace android
{

class SipUriTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipUriTest, CopyConstructor)
{
    SipUri* pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    pSipUri->SetUser("UserName");
    pSipUri->SetPassword("password");

    SipUri* pCopySipUri = new SipUri(*pSipUri);
    ASSERT_TRUE(pCopySipUri != nullptr);

    pSipUri->SipDelete();

    EXPECT_STREQ("UserName", pCopySipUri->GetUser());
    EXPECT_STREQ("password", pCopySipUri->GetPassword());

    pCopySipUri->SipDelete();
}

TEST_F(SipUriTest, IsValidComponent)
{
    SipUri* pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_FALSE, pSipUri->IsValidComponent(nullptr));
    EXPECT_EQ(SIP_FALSE, pSipUri->IsValidComponent("sip"));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_USER));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_PASSWORD));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_HOST));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_PORT));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_USER_PRM));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_METHOD));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_MADDR_PRM));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_TTL_PRM));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_TRNSPORT_PRM));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_LR_PRM));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_OTHER_PRM));
    EXPECT_EQ(SIP_TRUE, pSipUri->IsValidComponent(SIP_HEADERS));

    pSipUri->SipDelete();
}

TEST_F(SipUriTest, Encode)
{
    SipUri* pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    const SIP_INT32 BUFFER_SIZE = 4096;
    SIP_CHAR aBuffer[BUFFER_SIZE] = {
            0,
    };
    SIP_CHAR* pBuff = &(aBuffer[0]);

    AStringBuffer objBuffer(256);

    /* Empty SipUri, fail */
    EXPECT_EQ(SIP_FALSE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_FALSE, pSipUri->Encode(objBuffer, SIP_FALSE));

    /* user, password, host, port, uri params and header params present. success */
    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue?\
OnlyHeaderName&HeaderName=HeaderValue",
                    101));

    /* user,password,host,port,uri-params and header params present, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue?\
OnlyHeaderName&HeaderName=HeaderValue",
            &(aBuffer[0]));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue?\
OnlyHeaderName&HeaderName=HeaderValue",
            objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pSipUri->RemoveHdrParam("HeaderName");
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue?\
OnlyHeaderName",
            &(aBuffer[0]));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue?\
OnlyHeaderName",
            objBuffer.GetCharString());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("192.168.1.2:9090", 16));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    /* Only host and port present, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("192.168.1.2:9090", &(aBuffer[0]));
    EXPECT_STREQ("192.168.1.2:9090", objBuffer.GetCharString());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* host missing, fail */
    EXPECT_EQ(SIP_FALSE,
            pSipUri->Decode("UserName:password;OnlyUriName;\
UriName=UriValue?OnlyHeaderName&HeaderName=HeaderValue",
                    84));

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* HostName will be considered as host and port as unspecified, success */
    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("hostName;OnlyUriName;\
UriName=UriValue?OnlyHeaderName&HeaderName=HeaderValue",
                    75));

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("hostName;OnlyUriName;\
UriName=UriValue?OnlyHeaderName&HeaderName=HeaderValue",
            &(aBuffer[0]));
    EXPECT_STREQ("hostName;OnlyUriName;\
UriName=UriValue?OnlyHeaderName&HeaderName=HeaderValue",
            objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("UserName:password@192.168.1.2:9090", 34));

    /* user,password,host,port present.uri-params and header params absent, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090", &(aBuffer[0]));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090", objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("UserName:password@192.168.1.2:9090;\
OnlyUriName;UriName=UriValue",
                    63));

    /* user,password,host,port,uri-params present.header params absent, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue", &(aBuffer[0]));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090;OnlyUriName;UriName=UriValue",
            objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("UserName:password@192.168.1.2:9090?\
OnlyHeaderName&HeaderName=HeaderValue",
                    72));

    /* user,password,host,port,header params present.uri-params absent, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_TRUE));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090?OnlyHeaderName&HeaderName=HeaderValue",
            &(aBuffer[0]));
    EXPECT_STREQ("UserName:password@192.168.1.2:9090?OnlyHeaderName&HeaderName=HeaderValue",
            objBuffer.GetCharString());

    pBuff = &(aBuffer[0]);
    memset(pBuff, 0, BUFFER_SIZE);
    objBuffer = AString::ConstNull();

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("UserName:password@[2001::2]:9090", 32));

    /* user,password,host IPv6,port,header params present.uri-params absent, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(&pBuff));
    EXPECT_EQ(SIP_TRUE, pSipUri->Encode(objBuffer, SIP_FALSE));
    EXPECT_STREQ("UserName:password@[2001::2]:9090", &(aBuffer[0]));
    EXPECT_STREQ("UserName:password@[2001::2]:9090", objBuffer.GetCharString());

    pSipUri->SipDelete();
}

TEST_F(SipUriTest, Decode)
{
    SipUri* pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    EXPECT_EQ(SIP_FALSE, pSipUri->Decode(nullptr, 0));

    /* Only IPv4 host, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("192.168.1.2", 11));

    EXPECT_STREQ("192.168.1.2", pSipUri->GetHost());
    EXPECT_EQ(SIP_UNSPECIFIED_PORT, pSipUri->GetPort());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* Only IPv6 host, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("[2001::2]", 9));

    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(SIP_UNSPECIFIED_PORT, pSipUri->GetPort());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* Only IPv4 host with port, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("192.168.1.2:9090", 16));

    EXPECT_STREQ("192.168.1.2", pSipUri->GetHost());
    EXPECT_EQ(9090, pSipUri->GetPort());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* Only IPv6 host with port, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("[2001::2]:8080", 14));

    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(8080, pSipUri->GetPort());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* Only IPv6 host with port value 0, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("[2001::2]:0", 11));

    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(0, pSipUri->GetPort());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* Only IPv6 host with port value alphabet, SUCCESS */
    EXPECT_EQ(SIP_FALSE, pSipUri->Decode("[2001::2]:a", 11));

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* Only IPv6 host with port value alphabet, SUCCESS */
    EXPECT_EQ(SIP_FALSE, pSipUri->Decode("[2001::2]:12ab", 14));

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* user without password and host:port, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("UserName@192.168.1.2:9090", 25));

    EXPECT_STREQ("UserName", pSipUri->GetUser());
    EXPECT_STREQ("192.168.1.2", pSipUri->GetHost());
    EXPECT_EQ(9090, pSipUri->GetPort());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* user:password and host:port, SUCCESS */
    EXPECT_EQ(SIP_TRUE, pSipUri->Decode("UserName:Password@[2001::2]:8080", 32));

    EXPECT_STREQ("UserName", pSipUri->GetUser());
    EXPECT_STREQ("Password", pSipUri->GetPassword());
    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(8080, pSipUri->GetPort());
    EXPECT_EQ(0, pSipUri->GetUriParamCount());
    EXPECT_EQ(0, pSipUri->GetHdrParamCount());

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* user:password, host:port and Uri params, SUCCESS */
    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("UserName:Password@[2001::2]:8080;OnlyUriparam-name;"
                            "uriparam-name=uriparam-value",
                    79));

    EXPECT_STREQ("UserName", pSipUri->GetUser());
    EXPECT_STREQ("Password", pSipUri->GetPassword());
    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(8080, pSipUri->GetPort());
    EXPECT_EQ(2, pSipUri->GetUriParamCount());

    EXPECT_EQ(2, pSipUri->GetUriParamCount());
    SipNameValue* pNameVal = pSipUri->GetUriParam(0);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("OnlyUriparam-name", pNameVal->m_pszName);
    EXPECT_EQ(0, pNameVal->m_objValueList.GetSize());

    pNameVal = pSipUri->GetUriParam(1);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("uriparam-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("uriparam-value", pNameVal->m_objValueList.GetAt(0));

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* user:password, host:port and header params, SUCCESS */
    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("UserName:Password@[2001::2]:8080?OnlyHeaderName&"
                            "HeaderName=HeaderValue",
                    70));

    EXPECT_STREQ("UserName", pSipUri->GetUser());
    EXPECT_STREQ("Password", pSipUri->GetPassword());
    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(8080, pSipUri->GetPort());
    EXPECT_EQ(2, pSipUri->GetHdrParamCount());

    EXPECT_EQ(2, pSipUri->GetHdrParamCount());
    pNameVal = pSipUri->GetHdrParam(0);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("OnlyHeaderName", pNameVal->m_pszName);
    EXPECT_EQ(0, pNameVal->m_objValueList.GetSize());

    pNameVal = pSipUri->GetHdrParam(1);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("HeaderName", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("HeaderValue", pNameVal->m_objValueList.GetAt(0));

    pSipUri->SipDelete();

    pSipUri = new SipUri();
    ASSERT_TRUE(pSipUri != nullptr);

    /* user:password, host:port Uri params and header params, SUCCESS */
    EXPECT_EQ(SIP_TRUE,
            pSipUri->Decode("UserName:Password@[2001::2]:8080;OnlyUriparam-name;\
uriparam-name=uriparam-value?OnlyHeaderName&HeaderName=HeaderValue",
                    117));

    EXPECT_STREQ("UserName", pSipUri->GetUser());
    EXPECT_STREQ("Password", pSipUri->GetPassword());
    EXPECT_STREQ("2001::2", pSipUri->GetHost());
    EXPECT_EQ(8080, pSipUri->GetPort());

    EXPECT_EQ(2, pSipUri->GetUriParamCount());
    pNameVal = pSipUri->GetUriParam(0);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("OnlyUriparam-name", pNameVal->m_pszName);
    EXPECT_EQ(0, pNameVal->m_objValueList.GetSize());

    pNameVal = pSipUri->GetUriParam(1);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("uriparam-name", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("uriparam-value", pNameVal->m_objValueList.GetAt(0));

    EXPECT_EQ(2, pSipUri->GetHdrParamCount());
    pNameVal = pSipUri->GetHdrParam(0);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("OnlyHeaderName", pNameVal->m_pszName);
    EXPECT_EQ(0, pNameVal->m_objValueList.GetSize());

    pNameVal = pSipUri->GetHdrParam(1);
    ASSERT_TRUE(pNameVal != nullptr);
    EXPECT_STREQ("HeaderName", pNameVal->m_pszName);
    EXPECT_EQ(1, pNameVal->m_objValueList.GetSize());
    EXPECT_STREQ("HeaderValue", pNameVal->m_objValueList.GetAt(0));

    pSipUri->SipDelete();
}

}  // namespace android
