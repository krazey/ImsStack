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
#include "AString.h"
#include "call/IMtcCall.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/DialogInfoProxy.h"

const AString strNotificationBodySample1(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"
        "<dialog id=\"123456\">"
        "<state>confirmed</state>"
        "<duration>274</duration>"
        "<exclusive>false</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"true\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Bob\">sip:bob@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog>"
        "<dialog id=\"7890\">"
        "<state>early</state>"
        "<duration>274</duration>"
        "<exclusive>false</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"false\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Mark\">sip:Mark@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog></dialog-info>");

const AString strNotificationBodySample2(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"
        "<dialog id=\"123456\">"
        "<state>terminated</state>"
        "<duration>274</duration>"
        "<exclusive>true</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"false\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes>"
        "<mediaAttributes>"
        "<mediaType>video</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Bob\">sip:bob@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog>"
        "<dialog id=\"7890\">"
        "<state>confirmed</state>"
        "<duration>274</duration>"
        "<exclusive>false</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"true\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes>"
        "<mediaAttributes>"
        "<mediaType>video</mediaType>"
        "<mediaDirection>inactive</mediaDirection><port0/></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Mark\">sip:Mark@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog></dialog-info>");

namespace android
{

class DialogInfoTest : public ::testing::Test
{
public:
    DialogInfoTest() :
            objDialogInfoProxy(DialogInfoProxy())
    {
    }
    void ClearJniExternalCalls(ImsList<JniExternalCall*>& objJniExternalCalls);

public:
    DialogInfoProxy objDialogInfoProxy;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

void DialogInfoTest::ClearJniExternalCalls(ImsList<JniExternalCall*>& objJniExternalCalls)
{
    IMS_UINT32 nSize = objJniExternalCalls.GetSize();

    for (IMS_UINT32 index = 0; index < nSize; index++)
    {
        delete objJniExternalCalls.GetAt(index);
    }

    objJniExternalCalls.Clear();
}

TEST_F(DialogInfoTest, UpdateDialogInfo)
{
    AString strEntity("sip:alice@example.com");
    const AString& strDialogInfoEntity = objDialogInfoProxy.Update(strNotificationBodySample1);

    EXPECT_TRUE(strEntity.Equals(strDialogInfoEntity));
}

TEST_F(DialogInfoTest, GetJniExternalCalls1)
{
    objDialogInfoProxy.Update(strNotificationBodySample1);
    ImsList<JniExternalCall*> objJniExternalCalls = objDialogInfoProxy.GetJniExternalCalls();

    EXPECT_TRUE(objJniExternalCalls.GetSize() == 2);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_strCallId.EqualsIgnoreCase("123456"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_strAddress.EqualsIgnoreCase(
            "sip:bob@example.org"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_strLocalAddress.EqualsIgnoreCase(
            "sip:alice@example.com"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_bIsPullable);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_nCallState == 4);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_nCallType ==
            static_cast<IMS_SINT32>(CallType::VOIP));
    EXPECT_FALSE(objJniExternalCalls.GetValueAt(0)->m_bIsHeld);

    ClearJniExternalCalls(objJniExternalCalls);
}

TEST_F(DialogInfoTest, GetJniExternalCalls2)
{
    objDialogInfoProxy.Update(strNotificationBodySample2);
    ImsList<JniExternalCall*> objJniExternalCalls = objDialogInfoProxy.GetJniExternalCalls();

    EXPECT_TRUE(objJniExternalCalls.GetSize() == 2);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_strCallId.EqualsIgnoreCase("123456"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_strAddress.EqualsIgnoreCase(
            "sip:bob@example.org"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_strLocalAddress.EqualsIgnoreCase(
            "sip:alice@example.com"));
    EXPECT_FALSE(objJniExternalCalls.GetValueAt(0)->m_bIsPullable);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_nCallState == 5);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_nCallType ==
            static_cast<IMS_SINT32>(CallType::VT));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(0)->m_bIsHeld);

    EXPECT_TRUE(objJniExternalCalls.GetValueAt(1)->m_strCallId.EqualsIgnoreCase("7890"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(1)->m_strAddress.EqualsIgnoreCase(
            "sip:Mark@example.org"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(1)->m_strLocalAddress.EqualsIgnoreCase(
            "sip:alice@example.com"));
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(1)->m_bIsPullable);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(1)->m_nCallState == 4);
    EXPECT_TRUE(objJniExternalCalls.GetValueAt(1)->m_nCallType ==
            static_cast<IMS_SINT32>(CallType::VOIP));
    EXPECT_FALSE(objJniExternalCalls.GetValueAt(1)->m_bIsHeld);

    ClearJniExternalCalls(objJniExternalCalls);
}

}  // namespace android
