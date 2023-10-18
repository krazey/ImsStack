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
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import com.android.imsstack.util.ImsLog;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;

@RunWith(JUnit4.class)
public class ConferenceInfoTest {
    private String mCcid = "mCcid";

    private String mCallId0 = "1";
    private String mId0 = "ui0";
    private String mUserEntity0 = "ue0";
    private String mEndpointEntity0 = "ee0";
    private String mDisplayText0 = "dt0";
    private String mStatus0 = "s0";
    private int mSIPStatusCode0 = 0;
    private int mDisconnectedCause0 = 0;

    private String mCallId1 = "2";
    private String mId1 = "ui1";
    private String mUserEntity1 = "ue1";
    private String mEndpointEntity1 = "ee1";
    private String mDisplayText1 = "dt1";
    private String mStatus1 = "s1";
    private int mSIPStatusCode1 = 0;
    private int mDisconnectedCause1 = 0;

    @Mock ConferenceInfo.User.Listener mMockuserListener;

    private ConferenceInfo mTestConferenceInfo;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
        mTestConferenceInfo = new ConferenceInfo(mCcid);

        ImsLog.setDebugOn(true);
    }

    @After
    public void tearDown() throws Exception {
        mTestConferenceInfo = null;
    }

    @Test
    public void testAddUser() {
        assertEquals(0, mTestConferenceInfo.getUserCount());

        mTestConferenceInfo.addUser(null, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertEquals(1, mTestConferenceInfo.getUserCount());

        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertEquals(2, mTestConferenceInfo.getUserCount());
    }

    @Test
    public void testAddUserForInterimStatus() {
        assertEquals(0, mTestConferenceInfo.getUserCount());

        mTestConferenceInfo.addUserForInterimStatus(Integer.parseInt(mCallId0), mId0, mStatus0,
                mSIPStatusCode0, mDisconnectedCause0);

        assertEquals(1, mTestConferenceInfo.getUserCount());
    }

    @Test
    public void testGetCcid() {
        assertEquals(mCcid, mTestConferenceInfo.getCcid());
    }

    @Test
    public void testGetUsers() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);
        mTestConferenceInfo.addUser(mCallId1, mId1, mUserEntity1, mEndpointEntity1,
                mDisplayText1, mStatus1, mSIPStatusCode1, mDisconnectedCause1);
        List<ConferenceInfo.User> users = mTestConferenceInfo.getUsers();

        assertFalse(users.isEmpty());
        assertEquals(mCallId0, users.get(0).getCallId());
        assertEquals(mCallId1, users.get(1).getCallId());
    }

    @Test
    public void testGetUser() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertNull(mTestConferenceInfo.getUser("", ""));
        assertNull(mTestConferenceInfo.getUser("0", ""));
        assertNotNull(mTestConferenceInfo.getUser("", mId0));
        assertNotNull(mTestConferenceInfo.getUser("0", mId0));
        assertNotNull(mTestConferenceInfo.getUser(mCallId0, ""));
    }

    @Test
    public void testGetUserForCallConnectionId() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertNull(mTestConferenceInfo.getUserForCallConnectionId(10));
        assertEquals(mCallId0, mTestConferenceInfo.getUserForCallConnectionId(
                Integer.parseInt(mCallId0)).getCallId());
    }

    @Test
    public void testGetUserForEntity() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertNotNull(mTestConferenceInfo.getUserForEntity(mEndpointEntity0, mUserEntity0));
        assertNotNull(mTestConferenceInfo.getUserForEntity("", mUserEntity0));
        assertNull(mTestConferenceInfo.getUserForEntity("", ""));
    }

    @Test
    public void testHasActiveUser() {
        assertFalse(mTestConferenceInfo.hasActiveUser());

        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertTrue(mTestConferenceInfo.hasActiveUser());
    }

    @Test
    public void testNotifyUserStatusIfChanged() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);
        mTestConferenceInfo.updateUser(mCallId0, mId0, ConferenceInfo.User.STATUS_CONNECTED);
        mTestConferenceInfo.getUser(mCallId0, mId0).setListener(mMockuserListener);

        mTestConferenceInfo.notifyUserStatusIfChanged();

        verify(mMockuserListener, times(1)).onConferenceUserStatusUpdated(any());

        mTestConferenceInfo.notifyUserStatusIfChanged();

        verify(mMockuserListener, times(1)).onConferenceUserStatusUpdated(any());
    }

    @Test
    public void testRemoveUser() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertNotNull(mTestConferenceInfo.getUser(mCallId0, mId0));

        mTestConferenceInfo.removeUser(mCallId0, mId0);

        assertNull(mTestConferenceInfo.getUser(mCallId0, mId0));
    }

    @Test
    public void testReplaceImmatureCcid() {
        assertEquals(mCcid, mTestConferenceInfo.getCcid());

        mTestConferenceInfo.replaceImmatureCcid(mCcid + "0");

        assertEquals(mCcid + "0", mTestConferenceInfo.getCcid());
    }

    @Test
    public void testUpdateUser() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertEquals(mStatus0, mTestConferenceInfo.getUser(mCallId0, mId0).getStatus());

        mTestConferenceInfo.updateUser(mCallId0, mId0, ConferenceInfo.User.STATUS_CONNECTED);

        assertEquals(ConferenceInfo.User.STATUS_CONNECTED,
                mTestConferenceInfo.getUser(mCallId0, mId0).getStatus());

        mTestConferenceInfo.addUserForInterimStatus(
                Integer.parseInt(mCallId1), mId1, mStatus1, mSIPStatusCode1, mDisconnectedCause1);
        mTestConferenceInfo.updateUser(mCallId1, mId1, mUserEntity1, mEndpointEntity1,
                mDisplayText1, mStatus1, mSIPStatusCode1, mDisconnectedCause1);

        assertEquals(mUserEntity1, mTestConferenceInfo.getUser(mCallId1, mId1).getUser());
        assertEquals(mEndpointEntity1, mTestConferenceInfo.getUser(mCallId1, mId1).getEndpoint());
        assertEquals(mDisplayText1, mTestConferenceInfo.getUser(mCallId1, mId1).getDisplayText());
        assertEquals(mStatus1, mTestConferenceInfo.getUser(mCallId1, mId1).getStatus());
        assertEquals(
                mSIPStatusCode1, mTestConferenceInfo.getUser(mCallId1, mId1).getSIPStatusCode());
        assertEquals(mDisconnectedCause1,
                mTestConferenceInfo.getUser(mCallId1, mId1).getDisconnectedCause());
    }

    @Test
    public void testUpdateUserForEntity() {
        mTestConferenceInfo.addUser(mCallId0, mId0, mUserEntity0, mEndpointEntity0,
                mDisplayText0, mStatus0, mSIPStatusCode0, mDisconnectedCause0);

        assertEquals(mStatus0, mTestConferenceInfo.getUser(mCallId0, mId0).getStatus());

        mTestConferenceInfo.updateUserForEntity(
                mEndpointEntity0, mUserEntity0, ConferenceInfo.User.STATUS_CONNECTED);

        assertEquals(ConferenceInfo.User.STATUS_CONNECTED,
                mTestConferenceInfo.getUser(mCallId0, mId0).getStatus());
    }

    @Test
    public void testUpdateUserForInterimStatus() {
        mTestConferenceInfo.addUserForInterimStatus(
                Integer.parseInt(mCallId1), mId1, mStatus1, mSIPStatusCode1, mDisconnectedCause1);

        mTestConferenceInfo.updateUserForInterimStatus(mCallId1, mId1,
                ConferenceInfo.User.STATUS_CONNECTED, mSIPStatusCode1 + 1,
                mDisconnectedCause1 + 1);

        assertEquals(ConferenceInfo.User.STATUS_CONNECTED,
                mTestConferenceInfo.getUser(mCallId1, mId1).getStatus());
    }
}
