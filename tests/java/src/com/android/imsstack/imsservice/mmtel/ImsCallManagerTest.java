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

import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Bundle;
import android.telephony.ServiceState;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsCallSession;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsStreamMediaProfile;
import android.telephony.ims.SrvccCall;

import androidx.test.core.app.ApplicationProvider;

import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.enabler.mtc.CallTracker;
import com.android.imsstack.enabler.mtc.IECallStateTracker;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.util.ImsWakeLock;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;

import java.util.List;
import java.util.concurrent.ConcurrentHashMap;

@RunWith(JUnit4.class)
public class ImsCallManagerTest {
    protected Context mMockContext;
    private IMmTelCallListener mMockIMmTelCallListener;
    private TestImsCallManager mImsCallManager;
    private MtcApp mMockMtcApp;
    private MtcCall mMockMtcCall;
    private ImsCallSessionImpl mMockImsCallSession;
    private TestImsCallManager mImsCallManagernull;
    private ImsCallContext mMockCallContext;
    private static final String CALL_ID = "12";
    private static final String PENDING_CALL_ID = "13";
    private ImsCallManager.MtcAppCallListenerProxy mMtcAppCallListenerProxy;
    private ImsCallManager.MtcAppCallListenerProxy mMtcAppCallListenerProxyNull;
    private ImsCallManager.ImsCallTracker mImsCallTracker;
    private TestAppContext mTestAppContext;

    @Before
    public void setUp() {
        mMockContext = Mockito.spy(ApplicationProvider.getApplicationContext());
        mMockCallContext = Mockito.mock(ImsCallContext.class);

        mTestAppContext = new TestAppContext(mMockContext);
        mTestAppContext.setUp();

        MessageExecutor mExecutor = new MessageExecutor(ImsCallManager.class.getSimpleName());
        when(mMockCallContext.getExecutor()).thenReturn(mExecutor);
        when(mMockCallContext.getContext()).thenReturn(mMockContext);
        when(mMockCallContext.getPhoneId()).thenReturn(1);

        mMockMtcApp = Mockito.mock(MtcApp.class);
        mMockMtcCall = Mockito.mock(MtcCall.class);
        mMockImsCallSession = Mockito.mock(ImsCallSessionImpl.class);
        mMockIMmTelCallListener = Mockito.mock(IMmTelCallListener.class);

        ArgumentCaptor<ImsCallManager.MtcAppCallListenerProxy> mListenerArg =
                ArgumentCaptor.forClass(ImsCallManager.MtcAppCallListenerProxy.class);
        mImsCallManager = new TestImsCallManager(mMockCallContext, mMockMtcApp,
                mMockIMmTelCallListener);
        mImsCallTracker = mImsCallManager.new ImsCallTracker();
        verify(mMockMtcApp).setCallListener(mListenerArg.capture());
        mMtcAppCallListenerProxy = mListenerArg.getValue();
        mImsCallManagernull = new TestImsCallManager(mMockCallContext, mMockMtcApp, null);
    }

