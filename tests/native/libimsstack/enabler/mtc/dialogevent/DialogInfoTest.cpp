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
#include "ImsList.h"
#include "call/IMtcCall.h"
#include "dialogevent/DialogInfo.h"
#include "dialogevent/DialogInfoManager.h"
#include <gtest/gtest.h>

const AString strNotificationBodySample1(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"
        "<dialog id=\"123456\" call-id=\"callid1\" local-tag=\"localtag1\" "
        "remote-tag=\"remotetag1\">"
        "<state>confirmed</state>"
        "<duration>274</duration>"
        "<exclusive>false</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"yes\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Bob\">sip:bob@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog>"
        "<dialog id=\"7890\" call-id=\"callid2\" local-tag=\"localtag2\" remote-tag=\"remotetag2\">"
        "<state>early</state>"
        "<duration>274</duration>"
        "<exclusive>false</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"no\"/></target>"
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
        "<dialog id=\"123456\" call-id=\"callid1\" local-tag=\"localtag1\" "
        "remote-tag=\"remotetag1\">"
        "<state>early</state>"
        "<duration>222</duration>"
        "<exclusive>true</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"yes\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes>"
        "<mediaAttributes>"
        "<mediaType>video</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Bob\">sip:bob@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog>"
        "<dialog id=\"7890\" call-id=\"callid2\" local-tag=\"localtag2\" remote-tag=\"remotetag2\">"
        "<state event=\"rejected\" code=\"486\">confirmed</state>"
        "<duration>333</duration>"
        "<exclusive>false</exclusive>"
        "<replaces call-id=\"prevcallid\" local-tag=\"prevlocaltag\" remote-tag=\"prevremotetag\"/>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\">"
        "<param pname=\"isfocus\" pval=\"false\"/>"
        "<param pname=\"class\" pval=\"personal\"/>"
        "<param pname=\"+sip.rendering\" pval=\"no\"/></target>"
        "<mediaAttributes>"
        "<mediaType>audio</mediaType>"
        "<mediaDirection>sendrecv</mediaDirection></mediaAttributes>"
        "<mediaAttributes>"
        "<mediaType>video</mediaType>"
        "<mediaDirection>inactive</mediaDirection><port0/></mediaAttributes></local>"
        "<remote>"
        "<identity display=\"Mark\">sip:Mark@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog></dialog-info>");

const AString strNotificationBodySample3(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"
        "<dialog id=\"11111\" call-id=\"callid1\" local-tag=\"localtag1\" "
        "remote-tag=\"remotetag1\">"
        "<state>terminated</state>"
        "<duration>111</duration>"
        "<exclusive>true</exclusive>"
        "<local>"
        "<identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\"/>"
        "</local>"
        "<remote>"
        "<identity display=\"Bob\">sip:bob@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog>"
        "<dialog id=\"22222\" call-id=\"callid2\" local-tag=\"localtag2\" "
        "remote-tag=\"remotetag2\">"
        "<state>confirmed</state>"
        "<duration>222</duration>"
        "<exclusive>false</exclusive>"
        "<local>"
        "<target uri=\"sip:alice@pc33.example.com\"/>"
        "</local>"
        "<remote>"
        "<identity display=\"Mark\">sip:Mark@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog></dialog-info>");

const AString strNotificationBodySampleFullEmptyDialogs(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"
        "</dialog-info>");

const AString strNotificationBodySamplePartialEmptyDialogs(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"partial\" entity=\"sip:alice@example.com\">"
        "</dialog-info>");

