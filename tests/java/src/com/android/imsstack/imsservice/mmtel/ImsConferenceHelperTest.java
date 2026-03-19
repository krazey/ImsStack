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
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.telephony.ims.stub.ImsCallSessionImplBase;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.CallInfo;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.imsservice.mmtel.internal.ConferenceProxy;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class ImsConferenceHelperTest {
    private static final String TAG = ImsConferenceHelper.class.getName();

    @Mock protected ICallContext mICallContext;
    @Mock private ImsCallApp mImsCallApp;
    @Mock private ImsCallManager mImsCallManager;
    @Mock private ImsCallSessionImpl mFgImsCallSession, mBgImsCallSession;
    @Mock private MtcCall mMtcCall, mFgMtcCall, mBgMtcCall;
    @Mock private MtcApp mMtcApp;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private CarrierConfig mMockCarrierConfig;

    private ImsConferenceHelper mConfHelper;
    private String[] mParticipants = {"1234", "5678"};

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mConfHelper = ImsConferenceHelper.getInstance();

        // Setup ConfigInterface for CallFeature checks in ConferenceProxy
        when(mICallContext.getSlotId()).thenReturn(0);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, 0);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);

        CallInfo callInfo = new CallInfo();
        MediaInfo mediaInfo = new MediaInfo();
        when(mFgMtcCall.getCallInfo()).thenReturn(callInfo);
        when(mFgMtcCall.getMediaInfo()).thenReturn(mediaInfo);
        when(mBgMtcCall.getCallInfo()).thenReturn(callInfo);
        when(mBgMtcCall.getMediaInfo()).thenReturn(mediaInfo);
        when(mMtcCall.getCallInfo()).thenReturn(callInfo);
        when(mMtcCall.getMediaInfo()).thenReturn(mediaInfo);
    }

    @Test
    public void test_getInstance() {
        assertNotNull(mConfHelper);
    }

    @Test
    public void test_onConferenceProxyDisposed() {
        ConferenceProxy mConfProxy = Mockito.mock(ConferenceProxy.class);
        mConfHelper.onConferenceProxyDisposed(mConfProxy);
        verify(mConfProxy).dispose();
    }

    @Test
    public void test_setBackgroundSession() {
        ImsCallSessionImplBase mSession = Mockito.mock(ImsCallSessionImplBase.class);
        mConfHelper.setBackgroundSession(mSession);
        assertEquals(mConfHelper.getBackgroundSession(), mSession);
    }

    @Test
    public void test_setTransientConferenceSession() {
        ImsCallSessionImplBase mConfSession = Mockito.mock(ImsCallSessionImplBase.class);
        mConfHelper.setTransientConferenceSession(mConfSession);
        assertEquals(mConfHelper.getTransientConferenceSession(), mConfSession);
    }

    @Test
    public void test_extendToConference() {
        when(mICallContext.getApp()).thenReturn(mImsCallApp);
        when(mImsCallApp.getCallManager()).thenReturn(mImsCallManager);
        when(mImsCallManager.getActiveSession()).thenReturn(mFgImsCallSession);
        when(mFgImsCallSession.getMtcCall()).thenReturn(mFgMtcCall);
        when(mImsCallManager.getMtcApp()).thenReturn(mMtcApp);
        when(mMtcApp.createMtcCallAndAttach(18)).thenReturn(mMtcCall);

        assertEquals(true, mConfHelper.extendToConference(mICallContext, mParticipants));
        verify(mFgImsCallSession).setConferenceProxy(any(ConferenceProxy.class));

        mMtcCall = null;
        when(mMtcApp.createMtcCallAndAttach(18)).thenReturn(mMtcCall);
        assertEquals(false, mConfHelper.extendToConference(mICallContext, mParticipants));

        mFgMtcCall = null;
        when(mFgImsCallSession.getMtcCall()).thenReturn(mFgMtcCall);
        assertEquals(false, mConfHelper.extendToConference(mICallContext, mParticipants));

        mFgImsCallSession = null;
        when(mImsCallManager.getActiveSession()).thenReturn(mFgImsCallSession);
        assertEquals(false, mConfHelper.extendToConference(mICallContext, mParticipants));
    }

    @Test
    public void test_merge() {
        when(mICallContext.getApp()).thenReturn(mImsCallApp);
        when(mImsCallApp.getCallManager()).thenReturn(mImsCallManager);
        when(mImsCallManager.getActiveSession()).thenReturn(mFgImsCallSession);
        when(mImsCallManager.getHoldSession()).thenReturn(mBgImsCallSession);
        when(mFgImsCallSession.getMtcCall()).thenReturn(mFgMtcCall);
        when(mBgImsCallSession.getMtcCall()).thenReturn(mBgMtcCall);
        when(mImsCallManager.getMtcApp()).thenReturn(mMtcApp);
        when(mMtcApp.createMtcCallAndAttach(18)).thenReturn(mMtcCall);

        assertEquals(true, mConfHelper.merge(mICallContext));
        verify(mBgImsCallSession).setConferenceProxy(any(ConferenceProxy.class));
        verify(mFgImsCallSession).setConferenceProxy(any(ConferenceProxy.class));

        mBgMtcCall = null;
        when(mBgImsCallSession.getMtcCall()).thenReturn(mBgMtcCall);
        assertEquals(false, mConfHelper.merge(mICallContext));

        mMtcCall = null;
        when(mMtcApp.createMtcCallAndAttach(18)).thenReturn(mMtcCall);
        assertEquals(false, mConfHelper.merge(mICallContext));

        mFgMtcCall = null;
        when(mFgImsCallSession.getMtcCall()).thenReturn(mFgMtcCall);
        assertEquals(false, mConfHelper.merge(mICallContext));

        mBgImsCallSession = null;
        when(mImsCallManager.getHoldSession()).thenReturn(mBgImsCallSession);
        assertEquals(false, mConfHelper.merge(mICallContext));

        mFgImsCallSession = null;
        when(mImsCallManager.getActiveSession()).thenReturn(mFgImsCallSession);
        assertEquals(false, mConfHelper.merge(mICallContext));
    }

    @After
    public void tearDown() {
        mConfHelper = null;
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, 0);
    }
}