    @After
    public void tearDown() {
        mImsCallManager = null;
        mImsCallTracker = null;
        mImsCallManagernull = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    public void clearTest() {
        // verify api clearLock ,WifiCallWakeLock inner class
        Assert.assertNotNull(mImsCallManager.mWifiCallWakeLock);
        mImsCallManager.clear();
        verify(mMockMtcApp).setCallListener(null);
        Assert.assertNull(mImsCallManager.mWifiCallWakeLock);
        Assert.assertNull(mImsCallManager.mIncomingCallInfo);
    }

    @Test
    public void disposeTest() {
        mImsCallManager.dispose();
        verify(mMockMtcApp).setCallListener(null);
    }

    @Test
    public void closeAllSessionsTest() {
        // verify terminateAllPendingSessions
        mImsCallManager.closeAllSessions();
        Assert.assertTrue(mImsCallManager.getPendingSession().isEmpty());

        ImsCallSessionImpl pendingCallSession = Mockito.mock(ImsCallSessionImpl.class);
        when(pendingCallSession.getCallId()).thenReturn(PENDING_CALL_ID);
        mImsCallManager.getPendingSession().put(PENDING_CALL_ID, pendingCallSession);
        mImsCallManager.closeAllSessions();
        verify(pendingCallSession, times(2)).getState();
        verify(pendingCallSession).close();

        // verify terminateAllSessions
        mImsCallManager.closeAllSessions();
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());

        reset(mMockImsCallSession);
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        mImsCallManager.closeAllSessions();
        verify(mMockImsCallSession).notifyCallTerminatedByServiceUnavailable();
        verify(mMockImsCallSession).close();

        // verify removePendingSessions
        mImsCallManager.getPendingSession().clear();
        mImsCallManager.closeAllSessions();
        Assert.assertTrue(mImsCallManager.getPendingSession().isEmpty());
        when(pendingCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getPendingSession().put(CALL_ID, pendingCallSession);
        mImsCallManager.closeAllSessions();
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());
        Assert.assertEquals(0, mImsCallManager.getSession().size());

        // Verify destroyAllSessions->onCallDestroy ,removePendingSession
        reset(mMockImsCallSession);
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        when(pendingCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getPendingSession().put(CALL_ID, pendingCallSession);
        Assert.assertTrue(mImsCallManager.getPendingSession().containsKey(CALL_ID));
        mImsCallManager.closeAllSessions();
        Assert.assertFalse(mImsCallManager.getPendingSession().containsKey(CALL_ID));
        verify(mMockImsCallSession, times(2)).getCallId();
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());

        // Verify destroyAllSessions->onCallDestroy->updateCallProfileOnSingleCall
        reset(mMockImsCallSession);
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        mImsCallManager.closeAllSessions();
        verify(mMockImsCallSession, times(2)).getCallId();
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());
    }

    @Test
    public void createSessionTest() {
        ImsCallProfile profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VOICE);

        // verify checkAndExitEcbm()
        int callAttributes = MtcCall.FLAG_MO | MtcCall.FLAG_EMERGENCY;
        IECallStateTracker mockIECallStateTracker = Mockito.mock(IECallStateTracker.class);
        when(mMockCallContext.getECallStateTracker()).thenReturn(mockIECallStateTracker);
        when(mockIECallStateTracker.isEcbmEntered()).thenReturn(true);
        when(mMockMtcApp.createMtcCallAndAttach(callAttributes)).thenReturn(mMockMtcCall);
        when(mMockMtcCall.isEmergencyCall()).thenReturn(true);
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        ImsCallSessionImpl result = mImsCallManager.createSession(profile);
        verify(mockIECallStateTracker).exitEmergencyCallbackMode(anyBoolean());
        verify(mMockCallContext).getECallStateTracker();
        verify(mMockMtcApp).createMtcCallAndAttach((MtcCall.FLAG_EMERGENCY | MtcCall.FLAG_MO));
        verify(mMockMtcCall).isEmergencyCall();
        verify(mMockMtcCall, never()).open(anyInt(), anyInt(), anyBoolean(), anyBoolean(),
                anyBoolean());
        Assert.assertNotNull(result);
        verifyNoMoreInteractions(mMockMtcCall);

        //verify mtcCall.open
        Mockito.reset(mMockMtcApp);
        Mockito.reset(mMockMtcCall);
        when(mMockMtcApp.createMtcCallAndAttach(callAttributes)).thenReturn(mMockMtcCall);
        when(mMockMtcCall.isEmergencyCall()).thenReturn(false);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcCall).isEmergencyCall();
        verify(mMockMtcCall).open(anyInt(), anyInt(), anyBoolean(), anyBoolean(), anyBoolean());
        Assert.assertNotNull(result);
        verifyNoMoreInteractions(mMockMtcCall);

        // Verify Emergency call with call attribute video
        Mockito.reset(mMockMtcApp);
        profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VT);
        callAttributes |= MtcCall.FLAG_VIDEO_CALL;
        when(mMockMtcApp.createMtcCallAndAttach(callAttributes)).thenReturn(mMockMtcCall);
        mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(callAttributes);
        Assert.assertNotNull(result);

        // Verify Emergency call with call attribute video and Rtt
        Bundle extras = new Bundle();
        ImsStreamMediaProfile mMediaProfile = new ImsStreamMediaProfile(
                ImsStreamMediaProfile.RTT_MODE_FULL);
        callAttributes |= MtcCall.FLAG_RTT;
        profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VT, extras, mMediaProfile);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(callAttributes);
        Assert.assertNotNull(result);

        // Verify Emergency call with call attribute voice and Rtt
        profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VOICE, extras, mMediaProfile);
        callAttributes &= ~MtcCall.FLAG_VIDEO_CALL;
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(callAttributes);
        Assert.assertNotNull(result);

        // verify when mtccall is null
        reset(mMockMtcApp);
        when(mMockMtcApp.createMtcCallAndAttach(callAttributes)).thenReturn(null);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(callAttributes);
        Assert.assertNotNull(result);
        verifyNoMoreInteractions(mMockMtcApp);

        // case2 Normal call voice
        reset(mMockCallContext);
        reset(mMockMtcCall);
        extras.putInt(ImsCallProfile.EXTRA_DIALSTRING, ImsCallProfile.DIALSTRING_USSD);
        int normalCallAttributes = MtcCall.FLAG_MO;
        when(mMockMtcApp.createMtcCallAndAttach(normalCallAttributes)).thenReturn(mMockMtcCall);
        when(mMockMtcCall.isEmergencyCall()).thenReturn(false);
        when(mMockCallContext.getContext()).thenReturn(mMockContext);
        profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(normalCallAttributes);
        verify(mMockCallContext).getSrvccStateTracker();
        verify(mMockMtcCall).isEmergencyCall();
        verify(mMockMtcCall).open(anyInt(), anyInt(), anyBoolean(), anyBoolean(), anyBoolean());
        Assert.assertNotNull(result);

        // case2 Normal call voice and rtt
        normalCallAttributes = MtcCall.FLAG_MO | MtcCall.FLAG_RTT;
        profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE, extras, mMediaProfile);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(normalCallAttributes);
        Assert.assertNotNull(result);

        // Normal call with video and rtt
        normalCallAttributes |= MtcCall.FLAG_VIDEO_CALL;
        profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT, extras, mMediaProfile);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(normalCallAttributes);
        Assert.assertNotNull(result);

        reset(mMockMtcApp);
        when(mMockMtcApp.createMtcCallAndAttach(normalCallAttributes)).thenReturn(null);
        result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(normalCallAttributes);
        Assert.assertNotNull(result);
        verifyNoMoreInteractions(mMockMtcApp);
    }

    @Test
    public void createSessionEmergencyCallOverWiFiTest() {
        ImsCallProfile profile = new ImsCallProfile(ImsCallProfile.SERVICE_TYPE_EMERGENCY,
                ImsCallProfile.CALL_TYPE_VOICE);
        Bundle extras = new Bundle();
        extras.putString(ImsCallProfile.EXTRA_CALL_RAT_TYPE,
                String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_IWLAN));
        profile.mCallExtras.putBundle(ImsCallProfile.EXTRA_OEM_EXTRAS, extras);

        int callAttributes =
                MtcCall.FLAG_MO | MtcCall.FLAG_EMERGENCY | MtcCall.FLAG_WIFI_EMERGENCY;
        when(mMockMtcApp.createMtcCallAndAttach(callAttributes)).thenReturn(mMockMtcCall);
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);

        ImsCallSessionImpl result = mImsCallManager.createSession(profile);
        verify(mMockMtcApp).createMtcCallAndAttach(callAttributes);
        Assert.assertNotNull(result);
    }

    @Test
    public void getTakeSessionTest() {
        // verify false case condition
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        boolean takeSession = mImsCallManager.takeSession(mMockImsCallSession);
        Assert.assertFalse(takeSession);

        // verify takesession and removePendingSession same sessions
        when(mMockMtcCall.isOnHold()).thenReturn(false);
        when(mMockMtcCall.isInCall()).thenReturn(true);
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        mImsCallManager.getPendingSession().put(CALL_ID, mMockImsCallSession);
        Assert.assertEquals(mMockImsCallSession, mImsCallManager.getPendingSession().get(CALL_ID));
        Assert.assertFalse(mImsCallManager.getPendingSession().isEmpty());
        takeSession = mImsCallManager.takeSession(mMockImsCallSession);
        Assert.assertTrue(takeSession);
        verify(mMockCallContext, Mockito.times(1)).getSrvccStateTracker();
        verify(mMockImsCallSession).setCallConnectionId(anyInt());
        verify(mMockImsCallSession).takeCall();
        Assert.assertTrue(mImsCallManager.getPendingSession().isEmpty());

        // verify removePendingSession with different sessions
        mImsCallManager.getPendingSession().remove(CALL_ID);
        ImsCallSessionImpl sessionObj = mImsCallManagernull.createSession(new ImsCallProfile());
        mImsCallManager.getPendingSession().put(CALL_ID, sessionObj);
        Assert.assertNotEquals(mMockImsCallSession, mImsCallManager.getPendingSession());
        when(mMockMtcCall.isOnHold()).thenReturn(false);
        when(mMockMtcCall.isInCall()).thenReturn(true);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        Assert.assertFalse(mImsCallManager.getPendingSession().isEmpty());
        Assert.assertEquals(1, mImsCallManager.getPendingSession().size());
        Assert.assertNotNull(mImsCallManager.getPendingSession().get(CALL_ID));
        takeSession = mImsCallManager.takeSession(mMockImsCallSession);
        Assert.assertNotEquals(mMockImsCallSession,
                mImsCallManager.getPendingSession());
        Assert.assertNull(mImsCallManager.getPendingSession().get(CALL_ID));
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());
        Assert.assertTrue(takeSession);
    }

    @Test
    public void getConferenceCallTest() {
        Mockito.reset(mMockImsCallSession);
        ImsCallSessionImpl session = mImsCallManager.getConferenceCall();
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());
        Assert.assertNull(session);

        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        when(mMockImsCallSession.getProperty(MtcCall.EXTRA_CONFERENCE)).thenReturn("true");
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        session = mImsCallManager.getConferenceCall();
        Assert.assertNull(session);

        when(mMockImsCallSession.getState()).thenReturn(ImsCallSession.State.TERMINATED);
        session = mImsCallManager.getConferenceCall();
        Assert.assertNotNull(session);
        Assert.assertEquals(mMockImsCallSession, session);

        when(mMockImsCallSession.getState()).thenReturn(ImsCallSession.State.NEGOTIATING);
        session = mImsCallManager.getConferenceCall();
        Assert.assertNull(session);
    }

    @Test
    public void getConnectingSessionReturnsRingbackSessionTest() {
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        ImsCallSessionImpl implSession = mImsCallManager.getConnectingSession();
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());
        Assert.assertNull(implSession);

        when(mMockMtcCall.isOnHold()).thenReturn(false);
        when(mMockMtcCall.isInCall()).thenReturn(false);
        when(mMockImsCallSession.getMtcCall()).thenReturn(mMockMtcCall);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        implSession = mImsCallManager.getConnectingSession();
        Assert.assertEquals(mMockImsCallSession, implSession);
        verify(mMockImsCallSession).getMtcCall();
        verify(mMockMtcCall).isInCall();
        verify(mMockMtcCall).isOnHold();
    }

    @Test
    public void getConnectingSessionReturnsRingingSessionTest() {
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        ImsCallSessionImpl implSession = mImsCallManager.getConnectingSession();
        Assert.assertTrue(mImsCallManager.getPendingSession().isEmpty());
        Assert.assertNull(implSession);

        when(mMockImsCallSession.getMtcCall()).thenReturn(mMockMtcCall);
        mImsCallManager.getPendingSession().put(CALL_ID, mMockImsCallSession);
        implSession = mImsCallManager.getConnectingSession();
        Assert.assertNotNull(implSession);
        Assert.assertEquals(mMockImsCallSession, implSession);
        verify(mMockImsCallSession).getMtcCall();
    }

    @Test
    public void getActiveSessionTest() {
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        ImsCallSessionImpl implSession = mImsCallManager.getActiveSession();
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());
        Assert.assertNull(implSession);

        when(mMockMtcCall.isOnHold()).thenReturn(false);
        when(mMockMtcCall.isInCall()).thenReturn(true);
        when(mMockMtcCall.getCallId()).thenReturn(CALL_ID);
        when(mMockImsCallSession.getMtcCall()).thenReturn(mMockMtcCall);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        implSession = mImsCallManager.getActiveSession();
        Assert.assertNotNull(implSession);
        Assert.assertEquals(mMockImsCallSession, implSession);
        verify(mMockImsCallSession).getMtcCall();
        verify(mMockMtcCall).isInCall();
        verify(mMockMtcCall).isOnHold();
    }

    @Test
    public void getHoldSessionTest() {
        ImsCallSessionImpl result = mImsCallManager.getHoldSession();
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());
        Assert.assertNull(result);

        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        when(mMockMtcCall.isOnHold()).thenReturn(true);
        when(mMockMtcCall.getCallId()).thenReturn(CALL_ID);
        when(mMockImsCallSession.getMtcCall()).thenReturn(mMockMtcCall);
        result = mImsCallManager.getHoldSession();
        verify(mMockImsCallSession).getMtcCall();
        verify(mMockMtcCall).isOnHold();
        Assert.assertEquals(mMockImsCallSession, result);
    }

    @Test
    public void getSrvccCallsTest() {
        int preciseState = 0;
        ImsCallProfile profile = new ImsCallProfile();

        //For valid values of pending session
        ImsCallSessionImpl pendingCallSession = Mockito.mock(ImsCallSessionImpl.class);
        when(pendingCallSession.getCallId()).thenReturn(PENDING_CALL_ID);
        when(pendingCallSession.getPreciseState()).thenReturn(preciseState);
        when(pendingCallSession.getCallProfile()).thenReturn(profile);
        mImsCallManager.getPendingSession().put(PENDING_CALL_ID, pendingCallSession);

        //For valid values of session
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        when(mMockImsCallSession.getPreciseState()).thenReturn(preciseState);
        when(mMockImsCallSession.getCallProfile()).thenReturn(profile);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        SrvccCall srvccCall = new SrvccCall(CALL_ID, preciseState, profile);

        List<SrvccCall> newSrvccCall = mImsCallManager.getSrvccCalls();
        Assert.assertEquals(CALL_ID, newSrvccCall.get(0).getCallId());
        Assert.assertEquals(preciseState, newSrvccCall.get(0).getPreciseCallState());
        Assert.assertTrue(newSrvccCall.contains(srvccCall));

        srvccCall = new SrvccCall(CALL_ID, 1, profile);
        newSrvccCall = mImsCallManager.getSrvccCalls();
        Assert.assertFalse(newSrvccCall.contains(srvccCall));
        profile = null;
    }

    @Test
    public void getPhoneIdTest() {
        int result = mImsCallManager.getPhoneId();
        Assert.assertEquals(1, result);
    }

    @Test
    public void getMtcAppTest() {
        MtcApp result = mImsCallManager.getMtcApp();
        Assert.assertEquals(mMockMtcApp, result);
    }

    // WifiCallWakeLock innerclass Verification
    @Test
    public void initTest() {
        ImsCallManager.WifiCallWakeLock wifiCall = mImsCallManager.mWifiCallWakeLock;
        wifiCall.mWakeLock = null;
        mImsCallManager.mWifiCallWakeLock.init();
        Assert.assertNotNull(wifiCall.mWakeLock);
    }

    @Test
    public void clearLockTest() {
        mImsCallManager.mWifiCallWakeLock.clearLock();
        verify(mImsCallManager.mWifiCallWakeLock.mWakeLock).release(any(Object.class));
    }

    @Test
    public void wifiCallWakeLockClearTest() {
        Assert.assertNotNull(mImsCallManager.mWifiCallWakeLock.mWakeLock);
        mImsCallManager.mWifiCallWakeLock.clear();
        verify(mImsCallManager.mWakeLock).release(any());
        Assert.assertNull(mImsCallManager.mWifiCallWakeLock.mWakeLock);
    }

    @Test
    public void initStateAndLockTest() {
        mImsCallManager.mWifiCallWakeLock.initStateAndLock();
        verify(mImsCallManager.mWakeLock).release(any());
        Assert.assertNotNull(mImsCallManager.mWifiCallWakeLock.mWakeLock);
    }

    @Test
    public void onStateChangedTest() {
        mImsCallManager.mWifiCallWakeLock.clearLock();
        Assert.assertNotNull(mImsCallManager.mWifiCallWakeLock.mWakeLock);
        mImsCallManager.mWifiCallWakeLock.onStateChanged();
        ImsWakeLock mImsWakeLock = mImsCallManager.mWifiCallWakeLock.mWakeLock;
        verify(mImsWakeLock).release(any(Object.class));
    }

    // verify ImsCallTracker inner class
    @Test
    public void createCallIdTest() {
        String callId = mImsCallTracker.createCallId();
        Assert.assertNotNull(callId);
        String callId2 = mImsCallTracker.createCallId();
        Assert.assertNotEquals(callId, callId2);
        Assert.assertEquals(Integer.parseInt(callId) + 1, Integer.parseInt(callId2));

        int sessionId = Integer.parseInt(callId2) + 1;
        when(mMockImsCallSession.getCallId()).thenReturn(String.valueOf(sessionId));
        mImsCallManager.getSession().put(String.valueOf(sessionId), mMockImsCallSession);
        String callId3 = mImsCallTracker.createCallId();
        Assert.assertEquals(String.valueOf(sessionId + 1), callId3);

        int pendingId = Integer.parseInt(callId3) + 1;
        ImsCallSessionImpl mPendingSession = Mockito.mock(ImsCallSessionImpl.class);
        when(mPendingSession.getCallId()).thenReturn(String.valueOf(pendingId));
        mImsCallManager.getPendingSession().put(String.valueOf(pendingId), mPendingSession);
        String callId4 = mImsCallTracker.createCallId();
        Assert.assertEquals(String.valueOf(pendingId + 1), callId4);
    }

    @Test
    public void getActiveCallsTest() {
        Mockito.reset(mMockImsCallSession);
        Assert.assertEquals(0, mImsCallTracker.getActiveCalls());

        when(mMockImsCallSession.isInCall()).thenReturn(true);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        Assert.assertEquals(1, mImsCallTracker.getActiveCalls());
    }

    @Test
    public void updateCallStateTest() {
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallTracker.updateCallState(null,
                CallTracker.CALL_EVENT_CREATE, null);
        verifyNoMoreInteractions(mMockImsCallSession);

        mImsCallTracker.updateCallState(Mockito.mock(Object.class),
                CallTracker.CALL_EVENT_CREATE, null);
        verifyNoMoreInteractions(mMockImsCallSession);

        // Verify updateCallState, onCallCreate
        Assert.assertEquals(0, mImsCallManager.getSession().size());
        mImsCallTracker.updateCallState(mMockImsCallSession,
                CallTracker.CALL_EVENT_CREATE, null);
        Assert.assertEquals(1, mImsCallManager.getSession().size());
        verify(mMockCallContext).getSrvccStateTracker();

        String callId = "20";
        // Verify onCallIncomingReceived
        ImsCallSessionImpl mPendingSession = Mockito.mock(ImsCallSessionImpl.class);
        when(mPendingSession.getCallId()).thenReturn(callId);
        mImsCallManager.getPendingSession().put(callId, mPendingSession);
        mImsCallTracker.updateCallState(mPendingSession,
                CallTracker.CALL_EVENT_INCOMING_RECEIVED, null);
        verify(mMockIMmTelCallListener).onIncomingCallReceived(eq(mPendingSession));
        Assert.assertEquals(1, mImsCallManager.getPendingSession().size());

        // Verify onCallIncomingReceived ->rejectAndDestroyCall when mMmTelCallListener null
        reset(mMockIMmTelCallListener);
        ImsCallManager.ImsCallTracker mctNull = mImsCallManagernull.new ImsCallTracker();
        mImsCallManagernull.getPendingSession().put(callId, mPendingSession);
        mImsCallManagernull.getSession().put(callId, mPendingSession);
        mctNull.updateCallState(mPendingSession,
                CallTracker.CALL_EVENT_INCOMING_RECEIVED, null);
        verify(mMockIMmTelCallListener, never()).onIncomingCallReceived(
                eq(mPendingSession));
        verify(mPendingSession, timeout(100).times(2)).close();
        verify(mPendingSession, timeout(100)).reject(
                eq(ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE));

        // Verify onCallDestroy with pending session
        Assert.assertTrue(
                mImsCallManager.getPendingSession().containsKey(callId));
        mImsCallTracker.updateCallState(mPendingSession, CallTracker.CALL_EVENT_DESTROY, null);
        Assert.assertFalse(
                mImsCallManager.getPendingSession().containsKey(callId));

        // Verify onCallDestroy with session
        Assert.assertEquals(1, mImsCallManager.getSession().size());
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());
        mImsCallTracker.updateCallState(mMockImsCallSession, CallTracker.CALL_EVENT_DESTROY, null);
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());
        Assert.assertEquals(0, mImsCallManager.getSession().size());

        ImsCallSessionImpl session2 = Mockito.mock(ImsCallSessionImpl.class);
        mImsCallManager.getSession().put(CALL_ID, session2);
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());
        Assert.assertEquals(1, mImsCallManager.getSession().size());
        mImsCallTracker.updateCallState(mMockImsCallSession, CallTracker.CALL_EVENT_DESTROY, null);
        Assert.assertTrue(mImsCallManager.getSession().isEmpty());
        Assert.assertEquals(0, mImsCallManager.getSession().size());
    }

    // Test inner class MtcAppCallListenerProxy
    @Test
    public void onPreIncomingCallReceivedTest() {
        mMtcAppCallListenerProxy.onPreIncomingCallReceived(null, -1);
        verify(mMockMtcApp).getPendingCall(-1);
        Assert.assertEquals(0, mImsCallManager.getPendingSession().size());

        // Verify function onPreIncomingCallReceived->onCallPreIncomingReceived()
        ImsCallSessionImpl pendingSession = Mockito.mock(ImsCallSessionImpl.class);
        mImsCallManager.getPendingSession().put(PENDING_CALL_ID, pendingSession);
        when(mMockMtcCall.getNativeCallId()).thenReturn(4L);
        MtcCall call = Mockito.mock(MtcCall.class);
        when(mMockMtcApp.getPendingCall(4L)).thenReturn(call);
        when(mMockImsCallSession.getCallId()).thenReturn("4");
        mMtcAppCallListenerProxy.onPreIncomingCallReceived(mMockMtcApp, 4L);
        verify(mMockMtcApp).getPendingCall(4L);
        Assert.assertNotNull(mImsCallManager.getPendingSession().get("4"));
        Assert.assertNotNull(mImsCallManager.getPendingSession().get(PENDING_CALL_ID));

        reset(mMockMtcApp);
        reset(mMockImsCallSession);
        when(mMockMtcCall.getNativeCallId()).thenReturn(12L);
        when(mMockMtcApp.getPendingCall(12)).thenReturn(mMockMtcCall);
        ArgumentCaptor<ImsCallManager.MtcAppCallListenerProxy> mListenerArgNull =
                ArgumentCaptor.forClass(ImsCallManager.MtcAppCallListenerProxy.class);
        mImsCallManagernull = new TestImsCallManager(mMockCallContext, mMockMtcApp, null);
        verify(mMockMtcApp, times(1)).setCallListener(mListenerArgNull.capture());
        mMtcAppCallListenerProxyNull = mListenerArgNull.getValue();
        mImsCallManagernull.getSession().put(CALL_ID, mMockImsCallSession);
        mMtcAppCallListenerProxyNull.onPreIncomingCallReceived(mMockMtcApp, 12L);
        verify(mMockImsCallSession, timeout(100)).close();
        verify(mMockImsCallSession, timeout(100)).reject(
                eq(ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE));
    }

    @Test
    public void rejectAndDestroyCallExceptionTest() {
        ImsCallSessionImpl mPendingSession = Mockito.mock(ImsCallSessionImpl.class);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(mMockIMmTelCallListener)
            .onIncomingCallReceived(mPendingSession);
        doThrow(mockRuntimeException).when(mPendingSession)
            .reject(ImsReasonInfo.CODE_LOCAL_SERVICE_UNAVAILABLE);
        mImsCallTracker.updateCallState(mPendingSession, CallTracker.CALL_EVENT_INCOMING_RECEIVED,
                null);
        verify(mockRuntimeException).getMessage();
    }

    @Test
    public void terminateAllSessionsExceptionTest() {
        when(mMockImsCallSession.getCallId()).thenReturn(CALL_ID);
        mImsCallManager.getSession().put(CALL_ID, mMockImsCallSession);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(mMockImsCallSession).close();
        mImsCallManager.closeAllSessions();
        verify(mockRuntimeException).getMessage();
        verify(mMockImsCallSession).close();
    }

    @Test
    public void terminateAllPendingSessionsExceptionTest() {
        ImsCallSessionImpl pendingCallSession = Mockito.mock(ImsCallSessionImpl.class);
        when(pendingCallSession.getCallId()).thenReturn(PENDING_CALL_ID);
        mImsCallManager.getPendingSession().put(PENDING_CALL_ID, pendingCallSession);
        RuntimeException mockRuntimeException = Mockito.mock(RuntimeException.class);
        doThrow(mockRuntimeException).when(pendingCallSession).close();
        mImsCallManager.closeAllSessions();
        verify(mockRuntimeException).getMessage();
        verify(pendingCallSession).close();
    }

    class TestImsCallManager extends ImsCallManager {
        protected ImsWakeLock mWakeLock = Mockito.mock(ImsWakeLock.class);

        TestImsCallManager(ICallContext callContext, MtcApp mtcApp,
                IMmTelCallListener listener) {
            super(callContext, mtcApp, listener);
            mWifiCallWakeLock = new WifiCallWakeLock(mWakeLock);
        }

        public ConcurrentHashMap<String, ImsCallSessionImpl> getSession() {
            return mSessions;
        }

        public ConcurrentHashMap<String, ImsCallSessionImpl> getPendingSession() {
            return mPendingSessions;
        }

        public ImsCallProfile getIncomingCallInfo() {
            return mIncomingCallInfo;
        }

        @Override
        protected boolean isCallOverWifiSupported() {
            return true;
        }

        public ImsCallSessionImpl createImsCallSession(ICallContext callContext, CallTracker ct,
                MtcCall call, String callId, ImsCallProfile profile, boolean isMO) {
            return mMockImsCallSession;
        }
    }
}
