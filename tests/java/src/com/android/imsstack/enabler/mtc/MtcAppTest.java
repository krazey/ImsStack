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
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.core.agents.ISharedState;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.dialogs.DialogsInfo;
import com.android.imsstack.enabler.mtc.dialogs.IUDialogs;
import com.android.imsstack.jni.JniImsListener;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcAppTest extends ImsStackTest {
    @Mock private IBaseContext mContext;
    @Mock private IMtcCallManager mCM;
    @Mock private MtcEmergencyServiceManager mEmergencyServiceManager;
    @Mock private MtcApp.ServiceStateListener mServiceStateListener;
    @Mock private MtcApp.CallListener mCallListener;
    @Mock private ISharedState mSharedState;
    @Mock private MtcCall mMtcCall;
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
    }

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        mCommand = -1;
        MockitoAnnotations.initMocks(this);

        mTestMtcJniProxy = new TestMtcJniProxy();
        mTestMtcApp = new TestMtcApp(mContext, mCM, Looper.myLooper(),
                mEmergencyServiceManager, mTestMtcJniProxy);
        mTestMtcApp.setCallListener(mCallListener);
        mTestMtcApp.setServiceStateListener(mServiceStateListener);
    }

    @After
    public void tearDown() throws Exception {
        mTestMtcApp = null;
        mTestMtcJniProxy = null;
        super.tearDown();
    }

    @Test
    public void testInit() {
        doReturn(0).when(mContext).getSlotId();
        doReturn(true).when(mContext).isCommonPackageReady();
        doReturn(mSharedState).when(mContext).getSharedState();
        doReturn(true).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mContext, times(1)).addCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        assertTrue(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testInitFailCommonPackageNotReady() {
        doReturn(0).when(mContext).getSlotId();
        doReturn(false).when(mContext).isCommonPackageReady();
        doReturn(mSharedState).when(mContext).getSharedState();
        doReturn(true).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mContext, times(1)).addCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).init();
        verify(mEmergencyServiceManager, times(0)).setNativeObject(anyLong());

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testInitFailNotNativeBootCompleted() {
        doReturn(0).when(mContext).getSlotId();
        doReturn(true).when(mContext).isCommonPackageReady();
        doReturn(mSharedState).when(mContext).getSharedState();
        doReturn(false).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        mTestMtcApp.init();

        verify(mCM, times(1)).init();
        verify(mContext, times(1)).addCommonPackageListener(any());
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

        verify(mContext, times(1)).removeCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).clear();
        verify(mContext, times(1)).removeCommonPackageListener(any());
        verify(mCM, times(1)).clear();

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testClearWithoutNativeObj() {
        mTestMtcApp.setNativeObj(1);

        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.clear();

        verify(mEmergencyServiceManager, times(1)).setNativeObject(anyLong());

        verify(mContext, times(1)).removeCommonPackageListener(any());
        verify(mEmergencyServiceManager, times(1)).clear();
        verify(mContext, times(1)).removeCommonPackageListener(any());
        verify(mCM, times(1)).clear();

        assertFalse(mTestMtcApp.isServiceValid());
    }

    @Test
    public void testCreateCall() {
        mTestMtcApp.setNativeObj(1);

        assertTrue(mTestMtcApp.isServiceValid());

        doReturn(true).when(mMtcCall).isMO();
        mTestMtcApp.createCall(0);

        verify(mCM, times(1)).attachCall(any(MtcCall.class));

        verify(mEmergencyServiceManager, times(0)).setCall(any(MtcCall.class));
        verify(mEmergencyServiceManager, times(0)).openEmergencyService();

        doReturn(false).when(mMtcCall).isMO();
        mTestMtcApp.createCall(0);

        verify(mCM, times(1)).attachPreIncomingCall(any(MtcCall.class));

        verify(mEmergencyServiceManager, times(0)).setCall(any(MtcCall.class));
        verify(mEmergencyServiceManager, times(0)).openEmergencyService();

        int sessionAttributes = MtcCall.FLAG_EMERGENCY;
        mTestMtcApp.createCall(sessionAttributes);

        verify(mEmergencyServiceManager, times(1)).setCall(any(MtcCall.class));
        verify(mEmergencyServiceManager, times(1)).openEmergencyService();
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

        mTestMtcApp.notifySrvccStateChanged(1);
        processAllMessages();

        assertEquals(IUMtcService.SRVCC_STATE_CHANGED, mCommand);
    }

    @Test
    public void testSetTerminalBasedCallWaiting() {
        mTestMtcApp.setNativeObj(1);
        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.setTerminalBasedCallWaiting(true, true);
        processAllMessages();

        assertEquals(IUMtcService.SET_TERMINAL_BASED_CALL_WAITING, mCommand);
    }

    @Test
    public void testMsgNativeBootCompletedMsgCommonPackageReady() {
        doReturn(0).when(mContext).getSlotId();
        doReturn(true).when(mContext).isCommonPackageReady();
        doReturn(mSharedState).when(mContext).getSharedState();
        doReturn(true).when(mSharedState).isNativeBootCompleted();

        assertFalse(mTestMtcApp.isServiceValid());

        ((Handler) mTestMtcApp.getHandler()).sendEmptyMessage(1);
        processAllMessages();

        assertTrue(mTestMtcApp.isServiceValid());

        mTestMtcApp.setNativeObj(0);
        assertFalse(mTestMtcApp.isServiceValid());

        ((Handler) mTestMtcApp.getHandler()).sendEmptyMessage(2);
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

        Parcel parcel2 = Parcel.obtain();
        parcel2.writeInt(IUDialogs.NOTIFY_DIALOG_INFO);
        parcel2.setDataPosition(0);
        mTestMtcApp.getNativeListener().onMessage(parcel2);
        parcel2.recycle();

        verify(mCallListener, times(1)).onDialogStateChanged(
                any(MtcApp.class), any(DialogsInfo.class));
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
    }
}