namespace android
{

class DialogInfoTest : public ::testing::Test
{
public:
    DialogInfoTest() :
            objDialogInfoManager(DialogInfoManager())
    {
    }

public:
    DialogInfoManager objDialogInfoManager;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(DialogInfoTest, UpdateDialogInfo)
{
    AString strEntity("sip:alice@example.com");
    objDialogInfoManager.Update(strNotificationBodySample1);

    EXPECT_TRUE(strEntity.Equals(objDialogInfoManager.GetEntity()));
}

TEST_F(DialogInfoTest, UpdateUpdatesDialogListUsingSample1)
{
    objDialogInfoManager.Update(strNotificationBodySample1);

    const ImsList<Dialog*>& objDialogs = objDialogInfoManager.GetDialogs();
    EXPECT_TRUE(objDialogs.GetSize() == 2);

    Dialog& objDialog1 = *objDialogs.GetAt(0);
    EXPECT_TRUE(objDialog1.GetState().GetCode() == 0);
    EXPECT_TRUE(objDialog1.GetState().GetEvent() == Dialog::State::EVENT_IDLE);
    EXPECT_TRUE(objDialog1.GetState().GetState() == Dialog::State::STATE_CONFIRMED);

    EXPECT_TRUE(objDialog1.GetDuration() == 274);

    EXPECT_TRUE(objDialog1.GetReplaces().GetCallId().GetLength() == 0);
    EXPECT_TRUE(objDialog1.GetReplaces().GetLoalTag().GetLength() == 0);
    EXPECT_TRUE(objDialog1.GetReplaces().GetRemoteTag().GetLength() == 0);

    EXPECT_TRUE(objDialog1.GetReferredBy().GetDiaplay().GetLength() == 0);
    EXPECT_TRUE(objDialog1.GetReferredBy().GetUri().GetLength() == 0);

    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetIdentity().GetUri().Equals(
            "sip:alice@example.com"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetIdentity().GetDiaplay().Equals("Alice"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetTarget().GetUri().Equals(
            "sip:alice@pc33.example.com"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetTarget().GetParams().GetValue("isfocus").Equals(
            "false"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetTarget().GetParams().GetValue("class").Equals(
            "personal"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant()
                        .GetTarget()
                        .GetParams()
                        .GetValue("+sip.rendering")
                        .Equals("yes"));

    EXPECT_TRUE(
            objDialog1.GetRemoteParticipant().GetIdentity().GetUri().Equals("sip:bob@example.org"));
    EXPECT_TRUE(objDialog1.GetRemoteParticipant().GetIdentity().GetDiaplay().Equals("Bob"));
    EXPECT_TRUE(objDialog1.GetRemoteParticipant().GetTarget().GetUri().Equals(
            "sip:bobster@phone21.example.org"));

    EXPECT_TRUE(objDialog1.GetExtraInfo().GetExclusive().Equals("false"));
    EXPECT_TRUE(objDialog1.GetExtraInfo().GetMediaInfo().eAudioDirection == DIRECTION_SEND_RECEIVE);

    EXPECT_TRUE(objDialog1.GetId().Equals("123456"));
    EXPECT_TRUE(objDialog1.GetCallId().Equals("callid1"));
    EXPECT_TRUE(objDialog1.GetLocalTag().Equals("localtag1"));
    EXPECT_TRUE(objDialog1.GetRemoteTag().Equals("remotetag1"));
}

TEST_F(DialogInfoTest, UpdateUpdatesDialogListUsingSample2)
{
    objDialogInfoManager.Update(strNotificationBodySample2);

    const ImsList<Dialog*>& objDialogs = objDialogInfoManager.GetDialogs();
    EXPECT_TRUE(objDialogs.GetSize() == 2);

    // dialog1
    Dialog& objDialog1 = *objDialogs.GetAt(0);
    EXPECT_TRUE(objDialog1.GetState().GetCode() == 0);
    EXPECT_TRUE(objDialog1.GetState().GetEvent() == Dialog::State::EVENT_IDLE);
    EXPECT_TRUE(objDialog1.GetState().GetState() == Dialog::State::STATE_EARLY);

    EXPECT_TRUE(objDialog1.GetDuration() == 222);

    EXPECT_TRUE(objDialog1.GetReplaces().GetCallId().GetLength() == 0);
    EXPECT_TRUE(objDialog1.GetReplaces().GetLoalTag().GetLength() == 0);
    EXPECT_TRUE(objDialog1.GetReplaces().GetRemoteTag().GetLength() == 0);

    EXPECT_TRUE(objDialog1.GetReferredBy().GetDiaplay().GetLength() == 0);
    EXPECT_TRUE(objDialog1.GetReferredBy().GetUri().GetLength() == 0);

    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetIdentity().GetUri().Equals(
            "sip:alice@example.com"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetIdentity().GetDiaplay().Equals("Alice"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetTarget().GetUri().Equals(
            "sip:alice@pc33.example.com"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetTarget().GetParams().GetValue("isfocus").Equals(
            "false"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant().GetTarget().GetParams().GetValue("class").Equals(
            "personal"));
    EXPECT_TRUE(objDialog1.GetLocalParticipant()
                        .GetTarget()
                        .GetParams()
                        .GetValue("+sip.rendering")
                        .Equals("yes"));

    EXPECT_TRUE(
            objDialog1.GetRemoteParticipant().GetIdentity().GetUri().Equals("sip:bob@example.org"));
    EXPECT_TRUE(objDialog1.GetRemoteParticipant().GetIdentity().GetDiaplay().Equals("Bob"));
    EXPECT_TRUE(objDialog1.GetRemoteParticipant().GetTarget().GetUri().Equals(
            "sip:bobster@phone21.example.org"));

    EXPECT_TRUE(objDialog1.GetExtraInfo().GetExclusive().Equals("true"));
    EXPECT_TRUE(objDialog1.GetExtraInfo().GetMediaInfo().eAudioDirection == DIRECTION_SEND_RECEIVE);
    EXPECT_TRUE(objDialog1.GetExtraInfo().GetMediaInfo().eVideoDirection == DIRECTION_SEND_RECEIVE);

    EXPECT_TRUE(objDialog1.GetId().Equals("123456"));
    EXPECT_TRUE(objDialog1.GetCallId().Equals("callid1"));
    EXPECT_TRUE(objDialog1.GetLocalTag().Equals("localtag1"));
    EXPECT_TRUE(objDialog1.GetRemoteTag().Equals("remotetag1"));

    // dialog2
    Dialog& objDialog2 = *objDialogs.GetAt(1);
    EXPECT_TRUE(objDialog2.GetState().GetCode() == 486);
    EXPECT_TRUE(objDialog2.GetState().GetEvent() == Dialog::State::EVENT_REJECTED);
    EXPECT_TRUE(objDialog2.GetState().GetState() == Dialog::State::STATE_CONFIRMED);

    EXPECT_TRUE(objDialog2.GetDuration() == 333);

    EXPECT_TRUE(objDialog2.GetReplaces().GetCallId().Equals("prevcallid"));
    EXPECT_TRUE(objDialog2.GetReplaces().GetLoalTag().Equals("prevlocaltag"));
    EXPECT_TRUE(objDialog2.GetReplaces().GetRemoteTag().Equals("prevremotetag"));

    EXPECT_TRUE(objDialog2.GetReferredBy().GetDiaplay().GetLength() == 0);
    EXPECT_TRUE(objDialog2.GetReferredBy().GetUri().GetLength() == 0);

    EXPECT_TRUE(objDialog2.GetLocalParticipant().GetIdentity().GetUri().Equals(
            "sip:alice@example.com"));
    EXPECT_TRUE(objDialog2.GetLocalParticipant().GetIdentity().GetDiaplay().Equals("Alice"));
    EXPECT_TRUE(objDialog2.GetLocalParticipant().GetTarget().GetUri().Equals(
            "sip:alice@pc33.example.com"));
    EXPECT_TRUE(objDialog2.GetLocalParticipant().GetTarget().GetParams().GetValue("isfocus").Equals(
            "false"));
    EXPECT_TRUE(objDialog2.GetLocalParticipant().GetTarget().GetParams().GetValue("class").Equals(
            "personal"));
    EXPECT_TRUE(objDialog2.GetLocalParticipant()
                        .GetTarget()
                        .GetParams()
                        .GetValue("+sip.rendering")
                        .Equals("no"));

    EXPECT_TRUE(objDialog2.GetRemoteParticipant().GetIdentity().GetUri().Equals(
            "sip:Mark@example.org"));
    EXPECT_TRUE(objDialog2.GetRemoteParticipant().GetIdentity().GetDiaplay().Equals("Mark"));
    EXPECT_TRUE(objDialog2.GetRemoteParticipant().GetTarget().GetUri().Equals(
            "sip:bobster@phone21.example.org"));

    EXPECT_TRUE(objDialog2.GetExtraInfo().GetExclusive().Equals("false"));
    EXPECT_TRUE(objDialog2.GetExtraInfo().GetMediaInfo().eAudioDirection == DIRECTION_SEND_RECEIVE);
    EXPECT_TRUE(objDialog2.GetExtraInfo().GetMediaInfo().eVideoQuality == VIDEO_QUALITY_NOTUSED);
    EXPECT_TRUE(objDialog2.GetExtraInfo().GetMediaInfo().eVideoDirection == DIRECTION_INACTIVE);

    EXPECT_TRUE(objDialog2.GetId().Equals("7890"));
    EXPECT_TRUE(objDialog2.GetCallId().Equals("callid2"));
    EXPECT_TRUE(objDialog2.GetLocalTag().Equals("localtag2"));
    EXPECT_TRUE(objDialog2.GetRemoteTag().Equals("remotetag2"));
}

TEST_F(DialogInfoTest, TerminatedDialogIsDeletedFromTheList)
{
    objDialogInfoManager.Update(strNotificationBodySample3);

    const ImsList<Dialog*>& objDialogs = objDialogInfoManager.GetDialogs();
    EXPECT_TRUE(objDialogs.GetSize() == 1);
}

TEST_F(DialogInfoTest, EmptyDialogInfoReturnsSuccessIfFullState)
{
    EXPECT_EQ(IMS_SUCCESS, objDialogInfoManager.Update(strNotificationBodySampleFullEmptyDialogs));

    const ImsList<Dialog*>& objDialogs = objDialogInfoManager.GetDialogs();
    EXPECT_TRUE(objDialogs.GetSize() == 0);
}

TEST_F(DialogInfoTest, EmptyDialogInfoReturnsFailureIfPartialState)
{
    EXPECT_EQ(
            IMS_FAILURE, objDialogInfoManager.Update(strNotificationBodySamplePartialEmptyDialogs));

    const ImsList<Dialog*>& objDialogs = objDialogInfoManager.GetDialogs();
    EXPECT_TRUE(objDialogs.GetSize() == 0);
}

}  // namespace android
