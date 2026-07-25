/*
 * Copyright (C) 2026 The Android Open Source Project
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

#include "Credential.h"
#include "ImsStrLib.h"
#include "ISipHeader.h"
#include "SipAuHelper.h"
#include "SipStack.h"
#include "msg/SipMsgUtil.h"
#include <gtest/gtest.h>

namespace android
{

class SipAuHelperTest : public ::testing::Test
{
protected:
    void SetUp() override { SipMsgUtil::Init(); }

    static ::SipMessage* Decode(IN const IMS_CHAR* pszMessage)
    {
        ::SipMessage* pMessage = new ::SipMessage();
        if (!pMessage->Decode(pszMessage, IMS_StrLen(pszMessage)))
        {
            pMessage->SipDelete();
            return IMS_NULL;
        }
        return pMessage;
    }

    static AString FormAuthorization(IN IMS_SINT32 nCredentialType, IN IMS_BOOL bForceNonSessionAka)
    {
        const IMS_CHAR acChallenge[] =
                "SIP/2.0 401 Unauthorized\r\n"
                "Via: SIP/2.0/UDP 192.0.2.1;branch=z9hG4bK-test\r\n"
                "From: <sip:user@ims.example>;tag=local\r\n"
                "To: <sip:user@ims.example>;tag=remote\r\n"
                "Call-ID: auth-test\r\n"
                "CSeq: 1 REGISTER\r\n"
                "WWW-Authenticate: Digest realm=\"ims.example\",nonce=\"nonce\","
                "algorithm=AKAv1-MD5,qop=\"auth\"\r\n"
                "Content-Length: 0\r\n"
                "\r\n";
        const IMS_CHAR acRequest[] = "REGISTER sip:ims.example SIP/2.0\r\n"
                                     "Via: SIP/2.0/UDP 192.0.2.1;branch=z9hG4bK-test2\r\n"
                                     "From: <sip:user@ims.example>;tag=local\r\n"
                                     "To: <sip:user@ims.example>\r\n"
                                     "Call-ID: auth-test\r\n"
                                     "CSeq: 2 REGISTER\r\n"
                                     "Contact: <sip:user@192.0.2.1>\r\n"
                                     "Content-Length: 0\r\n"
                                     "\r\n";

        ::SipMessage* pChallenge = Decode(acChallenge);
        ::SipMessage* pRequest = Decode(acRequest);
        if (pChallenge == IMS_NULL || pRequest == IMS_NULL)
        {
            SipStack::FreeMessage(pChallenge);
            SipStack::FreeMessage(pRequest);
            return AString::ConstNull();
        }

        SipAuHelper objHelper;
        Credential objCredential("user", "password", "ims.example");
        objCredential.SetType(nCredentialType);
        if (!objHelper.SetChallenges(pChallenge) || !objHelper.AddCredential(objCredential) ||
                !objHelper.FormCredentials(pRequest, bForceNonSessionAka))
        {
            SipStack::FreeMessage(pChallenge);
            SipStack::FreeMessage(pRequest);
            return AString::ConstNull();
        }

        AString strAuthorization =
                SipStack::GetHeaderAsString(pRequest, ISipHeader::AUTHORIZATION, IMS_TRUE);
        SipStack::FreeMessage(pChallenge);
        SipStack::FreeMessage(pRequest);
        return strAuthorization;
    }
};

TEST_F(SipAuHelperTest, ForceNonSessionAkaOmitsQopParameters)
{
    AString strAuthorization = FormAuthorization(Credential::TYPE_AKAv1_MD5, IMS_TRUE);

    EXPECT_FALSE(strAuthorization.IsEmpty());
    EXPECT_FALSE(strAuthorization.Contains("qop="));
    EXPECT_FALSE(strAuthorization.Contains("cnonce="));
    EXPECT_FALSE(strAuthorization.Contains("nc="));
}

TEST_F(SipAuHelperTest, ForceNonSessionAkaDoesNotChangeMd5)
{
    AString strAuthorization = FormAuthorization(Credential::TYPE_MD5, IMS_TRUE);

    EXPECT_FALSE(strAuthorization.IsEmpty());
    EXPECT_TRUE(strAuthorization.Contains("qop=auth"));
    EXPECT_TRUE(strAuthorization.Contains("cnonce="));
    EXPECT_TRUE(strAuthorization.Contains("nc="));
}

}  // namespace android
