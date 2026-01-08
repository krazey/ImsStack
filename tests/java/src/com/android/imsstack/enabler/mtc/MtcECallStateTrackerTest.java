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

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.os.Looper;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.reg.MtcServiceState;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcECallStateTrackerTest extends ImsStackTest {
    private static final int SLOT_ID = 0;

    @Mock private IBaseContext mMockContext;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Mock private ICallStateTracker mMockICallStateTracker;
    @Mock private IServiceStateTracker mMockIServiceStateTracker;
    @Mock private EcbmListener mMockEcbmListener;
    @Mock private ISystem mMockISystem;
    @Mock private MtcCall mMockMtcCall;
    @Captor private ArgumentCaptor<IServiceStateTracker.Listener> mListenerCaptor;
    @Captor private ArgumentCaptor<MtcECallStateTracker.MtcECallStateListener>
            mMtcECallStateListenerCaptor;
    private MtcECallStateTracker mTestMtcECallStateTracker;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
        doReturn(mMockCarrierConfig).when(mMockConfigInterface).getCarrierConfig();
        doReturn(true).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOLTE_BOOL);
        doReturn(true).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOWIFI_BOOL);

        doReturn(SLOT_ID).when(mMockContext).getSlotId();
        doReturn(mMockIServiceStateTracker).when(mMockContext).getServiceStateTracker();
        doReturn(mContext).when(mMockContext).getContext();
        doReturn(Looper.myLooper()).when(mContext).getMainLooper();
        doReturn(mMockISystem).when(mMockContext).getSystem();
        doReturn(0).when(mMockContext).getSlotId();
        doReturn(true).when(mMockMtcCall).isEmergencyCall();

        mTestMtcECallStateTracker = new MtcECallStateTracker(
                mMockContext, mMockICallStateTracker);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        mTestMtcECallStateTracker = null;
        super.tearDown();
    }

    @Test
    public void testDispose() {
        mTestMtcECallStateTracker.setECallListener();
        mTestMtcECallStateTracker.dispose();

        verify(mMockIServiceStateTracker, times(1)).removeListener(any());
        verify(mMockICallStateTracker, times(1)).removeListener(any());
    }

    @Test
    public void testExitEmergencyCallbackMode() {
        mTestMtcECallStateTracker.setEcbmEntered(false);
        mTestMtcECallStateTracker.addEcbmListener(mMockEcbmListener);
        mTestMtcECallStateTracker.exitEmergencyCallbackMode(true);
        processAllMessages();

        verify(mMockEcbmListener, times(1)).onEcbmExited();
        verify(mMockISystem, times(1)).notifyEvent(
                ImsEventDef.IMS_EVENT_ECM_STATE, ImsEventDef.IMS_ECM_STATE_OFF_BY_NEW_ECALL, 0);
        assertFalse(mTestMtcECallStateTracker.isECallStarted());

        mTestMtcECallStateTracker.exitEmergencyCallbackMode(false);
        processAllMessages();

        verify(mMockEcbmListener, times(2)).onEcbmExited();
        verify(mMockISystem, times(1)).notifyEvent(
                ImsEventDef.IMS_EVENT_ECM_STATE, ImsEventDef.IMS_ECM_STATE_OFF, 0);

        mTestMtcECallStateTracker.setEcbmEntered(true);
        mTestMtcECallStateTracker.exitEmergencyCallbackMode(true);
        processAllMessages();

        verify(mMockEcbmListener, times(3)).onEcbmExited();
        verify(mMockISystem, times(2)).notifyEvent(
                ImsEventDef.IMS_EVENT_ECM_STATE, ImsEventDef.IMS_ECM_STATE_OFF_BY_NEW_ECALL, 0);
        assertFalse(mTestMtcECallStateTracker.isECallStarted());

        mTestMtcECallStateTracker.setEcbmEntered(true);
        mTestMtcECallStateTracker.exitEmergencyCallbackMode(false);
        processAllMessages();

        verify(mMockEcbmListener, times(4)).onEcbmExited();
        verify(mMockISystem, times(2)).notifyEvent(
                ImsEventDef.IMS_EVENT_ECM_STATE, ImsEventDef.IMS_ECM_STATE_OFF, 0);
        assertFalse(mTestMtcECallStateTracker.isECallStarted());

        mTestMtcECallStateTracker.removeEcbmListener(mMockEcbmListener);
        mTestMtcECallStateTracker.exitEmergencyCallbackMode(true);
        processAllMessages();

        verifyNoMoreInteractions(mMockEcbmListener);
    }

    @Test
    public void testECallStateHandlerEventEmergencyServiceStateChanged() {
        mTestMtcECallStateTracker.setEcbmEntered(false);
        assertFalse(mTestMtcECallStateTracker.isECallStarted());
        mTestMtcECallStateTracker.setState(IUMtcService.ES_IDLE);
        mTestMtcECallStateTracker.addEcbmListener(mMockEcbmListener);
        IServiceStateTracker.Listener listener = getServiceStateListener();
        listener.onEmergencyServiceStateChanged(null);
        processAllMessages();

        assertTrue(mTestMtcECallStateTracker.getState() == IUMtcService.ES_IDLE);
        assertFalse(mTestMtcECallStateTracker.isECallStarted());

        listener.onEmergencyServiceStateChanged(new MtcServiceState(
                IUMtcService.SERVICE_EMERGENCY, IUMtcService.ES_OPENED,
                IUMtcService.ES_IDLE_REASON_NONE));
        processAllMessages();

        assertTrue(mTestMtcECallStateTracker.isECallStarted());

        mTestMtcECallStateTracker.setEcbmEntered(false);
        listener.onEmergencyServiceStateChanged(new MtcServiceState(
                IUMtcService.SERVICE_EMERGENCY, IUMtcService.ES_IDLE,
                IUMtcService.ES_IDLE_REASON_WITH_ECM));
        processAllMessages();

        assertTrue(mTestMtcECallStateTracker.isECallStarted());
        verify(mMockEcbmListener, times(1)).onEcbmEntered();
        verify(mMockISystem, times(1)).notifyEvent(
                ImsEventDef.IMS_EVENT_ECM_STATE, ImsEventDef.IMS_ECM_STATE_ON, 0);

        mTestMtcECallStateTracker.setState(IUMtcService.ES_OPENED);
        listener.onEmergencyServiceStateChanged(new MtcServiceState(
                IUMtcService.SERVICE_EMERGENCY, IUMtcService.ES_IDLE,
                ImsEventDef.IMS_ECM_STATE_OFF));
        processAllMessages();

        assertFalse(mTestMtcECallStateTracker.isECallStarted());
    }

    @Test
    public void testMtcECallStateListenerOnCallCreated() {
        mTestMtcECallStateTracker.setECallListener();
        MtcECallStateTracker.MtcECallStateListener mtcECallStateListener = getECallListener();
        mtcECallStateListener.onCallCreated(mMockMtcCall);

        assertTrue(mTestMtcECallStateTracker.isECallState(1));
    }

    @Test
    public void testMtcECallStateListenerOnCallDestroyed() {
        mTestMtcECallStateTracker.setECallState(1);
        mTestMtcECallStateTracker.setProceedingExitEmergency(true);
        mTestMtcECallStateTracker.setECallListener();
        MtcECallStateTracker.MtcECallStateListener mtcECallStateListener = getECallListener();
        mtcECallStateListener.onCallDestroyed(mMockMtcCall);

        assertTrue(mTestMtcECallStateTracker.isECallState(0));
        assertTrue(mTestMtcECallStateTracker.isProceedingExitEmergency());

        mTestMtcECallStateTracker.setECallState(1);
        mTestMtcECallStateTracker.setProceedingExitEmergency(false);
        mtcECallStateListener.onCallDestroyed(mMockMtcCall);

        assertTrue(mTestMtcECallStateTracker.isProceedingExitEmergency());

        processAllFutureMessages();

        assertFalse(mTestMtcECallStateTracker.isProceedingExitEmergency());
        assertTrue(mTestMtcECallStateTracker.isECallState(0));
    }

    @Test
    public void testMtcECallStateListenerOnCallEstablishing() {
        mTestMtcECallStateTracker.setECallListener();
        MtcECallStateTracker.MtcECallStateListener mtcECallStateListener = getECallListener();
        mtcECallStateListener.onCallEstablishing(mMockMtcCall);

        assertTrue(mTestMtcECallStateTracker.isECallState(2));
    }

    @Test
    public void testMtcECallStateListenerOnCallEstablished() {
        mTestMtcECallStateTracker.setECallListener();
        MtcECallStateTracker.MtcECallStateListener mtcECallStateListener = getECallListener();
        mtcECallStateListener.onCallEstablished(mMockMtcCall);

        assertTrue(mTestMtcECallStateTracker.isECallState(3));
    }

    @Test
    public void testMtcECallStateListenerOnCallTerminated() {
        mTestMtcECallStateTracker.setECallListener();
        MtcECallStateTracker.MtcECallStateListener mtcECallStateListener = getECallListener();
        mTestMtcECallStateTracker.setECallState(2);
        mTestMtcECallStateTracker.setProceedingExitEmergency(false);
        doReturn(new CallReasonInfo()).when(mMockMtcCall).getTerminationReason();
        mtcECallStateListener.onCallTerminated(mMockMtcCall);
        processAllMessages();

        assertTrue(mTestMtcECallStateTracker.isProceedingExitEmergency());
        assertTrue(mTestMtcECallStateTracker.isECallState(0));

        processAllFutureMessages();

        assertFalse(mTestMtcECallStateTracker.isProceedingExitEmergency());

        mTestMtcECallStateTracker.setECallState(3);
        mTestMtcECallStateTracker.setProceedingExitEmergency(true);
        mtcECallStateListener.onCallTerminated(mMockMtcCall);
        processAllFutureMessages();

        assertTrue(mTestMtcECallStateTracker.isProceedingExitEmergency());
        assertTrue(mTestMtcECallStateTracker.isECallState(0));
    }

    @Test
    public void testMtcECallStateListenerOnCallTerminated2() {
        doReturn(false).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOLTE_BOOL);
        doReturn(false).when(mMockCarrierConfig).getBoolean(
                CarrierConfig.ImsEmergency.KEY_SUPPORT_ECBM_FOR_VOWIFI_BOOL);
        MtcECallStateTracker testMtcECallStateTracker = new MtcECallStateTracker(
                mMockContext, mMockICallStateTracker);
        testMtcECallStateTracker.setECallListener();
        MtcECallStateTracker.MtcECallStateListener mtcECallStateListener2 = getECallListener();
        testMtcECallStateTracker.setECallState(3);
        testMtcECallStateTracker.setProceedingExitEmergency(false);
        doReturn(new CallReasonInfo()).when(mMockMtcCall).getTerminationReason();

        mtcECallStateListener2.onCallTerminated(mMockMtcCall);
        processAllMessages();

        assertTrue(testMtcECallStateTracker.isProceedingExitEmergency());
        assertTrue(testMtcECallStateTracker.isECallState(0));

        processAllFutureMessages();

        assertFalse(testMtcECallStateTracker.isProceedingExitEmergency());
    }

    private IServiceStateTracker.Listener getServiceStateListener() {
        verify(mMockIServiceStateTracker).addListener(mListenerCaptor.capture());
        return mListenerCaptor.getValue();
    }

    private MtcECallStateTracker.MtcECallStateListener getECallListener() {
        verify(mMockICallStateTracker).addListener(mMtcECallStateListenerCaptor.capture());
        return mMtcECallStateListenerCaptor.getValue();
    }
}
