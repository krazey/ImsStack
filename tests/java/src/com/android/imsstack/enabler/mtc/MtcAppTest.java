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
import static org.junit.Assert.assertNotEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
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
import com.android.imsstack.core.agents.NativeStateInterface;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.externalcalls.ExternalCalls;
import com.android.imsstack.internal.ImsStackRegistry;
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
    @Mock private NativeStateInterface mNativeStateInterface;
    @Mock private IServiceStateTracker mServiceStateTracker;
    @Mock private MtcCall mMtcCall;
    @Mock private Executor mExecutor;
    @Captor ArgumentCaptor<ImsStackRegistry.ImsServiceListener> mImsServiceListenerCaptor;
    @Captor ArgumentCaptor<NativeStateInterface.Listener> mNativeStateListenerCaptor;
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
        mTestMtcApp = null;
        mTestMtcJniProxy = null;
        AppContext.deinit();
        super.tearDown();
    }

    @Test
    public void testInitWithNativeServiceIsReady() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mNativeStateInterface).isServiceReady();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mBaseContext, times(1)).addImsServiceListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
        assertTrue(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testInitWithNativeServiceIsNotReadyAndFinallyReady() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(false).when(mNativeStateInterface).isServiceReady();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mBaseContext, times(1)).addImsServiceListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mNativeStateInterface, times(1)).addListener(mNativeStateListenerCaptor.capture());

        mTestMtcApp.init();

        verify(mNativeStateInterface, times(1)).removeListener(
                any(NativeStateInterface.Listener.class));
        verify(mNativeStateInterface, times(2)).addListener(
                any(NativeStateInterface.Listener.class));


        assertFalse(mTestMtcApp.isServiceValid());

        doReturn(true).when(mNativeStateInterface).isServiceReady();
        mNativeStateListenerCaptor.getValue().onNativeServiceReady();

        verify(mNativeStateInterface, times(2)).removeListener(
                any(NativeStateInterface.Listener.class));
        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
        assertTrue(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testInitFailCommonPackageNotReady() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(false).when(mBaseContext).isImsServiceStarted();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mNativeStateInterface).isServiceReady();

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
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(false).when(mNativeStateInterface).isServiceReady();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mBaseContext, times(1)).addImsServiceListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());
        verify(mNativeStateInterface, times(1))
                .addListener(any(NativeStateInterface.Listener.class));

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testClearWithoutNativeObj() {
        assertFalse(mTestMtcApp.isServiceValid());

        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(false).when(mNativeStateInterface).isServiceReady();
        mTestMtcApp.init();

        mTestMtcApp.clear();

        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());
        verify(mEmergencyServiceManager, times(1)).clear();
        verify(mBaseContext, times(1)).removeImsServiceListener(any());
        verify(mCM, times(1)).clear();
        verify(mNativeStateInterface, times(1)).removeListener(
                any(NativeStateInterface.Listener.class));

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testClearWithNativeObj() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(true).when(mNativeStateInterface).isServiceReady();
        mTestMtcApp.init();

        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.clear();

        verify(mEmergencyServiceManager, times(1)).setNativeObject(0);
        verify(mBaseContext, times(1)).removeImsServiceListener(any());
        verify(mEmergencyServiceManager, times(1)).clear();
        verify(mCM, times(1)).clear();
        verify(mNativeStateInterface, times(0)).removeListener(
                any(NativeStateInterface.Listener.class));

        assertEquals(null, mTestMtcApp.mMmtelFeatureListener);
        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testGetCallManager() {
        assertEquals(mCM, mTestMtcApp.getCallManager());
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
    public void testcreateMtcCallAndAttachWithoutJniService() {
        assertFalse(mTestMtcApp.isServiceValid());

        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(false).when(mNativeStateInterface).isServiceReady();

        assertNull(mTestMtcApp.createMtcCallAndAttach(0));
    }

    @Test
    public void testcreateMtcCallAndAttachWithJniService() {
        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.setNativeObj(1);
        doReturn(true).when(mMtcCall).isMO();

        mTestMtcApp.createMtcCallAndAttach(0);

        assertTrue(mTestMtcApp.isServiceValid());
        verify(mCM, times(1)).attachCall(any(MtcCall.class));

        doReturn(false).when(mMtcCall).isMO();
        mTestMtcApp.createMtcCallAndAttach(0);

        verify(mCM, times(1)).attachPreIncomingCall(any(MtcCall.class));
    }

    @Test
    public void testDestroyCall() {
        mTestMtcApp.destroyCall(mMtcCall);
        verify(mMtcCall, times(1)).close();
    }

    @Test
    public void testCreateMtcCall() {
        assertNotNull(mTestMtcApp.createMtcCall(0));
    }

    @Test
    public void testGetPendingCall() {
        assertNull(mTestMtcApp.getPendingCall(1));

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
    public void testCloseWithNotBoundJniService() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(false).when(mNativeStateInterface).isServiceReady();
        mTestMtcApp.init();

        mTestMtcApp.close();

        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());
        verify(mCM, times(1)).dispose();
        verify(mNativeStateInterface, times(1)).removeListener(
                any(NativeStateInterface.Listener.class));
    }

    @Test
    public void testCloseWithBoundJniService() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(true).when(mNativeStateInterface).isServiceReady();
        mTestMtcApp.init();

        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        mTestMtcApp.close();

        verify(mEmergencyServiceManager, times(2)).setNativeObject(anyLong());
        verify(mCM, times(1)).dispose();
        verify(mNativeStateInterface, times(0)).removeListener(
                any(NativeStateInterface.Listener.class));
    }

    @Test
    public void testNotifySrvccStateChanged() {
        MmTelFeatureRegistry.Listener mmtelFeatureListener = mTestMtcApp.getMmtelFeatureListener();
        mmtelFeatureListener.onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        processAllMessages();

        assertNotEquals(IUMtcService.SRVCC_STATE_CHANGED, mCommand);

        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        mmtelFeatureListener.onSrvccStateChanged(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        processAllMessages();

        assertEquals(IUMtcService.SRVCC_STATE_CHANGED, mCommand);
    }

    @Test
    public void testSetTerminalBasedCallWaiting() {
        MmTelFeatureRegistry.Listener mmtelFeatureListener = mTestMtcApp.getMmtelFeatureListener();
        mmtelFeatureListener.onTerminalBasedCallWaitingStatusChanged();
        processAllMessages();

        assertNotEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);

        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        mmtelFeatureListener.onTerminalBasedCallWaitingStatusChanged();
        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
    }

    @Test
    public void testMsgNativeBootCompletedMsgCommonPackageReady() {
        doReturn(0).when(mBaseContext).getSlotId();
        doReturn(true).when(mBaseContext).isImsServiceStarted();
        doReturn(mNativeStateInterface).when(mBaseContext).getNativeStateInterface();
        doReturn(true).when(mNativeStateInterface).isServiceReady();

        assertFalse(mTestMtcApp.isServiceValid());

        ((Handler) mTestMtcApp.getHandler()).sendEmptyMessage(1);
        processAllMessages();

        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.setNativeObj(0);
        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();
        verify(mBaseContext, times(1)).addImsServiceListener(mImsServiceListenerCaptor.capture());
        mImsServiceListenerCaptor.getValue().onImsServiceStarted(0);
        mImsServiceListenerCaptor.getValue().onImsServiceStopped(0);
        processAllMessages();

        assertTrue(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testOnMessageForCallApp() {
        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        long nativeCallID = 1;
        doReturn(nativeCallID).when(mMtcCall).getNativeCallId();
        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.AUTO_REJECTED_CALL);
        parcel.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel.recycle();

        verify(mCM, times(1)).getPendingCall(0);
        verify(mCM, times(1)).attachPreIncomingCall(mMtcCall);
        verify(mCallListener, times(1)).onPreIncomingCallReceived(any(MtcApp.class), anyLong());
        verify(mMtcCall, times(1)).invokeIncomingCallReceivedForAutoRejecting(
                any(IncomingRejectedMtcCall.class));

        Parcel parcel1 = Parcel.obtain();
        parcel1.writeInt(IUMtcService.PRE_INCOMING_CALL);
        parcel1.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel1.recycle();

        verify(mCM, times(2)).attachPreIncomingCall(mMtcCall);
        verify(mCallListener, times(2)).onPreIncomingCallReceived(any(MtcApp.class), anyLong());
        verify(mMtcCall, times(1)).attach(anyLong());

        doReturn(mMtcCall).when(mCM).getPendingCall(nativeCallID);
        parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.AUTO_REJECTED_CALL);
        parcel.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);

        verify(mCM, times(1)).getPendingCall(nativeCallID);
        verify(mCM, times(2)).attachPreIncomingCall(mMtcCall);
        verify(mCallListener, times(2)).onPreIncomingCallReceived(any(MtcApp.class), anyLong());
        verify(mMtcCall, times(2)).invokeIncomingCallReceivedForAutoRejecting(
                any(IncomingRejectedMtcCall.class));

        parcel.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel.recycle();

        verify(mCM, times(2)).getPendingCall(0);

        doReturn(mExecutor).when(mBaseContext).getExecutor();
        mTestMtcApp.setCallListener(null);
        parcel1 = Parcel.obtain();
        parcel1.writeInt(IUMtcService.PRE_INCOMING_CALL);
        parcel1.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel);
        parcel1.recycle();

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
