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

package com.android.imsstack.enabler.mtc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ConferenceInfoHelperTest {
    private String mCcid1 = "mCcid1";
    private String mCcid2 = "mCcid2";
    private String mPeerConfUserId = "mPeerConfUserId";
    private String mPeerCallConnectionIdStr = "100";
    private int mPeerCallConnectionIdInt = 100;

    @Test
    public void testConferenceInfoHelper() {
        // getAnonymousId, setAnonymousId
        ConferenceInfoHelper.setAnonymousId(0);

        assertEquals(0, ConferenceInfoHelper.getAnonymousId());

        ConferenceInfoHelper.setAnonymousId(Integer.MAX_VALUE);

        assertEquals(1, ConferenceInfoHelper.getAnonymousId());

        // createConferenceInfo
        assertEquals(ConferenceInfoHelper.createConferenceInfo(null),
                ConferenceInfoHelper.createConferenceInfo(ConferenceInfo.IMMATURE_CCID));

        // destroyConferenceInfo
        ConferenceInfoHelper.destroyConferenceInfo(null);

        ConferenceInfoManager cim = ConferenceInfoManager.getInstance();
        assertFalse(cim.hasConferenceInfo());

        // destroyAllConferenceInfos
        ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.createConferenceInfo(mCcid2);

        ConferenceInfoHelper.destroyAllConferenceInfos();

        assertFalse(cim.hasConferenceInfo());

        // addConferenceUser
        assertFalse(ConferenceInfoHelper.addConferenceUser(
                null, mPeerConfUserId, mPeerCallConnectionIdInt));

        ConferenceInfoHelper.createConferenceInfo(mCcid1);

        assertTrue(ConferenceInfoHelper.addConferenceUser(
                mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt));

        // getConferenceInfo
        ConferenceInfoHelper.destroyAllConferenceInfos();
        assertEquals(ConferenceInfoHelper.createConferenceInfo(mCcid1),
                ConferenceInfoHelper.getConferenceInfo(mCcid1));

        // isConferenceUserRemovable
        ConferenceInfoHelper.destroyAllConferenceInfos();
        ConferenceInfo conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);

        assertFalse(ConferenceInfoHelper.isConferenceUserRemovable(mPeerConfUserId));

        conferenceInfo.updateUserForInterimStatus(mPeerCallConnectionIdStr, mPeerConfUserId,
                ConferenceInfo.User.STATUS_DISCONNECTED, 0, 0);

        assertTrue(ConferenceInfoHelper.isConferenceUserRemovable(mPeerConfUserId));

        // removeConferenceUser
        ConferenceInfoHelper.destroyAllConferenceInfos();
        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);

        assertEquals(1, conferenceInfo.getUserCount());

        ConferenceInfoHelper.removeConferenceUser(mPeerCallConnectionIdStr, mPeerConfUserId);

        assertEquals(0, conferenceInfo.getUserCount());

        // removeConferenceUsersByStatus
        assertEquals(0, ConferenceInfoHelper.removeConferenceUsersByStatus(mCcid1, null));
        assertEquals(0, ConferenceInfoHelper.removeConferenceUsersByStatus(
                null, ConferenceInfo.User.STATUS_CONNECTED));

        ConferenceInfoHelper.destroyAllConferenceInfos();
        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);

        assertEquals(0, ConferenceInfoHelper.removeConferenceUsersByStatus(
                mCcid1, ConferenceInfo.User.STATUS_CONNECTED));

        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);

        conferenceInfo.updateUserForInterimStatus(mPeerCallConnectionIdStr, mPeerConfUserId,
                ConferenceInfo.User.STATUS_CONNECTED, 0, 0);

        assertEquals(1, ConferenceInfoHelper.removeConferenceUsersByStatus(
                mCcid1, ConferenceInfo.User.STATUS_CONNECTED));

        // removeConferenceUsersOnInvitationFailed
        ConferenceInfoHelper.destroyAllConferenceInfos();
        ConferenceInfoHelper.removeConferenceUsersOnInvitationFailed("invalid");
        assertNull(ConferenceInfoHelper.getConferenceInfo("invalid"));

        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        assertEquals(0, conferenceInfo.getUserCount());
        ConferenceInfoHelper.removeConferenceUsersOnInvitationFailed(mCcid1);

        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);
        conferenceInfo.updateUserForInterimStatus(mPeerCallConnectionIdStr, mPeerConfUserId,
                ConferenceInfo.User.STATUS_PENDING, 0, 0);
        assertEquals(1, conferenceInfo.getUserCount());

        ConferenceInfoHelper.removeConferenceUsersOnInvitationFailed(mCcid1);

        assertEquals(0, conferenceInfo.getUserCount());

        // TODO : check that replaceImmatureCcid has been used and remove it if needed.

        // setListenerForConferenceUser
        ConferenceInfoHelper.destroyAllConferenceInfos();
        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);
        conferenceInfo.updateUserForInterimStatus(mPeerCallConnectionIdStr, mPeerConfUserId,
                ConferenceInfo.User.STATUS_PENDING, 0, 0);

        ConferenceInfo.User.Listener confUserListener =
                Mockito.mock(ConferenceInfo.User.Listener.class);
        ConferenceInfoHelper.setListenerForConferenceUser(
                mPeerCallConnectionIdStr, mPeerConfUserId, confUserListener);

        conferenceInfo.notifyUserStatusIfChanged();

        verify(confUserListener, times(1)).onConferenceUserStatusUpdated(any());

        // updateAndNotifyDisconnectedForAllConferenceUsers
        ConferenceInfoHelper.destroyAllConferenceInfos();
        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);

        reset(confUserListener);
        ConferenceInfoHelper.setListenerForConferenceUser(
                mPeerCallConnectionIdStr, mPeerConfUserId, confUserListener);

        ConferenceInfoHelper.updateAndNotifyDisconnectedForAllConferenceUsers(null);
        assertNull(ConferenceInfoHelper.getConferenceInfo(null));

        verify(confUserListener, never()).onConferenceUserStatusUpdated(any());

        ConferenceInfoHelper.updateAndNotifyDisconnectedForAllConferenceUsers(mCcid1);

        verify(confUserListener, times(1)).onConferenceUserStatusUpdated(any());

        // updateConferenceUser
        ConferenceInfoHelper.destroyAllConferenceInfos();

        assertFalse(ConferenceInfoHelper.updateConferenceUser(
                null, "", "", "", "", "", "", 0, 0));

        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);

        assertFalse(ConferenceInfoHelper.updateConferenceUser(
                conferenceInfo, mPeerCallConnectionIdStr, "", "", "", "",
                ConferenceInfo.User.STATUS_DISCONNECTED, 0, 0));
        assertTrue(ConferenceInfoHelper.updateConferenceUser(
                conferenceInfo, mPeerCallConnectionIdStr, "", "", "", "",
                ConferenceInfo.User.STATUS_CONNECTED, 0, 0));
        assertEquals(1, conferenceInfo.getUserCount());

        assertTrue(ConferenceInfoHelper.updateConferenceUser(
                conferenceInfo, ConferenceInfo.User.DEFAULT_CALL_ID, "", "", "", "",
                ConferenceInfo.User.STATUS_ON_HOLD, 0, 0));
        assertEquals(ConferenceInfo.User.STATUS_ON_HOLD,
                conferenceInfo.getUser(ConferenceInfo.User.DEFAULT_CALL_ID, "").getStatus());

        assertFalse(ConferenceInfoHelper.updateConferenceUser(
                conferenceInfo, mPeerCallConnectionIdStr, mPeerConfUserId, "", "", "",
                ConferenceInfo.User.STATUS_DISCONNECTING, 0, 0));
        assertTrue(ConferenceInfoHelper.updateConferenceUser(
                conferenceInfo, mPeerCallConnectionIdStr, mPeerConfUserId, "", "", "",
                ConferenceInfo.User.STATUS_CONNECTED, 0, 0));
        assertEquals(2, conferenceInfo.getUserCount());

        assertTrue(ConferenceInfoHelper.updateConferenceUser(conferenceInfo,
                mPeerCallConnectionIdStr, mPeerConfUserId, "", "", "",
                ConferenceInfo.User.STATUS_ALERTING, 0, 0));
        assertEquals(ConferenceInfo.User.STATUS_ALERTING,
                conferenceInfo.getUser(mPeerCallConnectionIdStr, mPeerConfUserId).getStatus());

        // updateConferenceUsersOnInvitationFailed
        ConferenceInfoHelper.destroyAllConferenceInfos();
        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);
        conferenceInfo.updateUserForInterimStatus(mPeerCallConnectionIdStr, mPeerConfUserId,
                ConferenceInfo.User.STATUS_PENDING, 0, 0);

        ConferenceInfoHelper.updateConferenceUsersOnInvitationFailed(mCcid1 + "0",
                ConferenceInfo.User.STATUS_CONNECT_FAIL, 0);
        assertEquals(ConferenceInfo.User.STATUS_PENDING,
                conferenceInfo.getUser(mPeerCallConnectionIdStr, mPeerConfUserId).getStatus());

        ConferenceInfoHelper.updateConferenceUsersOnInvitationFailed(mCcid1,
                ConferenceInfo.User.STATUS_CONNECT_FAIL, 0);
        assertEquals(ConferenceInfo.User.STATUS_CONNECT_FAIL,
                conferenceInfo.getUser(mPeerCallConnectionIdStr, mPeerConfUserId).getStatus());

        // updateConferenceUsersOnRemoval
        ConferenceInfoHelper.destroyAllConferenceInfos();
        conferenceInfo = ConferenceInfoHelper.createConferenceInfo(mCcid1);
        ConferenceInfoHelper.addConferenceUser(mCcid1, mPeerConfUserId, mPeerCallConnectionIdInt);
        conferenceInfo.updateUserForInterimStatus(mPeerCallConnectionIdStr, mPeerConfUserId,
                ConferenceInfo.User.STATUS_DISCONNECTING, 0, 0);

        ConferenceInfoHelper.updateConferenceUsersOnRemoval(mCcid1 + "0");
        assertEquals(ConferenceInfo.User.STATUS_DISCONNECTING,
                conferenceInfo.getUser(mPeerCallConnectionIdStr, mPeerConfUserId).getStatus());

        ConferenceInfoHelper.updateConferenceUsersOnRemoval(mCcid1);
        assertEquals(ConferenceInfo.User.STATUS_DISCONNECTED,
                conferenceInfo.getUser(mPeerCallConnectionIdStr, mPeerConfUserId).getStatus());

        ConferenceInfoHelper.destroyAllConferenceInfos();
    }
}
