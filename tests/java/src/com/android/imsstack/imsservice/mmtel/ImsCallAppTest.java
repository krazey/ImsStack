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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.clearInvocations;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.timeout;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.os.Looper;
import android.telecom.TelecomManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsStreamMediaProfile;

import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.imsservice.mmtel.base.IMmTelCallListener;
import com.android.imsstack.imsservice.mmtel.base.IMmTelFeatureCapabilityListener;
import com.android.imsstack.imsservice.mmtel.base.TtyModeTracker;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.MessageExecutor;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsCallAppTest {
    private static final int PHONE_ID = MSimUtils.DEFAULT_PHONE_ID;
    private ImsCallApp mImsCallApp;
    private MessageExecutor mExecutor;
    private MockIAosRegistration mAosReg;
    private int mSlotId0 = MSimUtils.DEFAULT_SLOT_ID;
    ImsUtImpl mImsUtImpl;
    ImsEcbmImpl mImsEcbmImpl;
    ImsMultiEndpointImpl mMultiEndpoint;
    ImsSmsImpl mImsSmsImpl;

    @Mock private static Context sMockContext;
    @Mock private Context mMockContext;
    @Mock private ImsCallContext mMockImsCallContext;
    @Mock private IMmTelFeatureCapabilityListener mMockFeatureCapabilityListener;
    @Mock private IMmTelCallListener mMockCallListener;
    @Mock private ImsCallManager mMockImsCallManager;
    @Mock private ImsFeatureManager mFeatureManager;
    @Mock private ImsRegistrationTracker mRegTracker;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mExecutor = new MessageExecutor(ImsCallApp.class.getSimpleName());
        when(mMockImsCallContext.getExecutor()).thenReturn(mExecutor);
        doReturn(Looper.getMainLooper()).when(mMockImsCallContext).getCallLooper();

        mImsCallApp = new ImsCallApp(PHONE_ID, mMockContext, mExecutor, mRegTracker,
                        mMockFeatureCapabilityListener, mMockCallListener, mMockImsCallContext,
                        mMockImsCallManager, mFeatureManager);
    }

    @BeforeClass
    public static void setUpOnce() {
        sMockContext = Mockito.mock(Context.class);
        AppContext.init(sMockContext);
    }

    @After
    public void tearDown() throws Exception {
        mImsCallApp = null;
        mExecutor = null;
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
    }

    @Test
    public void test_close() {
        mImsCallApp.close();
        assertNull(mImsEcbmImpl);
        assertNull(mMultiEndpoint);
        assertNull(mImsUtImpl);
        assertNull(mImsSmsImpl);

        mImsEcbmImpl = mImsCallApp.getEcbmInterface();
        mMultiEndpoint = mImsCallApp.getMultiEndpointInterface();
        mImsCallApp.close();
        verify(mMockImsCallContext, times(2)).dispose();
        verify(mFeatureManager).setRegistrationTracker(null);
        verify(mMockImsCallManager).dispose();
        verify(mFeatureManager).dispose();
    }

    @Test
    public void test_bindUnbindCallApp() {
        mImsCallApp.getEcbmInterface();
        mImsCallApp.getUtInterface();
        //for mInitCompleted == true
        mImsCallApp.bindCallApp();
        verify(mRegTracker).refreshCallRegistrationState();
        //for mInitCompleted == true
        mImsCallApp.unbindCallApp();
        verify(mFeatureManager).updateFeaturesOnServiceUpDown(false);
        clearInvocations(mFeatureManager);
        //for mInitCompleted == false
        mImsCallApp.unbindCallApp();
        verify(mFeatureManager, never()).updateFeaturesOnServiceUpDown(false);
        //for mInitCompleted == false
        mImsCallApp.bindCallApp();
        verify(mRegTracker, times(2)).refreshCallRegistrationState();
        verify(mFeatureManager).updateFeaturesOnServiceUpDown(true);
    }

    @Test
    public void test_isConnected() {
        assertEquals(false, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NONE,
                ImsCallProfile.CALL_TYPE_VT_NODIR));
        assertEquals(false, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT));

        when(mRegTracker.isCallRegistered()).thenReturn(false);
        assertEquals(false, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL, 0));

        when(mRegTracker.isCallVoiceAndVideoRegistered()).thenReturn(true);
        assertEquals(true, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE_N_VIDEO));
        assertEquals(true, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE));
        assertEquals(true, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE));

        when(mRegTracker.isCallVoiceAndVideoRegistered()).thenReturn(false);
        assertEquals(false, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VOICE));

        when(mRegTracker.isCallVideoRegistered()).thenReturn(true);
        assertEquals(true, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT_TX));

        when(mRegTracker.isCallVideoRegistered()).thenReturn(false);
        assertEquals(false, mImsCallApp.isConnected(ImsCallProfile.SERVICE_TYPE_NORMAL,
                ImsCallProfile.CALL_TYPE_VT_RX));
    }

    @Test
    public void test_createCallProfileForVoiceCall() {
        IDcNetWatcher dcnw = Mockito.mock(IDcNetWatcher.class);
        when(mMockImsCallContext.getDcNetWatcher()).thenReturn(dcnw);
        when(dcnw.is3G()).thenReturn(true);
        when(mRegTracker.isCallVoiceAndVideoRegistered()).thenReturn(true);

        ImsCallProfile callProfile = mImsCallApp.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VOICE);
        assertEquals(ImsCallProfile.SERVICE_TYPE_NORMAL, callProfile.mServiceType);
        assertEquals(ImsCallProfile.CALL_TYPE_VOICE, callProfile.mCallType);

        callProfile = mImsCallApp.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_EMERGENCY, ImsCallProfile.CALL_TYPE_VOICE);
        assertEquals(ImsCallProfile.SERVICE_TYPE_EMERGENCY, callProfile.mServiceType);
        assertEquals(ImsCallProfile.CALL_TYPE_VOICE, callProfile.mCallType);

        callProfile = mImsCallApp.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NONE, ImsCallProfile.CALL_TYPE_VOICE);
        assertEquals(ImsCallProfile.SERVICE_TYPE_NONE, callProfile.mServiceType);
        assertEquals(ImsCallProfile.CALL_TYPE_VOICE, callProfile.mCallType);

        assertEquals(ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                callProfile.getMediaProfile().getAudioQuality());
        assertEquals(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                callProfile.getMediaProfile().getAudioDirection());
    }

    @Test
    public void test_createCallProfileForVideoCall() {
        ImsCallProfile callProfile = mImsCallApp.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NONE, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE);
        assertEquals(ImsCallProfile.SERVICE_TYPE_NONE, callProfile.mServiceType);
        assertEquals(ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, callProfile.mCallType);
        assertEquals(ImsStreamMediaProfile.VIDEO_QUALITY_QVGA_PORTRAIT,
                callProfile.getMediaProfile().getVideoQuality());
        assertEquals(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                callProfile.getMediaProfile().getVideoDirection());
        assertEquals(ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                callProfile.getMediaProfile().getAudioQuality());
        assertEquals(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                callProfile.getMediaProfile().getAudioDirection());

        IDcNetWatcher dcnw = Mockito.mock(IDcNetWatcher.class);
        when(mMockImsCallContext.getDcNetWatcher()).thenReturn(dcnw);
        when(dcnw.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mRegTracker.isCallVoiceAndVideoRegistered()).thenReturn(true);
        when(mMockImsCallContext.hasAccessBearerCapabilitiesForHDCall()).thenReturn(true);

        callProfile = mImsCallApp.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_NORMAL, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE);
        assertEquals(ImsCallProfile.SERVICE_TYPE_NORMAL, callProfile.mServiceType);
        assertEquals(ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, callProfile.mCallType);

        callProfile = mImsCallApp.createCallProfile(
                ImsCallProfile.SERVICE_TYPE_EMERGENCY, ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE);
        assertEquals(ImsCallProfile.SERVICE_TYPE_EMERGENCY, callProfile.mServiceType);
        assertEquals(ImsCallProfile.CALL_TYPE_VIDEO_N_VOICE, callProfile.mCallType);
        assertEquals(ImsStreamMediaProfile.VIDEO_QUALITY_NONE,
                callProfile.getMediaProfile().getVideoQuality());
        assertEquals(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                callProfile.getMediaProfile().getVideoDirection());
        assertEquals(ImsStreamMediaProfile.AUDIO_QUALITY_NONE,
                callProfile.getMediaProfile().getAudioQuality());
        assertEquals(ImsStreamMediaProfile.DIRECTION_SEND_RECEIVE,
                callProfile.getMediaProfile().getAudioDirection());
    }

    @Test
    public void test_onBinderDied() {
        mImsCallApp.onBinderDied();
        verify(mMockImsCallManager, timeout(50)).closeAllSessions();
        assertFalse(mRegTracker.isCallRegistered());

        when(mRegTracker.isCallRegistered()).thenReturn(true);
        mImsCallApp.onBinderDied();
        assertTrue(mRegTracker.isCallRegistered());
    }

    @Test
    public void test_createCallSession() {
        ImsCallSessionImpl mockSession = Mockito.mock(ImsCallSessionImpl.class);
        ImsCallProfile imsCallProfile = new ImsCallProfile();
        when(mMockImsCallManager.createSession(imsCallProfile)).thenReturn(mockSession);
        ImsCallSessionImpl session = mImsCallApp.createCallSession(imsCallProfile);
        assertEquals(mockSession, session);
        verify(mMockImsCallManager).createSession(imsCallProfile);
    }

    @Test
    public void test_takeCallSession() {
        ImsCallSessionImpl callSession = Mockito.mock(ImsCallSessionImpl.class);
        assertFalse(mImsCallApp.takeCallSession(callSession));

        when(mMockImsCallManager.takeSession(callSession)).thenReturn(true);
        assertTrue(mImsCallApp.takeCallSession(callSession));
    }

    @Test
    public void test_getCallManager() {
        assertEquals(mMockImsCallManager, mImsCallApp.getCallManager());
    }

    @Test
    public void test_getEcbmInterface() {
        mImsEcbmImpl = mImsCallApp.getEcbmInterface();
        assertNotNull(mImsEcbmImpl);
        assertEquals(mImsEcbmImpl, mImsCallApp.getEcbmInterface());
    }

    @Test
    public void test_getMultiEndpointInterface() {
        mMultiEndpoint = mImsCallApp.getMultiEndpointInterface();
        assertNotNull(mMultiEndpoint);
        assertEquals(mMultiEndpoint, mImsCallApp.getMultiEndpointInterface());
    }

    @Test
    public void test_getUtInterface() {
        mImsUtImpl = mImsCallApp.getUtInterface();
        assertNotNull(mImsUtImpl);
        assertEquals(mImsUtImpl, mImsCallApp.getUtInterface());
    }

    @Test
    public void test_setTtyMode() {
        TtyModeTracker ttyModeTracker = null;
        mImsCallApp.setTtyMode(TelecomManager.TTY_MODE_OFF);
        assertNull(ttyModeTracker);

        ttyModeTracker = Mockito.mock(TtyModeTracker.class);
        when(mMockImsCallContext.getTtyModeTracker()).thenReturn(ttyModeTracker);
        mImsCallApp.setTtyMode(TelecomManager.TTY_MODE_FULL);
        verify(ttyModeTracker).setTtyMode(anyInt());
    }
}

