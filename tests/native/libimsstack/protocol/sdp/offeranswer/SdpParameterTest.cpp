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
#include <gtest/gtest.h>

#include "AStringArray.h"

#include "Sdp.h"
#include "SdpAttribute.h"
#include "SdpBandwidth.h"
#include "SdpDescription.h"
#include "offeranswer/SdpParameter.h"

namespace android
{

class SdpParameterTest : public ::testing::Test
{
protected:
    inline void SetUp() override { SetUpSdpLines(); }

    inline void TearDown() override { m_objSdpLines.RemoveAllElements(); }

    inline void SetUpSdpLines()
    {
        m_objSdpLines.AddElement("i=An IMS session");
        m_objSdpLines.AddElement("k=prompt");
        m_objSdpLines.AddElement("b=AS:38");
        m_objSdpLines.AddElement("b=RS:600");
        m_objSdpLines.AddElement("b=RR:2000");
        m_objSdpLines.AddElement("a=ptime:20");
        m_objSdpLines.AddElement("a=maxptime:40");
    }

    inline void VerifySdpLines(IN const SdpParameter& objParameter)
    {
        AString strSdp = objParameter.ToSdp();

        for (IMS_UINT32 i = 0; i < m_objSdpLines.GetCount(); ++i)
        {
            // SdpParameter only encodes the attribute lines.
            const AString& strLine = m_objSdpLines.GetElementAt(i);

            if (strLine.StartsWith("a="))
            {
                EXPECT_TRUE(strSdp.Contains(strLine));
            }
        }
    }

protected:
    AStringArray m_objSdpLines;
};

TEST_F(SdpParameterTest, Create)
{
    SdpDescription objDesc;
    ASSERT_TRUE(objDesc.Decode(m_objSdpLines));

    SdpParameter objParameter;
    EXPECT_TRUE(objParameter.Create(objDesc));
    VerifySdpLines(objParameter);

    EXPECT_NE(objParameter.GetAttribute(SdpAttribute::PTIME), IMS_NULL);
    EXPECT_NE(objParameter.GetAttribute(SdpAttribute::MAXPTIME), IMS_NULL);
    EXPECT_NE(objParameter.GetBandwidth(SdpBandwidth::TYPE_AS), IMS_NULL);
    EXPECT_NE(objParameter.GetBandwidth(SdpBandwidth::TYPE_RR), IMS_NULL);
    EXPECT_NE(objParameter.GetBandwidth(SdpBandwidth::TYPE_RS), IMS_NULL);
    EXPECT_NE(objParameter.GetEncryptionKey(), IMS_NULL);
    EXPECT_NE(objParameter.GetInformation(), IMS_NULL);
}

TEST_F(SdpParameterTest, CreateWithSetupAndConnection)
{
    m_objSdpLines.AddElement("a=setup:active");
    m_objSdpLines.AddElement("a=connection:new");

    SdpDescription objDesc;
    ASSERT_TRUE(objDesc.Decode(m_objSdpLines));

    SdpParameter objParameter;
    EXPECT_TRUE(objParameter.Create(objDesc));
    VerifySdpLines(objParameter);

    EXPECT_EQ(Sdp::SETUP_ACTIVE, objParameter.GetAttributeSetup());
    EXPECT_EQ(Sdp::CONNECTION_NEW, objParameter.GetAttributeConnection());
}

TEST_F(SdpParameterTest, CreateWithInvalidSetupAndConnection)
{
    m_objSdpLines.AddElement("a=setup:invalid");
    m_objSdpLines.AddElement("a=connection:invalid");

    SdpDescription objDesc;
    ASSERT_TRUE(objDesc.Decode(m_objSdpLines));

    SdpParameter objParameter;
    EXPECT_TRUE(objParameter.Create(objDesc));
    EXPECT_EQ(Sdp::SETUP_NONE, objParameter.GetAttributeSetup());
    EXPECT_EQ(Sdp::CONNECTION_NONE, objParameter.GetAttributeConnection());

    AString strSdp = objParameter.ToSdp();
    EXPECT_TRUE(strSdp.GetLength() > 0);
}

}  // namespace android
