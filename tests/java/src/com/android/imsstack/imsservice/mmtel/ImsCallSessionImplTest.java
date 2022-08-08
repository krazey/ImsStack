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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Matchers.eq;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyObject;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.os.Bundle;
import android.os.Handler;
import android.telecom.Connection.RttModifyStatus;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.util.Log;

import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

import java.util.HashMap;
import java.util.Map;

@RunWith(JUnit4.class)
public class ImsCallSessionImplTest {
    private static final String LOG_TAG = "ImsCallSessionImplTest";
    private ImsCallContext mMockCallContext;
    private CallTracker mMockCallTracker;
    private MtcCall mMockMtcCall;
    private ImsCallProfile mImsCallProfile;
    private ImsCallSessionImpl mImsCallSession;
    private ImsCallSessionListener mMockImsCallSessionListener;
    private ImsCallSessionCallback mMockImsCallSessionCallback;
    private Handler mHandler;
    private String mCallId;
    private MediaInfo mMockMediaInfo;
    private ImsCallApp mMockImsCallApp;
    private ImsCallManager mMockImsCallManager;
    private CallInfo mMockCallInfo;
    private Map<Integer, Boolean> mCallFeaturemap = new HashMap<Integer, Boolean>();

    @Before
    public void setUp() throws Exception {
        mMockCallContext = Mockito.mock(ImsCallContext.class);
        mMockCallTracker = Mockito.mock(CallTracker.class);
        mMockCallInfo = Mockito.mock(CallInfo.class);
        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        mMockImsCallManager = Mockito.mock(ImsCallManager.class);
        mMockImsCallSessionCallback = Mockito.mock(ImsCallSessionCallback.class);
        mMockImsCallSessionListener = Mockito.mock(ImsCallSessionListener.class);
        mMockMediaInfo = Mockito.mock(MediaInfo.class);
        mMockMtcCall = Mockito.mock(MtcCall.class);
        mCallId = "1";
        mImsCallProfile = new ImsCallProfile();
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker,
                mMockMtcCall, mCallId, mImsCallProfile, true, mMockImsCallSessionCallback);

