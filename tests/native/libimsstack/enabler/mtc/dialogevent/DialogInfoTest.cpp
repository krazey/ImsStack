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
#include "dialogevent/DialogInfo.h"
#include "dialogevent/DialogInfoUpdater.h"

const AString strSampleNotificationBody(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
        "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"
        "<dialog id=\"123456\"><state>confirmed</state><duration>274</duration>"
        "<local><identity display=\"Alice\">sip:alice@example.com</identity>"
        "<target uri=\"sip:alice@pc33.example.com\"><param pname=\"isfocus\" pval=\"true\"/>"
        "<param pname=\"class\" pval=\"personal\"/></target></local>"
        "<remote><identity display=\"Bob\">sip:bob@example.org</identity>"
        "<target uri=\"sip:bobster@phone21.example.org\"/></remote></dialog></dialog-info>");

namespace android
{

class DialogInfoTest : public ::testing::Test
{
public:
    DialogInfoTest() :
            objDialogInfoUpdater(DialogInfoUpdater()),
            strDialogInfoEntity(objDialogInfoUpdater.Update(strSampleNotificationBody))
    {
    }

public:
    DialogInfoUpdater objDialogInfoUpdater;
    const AString& strDialogInfoEntity;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(DialogInfoTest, UpdateDialogInfo)
{
    AString strEntity("sip:alice@example.com");

    EXPECT_TRUE(strEntity.Equals(strDialogInfoEntity));
}

}  // namespace android
