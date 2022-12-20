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

package com.android.imsstack.enabler.ssc;

import static org.junit.Assert.assertEquals;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.CarrierConfigManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.agents.AlarmTimerAgent;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.IAlarmTimer;
import com.android.imsstack.core.agents.dcm.DcNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.CarrierConfig.Assets;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.service.AosService;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class SscServiceStateTest {
    private static final int SLOT_0 = 0;

    private FakeSscServiceState mSscServiceState;

    private final int mTimerId = 1;
    private final int mBlockTimer = 1;
    private TestableLooper mLooper;

    @Mock private AlarmTimerAgent mMockAlarmTimer;
    @Mock private AosService mMockAosService;
    @Mock private CarrierConfig mMockCarrierConfig;
    @Mock private ConfigAgent mMockConfigAgent;
    @Mock private DcNetWatcher mMockDcNetWatcher;
    @Mock private IUtInterface mMockUtInterface;

    @Before
    public void setup() throws Exception {
        MockitoAnnotations.initMocks(this);

        SscConfig.setConfigAgent(SLOT_0, mMockConfigAgent);
        when(mMockConfigAgent.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(true);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(false);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Assets.KEY_UT_TEMPORARY_BLOCK_TIMER_WITH_ANY_REASON_SEC_INT))
                .thenReturn(mBlockTimer);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Assets.KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT)).thenReturn(mBlockTimer);

        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);
        when(mMockAlarmTimer.startTimer(anyLong(), anyLong())).thenReturn(true);

        AosFactory.getInstance().mAosServices.put(SLOT_0, mMockAosService);
        UtFactory.getInstance().setUtInterfaceForSlot(SLOT_0, mMockUtInterface);

        HandlerThread handlerThread = new HandlerThread("SscServiceStateTest");
        handlerThread.start();

        Looper looper = handlerThread.getLooper();
        mLooper = new TestableLooper(looper);

        mSscServiceState = new FakeSscServiceState(SLOT_0, looper);
        mSscServiceState.init();

        processDelayedMessage();
        verify(mMockUtInterface).onServiceStateChanged();
    }

    @After
    public void tearDown() {
        mSscServiceState.deInit();
        mLooper.destroy();
    }

    @Test
    public void testIsUtAvailable_utDisabledByCarrierConfig() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(false);
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testIsUtAvailable_utRequiresImsRegistration() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL))
                .thenReturn(true);

        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;
        assertEquals(false, mSscServiceState.isUtAvailable());

        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSetErrorResponseCode_codeNoneBlock() {
        int[] emptyBlockErrorCodes = {};
        int errorCode = 403;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY))
                .thenReturn(emptyBlockErrorCodes);
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.Assets.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY))
                .thenReturn(emptyBlockErrorCodes);

        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setErrorResponseCode(errorCode);
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSetErrorResponseCode_codeTempBlock() {
        int[] tempBlockErrorCodes = {480};
        int errorCode = 480;
        when(mMockCarrierConfig.getIntArray(Assets.KEY_UT_HTTP_TEMPORARY_ERROR_CODE_INT_ARRAY))
                .thenReturn(tempBlockErrorCodes);

        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setErrorResponseCode(errorCode);
        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 60 * 1000);
        assertEquals(false, mSscServiceState.isUtAvailable());

        handleBlockTimerExpired();
    }

    @Test
    public void testSetErrorResponseCode_codePermBlock() {
        int[] permBlockErrorCodes = {499};
        int errorCode = 403;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_HTTP_PERMANENT_ERROR_CODE_INT_ARRAY))
                .thenReturn(permBlockErrorCodes);

        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setErrorResponseCode(errorCode);
        assertEquals(false, mSscServiceState.isUtAvailable());
        verifyNoMoreInteractions(mMockAlarmTimer);
        handleAirplaneModeOn();
    }

    @Test
    public void testSetPdnConnectionFailed_temporaryCause() {
        int[] tempBlockSmCodes = {33};
        int smCause = 33;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.Assets.KEY_UT_SM_CAUSE_TEMPORARY_BLOCK_INT_ARRAY))
                .thenReturn(tempBlockSmCodes);

        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionFailed(smCause);
        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 60 * 1000);
        assertEquals(false, mSscServiceState.isUtAvailable());
        handleBlockTimerExpired();
    }

    @Test
    public void testSetPdnConnectionFailed_permanentCause() {
        int[] permBlockSmCodes = {33};
        int smCause = 33;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfig.ImsSs.KEY_UT_SM_CAUSE_PERMANENT_BLOCK_INT_ARRAY))
                .thenReturn(permBlockSmCodes);

        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionFailed(smCause);
        assertEquals(false, mSscServiceState.isUtAvailable());
        verifyNoMoreInteractions(mMockAlarmTimer);
        handleAirplaneModeOn();
    }

    @Test
    public void testSetDnsQueryFailed() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setDnsQueryFailed(true);
        handleTemporaryBlockReason();
        assertEquals(true, mSscServiceState.getDnsQueryFailed());
        handleBlockTimerExpired();
    }

    @Test
    public void testSetGbaRequestFailed() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setGbaRequestFailed(true);
        handleTemporaryBlockReason();
        assertEquals(true, mSscServiceState.getGbaRequestFailed());
        handleBlockTimerExpired();
    }

    @Test
    public void testSetPdnConnectionTimeout() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionTimeout(true);
        handleTemporaryBlockReason();
        assertEquals(true, mSscServiceState.getPdnConnectionTimeout());
        handleBlockTimerExpired();
    }

    @Test
    public void testSetSocketConnectionExpired() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setSocketConnectionExpired(true);
        handleTemporaryBlockReason();
        assertEquals(true, mSscServiceState.getSocketConnectionExpired());
        handleBlockTimerExpired();
    }

    @Test
    public void testRegistrationListener_notifyRegisteredWhenNotRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;

        mSscServiceState.mRegiStateListener.notifyRegistered(0, 0, null);
        processDelayedMessage();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyRegisteredWhenRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;

        mSscServiceState.mRegiStateListener.notifyRegistered(0, 0, null);
        processDelayedMessage();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyRegistering() {
        mSscServiceState.mRegiStateListener.notifyRegistering(0, 0, null);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyDeregisteredWhenNotRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;

        mSscServiceState.mRegiStateListener.notifyDeregistered(0, 0);
        processDelayedMessage();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyDeregisteredWhenRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;

        mSscServiceState.mRegiStateListener.notifyDeregistered(0, 0);
        processDelayedMessage();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyTechnologyChangeFailed() {
        mSscServiceState.mRegiStateListener.notifyTechnologyChangeFailed(0, 0);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyAssociatedUriChanged() {
        mSscServiceState.mRegiStateListener.notifyAssociatedUriChanged(null);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyCapabilitiesUpdateFailed() {
        mSscServiceState.mRegiStateListener.notifyCapabilitiesUpdateFailed(0, 0, 0);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    private Message getMessage(int what) {
        return Message.obtain(mSscServiceState.mHandler, what);
    }

    private void processDelayedMessage() {
        mLooper.moveTimeForward(1000);
        mLooper.processAllMessages();
    }

    private void handleTemporaryBlockReason() {
        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 1000);
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    private void handleBlockTimerExpired() {
        ((Handler) mSscServiceState.mHandler)
                .handleMessage(getMessage(SscServiceState.EVENT_UT_BLOCK_TIMER_EXPIRED));
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    private void handleAirplaneModeOn() {
        when(mMockDcNetWatcher.isAirplaneMode()).thenReturn(true);
        ((Handler) mSscServiceState.mHandler)
                .handleMessage(getMessage(SscServiceState.EVENT_AIRPLANE_MODE_CHANGED));

        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    private class FakeSscServiceState extends SscServiceState {
        FakeSscServiceState(int slotId, Looper looper) {
            super(slotId, looper);
        }

        @Override
        protected IAlarmTimer getTimerAgent() {
            return mMockAlarmTimer;
        }

        @Override
        protected IDcNetWatcher getDcNetWatcher() {
            return mMockDcNetWatcher;
        }
    }
}