        MessageExecutor mExecutor = new MessageExecutor(ImsCallUtils.class.getSimpleName());
        when(mMockCallContext.getExecutor()).thenReturn(mExecutor);
        Handler handler = new MessageExecutor(ImsCallUtils.class.getSimpleName());
        when(mMockCallContext.getCallHandler()).thenReturn(handler);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallSession = null;
        mImsCallProfile = null;
    }

    @Test
    public void testGetCallId() {
        assertEquals("1", mImsCallSession.getCallId());
        mImsCallSession = createImsCallSession("");
        assertNotNull(mImsCallSession.getCallId());
    }

    @Test
    public void testGetCallProfile() {
        assertNotNull(mImsCallSession.getCallProfile());
    }

    @Test
    public void testGetLocalCallProfile() {
        ImsCallProfile callProfile = mImsCallSession.getLocalCallProfile();
        assertEquals(mImsCallProfile.getServiceType(), callProfile.getServiceType());
        assertEquals(mImsCallProfile.getCallType(), callProfile.getCallType());
    }

    @Test
    public void testGetRemoteCallProfile() {
        ImsCallProfile callProfile = mImsCallSession.getRemoteCallProfile();
        assertEquals(mImsCallProfile.getServiceType(), callProfile.getServiceType());
        assertEquals(mImsCallProfile.getCallType(), callProfile.getCallType());
    }

    @Test
    public void testGetProperty() {
        assertEquals(mImsCallSession.getProperty(null), null);
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker, null,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback);
        assertEquals(mImsCallSession.getProperty(""), null);

        when(mMockMtcCall.getCallExtraBoolean(Call.EXTRA_CONFERENCE, false))
                .thenReturn(true);
        when(mMockMtcCall.getCallExtraInt(Call.EXTRA_OIR, -1))
                .thenReturn(ImsCallProfile.OIR_PRESENTATION_RESTRICTED);
        when(mMockMtcCall.getCallExtra(Call.EXTRA_USSD, null)).thenReturn("123");

        mImsCallSession = createImsCallSession("2");
        assertEquals(mImsCallSession.getProperty(Call.EXTRA_CONFERENCE), "true");
        assertEquals(mImsCallSession.getProperty(Call.EXTRA_OIR), "1");
        assertEquals(mImsCallSession.getProperty(Call.EXTRA_USSD), "123");
    }

    @Test
    public void testGetState() {
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.TERMINATED);
    }

    @Test
    public void testIsInCall() {
        when(mMockMtcCall.isInCall()).thenReturn(true);
        assertTrue(mImsCallSession.isInCall());
    }

    @Test
    public void testSetListener() {
        mImsCallSession.setListener(mMockImsCallSessionListener);
        verify(mMockImsCallSessionCallback).setListener(any(ImsCallSessionListener.class));
    }

    @Test
    public void testStart() {
        //verify start failed.
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker, null,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback);
        mImsCallSession.start(null, mImsCallProfile);
        //CallDetails.CALL_END_FINISHED = 0x80000000
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));

        //verify setRttGttInfo
        mImsCallProfile = new ImsCallProfile();
        mImsCallProfile.getMediaProfile().setRttMode(1);
        mImsCallSession = createImsCallSession("2");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mCallFeaturemap.put(ImsCallSessionImpl.CF_TTY, false);
        mImsCallSession.start(null, mImsCallProfile);
        verify(mMockMtcCall, times(1)).start(anyInt(), anyObject(), anyObject(),
                any(MediaInfo.class), any(SuppInfo.class));

        //verify setGttInfo
        mImsCallProfile.getMediaProfile().setRttMode(0);
        mImsCallSession = createImsCallSession("3");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, false);
        mCallFeaturemap.put(ImsCallSessionImpl.CF_TTY, true);
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);
        mImsCallSession.start(null, mImsCallProfile);
        verify(mMockMtcCall, times(2)).start(anyInt(), anyObject(), anyObject(),
                any(MediaInfo.class), any(SuppInfo.class));
    }

    @Test
    public void testStartConference() {
        String[] participants = {"1234", "5678"};
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker, null,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback);
        mImsCallSession.startConference(participants, mImsCallProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));

        mImsCallSession = createImsCallSession("2");
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mImsCallSession.startConference(participants, mImsCallProfile);
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.TERMINATED);

        mImsCallSession = createImsCallSession("3");
        mImsCallSession.startConference(participants, mImsCallProfile);
        verify(mMockMtcCall).startConference(anyInt(), any(UsersInfo.class), any(MediaInfo.class),
                any(SuppInfo.class));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.NEGOTIATING);
    }

    @Test
    public void testAccept() {
        //verify Illegal state
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID, ImsStreamMediaProfile.RTT_MODE_DISABLED);
        mImsCallSession.accept(ImsCallProfile.CALL_TYPE_VOICE, imsStreamMediaProfile);
        verify(mMockMtcCall, never()).accept(anyInt(), any(MediaInfo.class));

        //verify  No default media
        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, new Bundle(), imsStreamMediaProfile);

        mImsCallSession = createImsCallSession("2");
        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        mImsCallSession.accept(ImsCallProfile.CALL_TYPE_VOICE, imsStreamMediaProfile);
        verify(mMockMtcCall, times(1)).accept(anyInt(), any(MediaInfo.class));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        //verify default media
        mImsCallSession = createImsCallSession("3");
        imsStreamMediaProfile = null;
        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        mImsCallSession.accept(ImsCallProfile.CALL_TYPE_VOICE, imsStreamMediaProfile);
        verify(mMockMtcCall, times(2)).accept(anyInt(), any(MediaInfo.class));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testReject() {
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.reject(ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        verify(mMockMtcCall).terminate(anyInt());
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.TERMINATING);

        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mImsCallSession.reject(ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));
        verify(mMockImsCallSessionCallback).invokeTerminated(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));

        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mImsCallSession.reject(ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        verify(mMockMtcCall, times(1)).reject(CallReasonInfo.CODE_USER_DECLINE);
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.ESTABLISHED);

        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        mImsCallSession.reject(ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED);
        verify(mMockMtcCall, times(2)).reject(anyInt());
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.TERMINATED);

        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        mImsCallSession.reject(ImsReasonInfo.CODE_USER_IGNORE);
        verify(mMockMtcCall, times(3)).reject(anyInt());
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.TERMINATED);
    }

    @Test
    public void testBlindTransfer() {
        String transferTarget = "123";
        boolean isConfirmationRequired = false;
        //When Call on hold
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        mImsCallSession.transfer(transferTarget, isConfirmationRequired);
        verify(mMockMtcCall).transfer(transferTarget);

        //When call not on hold
        mImsCallSession = createImsCallSession("2");
        when(mMockMtcCall.isOnHold()).thenReturn(false);
        when(mMockMtcCall.getCallInfo()).thenReturn(mMockCallInfo);
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);
        mImsCallSession.transfer(transferTarget, isConfirmationRequired);
        verify(mMockMtcCall).hold(any(MediaInfo.class));
        verify(mMockMtcCall).transfer(transferTarget);
    }

    @Test
    public void testConsultativeTransfer() {
        //When Call on hold
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getActiveSession()).thenReturn(mImsCallSession);
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        mImsCallSession.transfer(new ImsCallSessionImplBase());
        verify(mMockMtcCall).transfer(null);

        //When ECT on background call
        mImsCallSession = createImsCallSession("2");
        ImsCallSessionImpl imsCallSession = createImsCallSession("3");
        when(mMockImsCallManager.getActiveSession()).thenReturn(imsCallSession);
        when(mMockMtcCall.isOnHold()).thenReturn(false);
        when(mMockMtcCall.getCallInfo()).thenReturn(mMockCallInfo);
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);
        mImsCallSession.transfer(new ImsCallSessionImplBase());
        verify(mMockMtcCall).hold(any(MediaInfo.class));
        verify(mMockMtcCall).transfer(null);
    }

    @Test
    public void testTerminate() {
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mImsCallSession.terminate(501);
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));

        //Verify Ringing state terminate
        mImsCallSession = createImsCallSession("2");
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mImsCallSession.getCallDetails().set(0x00000040);
        mImsCallSession.terminate(501);
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));
        verify(mMockImsCallSessionCallback).invokeTerminated(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));

        mImsCallSession = createImsCallSession("3");
        mImsCallSession.terminate(501);
        verify(mMockMtcCall).terminate(anyInt());
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.TERMINATING);
    }

    @Test
    public void testHold() {
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID, ImsStreamMediaProfile.RTT_MODE_DISABLED);
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mImsCallSession.hold(imsStreamMediaProfile);
        verify(mMockImsCallSessionCallback, times(1)).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        mImsCallSession.hold(imsStreamMediaProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));
        verify(mMockImsCallSessionCallback, times(2)).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));
        verify(mMockImsCallSessionCallback).invokeTerminated(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));

        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mImsCallSession.hold(imsStreamMediaProfile);
        verify(mMockImsCallSessionCallback, times(3)).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        mImsCallSession = createImsCallSession("2");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_AUDIO_HOLD_WITH_INACTIVE, true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.hold(imsStreamMediaProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x01000000));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).hold(any(MediaInfo.class));

        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), imsStreamMediaProfile);
        mImsCallSession = createImsCallSession("3");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_VIDEO_HOLD_WITH_INACTIVE, true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.hold(imsStreamMediaProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x01000000));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(2)).hold(any(MediaInfo.class));
    }

    @Test
    public void testResume() {
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID, ImsStreamMediaProfile.RTT_MODE_DISABLED);
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mImsCallSession.resume(imsStreamMediaProfile);
        verify(mMockImsCallSessionCallback, times(1)).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));
        verify(mMockImsCallSessionCallback, times(1)).invokeTerminated(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x80000000));
        verify(mMockImsCallSessionCallback, times(2)).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        mImsCallSession = createImsCallSession("2");
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x02000000));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).resume(any(MediaInfo.class));

        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), imsStreamMediaProfile);
        mImsCallSession = createImsCallSession("3");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_VIDEO_HOLD_WITH_INACTIVE, true);
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x02000000));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(2)).resume(any(MediaInfo.class));
    }

    @Test
    public void testMerge() {
        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        when(mMockImsCallSessionCallback.hasListener()).thenReturn(true);
        mImsCallSession.merge();
        sleep(50);
        verify(mMockImsCallSessionCallback).invokeMergeFailed(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));

        mImsCallSession = createImsCallSession("2");
        mImsCallSession.merge();
        sleep(50);
        verify(mMockImsCallSessionCallback).invokeMergeFailed(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));
    }

    @Test
    public void testUpdate() {
        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID, ImsStreamMediaProfile.RTT_MODE_DISABLED);

        mImsCallSession.update(ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO, imsStreamMediaProfile);
        verify(mMockMtcCall, never()).update(anyInt(), any(MediaInfo.class));

        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.update(ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO, imsStreamMediaProfile);
        sleep(50);
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).update(anyInt(), any(MediaInfo.class));
    }

    @Test
    public void testInviteParticipants() {
        String[] participant = null;
        mImsCallSession.inviteParticipants(participant);
        verify(mMockImsCallSessionCallback, times(1)).invokeInviteParticipantsRequestFailed(
                any(ImsCallSessionImplBase.class),  any(ImsReasonInfo.class));
    }

    @Test
    public void testRemoveParticipants() {
        String[] participant = {};
        mImsCallSession.removeParticipants(participant);
        verify(mMockImsCallSessionCallback, times(1)).invokeRemoveParticipantsRequestFailed(
                any(ImsCallSessionImplBase.class),  any(ImsReasonInfo.class));

        String[] participants = {"callId: 2"};
        when(mMockMtcCall.getCallExtraBoolean(Call.EXTRA_CONFERENCE_EVENT, false))
                .thenReturn(true);
    }

    @Test
    public void testSendDtmf() {
        mImsCallSession.sendDtmf('1', null);
        verify(mMockMtcCall, times(1)).sendDtmf(any(Character.class));
        mImsCallSession.sendDtmf('#', null);
        verify(mMockMtcCall, times(2)).sendDtmf(any(Character.class));
    }

    @Test
    public void testStartDtmf() {
        mImsCallSession.startDtmf('A');
        verify(mMockMtcCall, times(1)).sendDtmf(any(Character.class));
        mImsCallSession.startDtmf('#');
        verify(mMockMtcCall, times(2)).sendDtmf(any(Character.class));
    }

    @Test
    public void testSendUssd() {
        String ussdMessage = "*#1234#*";
        mImsCallSession.sendUssd(ussdMessage);
        verify(mMockMtcCall).sendUssd(any(String.class));
    }

    @Test
    public void testGetImsVideoCallProvider() {
        assertNotNull(mImsCallSession.getImsVideoCallProvider());
    }

    @Test
    public void testIsMultiparty() {
        assertFalse(mImsCallSession.isMultiparty());
        mImsCallSession.getCallProfile().setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE, true);
        assertTrue(mImsCallSession.isMultiparty());
    }

    @Test
    public void testSendRttModifyRequest() {
        ImsCallProfile toProfile = null;
        mImsCallSession.sendRttModifyRequest(toProfile);
        verify(mMockImsCallSessionCallback, times(1)).invokeRttModifyResponseReceived(any(
                ImsCallSessionImplBase.class), eq(RttModifyStatus.SESSION_MODIFY_REQUEST_INVALID));

        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mImsCallSession.sendRttModifyRequest(toProfile);
        verify(mMockImsCallSessionCallback, times(2)).invokeRttModifyResponseReceived(any(
                ImsCallSessionImplBase.class), eq(RttModifyStatus.SESSION_MODIFY_REQUEST_INVALID));

        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID, ImsStreamMediaProfile.RTT_MODE_FULL);

        toProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, new Bundle(), imsStreamMediaProfile);

        mImsCallSession.setState(ImsCallSessionImplBase.State.IDLE);
        mImsCallSession.sendRttModifyRequest(toProfile);
        verify(mMockImsCallSessionCallback, times(1)).invokeRttModifyResponseReceived(any(
                ImsCallSessionImplBase.class), eq(RttModifyStatus.SESSION_MODIFY_REQUEST_FAIL));
        mImsCallSession.close();
        verify(mMockImsCallSessionCallback, times(1)).setListener(eq(null));

        mImsCallProfile.updateMediaProfile(toProfile);
        mImsCallSession = createImsCallSession("2");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.sendRttModifyRequest(toProfile);
        verify(mMockImsCallSessionCallback, times(1)).invokeRttModifyResponseReceived(any(
                ImsCallSessionImplBase.class), eq(RttModifyStatus.SESSION_MODIFY_REQUEST_SUCCESS));

        mImsCallProfile = new ImsCallProfile();
        mImsCallSession = createImsCallSession("3");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.sendRttModifyRequest(toProfile);
        assertTrue(mImsCallSession.getCallDetails().is(0x10000000));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).update(anyInt(), any(MediaInfo.class));
    }

    @Test
    public void testSendRttModifyResponse() {
        mImsCallSession.sendRttModifyResponse(true);
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mImsCallSession.sendRttModifyResponse(true);
        verify(mMockMtcCall, never()).accept(anyInt(), any(MediaInfo.class));

        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mImsCallSession.sendRttModifyResponse(true);
        verify(mMockMtcCall).accept(anyInt(), any(MediaInfo.class));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
    }

    @Test
    public void testSendRttMessage() {
        mImsCallSession.sendRttMessage("Hello");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mImsCallSession.sendRttMessage("Hello");
        verify(mMockMtcCall, never()).sendRttMessage(eq("Hello"));

        ImsStreamMediaProfile imsStreamMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.AUDIO_QUALITY_AMR,
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                ImsStreamMediaProfile.DIRECTION_INVALID, ImsStreamMediaProfile.RTT_MODE_FULL);

        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, new Bundle(), imsStreamMediaProfile);
        mImsCallSession = createImsCallSession("2");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_RTT, true);
        mImsCallSession.sendRttMessage("Hello");
        verify(mMockMtcCall).sendRttMessage(eq("Hello"));
    }

    private ImsCallSessionImpl createImsCallSession(String callId) {
        ImsCallSessionImpl callSession =  new TestImsCallSessionImpl(mMockCallContext,
                mMockCallTracker, mMockMtcCall, callId, mImsCallProfile, true,
                mMockImsCallSessionCallback);
        mCallFeaturemap = new HashMap<Integer, Boolean>();
        return callSession;
    }

    private class TestImsCallSessionImpl extends ImsCallSessionImpl {
        TestImsCallSessionImpl(ICallContext callContext,
                CallTracker ct, MtcCall call, String callId, ImsCallProfile profile, boolean isMO,
                ImsCallSessionCallback callBack) {
            super(callContext, ct, call, callId, profile, isMO, callBack);
        }

        @Override
        protected boolean isCallFeatureSupported(int feature) {
            if (mCallFeaturemap.containsKey(feature)) {
                return mCallFeaturemap.get(feature);
            } else {
                return false;
            }
        }
    }

    private void sleep(long ms) {
        try {
            Thread.sleep(ms);
        } catch (Exception e) {
            Log.d(LOG_TAG, "InterruptedException");
        }
    }
}
