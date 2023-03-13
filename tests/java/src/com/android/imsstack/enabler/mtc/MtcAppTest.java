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
import static org.mockito.Mockito.anyInt;
import static org.mockito.Mockito.anyLong;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.os.Handler;
import android.os.Looper;
import android.os.Parcel;
import android.telephony.emergency.EmergencyNumber;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.ICommonPackageListener;
import com.android.imsstack.core.agents.ISharedState;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.externalcalls.ExternalCalls;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.jni.JniImsListener;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Captor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.Executor;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcAppTest extends ImsStackTest {
    @Mock private IBaseContext mBaseContext;
    @Mock private IMtcCallManager mCM;
    @Mock private MtcEmergencyServiceManager mEmergencyServiceManager;
    @Mock private MtcApp.ServiceStateListener mServiceStateListener;
    @Mock private MtcApp.CallListener mCallListener;
    @Mock private ISharedState mSharedState;
    @Mock private IServiceStateTracker mServiceStateTracker;
    @Mock private MtcCall mMtcCall;
    @Mock private Executor mExecutor;
    @Captor ArgumentCaptor<ICommonPackageListener> mCommonPackageListener;
    int mCommand;

    private TestMtcJniProxy mTestMtcJniProxy;
    private TestMtcApp mTestMtcApp;

    private class TestMtcJniProxy extends MtcJniProxy {
        @Override
        public long getJniInterfaceAndSetListener(
                int nSlot, int category, JniImsListener listener) {
            return (long) 1;
        }

        @Override
        public void releaseJniInterfaceAndrRemoveListener(long nativeObj, JniImsListener listener) {
        }

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

    private class TestMtcApp extends MtcApp {
        TestMtcApp(IBaseContext context, IMtcCallManager mtcCallManager, Looper looper,
                MtcEmergencyServiceManager mtcEmergencyServiceManager, MtcJniProxy mtcJniProxy) {
            super(context, mtcCallManager, looper, mtcEmergencyServiceManager, mtcJniProxy);
        }

        @Override
        public void initializeState() {
        }

        @Override
        public MtcCall createMtcCall(int callAttributes) {
            return mMtcCall;
        }

        public MmTelFeatureRegistry.Listener getMmtelFeatureListener() {
            mMmtelFeatureListener = new MmtelFeatureListener();
            return mMmtelFeatureListener;
        }
    }

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        mCommand = -1;
        MockitoAnnotations.initMocks(this);
        AppContext.init(mContext);

        doReturn(mServiceStateTracker).when(mBaseContext).getServiceStateTracker();

        mTestMtcJniProxy = new TestMtcJniProxy();
        mTestMtcApp = new TestMtcApp(mBaseContext, mCM, Looper.myLooper(),
                mEmergencyServiceManager, mTestMtcJniProxy);
        mTestMtcApp.setCallListener(mCallListener);
        mTestMtcApp.setServiceStateListener(mServiceStateListener);
    }

    @After
    public void tearDown() throws Exception {
        mTestMtcApp.clear();
        mTestMtcApp = null;
        mTestMtcJniProxy = null;
        AppContext.deinit();
        super.tearDown();
    }

    @Test
    public void testInit() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(true).when(mBaseContext).isCommonPackageReady();
        doReturn(mSharedState).when(mBaseContext).getSharedState();
        doReturn(true).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mBaseContext, times(1)).addCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
        assertTrue(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testInitFailCommonPackageNotReady() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(false).when(mBaseContext).isCommonPackageReady();
        doReturn(mSharedState).when(mBaseContext).getSharedState();
        doReturn(true).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();

        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testInitFailNotNativeBootCompleted() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(true).when(mBaseContext).isCommonPackageReady();
        doReturn(mSharedState).when(mBaseContext).getSharedState();
        doReturn(false).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mBaseContext, times(1)).addCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());
        verify(mSharedState, times(1)).unregisterForNativeBootComplete(any());
        verify(mSharedState, times(1)).registerForNativeBootComplete(any(), anyInt(), any());

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testClearWithNativeObj() {
        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.clear();

        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());

        verify(mBaseContext, times(1)).removeCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).clear();
        verify(mBaseContext, times(1)).removeCommonPackageListener(any());
        verify(mCM, times(1)).clear();

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testClearWithoutNativeObj() {
        mTestMtcApp.setNativeObj(1);
        mTestMtcApp.getMmtelFeatureListener();

        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.clear();

        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        verify(mBaseContext, times(1)).removeCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).clear();
        verify(mBaseContext, times(1)).removeCommonPackageListener(any());
        verify(mCM, times(1)).clear();

        assertEquals(null, mTestMtcApp.mMmtelFeatureListener);
        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testOpenEmergencyService() {
        mTestMtcApp.openEmergencyService(
                mMtcCall, EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY);

        verify(mEmergencyServiceManager, times(1)).setCall(any(MtcCall.class));
        verify(mEmergencyServiceManager, times(1)).openEmergencyService(
                EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY, mServiceStateTracker);
    }

    @Test
    public void testCreateCall() {
        mTestMtcApp.setNativeObj(1);

        assertTrue(mTestMtcApp.isServiceValid());

        doReturn(true).when(mMtcCall).isMO();
        mTestMtcApp.createCall(0);

        verify(mCM, times(1)).attachCall(any(MtcCall.class));

        doReturn(false).when(mMtcCall).isMO();
        mTestMtcApp.createCall(0);

        verify(mCM, times(1)).attachPreIncomingCall(any(MtcCall.class));

        int sessionAttributes = MtcCall.FLAG_EMERGENCY;
        mTestMtcApp.createCall(sessionAttributes);
    }

    @Test
    public void testDestroyCall() {
        mTestMtcApp.destroyCall(mMtcCall);
        verify(mMtcCall, times(1)).close();
    }

    @Test
    public void testGetPendingCall() {
        doReturn(mMtcCall).when(mCM).getPendingCall(1);
        doReturn(false).when(mMtcCall).isTerminated();

        MtcCall mtcCall = mTestMtcApp.getPendingCall(1);

        verify(mMtcCall, times(1)).isTerminated();
        assertEquals(mMtcCall, mtcCall);

        doReturn(true).when(mMtcCall).isTerminated();

        mtcCall = mTestMtcApp.getPendingCall(1);

        verify(mMtcCall, times(1)).close();
        assertEquals(null, mtcCall);
    }

    @Test
    public void testClose() {
        mTestMtcApp.close();
        verify(mCM, times(1)).dispose();
    }

    @Test
    public void testNotifySrvccStateChanged() {
        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        MmTelFeatureRegistry.Listener mmtelFeatureListener = mTestMtcApp.getMmtelFeatureListener();
        mmtelFeatureListener.onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        processAllMessages();

        assertEquals(IUMtcService.SRVCC_STATE_CHANGED, mCommand);
    }

    @Test
    public void testSetTerminalBasedCallWaiting() {
        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        MmTelFeatureRegistry.Listener mmtelFeatureListener = mTestMtcApp.getMmtelFeatureListener();
        mmtelFeatureListener.onTerminalBasedCallWaitingStatusChanged();
        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
    }

    @Test
    public void testMsgNativeBootCompletedMsgCommonPackageReady() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(true).when(mBaseContext).isCommonPackageReady();
        doReturn(mSharedState).when(mBaseContext).getSharedState();
        doReturn(true).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        ((Handler) mTestMtcApp.getHandler()).sendEmptyMessage(1);
        processAllMessages();

        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.setNativeObj(0);
        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();
        verify(mBaseContext, times(1)).addCommonPackageListener(mCommonPackageListener.capture());
        mCommonPackageListener.getValue().onCommonPackageReady(0);
        mCommonPackageListener.getValue().onCommonPackageStop(0);
        processAllMessages();

        assertTrue(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testOnMessageForCallApp() {
        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.PRE_INCOMING_CALL);
        parcel.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel.recycle();

        verify(mCallListener, times(1)).onPreIncomingCallReceived(any(MtcApp.class), anyLong());
        verify(mMtcCall, times(1)).attach(anyLong());

        doReturn(mExecutor).when(mBaseContext).getExecutor();
        mTestMtcApp.setCallListener(null);
        parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.PRE_INCOMING_CALL);
        parcel.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel.recycle();

        verify(mExecutor, times(1)).execute(any());

        mTestMtcApp.setCallListener(mCallListener);
        Parcel parcel2 = Parcel.obtain();
        parcel2.writeInt(IUMtcService.EXTERNAL_CALLS_CHANGED);
        parcel2.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel2);
        parcel2.recycle();

        verify(mCallListener, times(1)).onExternalCallStateChanged(
                any(MtcApp.class), any(ExternalCalls.class));
    }

    @Test
    public void testOnMessageForRegApp() {
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.SERVICE_CHANGED);
        parcel.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel.recycle();

        verify(mServiceStateListener, times(1)).onServiceStateChanged(any(MtcApp.class),
                anyInt(), anyInt());

        Parcel parcel2 = Parcel.obtain();
        parcel2.writeInt(IUMtcService.E_SERVICE_CHANGED);
        parcel2.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel2);
        parcel2.recycle();

        verify(mEmergencyServiceManager, times(1)).onEmergencyServiceStateChanged(anyInt(),
                anyInt(), anyInt());
        verify(mServiceStateListener, times(1)).onEmergencyServiceStateChanged(any(MtcApp.class),
                anyInt(), anyInt());

        mTestMtcApp.setNativeObj(1);
        Parcel parcel3 = Parcel.obtain();
        parcel3.writeInt(IUMtcService.JNI_READY);
        parcel3.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel3);
        parcel3.recycle();

        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
    }
}
