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
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyString;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Bundle;
import android.telecom.Connection.RttModifyStatus;
import android.telephony.PreciseCallState;
import android.telephony.emergency.EmergencyNumber;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSessionListener;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.ImsSuppServiceNotification;
import android.telephony.ims.RtpHeaderExtension;
import android.telephony.ims.stub.ImsCallSessionImplBase;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.media.MediaTestUtils;
import com.android.imsstack.enabler.mtc.Call;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.CallReasonInfo;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.IServiceStateTracker;
import com.android.imsstack.enabler.mtc.IUMtcCall;
import com.android.imsstack.enabler.mtc.IUMtcService;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcEmergencyServiceManager;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.enabler.mtc.SuppServiceUtils.SuppService;
import com.android.imsstack.enabler.mtc.conf.UsersInfo;
import com.android.imsstack.enabler.mtc.reg.MtcServiceState;
import com.android.imsstack.imsservice.mmtel.ImsCallSessionImpl.ImsCallExtManagerProxy;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class ImsCallSessionImplTest extends ImsStackTest {
    private static final String LOG_TAG = "ImsCallSessionImplTest";
    private Context mMockContext;
    private CarrierConfig mMockCarrierConfig;
    private ConfigInterface mMockConfigInterface;
    private ImsCallContext mMockCallContext;
    private CallTracker mMockCallTracker;
    private IServiceStateTracker mMockServiceStateTracker;
    private MtcApp mMockMtcApp;
    private MtcCall mMockMtcCall;
    private MtcEmergencyServiceManager mMockMtcEmergencyServiceManager;
    private ImsCallProfile mImsCallProfile;
    private TestImsCallSessionImpl mImsCallSession;
    private ImsCallSessionListener mMockImsCallSessionListener;
    private ImsCallSessionCallback mMockImsCallSessionCallback;
    private String mCallId;
    private MediaInfo mMockMediaInfo;
    private ImsCallApp mMockImsCallApp;
    private ImsCallManager mMockImsCallManager;
    private CallInfo mMockCallInfo;
    private ArgumentCaptor<MediaInfo> mMediaInfoCaptor;
    private Map<Integer, Boolean> mCallFeaturemap = new HashMap<Integer, Boolean>();
    private ImsCallSessionImpl.CallDetails mCallDetails;
    private ImsVideoCallSession mVideoCallSession;
    public static final String SESSION_ID = "f81d4fae7dec11d0a76500a0c91e6bf6";

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        mMockContext = Mockito.mock(Context.class);
        AppContext.init(mMockContext);

        mMockCallContext = Mockito.mock(ImsCallContext.class);
        mMockCallTracker = Mockito.mock(CallTracker.class);
        mMockServiceStateTracker = Mockito.mock(IServiceStateTracker.class);
        mMockCallInfo = Mockito.mock(CallInfo.class);
        mMockImsCallApp = Mockito.mock(ImsCallApp.class);
        mMockImsCallManager = Mockito.mock(ImsCallManager.class);
        mMockImsCallSessionCallback = Mockito.mock(ImsCallSessionCallback.class);
        mMockImsCallSessionListener = Mockito.mock(ImsCallSessionListener.class);
        mMockMediaInfo = Mockito.mock(MediaInfo.class);
        mMockMtcApp = Mockito.mock(MtcApp.class);
        mMockMtcCall = Mockito.mock(MtcCall.class);
        mMockMtcEmergencyServiceManager = Mockito.mock(MtcEmergencyServiceManager.class);
        mMediaInfoCaptor = ArgumentCaptor.forClass(MediaInfo.class);
        mCallId = "1";
        mImsCallProfile = new ImsCallProfile();

        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        mVideoCallSession = Mockito.mock(ImsVideoCallSession.class);
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker,
                mMockMtcCall, mCallId, mImsCallProfile, true, mMockImsCallSessionCallback,
                mVideoCallSession);

        MessageExecutor executor = new MessageExecutor(mTestableLooper.getLooper());
        when(mMockCallContext.getExecutor()).thenReturn(executor);
        when(mMockCallContext.getCallHandler()).thenReturn(executor);
        when(mMockCallContext.getCallLooper()).thenReturn(mTestableLooper.getLooper());
        when(mMockCallContext.getServiceStateTracker()).thenReturn(mMockServiceStateTracker);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface,
                MSimUtils.DEFAULT_SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mImsCallSession = null;
        mImsCallProfile = null;
        mMediaInfoCaptor = null;
        mMockContext = null;
        mCallFeaturemap.clear();
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, MSimUtils.DEFAULT_SLOT_ID);
        AppContext.deinit();
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
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);
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
        processAllFutureMessages();

        verify(mMockImsCallSessionCallback).setListener(any(ImsCallSessionListener.class));
        verify(mMockImsCallSessionCallback, never()).invokeStartFailed(
                eq(mImsCallSession), any(ImsReasonInfo.class));

        mImsCallSession.mImmediateCallEndReason = new ImsReasonInfo(
                ImsReasonInfo.CODE_LOCAL_CALL_EXCEEDED, ImsReasonInfo.CODE_UNSPECIFIED, null);
        mImsCallSession.setListener(mMockImsCallSessionListener);
        processAllFutureMessages();

        verify(mMockImsCallSessionCallback, times(1)).invokeStartFailed(
                eq(mImsCallSession), any(ImsReasonInfo.class));
    }

    @Test
    public void testStart() {
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getMtcApp()).thenReturn(mMockMtcApp);
        when(mMockMtcApp.getMtcEmergencyServiceManager())
                .thenReturn(mMockMtcEmergencyServiceManager);

        //verify start failed.
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker, null,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);
        mImsCallSession.start(null, mImsCallProfile);
        //CallDetails.CALL_END_FINISHED = mCallDetails.CALL_END_FINISHED
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));

        //verify setRttGttInfo
        mImsCallProfile = new ImsCallProfile();
        mImsCallProfile.getMediaProfile().setRttMode(1);
        mImsCallSession = createImsCallSession("2");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_TTY, false);
        mImsCallSession.start(null, mImsCallProfile);
        verify(mMockMtcCall, times(1)).start(anyInt(), any(), any(),
                any(MediaInfo.class), any(SuppInfo.class));

        //verify setGttInfo
        mImsCallProfile.getMediaProfile().setRttMode(0);
        mImsCallSession = createImsCallSession("3");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_TTY, true);
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);
        mImsCallSession.start(null, mImsCallProfile);
        verify(mMockMtcCall, times(2)).start(anyInt(), any(), any(),
                any(MediaInfo.class), any(SuppInfo.class));
    }

    @Test
    public void testStartWithOutgoingcallBarring() {
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getMtcApp()).thenReturn(mMockMtcApp);
        when(mMockMtcApp.isOutgoingCallBarringActivated(anyInt(), anyString())).thenReturn(true);

        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);

        mImsCallSession.start("1234", mImsCallProfile);
        processAllFutureMessages();

        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        verify(mMockImsCallSessionCallback).invokeStartFailed(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));
        verify(mMockMtcCall, times(0)).isEmergencyCall();
    }

    @Test
    public void testStartEmergencyCallAndNormalServiceOpened() {
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getMtcApp()).thenReturn(mMockMtcApp);
        final ArgumentCaptor<IServiceStateTracker.Listener> listenerCaptor =
                ArgumentCaptor.forClass(IServiceStateTracker.Listener.class);
        when(mMockServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY))
                .thenReturn(false);

        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_EMERGENCY, ImsCallProfile.CALL_TYPE_VOICE);
        mImsCallProfile.setCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, true);
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);

        mImsCallSession.start(null, mImsCallProfile);

        verify(mMockServiceStateTracker).addListener(listenerCaptor.capture());
        final IServiceStateTracker.Listener listener = listenerCaptor.getValue();

        listener.onNormalServiceStateChanged(
                new MtcServiceState(IUMtcService.SERVICE_VOIP, IUMtcService.ES_OPENED, 0));
        mTestableLooper.processAllMessages();

        verify(mMockMtcCall, times(0)).start(anyInt(), any(), any(),
                any(MediaInfo.class), any(SuppInfo.class));
    }

    @Test
    public void testStartEmergencyCallAndEmergencyServiceOpened() {
        final ArgumentCaptor<IServiceStateTracker.Listener> listenerCaptor =
                ArgumentCaptor.forClass(IServiceStateTracker.Listener.class);
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getMtcApp()).thenReturn(mMockMtcApp);
        when(mMockMtcApp.getMtcEmergencyServiceManager())
                .thenReturn(mMockMtcEmergencyServiceManager);
        when(mMockServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY))
                .thenReturn(false);

        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_EMERGENCY, ImsCallProfile.CALL_TYPE_VOICE);
        mImsCallProfile.setCallExtraBoolean(ImsCallProfile.EXTRA_EMERGENCY_CALL, true);
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);

        mImsCallSession.start(null, mImsCallProfile);

        verify(mMockServiceStateTracker).addListener(listenerCaptor.capture());
        final IServiceStateTracker.Listener listener = listenerCaptor.getValue();

        listener.onEmergencyServiceStateChanged(
                new MtcServiceState(IUMtcService.SERVICE_EMERGENCY, IUMtcService.ES_OPENED, 0));
        mTestableLooper.processAllMessages();

        verify(mMockMtcCall, times(1)).start(anyInt(), any(), any(),
                any(MediaInfo.class), any(SuppInfo.class));
    }

    @Test
    public void testStartConference() {
        String[] participants = {"1234", "5678"};
        mImsCallSession = new TestImsCallSessionImpl(mMockCallContext, mMockCallTracker, null,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);
        mImsCallSession.startConference(participants, mImsCallProfile);
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));

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
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        verify(mMockImsCallSessionCallback).invokeTerminated(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));
        assertTrue(mImsCallSession.isCacheCallReasonInfoNull());

        verifyWaitOrNotifyCallTerminated();

        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mImsCallSession.reject(ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        verify(mMockMtcCall, times(1)).reject(
                CallReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION);
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
        ImsCallSessionImpl imsCallSession = createImsCallSession("2");
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getActiveSession()).thenReturn(imsCallSession);
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        mImsCallSession.transfer(new ImsCallSessionImplBase());
        verify(mMockMtcCall).transfer(null);

        //When call is not on hold
        mImsCallSession = createImsCallSession("3");
        imsCallSession = createImsCallSession("4");
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
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));

        //verify Ringing state terminate
        mImsCallSession = createImsCallSession("2");
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        mCallDetails.set(mCallDetails.MO_STARTED);
        mImsCallSession.terminate(501);
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
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
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
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
        assertTrue(mCallDetails.is(mCallDetails.ON_HOLDING));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).hold(any(MediaInfo.class));

        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), imsStreamMediaProfile);
        mImsCallSession = createImsCallSession("3");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_VIDEO_HOLD_WITH_INACTIVE, true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.hold(imsStreamMediaProfile);
        assertTrue(mCallDetails.is(mCallDetails.ON_HOLDING));
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
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        verify(mMockImsCallSessionCallback, times(2)).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                 ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, new Bundle(), imsStreamMediaProfile);
        mImsCallSession = createImsCallSession("2");
        when(mMockMtcCall.isOnHeld()).thenReturn(true);
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mCallDetails.is(mCallDetails.ON_UNHOLDING));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).resume(mMediaInfoCaptor.capture());
        assertTrue(mMediaInfoCaptor.getValue().ADir == ImsStreamMediaProfile.DIRECTION_RECEIVE);

        mCallFeaturemap.put(ImsCallSessionImpl.CF_VIDEO_HOLD_WITH_INACTIVE, true);
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mCallDetails.is(mCallDetails.ON_UNHOLDING));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(2)).resume(mMediaInfoCaptor.capture());
        assertTrue(mMediaInfoCaptor.getValue().VDir == ImsStreamMediaProfile.DIRECTION_INACTIVE);

        mImsCallSession = createImsCallSession("3");
        when(mMockMtcCall.isOnHeld()).thenReturn(false);
        when(mVideoCallSession.isCameraOn()).thenReturn(false);
        imsStreamMediaProfile.mVideoDirection = ImsStreamMediaProfile.DIRECTION_RECEIVE;
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mCallDetails.is(mCallDetails.ON_UNHOLDING));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(3)).resume(mMediaInfoCaptor.capture());
        assertTrue(mMediaInfoCaptor.getValue().VDir == ImsStreamMediaProfile.DIRECTION_RECEIVE);

        when(mMockMtcCall.isOnHeld()).thenReturn(false);
        when(mVideoCallSession.isCameraOn()).thenReturn(true);
        when(mMockMtcCall.is1WayVideo()).thenReturn(true);
        mCallFeaturemap.put(ImsCallSessionImpl.CF_ONE_WAY_VIDEO_LOCAL, true);
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mCallDetails.is(mCallDetails.ON_UNHOLDING));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(4)).resume(mMediaInfoCaptor.capture());
        assertTrue(mMediaInfoCaptor.getValue().VDir == ImsStreamMediaProfile.DIRECTION_SEND);

        when(mMockMtcCall.is1WayVideo()).thenReturn(false);
        mCallFeaturemap.put(ImsCallSessionImpl.CF_ONE_WAY_VIDEO_LOCAL, false);
        mCallFeaturemap.put(ImsCallSessionImpl.CF_ONE_WAY_VIDEO_REMOTE, true);
        mImsCallSession.resume(imsStreamMediaProfile);
        assertTrue(mCallDetails.is(mCallDetails.ON_UNHOLDING));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(5)).resume(mMediaInfoCaptor.capture());
        assertTrue(mMediaInfoCaptor.getValue().VDir == ImsStreamMediaProfile.DIRECTION_RECEIVE);
    }

    @Test
    public void testMerge() {
        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        when(mMockImsCallSessionCallback.hasListener()).thenReturn(true);
        mImsCallSession.merge();
        processAllMessages();
        verify(mMockImsCallSessionCallback).invokeMergeFailed(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));

        mImsCallSession = createImsCallSession("2");
        mImsCallSession.merge();
        processAllMessages();
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

        when(mVideoCallSession.isClearedSessionModificationInfo()).thenReturn(true);
        mImsCallSession.update(ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO, imsStreamMediaProfile);
        verify(mMockMtcCall, never()).update(anyInt(), any(MediaInfo.class));
        verify(mMockImsCallSessionCallback, times(1)).invokeUpdated(
                any(ImsCallSessionImplBase.class), any(ImsCallProfile.class));

        when(mVideoCallSession.isClearedSessionModificationInfo()).thenReturn(true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.update(ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO, imsStreamMediaProfile);
        processAllMessages();
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.ESTABLISHED);
        verify(mMockMtcCall, never()).update(anyInt(), any(MediaInfo.class));
        verify(mMockImsCallSessionCallback, times(2)).invokeUpdated(
                any(ImsCallSessionImplBase.class), any(ImsCallProfile.class));

        when(mVideoCallSession.isClearedSessionModificationInfo()).thenReturn(false);
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.update(ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO, imsStreamMediaProfile);
        processAllMessages();
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
    public void testRemoveParticipantsForError() {
        String[] participants = {"callId:2"};
        when(mMockMtcCall.getCallExtraBoolean(Call.EXTRA_CONFERENCE_EVENT, false))
            .thenReturn(true);
        mImsCallSession.removeParticipants(participants);
        verify(mMockImsCallSessionCallback, never()).invokeRemoveParticipantsRequestFailed(
                any(ImsCallSessionImplBase.class),  any(ImsReasonInfo.class));

        String[] participant = {};
        mImsCallSession.removeParticipants(participant);
        verify(mMockImsCallSessionCallback, times(1)).invokeRemoveParticipantsRequestFailed(
                any(ImsCallSessionImplBase.class),  any(ImsReasonInfo.class));
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
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.sendRttModifyRequest(toProfile);
        verify(mMockImsCallSessionCallback, times(1)).invokeRttModifyResponseReceived(any(
                ImsCallSessionImplBase.class), eq(RttModifyStatus.SESSION_MODIFY_REQUEST_SUCCESS));

        mImsCallProfile = new ImsCallProfile();
        mImsCallSession = createImsCallSession("3");
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        mImsCallSession.sendRttModifyRequest(toProfile);
        assertTrue(mCallDetails.is(mCallDetails.RTT_TURNING_ON));
        assertEquals(mImsCallSession.getState(), ImsCallSessionImplBase.State.RENEGOTIATING);
        verify(mMockMtcCall, times(1)).update(anyInt(), any(MediaInfo.class));
    }

    @Test
    public void testSendRttModifyResponse() {
        mImsCallSession.sendRttModifyResponse(true);
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
        mImsCallSession.sendRttMessage("Hello");
        verify(mMockMtcCall).sendRttMessage(eq("Hello"));
    }

    @Test
    public void testGetPreciseState() {
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_ACTIVE, mImsCallSession.getPreciseState());

        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHING);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_ALERTING,
                mImsCallSession.getPreciseState());

        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_DISCONNECTED,
                mImsCallSession.getPreciseState());

        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATING);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_DISCONNECTING,
                mImsCallSession.getPreciseState());

        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_HOLDING,
                mImsCallSession.getPreciseState());

        when(mMockMtcCall.isOnHold()).thenReturn(false);
        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mCallDetails.set(mCallDetails.ON_HOLDING);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_HOLDING,
                mImsCallSession.getPreciseState());

        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_DIALING,
                mImsCallSession.getPreciseState());

        mCallDetails.clear(mCallDetails.MO);
        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_INCOMING,
                mImsCallSession.getPreciseState());

        when(mMockCallTracker.getActiveCalls()).thenReturn(1);
        mImsCallSession.setState(ImsCallSessionImplBase.State.IDLE);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_WAITING,
                mImsCallSession.getPreciseState());

        when(mMockMtcCall.isOnPreIncoming()).thenReturn(true);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_INCOMING_SETUP,
                mImsCallSession.getPreciseState());

        when(mMockMtcCall.isOnPreIncoming()).thenReturn(false);
        mImsCallSession.setState(ImsCallSessionImplBase.State.INVALID);
        assertEquals(PreciseCallState.PRECISE_CALL_STATE_NOT_VALID,
                mImsCallSession.getPreciseState());
    }

    @Test
    public void testSendRtpHeaderExtensions() {
        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        mImsCallSession.sendRtpHeaderExtensions(extensions);
        verify(mMockMtcCall, times(1)).sendRtpHeaderExtensions(eq(extensions));
    }

    @Test
    public void testOnCallRtpHeaderExtensionsReceived() {
        Set<RtpHeaderExtension> extensions = MediaTestUtils.createRtpExtensionsSet();
        mImsCallSession.getMtcCallListenerProxy().onCallRtpHeaderExtensionsReceived(
                mMockMtcCall, extensions);
        verify(mMockImsCallSessionCallback, times(1)).invokeRtpHeaderExtensionsReceived(
                eq(extensions));
    }

    @Test
    public void testNotifyAnbr() {
        mImsCallSession.callSessionNotifyAnbr(1, 1, 244);
        verify(mMockMtcCall, times(1)).notifyAnbr(eq(1), eq(1), eq(244));
    }

    @Test
    public void testAlertUser() {
        mImsCallSession.alertUser();
        verify(mMockMtcCall, times(1)).alertUser();

        when(mMockMtcCall.isTerminatedByAutoRejectedCall()).thenReturn(true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.NEGOTIATING);
        mImsCallSession.alertUser();
        processAllFutureMessages();

        verify(mMockImsCallSessionCallback, never()).invokeStartFailed(
                eq(mImsCallSession), any(ImsReasonInfo.class));

        mImsCallSession.setListener(mMockImsCallSessionListener);
        mImsCallSession.alertUser();
        processAllFutureMessages();

        verify(mMockImsCallSessionCallback, times(1)).invokeStartFailed(
                eq(mImsCallSession), any(ImsReasonInfo.class));
    }

    private void verifyWaitOrNotifyCallTerminated() {
        mCallDetails.set(mCallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END);
        mCallDetails.clear(mCallDetails.CALL_END_FINISHED);
        mImsCallSession.reject(ImsReasonInfo.CODE_LOCAL_ILLEGAL_STATE);
        assertFalse(mImsCallSession.isCacheCallReasonInfoNull());
    }

    @Test
    public void testOnCallInitiating() {
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onCallInitiating(mockMtcCall, mMockCallInfo,
                mMockMediaInfo, 0);
        verify(mMockImsCallSessionCallback, never()).invokeInitiating(any(
                    ImsCallSessionImplBase.class), any(ImsCallProfile.class));

        int state = mImsCallSession.getState();
        mImsCallSession.getCallListenerProxy().onCallInitiating(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, 0);

        assertFalse(mImsCallSession.getState() == ImsCallSessionImplBase.State.INITIATED);
        assertEquals(state, mImsCallSession.getState());
        verify(mMockImsCallSessionCallback).invokeInitiating(any(
                ImsCallSessionImplBase.class), any(ImsCallProfile.class));
    }

    @Test
    public void testOnCallProgressing() {
        SuppInfo suppInfo = new SuppInfo();
        suppInfo.addServiceBool(SuppInfo.SUPP_TYPE_CW, false);
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onCallProgressing(mockMtcCall, mMockCallInfo,
                mMockMediaInfo, suppInfo);
        assertFalse(mImsCallSession.getState() == ImsCallSessionImplBase.State.ESTABLISHING);

        mImsCallSession = createImsCallSession("1");
        mImsCallSession.getCallListenerProxy().onCallProgressing(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, suppInfo);
        ImsCallProfile remoteProfile = mImsCallSession.getRemoteCallProfile();
        assertEquals(ImsCallSessionImplBase.State.ESTABLISHING, mImsCallSession.getState());
        assertEquals(ImsCallProfile.CALL_RESTRICT_CAUSE_HD, remoteProfile.getRestrictCause());
        verify(mMockImsCallSessionCallback).invokeProgressing(any(
                ImsCallSessionImplBase.class), any(ImsStreamMediaProfile.class));
        verify(mMockImsCallSessionCallback, never()).invokeSuppServiceReceived(any(
                ImsCallSessionImplBase.class), any(ImsSuppServiceNotification.class));

        MediaInfo mediaInfo = new MediaInfo();
        mediaInfo.AQuality = ImsStreamMediaProfile.AUDIO_QUALITY_AMR_WB;
        SuppService suppServiceCw = new SuppService();
        suppServiceCw.type = SuppInfo.SUPP_TYPE_CW;
        suppServiceCw.boolValue = true;
        suppInfo.updateService(SuppInfo.SUPP_TYPE_CW, suppServiceCw);
        SuppService suppServiceSessionId = new SuppService();
        suppServiceSessionId.type = SuppInfo.SUPP_TYPE_SESSION_ID;
        suppServiceSessionId.strValue = SESSION_ID;
        suppInfo.updateService(SuppInfo.SUPP_TYPE_SESSION_ID, suppServiceSessionId);
        mImsCallSession = createImsCallSession("2");
        ImsCallExtManagerProxy mockProxy = Mockito.mock(ImsCallExtManagerProxy.class);
        mImsCallSession.setImsCallExtManagerProxy(mockProxy);
        when(mMockMtcCall.getRemoteNumber()).thenReturn("123");
        mImsCallSession.getCallListenerProxy().onCallProgressing(mMockMtcCall, mMockCallInfo,
                mediaInfo, suppInfo);
        remoteProfile = mImsCallSession.getRemoteCallProfile();
        assertEquals(ImsCallSessionImplBase.State.ESTABLISHING, mImsCallSession.getState());
        assertEquals(ImsCallProfile.CALL_RESTRICT_CAUSE_NONE, remoteProfile.getRestrictCause());
        verify(mMockImsCallSessionCallback, times(2)).invokeProgressing(any(
                ImsCallSessionImplBase.class), any(ImsStreamMediaProfile.class));
        verify(mMockImsCallSessionCallback).invokeSuppServiceReceived(any(
                ImsCallSessionImplBase.class), any(ImsSuppServiceNotification.class));
        verify(mockProxy).reportCallInfo(anyInt(), anyString(), any(byte[].class),
                any(byte[].class), anyString());
    }

    @Test
    public void testOnCallStarted() {
        SuppInfo suppInfo = new SuppInfo();
        suppInfo.addServiceBool(SuppInfo.SUPP_TYPE_CW, false);
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onCallStarted(mockMtcCall, mMockCallInfo,
                mMockMediaInfo, suppInfo);
        assertFalse(mImsCallSession.getState() == ImsCallSessionImplBase.State.ESTABLISHED);

        Bundle bundle = new Bundle();
        bundle.putInt(Call.EXTRA_CALL_CONNECTION_ID, 1);
        mMockCallInfo.videoCapable = true;
        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, bundle, new ImsStreamMediaProfile());
        ImsCallProfile localProfile = mImsCallSession.getLocalCallProfile();
        SuppService suppService = new SuppService();
        suppService.type = SuppInfo.SUPP_TYPE_TIP;
        suppService.intValue = SuppInfo.TIP_NONE;
        suppInfo.updateService(SuppInfo.SUPP_TYPE_TIP, suppService);
        SuppService suppServiceSessionId = new SuppService();
        suppServiceSessionId.type = SuppInfo.SUPP_TYPE_SESSION_ID;
        suppServiceSessionId.strValue = SESSION_ID;
        suppInfo.updateService(SuppInfo.SUPP_TYPE_SESSION_ID, suppServiceSessionId);
        mImsCallSession = createImsCallSession("1");
        ImsCallExtManagerProxy mockProxy = Mockito.mock(ImsCallExtManagerProxy.class);
        mImsCallSession.setImsCallExtManagerProxy(mockProxy);
        when(mMockMtcCall.getRemoteNumber()).thenReturn("123");
        mImsCallSession.getCallListenerProxy().onCallStarted(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, suppInfo);
        assertEquals(ImsCallSessionImplBase.State.ESTABLISHED, mImsCallSession.getState());
        assertEquals(ImsCallProfile.CALL_RESTRICT_CAUSE_NONE, localProfile.getRestrictCause());
        //verify updateCallProfile() -> updateCallTypeChangeCapability()
        assertEquals(ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, mImsCallProfile.getCallType());
        verify(mMockMtcCall).setCallConnectionId(1);
        verify(mMockImsCallSessionCallback).invokeStarted(any(
                ImsCallSessionImplBase.class), any(ImsCallProfile.class));
        verify(mMockImsCallSessionCallback, never()).invokeTtyModeReceived(any(
                ImsCallSessionImplBase.class), any(Integer.class));
        //MO_STARTED
        assertTrue(mCallDetails.is(mCallDetails.MO_STARTED));
        verify(mockProxy).reportCallInfo(anyInt(), anyString(), any(byte[].class),
                any(byte[].class), anyString());

        //verify onTtyModeReceived() onwards
        mMockMediaInfo.GTTMode = MediaInfo.GTTMODE_FULL;
        mImsCallSession = createImsCallSession("2");
        mCallFeaturemap.put(ImsCallSessionImpl.CF_TTY, true);
        mImsCallSession.getCallListenerProxy().onCallStarted(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, suppInfo);
        verify(mMockImsCallSessionCallback).invokeTtyModeReceived(any(
                ImsCallSessionImplBase.class), any(Integer.class));
        //MO_STARTED
        assertTrue(mCallDetails.is(mCallDetails.MO_STARTED));
    }

    @Test
    public void testOnCallStartFailed() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onCallStartFailed(mockMtcCall, mockCallReasonInfo);
        assertFalse(mImsCallSession.getState() == ImsCallSessionImplBase.State.TERMINATED);

        //set CALL_END_FINISHED
        mCallDetails.set(mCallDetails.CALL_END_FINISHED);
        mImsCallSession.getCallListenerProxy().onCallStartFailed(mMockMtcCall, mockCallReasonInfo);
        assertFalse(mImsCallSession.getState() == ImsCallSessionImplBase.State.TERMINATED);

        mCallDetails.clear(mCallDetails.CALL_END_FINISHED);
        when(mMockMtcCall.isOnPreIncoming()).thenReturn(true);
        mImsCallSession.getCallListenerProxy().onCallStartFailed(mMockMtcCall, mockCallReasonInfo);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());
        processAllMessages();
        verify(mMockImsCallSessionCallback, times(1)).setListener(eq(null));

        //verify checkAndSetImmediateCallEndReason
        mockCallReasonInfo.mCode = CallReasonInfo.CODE_SIP_BAD_REQUEST;
        when(mMockImsCallSessionCallback.hasListener()).thenReturn(false);
        when(mMockMtcCall.isOnPreIncoming()).thenReturn(false);
        mImsCallSession = createImsCallSession("1");
        mImsCallSession.getCallListenerProxy().onCallStartFailed(mMockMtcCall, mockCallReasonInfo);
        ImsReasonInfo reasonInfo = mImsCallSession.mImmediateCallEndReason;
        assertEquals(ImsReasonInfo.CODE_SIP_BAD_REQUEST, reasonInfo.mCode);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());

        mockCallReasonInfo.mCode = CallReasonInfo.CODE_SIP_REDIRECTED;
        when(mMockImsCallSessionCallback.hasListener()).thenReturn(true);
        mImsCallSession.setState(ImsCallSessionImplBase.State.IDLE);
        mImsCallSession.getCallListenerProxy().onCallStartFailed(mMockMtcCall, mockCallReasonInfo);
        reasonInfo = mImsCallSession.mImmediateCallEndReason;
        assertEquals(ImsReasonInfo.CODE_SIP_REDIRECTED, reasonInfo.mCode);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());

        //verify notifyCallStartFailedWithDelay()getCallDetails
        mImsCallSession.setState(ImsCallSessionImplBase.State.ESTABLISHED);
        //for isOutgoingCallsBarred() branch
        mockCallReasonInfo.mCode = CallReasonInfo.CODE_SIP_USER_REJECTED;
        mockCallReasonInfo.mExtraCode = 53;
        mImsCallSession.getCallListenerProxy().onCallStartFailed(mMockMtcCall, mockCallReasonInfo);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        processAllMessages();
        verify(mMockImsCallSessionCallback).invokeStartFailed(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));
        processAllMessages();
        //verify isOutgoingCallsBarred() branch
        verify(mMockImsCallSessionCallback).invokeSuppServiceReceived(
                any(ImsCallSessionImplBase.class), any(ImsSuppServiceNotification.class));
    }

    @Test
    public void testOnCallTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onCallTerminated(mockMtcCall, mockCallReasonInfo);
        assertFalse(mImsCallSession.getState() == ImsCallSessionImplBase.State.TERMINATED);

        //verify "closeInternal on CLOSE_PENDING"
        mCallDetails.set(mCallDetails.CLOSE_PENDING);
        mCallDetails.set(mCallDetails.IMPLICIT_TERMINATED);
        mImsCallSession.getCallListenerProxy().onCallTerminated(mMockMtcCall, mockCallReasonInfo);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());
        processAllMessages();
        verify(mMockImsCallSessionCallback, times(1)).setListener(eq(null));

        mImsCallSession = createImsCallSession("1");
        mCallDetails.set(mCallDetails.IMPLICIT_TERMINATED);
        mockCallReasonInfo.mCode = CallReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE;
        mImsCallSession.getCallListenerProxy().onCallTerminated(mMockMtcCall, mockCallReasonInfo);
        //verify clear IMPLICIT_TERMINATED
        assertFalse(mCallDetails.is(mCallDetails.IMPLICIT_TERMINATED));
        //verify checkAndHandleConferenceOnCallTerminated
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        verify(mMockImsCallSessionCallback).invokeTerminated(any(ImsCallSessionImplBase.class),
                any(ImsReasonInfo.class));
        processAllMessages();
        verify(mMockMtcCall, times(2)).setListener(null);
        verify(mMockMtcCall, times(2)).close();

        //verify SESSION_TERMINATED_ON_CONFERENCE
        mImsCallSession = createImsCallSession("2");
        ConferenceProxy confProxy = new ConferenceProxy(mMockCallContext);
        mImsCallSession.setConferenceProxy(confProxy);
        mImsCallSession.getCallListenerProxy().onCallTerminated(mMockMtcCall, mockCallReasonInfo);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());
        assertTrue(mCallDetails.is(mCallDetails.SESSION_TERMINATED_ON_CONFERENCE));
        //verify else if of checkAndHandleConferenceOnCallTerminated and
        mockCallReasonInfo.mExtraCode = CallReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE;
        mImsCallSession.setTerminationReason(ImsReasonInfo.CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE);
        mImsCallSession.getCallListenerProxy().onCallTerminated(mMockMtcCall, mockCallReasonInfo);
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());

        //checkAndHandleTransferOnCallTerminated
        mImsCallProfile.setCallExtraBoolean(ImsCallProfile.EXTRA_CONFERENCE, true);
        mImsCallSession = createImsCallSession("3");
        mockCallReasonInfo.mCode = CallReasonInfo.CODE_USER_TERMINATED;
        mockCallReasonInfo.mExtraCode = CallReasonInfo.EXTRA_USER_TERMINATED_ECT;
        mCallDetails.set(mCallDetails.MO_STARTED);
        mImsCallSession.getCallListenerProxy().onCallTerminated(mMockMtcCall, mockCallReasonInfo);
        verify(mMockMtcCall, times(2)).setListener(null);
        verify(mMockMtcCall, times(2)).close();
        assertFalse(mCallDetails.is(mCallDetails.ON_ECT));
        assertFalse(mCallDetails.is(mCallDetails.IMPLICIT_ON_HOLD));
        assertEquals(ImsCallSessionImplBase.State.TERMINATED, mImsCallSession.getState());

        //verify notifyCallTerminated()
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        verify(mMockImsCallSessionCallback, times(2)).invokeTerminated(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        // verify not notifies the Telephony.
        mImsCallSession = createImsCallSession("4");
        mCallDetails.set(mCallDetails.MO_STARTED);
        CallReasonInfo mockSrvccReasonInfo = Mockito.mock(CallReasonInfo.class);
        mockSrvccReasonInfo.mCode = CallReasonInfo.CODE_LOCAL_CALL_VCC_ON_PROGRESSING;
        mImsCallSession.getCallListenerProxy().onCallTerminated(mMockMtcCall, mockSrvccReasonInfo);
        assertTrue(mCallDetails.is(mCallDetails.CALL_END_FINISHED));
        verify(mMockImsCallSessionCallback, times(2)).invokeTerminated(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));
    }

    @Test
    public void testOnCallHeld() {
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);
        mImsCallSession.getCallListenerProxy().onCallHeld(mockMtcCall, null, null, null);
        verify(mMockImsCallSessionCallback, never()).invokeHeld(any(ImsCallSessionImplBase.class),
                any(ImsCallProfile.class));

        when(mMockMtcCall.isOnHold()).thenReturn(true);
        mImsCallSession.transfer("1234", true);
        mImsCallSession.getCallListenerProxy().onCallHeld(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, mockSuppInfo);
        processAllMessages();
        verify(mMockMtcCall, times(2)).transfer("1234");
        //verify IMPLICIT_ON_HOLD
        assertTrue(mCallDetails.is(mCallDetails.IMPLICIT_ON_HOLD));

        mCallDetails.clear(mCallDetails.ON_ECT);
        mCallDetails.clear(mCallDetails.IMPLICIT_ON_HOLD);
        mImsCallSession.transfer(null, true);
        mImsCallSession.getCallListenerProxy().onCallHeld(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, mockSuppInfo);
        processAllMessages();
        verify(mMockMtcCall, times(2)).transfer(null);
        //verify IMPLICIT_ON_HOLD
        assertTrue(mCallDetails.is(mCallDetails.IMPLICIT_ON_HOLD));

        mImsCallSession = createImsCallSession("1");
        ConferenceProxy confProxy = new ConferenceProxy(mMockCallContext);
        mImsCallSession.setConferenceProxy(confProxy);
        mImsCallSession.setState(ImsCallSessionImplBase.State.RENEGOTIATING);
        mImsCallSession.getCallListenerProxy().onCallHeld(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, mockSuppInfo);
        assertEquals(ImsCallSessionImplBase.State.ESTABLISHED, mImsCallSession.getState());
        verify(mMockImsCallSessionCallback, never()).invokeHeld(any(ImsCallSessionImplBase.class),
                any(ImsCallProfile.class));

        mMockMediaInfo.GTTMode = ImsStreamMediaProfile.RTT_MODE_FULL;
        mImsCallSession.setConferenceProxy(null);
        mImsCallSession.getCallListenerProxy().onCallHeld(mMockMtcCall, mMockCallInfo,
                mMockMediaInfo, mockSuppInfo);
        verify(mMockImsCallSessionCallback).invokeHeld(any(ImsCallSessionImplBase.class),
                any(ImsCallProfile.class));
        verify(mMockImsCallSessionCallback).invokeUpdated(any(ImsCallSessionImplBase.class),
                any(ImsCallProfile.class));
    }

    @Test
    public void testOnCallHoldFailedWithUserTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        mImsCallSession.setTerminationReason(ImsReasonInfo.CODE_USER_TERMINATED);
        mImsCallSession.getCallListenerProxy().onCallHoldFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallHoldFailedWithCallTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        when(mMockMtcCall.isTerminated()).thenReturn(true);
        mImsCallSession.setTerminationReason(0);
        mImsCallSession.getCallListenerProxy().onCallHoldFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_SUPP_SVC_FAILED, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallHoldFailedWithTerminatedReasonOtherThanUserTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        mImsCallSession.setTerminationReason(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR);
        mImsCallSession.getCallListenerProxy().onCallHoldFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_SUPP_SVC_FAILED, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallHoldFailedWithCallReasonInfo() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        when(mMockMtcCall.isTerminated()).thenReturn(false);
        mockCallReasonInfo.mCode = CallReasonInfo.CODE_MEDIA_NO_DATA;
        mImsCallSession.setTerminationReason(0);
        mImsCallSession.getCallListenerProxy().onCallHoldFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeHoldFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_MEDIA_NO_DATA, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallResumeFailedWithUserTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        mImsCallSession.setTerminationReason(ImsReasonInfo.CODE_USER_TERMINATED);
        mImsCallSession.getCallListenerProxy().onCallResumeFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_LOCAL_CALL_TERMINATED, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallResumeFailedWithCallTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        when(mMockMtcCall.isTerminated()).thenReturn(true);
        mImsCallSession.setTerminationReason(0);
        mImsCallSession.getCallListenerProxy().onCallResumeFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_SUPP_SVC_FAILED, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallResumeFailedWithTerminatedReasonOtherThanUserTerminated() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        mImsCallSession.setTerminationReason(ImsReasonInfo.CODE_LOCAL_INTERNAL_ERROR);
        mImsCallSession.getCallListenerProxy().onCallResumeFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_SUPP_SVC_FAILED, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallResumeFailedWithCallReasonInfo() {
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);

        mImsCallSession.setConferenceProxy(null);

        ArgumentCaptor<ImsReasonInfo> reasonInfoCaptor =
                ArgumentCaptor.forClass(ImsReasonInfo.class);

        when(mMockMtcCall.isTerminated()).thenReturn(false);
        mockCallReasonInfo.mCode = CallReasonInfo.CODE_MEDIA_NO_DATA;
        mImsCallSession.setTerminationReason(0);
        mImsCallSession.getCallListenerProxy().onCallResumeFailed(mMockMtcCall, mockCallReasonInfo);
        verify(mMockImsCallSessionCallback).invokeResumeFailed(
                any(ImsCallSessionImplBase.class), reasonInfoCaptor.capture());
        ImsReasonInfo capturedReasonInfo = reasonInfoCaptor.getValue();
        assertEquals(ImsReasonInfo.CODE_MEDIA_NO_DATA, capturedReasonInfo.mCode);
    }

    @Test
    public void testOnCallUpdatedWithoutMultiparty() {
        mImsCallSession.setConferenceProxy(null);

        mMockMediaInfo.GTTMode = MediaInfo.GTTMODE_INACTIVE;
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);

        MediaInfo mNewMockMediaInfo = Mockito.mock(MediaInfo.class);
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);

        mImsCallSession.getCallListenerProxy().onCallUpdated(mMockMtcCall, mMockCallInfo,
                mNewMockMediaInfo, mockSuppInfo);
        verify(mMockImsCallSessionCallback, never()).invokeMultipartyStateChanged(
                any(ImsCallSessionImplBase.class), anyBoolean());
    }

    @Test
    public void testOnCallUpdatedWithMultiparty() {
        mImsCallSession.setConferenceProxy(null);

        mMockMediaInfo.GTTMode = MediaInfo.GTTMODE_INACTIVE;
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);

        mMockCallInfo.isConf = true;
        MediaInfo mNewMockMediaInfo = Mockito.mock(MediaInfo.class);
        mNewMockMediaInfo.GTTMode = MediaInfo.GTTMODE_INACTIVE;
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);

        mImsCallSession.getCallListenerProxy().onCallUpdated(mMockMtcCall, mMockCallInfo,
                mNewMockMediaInfo, mockSuppInfo);
        verify(mMockImsCallSessionCallback).invokeMultipartyStateChanged(
                any(ImsCallSessionImplBase.class), eq(true));
    }

    @Test
    public void testOnCallUpdateReceivedForDowngradeToAudioFromRtt() {
        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        mImsCallProfile.getMediaProfile().mAudioDirection =
                ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE;
        mImsCallProfile.getMediaProfile().setRttMode(ImsStreamMediaProfile.RTT_MODE_FULL);
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);

        assertEquals(IUMtcCall.CALLTYPE_RTT, ImsCallUtils.getCallTypeFromProfile(
                mImsCallProfile.getCallType(), mImsCallProfile.getMediaProfile().isRttCall()));

        mMockMediaInfo.GTTMode = MediaInfo.GTTMODE_FULL;
        when(mMockMtcCall.getMediaInfo()).thenReturn(mMockMediaInfo);

        MediaInfo mNewMockMediaInfo = Mockito.mock(MediaInfo.class);
        mNewMockMediaInfo.ADir = MediaInfo.DIRECTION_RECEIVE;
        mNewMockMediaInfo.GTTMode = MediaInfo.GTTMODE_INACTIVE;
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);
        mImsCallSession.getCallListenerProxy().onCallUpdateReceived(
                    mMockMtcCall, mMockCallInfo, mNewMockMediaInfo, mockSuppInfo);
        processAllMessages();

        ArgumentCaptor<Integer> callTypeCaptor = ArgumentCaptor.forClass(Integer.class);
        ArgumentCaptor<MediaInfo> mediaInfoCaptor = ArgumentCaptor.forClass(MediaInfo.class);

        verify(mMockMtcCall).accept(callTypeCaptor.capture(), mediaInfoCaptor.capture());
        int callType = callTypeCaptor.getValue();
        assertEquals(IUMtcCall.CALLTYPE_VOIP, callType);
        assertEquals(MediaInfo.DIRECTION_RECEIVE, mediaInfoCaptor.getValue().ADir);
    }

    @Test
    public void testOnCallUpdateReceivedForUpgradeToVideoWithSyncVideoCapaDynamically() {
        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);

        mMockCallInfo.videoCapable = true;
        mMockCallInfo.callType = IUMtcCall.CALLTYPE_VT;
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);
        mImsCallSession.getCallListenerProxy().onCallUpdateReceived(
                    mMockMtcCall, mMockCallInfo, mMockMediaInfo, mockSuppInfo);

        verify(mMockImsCallSessionCallback).invokeUpdated(
                any(ImsCallSessionImplBase.class), any(ImsCallProfile.class));
        verify(mVideoCallSession, never()).receiveSessionModifyRequest(
                ImsVideoCallSession.MODIFICATION_CALL_TYPE, mMockMediaInfo);

        processAllFutureMessages();

        verify(mVideoCallSession).receiveSessionModifyRequest(
                ImsVideoCallSession.MODIFICATION_CALL_TYPE, mMockMediaInfo);
    }

    @Test
    public void testOnCallUpdateReceivedForUpgradeToVideoWithVideoCapa() {
        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);
        mImsCallSession.getCallProfile().setCallExtraBoolean(
                ImsCallProfile.EXTRA_CALL_MODE_CHANGEABLE, true);
        mMockCallInfo.videoCapable = true;
        mMockCallInfo.callType = IUMtcCall.CALLTYPE_VT;
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);
        mImsCallSession.getCallListenerProxy().onCallUpdateReceived(
                    mMockMtcCall, mMockCallInfo, mMockMediaInfo, mockSuppInfo);

        verify(mMockImsCallSessionCallback, never()).invokeUpdated(
                any(ImsCallSessionImplBase.class), any(ImsCallProfile.class));
        verify(mVideoCallSession).receiveSessionModifyRequest(
                ImsVideoCallSession.MODIFICATION_CALL_TYPE, mMockMediaInfo);
    }

    @Test
    public void testOnCallRatChanged() {
        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE);
        mImsCallProfile.setCallExtraInt(ImsCallProfile.EXTRA_CALL_NETWORK_TYPE, 0);
        mImsCallProfile.getMediaProfile().mVideoDirection =
                ImsStreamMediaProfile.DIRECTION_INACTIVE;
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);

        mImsCallSession.getCallListenerProxy().onCallRatChanged(1);
        processAllMessages();

        ArgumentCaptor<ImsCallProfile> profileCaptor =
                ArgumentCaptor.forClass(ImsCallProfile.class);
        verify(mMockImsCallSessionCallback).invokeUpdated(
                any(ImsCallSessionImplBase.class), profileCaptor.capture());
        assertEquals(ImsStreamMediaProfile.DIRECTION_INVALID,
                profileCaptor.getValue().getMediaProfile().getVideoDirection());
    }

    @Test
    public void testOnCallTransferred() {
        mImsCallSession.mTransferTargetSession = createImsCallSession("1");
        ImsCallSessionImpl targetSession = mImsCallSession.mTransferTargetSession;
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.mIsEctConfirmationRequired = false;
        targetSession.getCallDetails().set(mCallDetails.ON_ECT);
        mImsCallSession.getCallListenerProxy().onCallTransferred(mockMtcCall);
        assertTrue(mCallDetails.is(mCallDetails.ON_ECT));

        //verify {@link #invokeCallSessionTransferred} not called
        mImsCallSession.getCallListenerProxy().onCallTransferred(mMockMtcCall);
        assertFalse(targetSession.getCallDetails().is(mCallDetails.ON_ECT));
        assertFalse(targetSession.getCallDetails().is(mCallDetails.IMPLICIT_ON_HOLD));
        assertFalse(mImsCallSession.mIsEctConfirmationRequired);
        verify(mMockImsCallSessionCallback, never()).invokeCallSessionTransferred(
                any(ImsCallSessionImplBase.class));

        //verify {@link #invokeCallSessionTransferred} is called
        mImsCallSession = createImsCallSession("2");
        mImsCallSession.mTransferTargetSession = createImsCallSession("3");
        targetSession = mImsCallSession.mTransferTargetSession;
        targetSession.getCallDetails().set(mCallDetails.ON_ECT);
        mImsCallSession.mIsEctConfirmationRequired = true;
        mImsCallSession.getCallListenerProxy().onCallTransferred(mMockMtcCall);
        processAllMessages();
        verify(mMockImsCallSessionCallback).invokeCallSessionTransferred(
                any(ImsCallSessionImplBase.class));
        assertFalse(mImsCallSession.mIsEctConfirmationRequired);
        assertEquals(null, mImsCallSession.mTransferTargetSession);
    }

    @Test
    public void testOnCallTransferFailed() {
        mImsCallSession.mTransferTargetSession = createImsCallSession("1");
        ImsCallSessionImpl targetSession = mImsCallSession.mTransferTargetSession;
        CallReasonInfo mockCallReasonInfo = Mockito.mock(CallReasonInfo.class);
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);

        targetSession.getCallDetails().set(mCallDetails.ON_ECT);
        mImsCallSession.getCallListenerProxy().onCallTransferFailed(mockMtcCall,
                mockCallReasonInfo);
        assertTrue(targetSession.getCallDetails().is(mCallDetails.ON_ECT));

        //verify {@link #resume} and {@link #nvokeCallSessionTransferFailed} is not called
        MtcCall targetCall = targetSession.getMtcCall();
        when(targetCall.isOnHold()).thenReturn(false).thenReturn(true);
        mImsCallSession.getCallListenerProxy().onCallTransferFailed(mMockMtcCall,
                mockCallReasonInfo);
        verify(mMockMtcCall, never()).resume(any(MediaInfo.class));
        assertFalse(targetSession.getCallDetails().is(mCallDetails.ON_ECT));
        assertFalse(targetSession.getCallDetails().is(mCallDetails.IMPLICIT_ON_HOLD));
        assertFalse(mImsCallSession.mIsEctConfirmationRequired);
        verify(mMockImsCallSessionCallback, never()).invokeCallSessionTransferFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        //verify {@link #resume} and {@link #nvokeCallSessionTransferFailed} is called
        mImsCallSession = createImsCallSession("2");
        mImsCallSession.mTransferTargetSession = createImsCallSession("3");
        targetSession = mImsCallSession.mTransferTargetSession;
        targetSession.getCallDetails().set(mCallDetails.ON_ECT);
        mImsCallSession.mIsEctConfirmationRequired = true;
        // set state IMPLICIT_ON_HOLD
        targetSession.getCallDetails().set(mCallDetails.IMPLICIT_ON_HOLD);
        when(targetCall.getCallInfo()).thenReturn(mMockCallInfo);
        when(targetCall.getMediaInfo()).thenReturn(mMockMediaInfo);
        mImsCallSession.getCallListenerProxy().onCallTransferFailed(mMockMtcCall,
                mockCallReasonInfo);
        processAllMessages();
        verify(mMockMtcCall).resume(any(MediaInfo.class));
        assertFalse(targetSession.getCallDetails().is(mCallDetails.ON_ECT));
        assertFalse(targetSession.getCallDetails().is(mCallDetails.IMPLICIT_ON_HOLD));
        processAllMessages();
        verify(mMockImsCallSessionCallback).invokeCallSessionTransferFailed(
                any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));
        assertFalse(mImsCallSession.mIsEctConfirmationRequired);
        assertEquals(null, mImsCallSession.mTransferTargetSession);
    }

    @Test
    public void testOnCallTransferReceived() {
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        SuppInfo mockSuppInfo = Mockito.mock(SuppInfo.class);
        mImsCallSession.getCallListenerProxy().onCallTransferReceived(mockMtcCall, mMockMtcCall,
                mMockCallInfo, mMockMediaInfo, mockSuppInfo);
        verify(mMockImsCallSessionCallback, never()).invokeUpdated(
                any(ImsCallSessionImplBase.class), any(ImsCallProfile.class));

        mImsCallSession = createImsCallSession("1");
        mImsCallSession.getCallListenerProxy().onCallTransferReceived(mMockMtcCall, mockMtcCall,
                mMockCallInfo, mMockMediaInfo, mockSuppInfo);
        verify(mockMtcCall).setListener(mImsCallSession.getCallListenerProxy());
        verify(mMockMtcCall, times(2)).setListener(null);
        verify(mMockImsCallSessionCallback).invokeUpdated(any(ImsCallSessionImplBase.class),
                any(ImsCallProfile.class));
        verify(mMockImsCallSessionCallback).invokeSuppServiceReceived(
                any(ImsCallSessionImplBase.class), any(ImsSuppServiceNotification.class));
    }

    @Test
    public void testOnCallInfoUpdated() {
        MtcCall mockMtcCall = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onCallInfoUpdated(mockMtcCall,
                IUMtcCall.INFO_TYPE_USSD, null, 0, false);
        verify(mMockImsCallSessionCallback, never()).invokeUssdMessageReceived(
                any(ImsCallSessionImplBase.class), any(Integer.class), any(String.class));

        mImsCallSession.getCallListenerProxy().onCallInfoUpdated(mMockMtcCall,
                IUMtcCall.INFO_TYPE_USSD, "123", 0, false);
        verify(mMockImsCallSessionCallback).invokeUssdMessageReceived(
                any(ImsCallSessionImplBase.class), eq(0), eq("123"));

        mImsCallSession.getCallListenerProxy().onCallInfoUpdated(mMockMtcCall,
                IUMtcCall.INFO_TYPE_MEDIA_DTMF_RECEIVED, "123", 0, false);
        verify(mMockImsCallSessionCallback).invokeDtmfReceived(any(ImsCallSessionImplBase.class),
                eq('1'));
    }

    @Test
    public void testOnCallRttMessageReceived() {
        mImsCallSession.getCallListenerProxy().onCallRttMessageReceived(mMockMtcCall, "123");
        verify(mMockImsCallSessionCallback, never()).invokeRttMessageReceived(
                any(ImsCallSessionImplBase.class), eq("123"));

        mImsCallSession.getCallListenerProxy().onCallRttMessageReceived(mMockMtcCall, "123");
        verify(mMockImsCallSessionCallback, never()).invokeRttMessageReceived(
                any(ImsCallSessionImplBase.class), eq("123"));

        //verify isRttCall() true case
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.RTT_MODE_FULL);
        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, new Bundle(), mediaProfile);
        mImsCallSession = createImsCallSession("1");
        mImsCallSession.getCallListenerProxy().onCallRttMessageReceived(mMockMtcCall, "123");
        verify(mMockImsCallSessionCallback).invokeRttMessageReceived(
                any(ImsCallSessionImplBase.class), eq("123"));
    }

    @Test
    public void testOnCallRttAudioIndication() {
        mImsCallSession.getCallListenerProxy().onCallRttAudioIndication(mMockMtcCall, true);
        verify(mMockImsCallSessionCallback, never()).invokeRttAudioIndicatorChanged(
                any(ImsCallSessionImplBase.class), any(ImsStreamMediaProfile.class));

        mImsCallSession.getCallListenerProxy().onCallRttAudioIndication(mMockMtcCall, true);
        verify(mMockImsCallSessionCallback, never()).invokeRttAudioIndicatorChanged(
                any(ImsCallSessionImplBase.class), any(ImsStreamMediaProfile.class));

        //verify isRttCall() true case
        ImsStreamMediaProfile mediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.RTT_MODE_FULL);
        mImsCallProfile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, new Bundle(), mediaProfile);
        mImsCallSession = createImsCallSession("1");
        mImsCallSession.getCallListenerProxy().onCallRttAudioIndication(mMockMtcCall, true);
        verify(mMockImsCallSessionCallback).invokeRttAudioIndicatorChanged(
                any(ImsCallSessionImplBase.class), any(ImsStreamMediaProfile.class));
    }

    @Test
    public void testOnAudioSessionOpened() {
        MtcCall mockMtcCall  = Mockito.mock(MtcCall.class);
        mImsCallSession.getCallListenerProxy().onAudioSessionOpened(mockMtcCall);
        assertFalse(mCallDetails.is(mCallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END));

        mImsCallSession.getCallListenerProxy().onAudioSessionOpened(mMockMtcCall);
        assertTrue(mCallDetails.is(mCallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END));
    }

    @Test
    public void testOnAudioSessionClosed() {
        MtcCall mockMtcCall  = Mockito.mock(MtcCall.class);
        ConferenceProxy mockConferenceProxy = Mockito.mock(ConferenceProxy.class);
        mImsCallSession.getCallListenerProxy().onAudioSessionClosed(mockMtcCall);
        verify(mMockImsCallSessionCallback, never())
                .invokeTerminated(any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        //verify conference call case
        mCallDetails.set(mCallDetails.MERGED);
        mCallDetails.set(mCallDetails.SESSION_TERMINATED_ON_CONFERENCE);
        mImsCallSession.setConferenceProxy(mockConferenceProxy);
        mImsCallSession.getCallListenerProxy().onAudioSessionClosed(mMockMtcCall);
        assertFalse(mCallDetails.is(mCallDetails.SESSION_TERMINATED_ON_CONFERENCE));

        //verify normal call case
        mImsCallSession = createImsCallSession("1");
        mImsCallSession.setState(ImsCallSessionImplBase.State.TERMINATED);
        verifyWaitOrNotifyCallTerminated();
        mCallDetails.set(mCallDetails.CLOSE_PENDING);
        mImsCallSession.getCallListenerProxy().onAudioSessionClosed(mMockMtcCall);
        assertFalse(mCallDetails.is(mCallDetails.WAIT_AUDIO_SESSION_CLOSE_ON_CALL_END));
        verify(mMockImsCallSessionCallback)
                .invokeTerminated(any(ImsCallSessionImplBase.class), any(ImsReasonInfo.class));

        //verify closeSessionWithDelay is called.
        assertFalse(mCallDetails.is(mCallDetails.CLOSE_PENDING));
    }

    @Test
    public void testOnCallQualityChanged() {
        MtcCall mockMtcCall  = Mockito.mock(MtcCall.class);
        mCallDetails.set(mCallDetails.CLOSE_PENDING);
        mImsCallSession.getCallListenerProxy().onCallQualityChanged(mockMtcCall, null);
        verify(mMockImsCallSessionCallback, never()).invokeCallQualityChanged(null);

        mCallDetails.clear(mCallDetails.CLOSE_PENDING);
        mImsCallSession.getCallListenerProxy().onCallQualityChanged(mMockMtcCall, null);
        verify(mMockImsCallSessionCallback).invokeCallQualityChanged(null);
    }

    @Test
    public void testOnNotifyIncomingDtmfReceived() {
        char digit = '5';
        int dtmfDigit = 5;
        mImsCallSession.getCallListenerProxy().onNotifyIncomingDtmfReceived(
                mMockMtcCall, dtmfDigit);
        verify(mMockImsCallSessionCallback).invokeDtmfReceived(
                any(ImsCallSessionImplBase.class), eq(digit));
    }

    @Test
    public void testGetImsConferenceState() {
        assertNotNull(mImsCallSession.getImsConferenceState());
    }

    @Test
    public void testOnEmergencyCallFailedByAlreadyOpenedServiceClosed() {
        mImsCallProfile = new ImsCallProfile(
                ImsCallProfile.SERVICE_TYPE_EMERGENCY, ImsCallProfile.CALL_TYPE_VOICE);
        mImsCallSession = new TestImsCallSessionImpl(
                mMockCallContext, mMockCallTracker, mMockMtcCall,
                mCallId, mImsCallProfile, true, mMockImsCallSessionCallback, mVideoCallSession);
        when(mMockCallContext.getApp()).thenReturn(mMockImsCallApp);
        when(mMockImsCallApp.getCallManager()).thenReturn(mMockImsCallManager);
        when(mMockImsCallManager.getMtcApp()).thenReturn(mMockMtcApp);
        when(mMockServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY))
                .thenReturn(false);

        mImsCallSession.getEmergencyCallFailureListener()
                .onEmergencyCallFailedByAlreadyOpenedServiceClosed();

        verify(mMockMtcApp).openEmergencyService(mMockMtcCall,
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY);
    }

    private TestImsCallSessionImpl createImsCallSession(String callId) {
        TestImsCallSessionImpl callSession =  new TestImsCallSessionImpl(mMockCallContext,
                mMockCallTracker, mMockMtcCall, callId, mImsCallProfile, true,
                mMockImsCallSessionCallback, mVideoCallSession);
        mCallFeaturemap = new HashMap<Integer, Boolean>();
        mCallDetails = callSession.getCallDetails();
        return callSession;
    }

    private class TestImsCallSessionImpl extends ImsCallSessionImpl {
        private ConferenceProxy mConferenceProxy = null;

        TestImsCallSessionImpl(ICallContext callContext,
                CallTracker ct, MtcCall call, String callId, ImsCallProfile profile, boolean isMO,
                ImsCallSessionCallback callBack, ImsVideoCallSession videoCallSession) {
            super(callContext, ct, call, callId, profile, isMO, callBack, videoCallSession);
            mCallDetails = this.getCallDetails();
        }

        @Override
        protected boolean isCallFeatureSupported(int feature) {
            if (mCallFeaturemap.containsKey(feature)) {
                return mCallFeaturemap.get(feature);
            } else {
                return false;
            }
        }

        @Override
        public boolean isConferenceTransitionInProgress() {
            return (mConferenceProxy != null) ? true : false;
        }

        @Override
        public void setConferenceProxy(ConferenceProxy confProxy) {
            mConferenceProxy = confProxy;
        }

        @Override
        protected void closeSessionWithDelay(final ImsCallSessionImpl session, long delay) {
            super.closeSessionWithDelay(session, 0);
        }
    }
}
