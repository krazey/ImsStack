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
import static org.mockito.Mockito.doReturn;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.reg.MtcServiceState;
import com.android.imsstack.internal.enabler.ImsStateStore;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class MtcServiceStateTrackerTest extends ImsStackTest {
    private static final int EVENT_EMERGENCY_SERVICE_STATE_CHANGED = 101;
    private static final int EVENT_SERVICE_STATE_CHANGED = 102;

    private int mSlotId = 0;
    private int mMsgWhat = 0;
    private int mServiceType = 0;
    @Mock private IBaseContext mMockContext;
    private ServiceStateHandler mServiceStateHandler;
    private ServiceStateListener mServiceStateListener;
    @Mock private MtcApp mMockMtcApp;
    private MtcServiceStateTracker mTestMtcServiceStateTracker;

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);

        mServiceStateHandler = new ServiceStateHandler(Looper.myLooper());
        mServiceStateListener = new ServiceStateListener();
        mTestMtcServiceStateTracker = new MtcServiceStateTracker(mMockContext);

        doReturn(mSlotId).when(mMockContext).getPhoneId();
        doReturn(mSlotId).when(mMockContext).getSlotId();

        mMsgWhat = 0;
        mServiceType = 0;
    }

    @After
    public void tearDown() throws Exception {
        mTestMtcServiceStateTracker = null;
        super.tearDown();
    }

    private class ServiceStateListener implements IServiceStateTracker.Listener {
        @Override
        public void onEmergencyServiceStateChanged(MtcServiceState serviceState) {
            Message.obtain(mServiceStateHandler, EVENT_EMERGENCY_SERVICE_STATE_CHANGED,
                    serviceState).sendToTarget();
        }

        @Override
        public void onNormalServiceStateChanged(MtcServiceState serviceState) {
            Message.obtain(mServiceStateHandler, EVENT_SERVICE_STATE_CHANGED,
                    serviceState).sendToTarget();
        }
    }

    private class ServiceStateHandler extends Handler {
        ServiceStateHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_EMERGENCY_SERVICE_STATE_CHANGED: {
                    mMsgWhat = EVENT_EMERGENCY_SERVICE_STATE_CHANGED;
                    break;
                }
                case EVENT_SERVICE_STATE_CHANGED: {
                    mMsgWhat = EVENT_SERVICE_STATE_CHANGED;
                    MtcServiceState ss = (MtcServiceState) msg.obj;
                    if (ss != null) {
                        mServiceType = ss.mServiceType;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    @Test
    public void testClear() {
        mTestMtcServiceStateTracker.setServiceState(IUMtcService.SERVICE_VOIP);
        mTestMtcServiceStateTracker.setEmergencyServiceState(IUMtcService.SERVICE_VOIP);
        mTestMtcServiceStateTracker.clear();

        assertTrue(mTestMtcServiceStateTracker.isServiceState(IUMtcService.SERVICE_NONE));
        assertTrue(mTestMtcServiceStateTracker.isEmergencyServiceState(IUMtcService.SERVICE_NONE));
    }

    @Test
    public void testOnEmergencyServiceStateChanged() {
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        mTestMtcServiceStateTracker.onEmergencyServiceStateChanged(
                mMockMtcApp, IUMtcService.ES_OPENING, IUMtcService.ES_IDLE_REASON_NONE);
        processAllMessages();

        assertTrue(mTestMtcServiceStateTracker.isEmergencyServiceState(IUMtcService.ES_OPENING));
        assertEquals(EVENT_EMERGENCY_SERVICE_STATE_CHANGED, mMsgWhat);
    }

    @Test
    public void testOnServiceStateChanged() {
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        mTestMtcServiceStateTracker.onServiceStateChanged(mMockMtcApp,
                IUMtcService.SERVICE_EMERGENCY, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        processAllMessages();

        assertEquals(0, mMsgWhat);

        mTestMtcServiceStateTracker.setServiceState(IUMtcService.SERVICE_VOIP);
        mTestMtcServiceStateTracker.onServiceStateChanged(
                mMockMtcApp, IUMtcService.SERVICE_VOIP, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        processAllMessages();

        assertEquals(ImsStateStore.getMmTelState(mSlotId).getRegisteredServiceType(),
                ImsStateStore.STATE_INACTIVE);
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mTestMtcServiceStateTracker.setServiceState(IUMtcService.SERVICE_VOIP);
        mTestMtcServiceStateTracker.onServiceStateChanged(
                mMockMtcApp, -1, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        processAllMessages();

        mTestMtcServiceStateTracker.onServiceStateChanged(
                mMockMtcApp, -1, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mTestMtcServiceStateTracker.setServiceState(IUMtcService.SERVICE_NONE);
        mTestMtcServiceStateTracker.onServiceStateChanged(
                mMockMtcApp, IUMtcService.SERVICE_VOIP, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        processAllMessages();

        assertTrue(mTestMtcServiceStateTracker.isServiceState(IUMtcService.SERVICE_VOIP));
        assertEquals(IUMtcService.SERVICE_VOIP,
                ImsStateStore.getMmTelState(mSlotId).getRegisteredServiceType());
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);
    }

    @Test
    public void testIsServiceRegistered() {
        assertFalse(
                mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY));

        mTestMtcServiceStateTracker.setEmergencyServiceState(IUMtcService.ES_OPENED);

        assertTrue(mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_EMERGENCY));

        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_UC);

        assertTrue(mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_VOIP));
        assertTrue(mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_UC));
        assertFalse(mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_VT));

        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_VT);

        assertTrue(mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_VT));

        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_VOIP);

        assertFalse(mTestMtcServiceStateTracker.isServiceRegistered(IUMtcService.SERVICE_VT));
    }

    @Test
    public void testRegisterForEmergencyServiceStateChanged() {
        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_NONE);
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        processAllMessages();

        assertEquals(0, mMsgWhat);

        mTestMtcServiceStateTracker.setEmergencyServiceState(IUMtcService.ES_UNAVAILABLE);
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        processAllMessages();

        assertEquals(EVENT_EMERGENCY_SERVICE_STATE_CHANGED, mMsgWhat);
    }

    @Test
    public void testUnregisterForEmergencyServiceStateChanged() {
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        mTestMtcServiceStateTracker.onEmergencyServiceStateChanged(
                mMockMtcApp, IUMtcService.ES_OPENING, IUMtcService.ES_IDLE_REASON_NONE);
        processAllMessages();

        assertTrue(mTestMtcServiceStateTracker.isEmergencyServiceState(IUMtcService.ES_OPENING));
        assertEquals(EVENT_EMERGENCY_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mTestMtcServiceStateTracker.removeListener(mServiceStateListener);
        mTestMtcServiceStateTracker.onEmergencyServiceStateChanged(
                mMockMtcApp, IUMtcService.ES_OPENING, IUMtcService.ES_IDLE_REASON_NONE);
        processAllMessages();

        assertEquals(0, mMsgWhat);
    }

    @Test
    public void testRegisterForServiceStateChanged() {
        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_VOIP);
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        processAllMessages();

        assertEquals(IUMtcService.SERVICE_VOIP, mServiceType);
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mServiceType = 0;
        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_UC);
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        processAllMessages();

        assertEquals(IUMtcService.SERVICE_UC, mServiceType);
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mServiceType = 0;
        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_VT);
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        processAllMessages();

        assertEquals(IUMtcService.SERVICE_VT, mServiceType);
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mServiceType = 0;
        MtcStateUtils.updateRegState(null, mSlotId, IUMtcService.SERVICE_NONE);
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        processAllMessages();

        assertEquals(0, mMsgWhat);
        assertEquals(0, mServiceType);
    }

    @Test
    public void testUnregisterForServiceStateChanged() {
        mTestMtcServiceStateTracker.addListener(mServiceStateListener);
        mTestMtcServiceStateTracker.setServiceState(IUMtcService.SERVICE_NONE);
        mTestMtcServiceStateTracker.onServiceStateChanged(
                mMockMtcApp, IUMtcService.SERVICE_VOIP, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        processAllMessages();

        assertTrue(mTestMtcServiceStateTracker.isServiceState(IUMtcService.SERVICE_VOIP));
        assertEquals(IUMtcService.SERVICE_VOIP,
                ImsStateStore.getMmTelState(mSlotId).getRegisteredServiceType());
        assertEquals(EVENT_SERVICE_STATE_CHANGED, mMsgWhat);

        mMsgWhat = 0;
        mTestMtcServiceStateTracker.removeListener(mServiceStateListener);

        mTestMtcServiceStateTracker.setServiceState(IUMtcService.SERVICE_NONE);
        mTestMtcServiceStateTracker.onServiceStateChanged(
                mMockMtcApp, IUMtcService.SERVICE_VOIP, IUMtcService.SERVICESTATUS_REASON_UNKNOWN);
        processAllMessages();

        assertEquals(0, mMsgWhat);
    }

    @Test
    public void testHandleEmergencyCallDestroyed() {
        mTestMtcServiceStateTracker.setEmergencyServiceState(IUMtcService.ES_OPENED);
        mTestMtcServiceStateTracker.handleEmergencyCallDestroyed();
        assertTrue(mTestMtcServiceStateTracker.isEmergencyServiceState(IUMtcService.ES_OPENED));

        mTestMtcServiceStateTracker.setEmergencyServiceState(IUMtcService.ES_UNAVAILABLE);
        mTestMtcServiceStateTracker.setEmergencyServiceReason(
                IUMtcService.ES_UNAVAILABLE_REASON_NONE);
        mTestMtcServiceStateTracker.handleEmergencyCallDestroyed();
        assertTrue(mTestMtcServiceStateTracker.isEmergencyServiceState(IUMtcService.ES_IDLE));
        assertTrue(mTestMtcServiceStateTracker.isEmergencyServiceState(
                IUMtcService.ES_IDLE_REASON_NONE));
    }
}
