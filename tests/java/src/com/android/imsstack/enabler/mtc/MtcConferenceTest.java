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
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Looper;
import android.os.Parcel;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.mtc.conf.IUConf;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcConferenceTest extends ImsStackTest {
    private final int mInvalidCommand = -1;
    @Mock private MtcConference.Listener mMockConferenceListener;
    @Mock private MtcCall mMockMtcCall;
    @Mock private ConferenceTracker mMockConferenceTracker;
    private int mCommand;

    private TestMtcJniProxy mTestMtcJniProxy;
    private MtcConference mTestConference;

    private class TestMtcJniProxy extends MtcJniProxy {
        @Override
        public void sendDataToNative(long nativeObj, Parcel parcel) {
            if (parcel == null) {
                return;
            }

            parcel.setDataPosition(0);
            mCommand = parcel.readInt();

            parcel.recycle();
            parcel = null;
        }
    }

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        mCommand = mInvalidCommand;

        mTestConference = new MtcConference(
                Looper.myLooper(), mMockMtcCall, mMockConferenceTracker);
        mTestConference.setListener(mMockConferenceListener);
        mTestMtcJniProxy = new TestMtcJniProxy();
        mTestConference.setMtcJniProxy(mTestMtcJniProxy);
    }

    @After
    public void tearDown() throws Exception {
        mTestMtcJniProxy = null;
        mTestConference = null;
        super.tearDown();
    }

    public void sendMessage(int command) {
        Parcel parcel = Parcel.obtain();
        parcel.setDataPosition(0);
        mTestConference.handleMessage(command, parcel);
        parcel.recycle();
    }

    public void sendMessage(int command, int send) {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(send);
        parcel.setDataPosition(0);
        mTestConference.handleMessage(command, parcel);
        parcel.recycle();
    }

    @Test
    public void testDispose() {
        mTestConference.dispose();

        mTestConference.extendToConference(new UsersInfo());
        processAllMessages();

        assertEquals(mInvalidCommand, mCommand);
        verifyNoMoreInteractions(mMockMtcCall);

        sendMessage(IUConf.EXPANDED);

        verifyNoMoreInteractions(mMockMtcCall);
        verifyNoMoreInteractions(mMockConferenceListener);
    }

    @Test
    public void testGetParent() {
        assertEquals(mMockMtcCall, mTestConference.getParent());
    }

    @Test
    public void testIsSameCall() {
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        MtcConference MockMtcConference = Mockito.mock(MtcConference.class);
        doReturn(mockMtcCall).when(MockMtcConference).getParent();

        assertTrue(mTestConference.isSameCall(mTestConference));
        assertFalse(mTestConference.isSameCall(MockMtcConference));
    }

    @Test
    public void testExtendToConference() {
        mTestConference.extendToConference(new UsersInfo());
        processAllMessages();

        assertEquals(IUConf.EXPAND, mCommand);
        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_EXTEND_TO_CONFERENCE), any());

        mTestConference.dispose();
        mTestConference.setListener(mMockConferenceListener);

        mTestConference.extendToConference(new UsersInfo());
        processAllMessages();

        verify(mMockConferenceListener).onCallConferenceExtendFailed(
                eq(mTestConference), any());
    }

    @Test
    public void testMerge() {
        mTestConference.merge(new UsersInfo());
        processAllMessages();

        assertEquals(IUConf.MERGE, mCommand);
        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_MERGE), any());
    }

    @Test
    public void testInviteParticipants() {
        mTestConference.inviteParticipants(new UsersInfo());
        processAllMessages();

        assertEquals(IUConf.JOIN, mCommand);
        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_INVITE_PARTICIPANTS), any());
    }

    @Test
    public void testRemoveParticipants() {
        mTestConference.removeParticipants(new UsersInfo());
        processAllMessages();

        assertEquals(IUConf.DROP, mCommand);
        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_REMOVE_PARTICIPANTS), any());
    }

    @Test
    public void testDeleteParticipants() {
        mTestConference.deleteParticipants(new UsersInfo());
        processAllMessages();

        assertEquals(IUConf.DELETE, mCommand);
        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_DELETE_PARTICIPANTS), any());
    }

    @Test
    public void testHandleMessageExpanded() {
        sendMessage(IUConf.EXPANDED);

        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_EXTENDED), any());
        verify(mMockConferenceListener).onCallConferenceExtended(
                eq(mTestConference), anyLong(), any(), any(), any());
    }

    @Test
    public void testHandleMessageExpandfailed() {
        sendMessage(IUConf.EXPANDFAILED);

        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_EXTEND_FAILED), any());
        verify(mMockConferenceListener).onCallConferenceExtendFailed(eq(mTestConference), any());
    }

    @Test
    public void testHandleMessageExpandedBy() {
        sendMessage(IUConf.EXPANDED_BY);

        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_EXTEND_RECEIVED), any());
        verify(mMockConferenceListener).onCallConferenceExtendReceived(
                eq(mTestConference), anyLong(), any(), any(), any());
    }

    @Test
    public void testHandleMessageMerged() {
        sendMessage(IUConf.MERGED);

        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_MERGED), any());
        verify(mMockConferenceListener).onCallMerged(
                eq(mTestConference), any(), any(), any(), any());
    }

    @Test
    public void testHandleMessageMergefailed() {
        sendMessage(IUConf.MERGEFAILED);

        verify(mMockConferenceTracker).updateConferenceState(
                eq(mTestConference), eq(ConferenceTracker.EVENT_MERGE_FAILED), any());
        verify(mMockConferenceListener).onCallMergeFailed(eq(mTestConference), any());
    }

    @Test
    public void testHandleMessageJoined() {
        sendMessage(IUConf.JOINED, 0);

        verify(mMockConferenceTracker).updateConferenceState(eq(mTestConference),
                eq(ConferenceTracker.EVENT_INVITE_PARTICIPANTS_REQUEST_FAILED), any());
        verify(mMockConferenceListener).onCallInviteParticipantsRequestFailed(
                eq(mTestConference), any());

        sendMessage(IUConf.JOINED, 1);

        verify(mMockConferenceTracker).updateConferenceState(eq(mTestConference),
                eq(ConferenceTracker.EVENT_INVITE_PARTICIPANTS_REQUEST_DELIVERED), any());
        verify(mMockConferenceListener).onCallInviteParticipantsRequestDelivered(
                eq(mTestConference));
    }

    @Test
    public void testHandleMessageDropped() {
        sendMessage(IUConf.DROPPED, 0);

        verify(mMockConferenceTracker).updateConferenceState(eq(mTestConference),
                eq(ConferenceTracker.EVENT_REMOVE_PARTICIPANTS_REQUEST_FAILED), any());
        verify(mMockConferenceListener).onCallRemoveParticipantsRequestFailed(
                eq(mTestConference), any());

        sendMessage(IUConf.DROPPED, 1);

        verify(mMockConferenceTracker).updateConferenceState(eq(mTestConference),
                eq(ConferenceTracker.EVENT_REMOVE_PARTICIPANTS_REQUEST_DELIVERED), any());
        verify(mMockConferenceListener).onCallRemoveParticipantsRequestDelivered(
                eq(mTestConference));
    }

    @Test
    public void testHandleMessageDeleted() {
        sendMessage(IUConf.DELETED, 0);

        verify(mMockConferenceTracker).updateConferenceState(eq(mTestConference),
                eq(ConferenceTracker.EVENT_DELETE_PARTICIPANTS_REQUEST_COMPLETED), any());
        verify(mMockConferenceListener).onCallDeleteParticipantsRequestCompleted(
                eq(mTestConference), any());

        sendMessage(IUConf.DELETED, 1);

        verify(mMockConferenceTracker).updateConferenceState(eq(mTestConference),
                eq(ConferenceTracker.EVENT_DELETE_PARTICIPANTS_REQUEST_COMPLETED), eq(null));
        verify(mMockConferenceListener).onCallDeleteParticipantsRequestCompleted(
                eq(mTestConference), eq(null));
    }

    @Test
    public void testHandleMessageNotifyUsersInfo() {
        sendMessage(IUConf.NOTIFY_USERS_INFO);

        verify(mMockConferenceListener).onCallConferenceStateUpdated(eq(mTestConference), any());
    }
}
