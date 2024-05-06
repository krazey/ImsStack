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

#include "AString.h"
#include "ImsTypeDef.h"
#include "dialogevent/DialogInfoManager.h"
#include <gtest/gtest.h>

class DialogInfoManagerTest : public ::testing::Test
{
public:
    DialogInfoManager objManager;
};

TEST_F(DialogInfoManagerTest, GetDialogsReturnsEmptyListInitially)
{
    EXPECT_EQ(objManager.GetDialogs().GetSize(), 0);
}

TEST_F(DialogInfoManagerTest, UpdateFailsIfXmlParsingFails)
{
    EXPECT_EQ(objManager.Update("arbitrary string"), IMS_FAILURE);

    EXPECT_EQ(objManager.GetState(), DialogInfo::STATE_INVALID);
    EXPECT_EQ(objManager.GetVersion(), 0);
    EXPECT_EQ(objManager.GetEntity(), AString::ConstNull());
}

TEST_F(DialogInfoManagerTest, UpdateFailsIfXmlNotHaveDialogInfoRootElement)
{
    const AString strNoDialogInfo("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                  "<not-dialog-info></not-dialog-info>");

    EXPECT_EQ(objManager.Update(strNoDialogInfo), IMS_FAILURE);

    EXPECT_EQ(objManager.GetState(), DialogInfo::STATE_INVALID);
    EXPECT_EQ(objManager.GetVersion(), 0);
    EXPECT_EQ(objManager.GetEntity(), AString::ConstNull());
}

TEST_F(DialogInfoManagerTest, UpdateFailsIfXmlNotHaveMandatoryInfo)
{
    const AString strIncompleteDialogInfo(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\">"
            "</dialog-info>");

    EXPECT_EQ(objManager.Update(strIncompleteDialogInfo), IMS_FAILURE);

    EXPECT_EQ(objManager.GetState(), DialogInfo::STATE_INVALID);
    EXPECT_EQ(objManager.GetVersion(), 0);
    EXPECT_EQ(objManager.GetEntity(), AString::ConstNull());
}

TEST_F(DialogInfoManagerTest, UpdateKeepsPreviousInfoIfFails)
{
    const AString strDialogInfo("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
                                "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                                "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
                                "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"

                                "<dialog id=\"123456\">"
                                "<state>confirmed</state>"
                                "</dialog>"

                                "</dialog-info>");
    const AString strIncompleteDialogInfo(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
            "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
            "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\">"
            "</dialog-info>");

    objManager.Update(strDialogInfo);
    objManager.Update(strIncompleteDialogInfo);

    EXPECT_EQ(objManager.GetState(), DialogInfo::STATE_FULL);
    EXPECT_EQ(objManager.GetVersion(), 1);
    EXPECT_EQ(objManager.GetEntity(), "sip:alice@example.com");
    EXPECT_EQ(objManager.GetDialogs().GetSize(), 1);
}

TEST_F(DialogInfoManagerTest, UpdateSuccess)
{
    const AString strDialogInfo("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" "
                                "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                                "xsi:schemaLocation=\"urn:ietf:params:xml:ns:dialog-info\" "
                                "version=\"1\" state=\"full\" entity=\"sip:alice@example.com\">"

                                "<dialog id=\"123456\">"
                                "<state>confirmed</state>"
                                "</dialog>"

                                "</dialog-info>");

    EXPECT_EQ(objManager.Update(strDialogInfo), IMS_SUCCESS);

    EXPECT_EQ(objManager.GetState(), DialogInfo::STATE_FULL);
    EXPECT_EQ(objManager.GetVersion(), 1);
    EXPECT_EQ(objManager.GetEntity(), "sip:alice@example.com");
    EXPECT_EQ(objManager.GetDialogs().GetSize(), 1);
}
