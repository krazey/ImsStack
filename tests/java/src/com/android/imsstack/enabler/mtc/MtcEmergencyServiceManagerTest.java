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
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.anyBoolean;
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.os.Looper;
import android.os.Parcel;
import android.telephony.CarrierConfigManager;
import android.telephony.emergency.EmergencyNumber;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.IBaseContext;

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
public class MtcEmergencyServiceManagerTest extends ImsStackTest {
    private int mCommand;
    private int mEmergencyRouting;
    private final int mInvalid = -1;
    private long mNativeObject;
    private static final int SLOT_ID = 0;

    @Mock private IBaseContext mMockContext;
    @Mock private MtcJniProxy mMockMtcJniProxy;
    @Mock private MtcCall mMockMtcCall;
    @Mock private IServiceStateTracker mServiceStateTracker;
    @Mock private ICallStateTracker mICallStateTracker;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigInterface mMockConfigInterface;
    @Captor ArgumentCaptor<MtcEmergencyServiceManager.ECallStateListener> mECallStateListenerCaptor;

    private MtcEmergencyServiceManager mTestMtcEmergencyServiceManager;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        doReturn(mServiceStateTracker).when(mMockContext).getServiceStateTracker();

        mCommand = mInvalid;
        mEmergencyRouting = mInvalid;
        mNativeObject = mInvalid;
        mTestMtcEmergencyServiceManager = new MtcEmergencyServiceManager(
                mMockContext, mICallStateTracker, mMockMtcJniProxy);

        doReturn(Looper.myLooper()).when(mMockContext).getCallLooper();
        doAnswer(invocation -> {
                    mNativeObject = (long) invocation.getArgument(0);

                    Parcel parcel = (Parcel) invocation.getArgument(1);
                    if (parcel == null) {
                        return null;
                    }

                    parcel.setDataPosition(0);
                    mCommand = parcel.readInt();
                    if (mCommand == IUMtcService.OPEN_EMERGENCY_SERVICE) {
                        mEmergencyRouting = parcel.readInt();
                    }

                    parcel.recycle();
                    parcel = null;
                    return null;
                }
                ).when(mMockMtcJniProxy).sendDataToNative(anyLong(), any(Parcel.class));

        mTestMtcEmergencyServiceManager.init();
        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        mTestMtcEmergencyServiceManager = null;
        super.tearDown();
    }

    @Test
    public void testOpenEmergencyService() {
        mTestMtcEmergencyServiceManager.setNativeObject(1);
        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        processAllMessages();

        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
    }

    @Test
    public void testOpenEmergencyServiceOverWiFi() {
        when(mMockMtcCall.getCallExtraBoolean(Call.EXTRA_WIFI_E_CALL, false))
                .thenReturn(true);
        mTestMtcEmergencyServiceManager.setNativeObject(1);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsWfc.KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL))
                .thenReturn(true);

        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        processAllMessages();

        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
        assertEquals(IUMtcCall.SERVICETYPE_EMERGENCY, mEmergencyRouting);

        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsWfc.KEY_EMERGENCY_CALL_OVER_EMERGENCY_PDN_BOOL))
                .thenReturn(false);

        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        processAllMessages();

        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
        assertEquals(IUMtcCall.SERVICETYPE_NORMAL, mEmergencyRouting);
    }

    @Test
    public void testStopEmergencyService() {
        mTestMtcEmergencyServiceManager.setNativeObject(1);
        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        verify(mICallStateTracker).addListener(mECallStateListenerCaptor.capture());

        MtcEmergencyServiceManager.ECallStateListener eCallStateListener =
                mECallStateListenerCaptor.getValue();

        eCallStateListener.onCallDestroyed(mMockMtcCall);
        processAllMessages();

        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.STOP_EMERGENCY_SERVICE, mCommand);
    }

    @Test
    public void testOnEmergencyServiceStateChanged() {
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(IUMtcService.ES_IDLE, 0, 0);
        verifyNoMoreInteractions(mMockMtcCall);

        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_OPENING, 0, 0);
        verifyNoMoreInteractions(mMockMtcCall);

        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_UNAVAILABLE, 0, 0);
        verifyNoMoreInteractions(mMockMtcCall);

        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_OPENED, 0, 0);
        verify(mMockMtcCall, times(1)).createNativeCallObject();
        verify(mMockMtcCall, times(1)).open(anyInt(), anyInt(), anyBoolean(), anyBoolean());
    }

    @Test
    public void testOnCallDestroyedWithWrongMtcCall() {
        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.setNativeObject(1);
        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        verify(mICallStateTracker).addListener(mECallStateListenerCaptor.capture());
        MtcEmergencyServiceManager.ECallStateListener eCallStateListener =
                mECallStateListenerCaptor.getValue();
        eCallStateListener.onCallDestroyed(null);
        verify(mServiceStateTracker, times(0)).handleEmergencyCallDestroyed();
        verify(mICallStateTracker, times(0)).removeListener(any());
    }

    @Test
    public void testOnCallDestroyedInUnavailableState() {
        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.setNativeObject(1);
        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        verify(mICallStateTracker).addListener(mECallStateListenerCaptor.capture());
        MtcEmergencyServiceManager.ECallStateListener eCallStateListener =
                mECallStateListenerCaptor.getValue();
        processAllMessages();
        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
        mTestMtcEmergencyServiceManager.onEmergencyServiceStateChanged(
                IUMtcService.ES_UNAVAILABLE, 0, 0);
        eCallStateListener.onCallDestroyed(mMockMtcCall);
        verify(mServiceStateTracker, times(1)).handleEmergencyCallDestroyed();
        verify(mICallStateTracker, times(1)).removeListener(eCallStateListener);
        processAllMessages();
        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
    }

    @Test
    public void testOnCallDestroyedWithoutStateChange() {
        mTestMtcEmergencyServiceManager.setCall(mMockMtcCall);
        mTestMtcEmergencyServiceManager.setNativeObject(1);
        mTestMtcEmergencyServiceManager.openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
        verify(mICallStateTracker).addListener(mECallStateListenerCaptor.capture());
        MtcEmergencyServiceManager.ECallStateListener eCallStateListener =
                mECallStateListenerCaptor.getValue();
        processAllMessages();
        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.OPEN_EMERGENCY_SERVICE, mCommand);
        eCallStateListener.onCallDestroyed(mMockMtcCall);
        verify(mServiceStateTracker, times(1)).handleEmergencyCallDestroyed();
        verify(mICallStateTracker, times(1)).removeListener(eCallStateListener);
        processAllMessages();
        assertEquals(1, mNativeObject);
        assertEquals(IUMtcService.STOP_EMERGENCY_SERVICE, mCommand);
    }
}
