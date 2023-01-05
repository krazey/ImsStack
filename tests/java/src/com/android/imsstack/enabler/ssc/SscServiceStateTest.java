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
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CarrierConfigManager;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.AlarmTimerAgent;
import com.android.imsstack.core.agents.ConfigAgent;
import com.android.imsstack.core.agents.IAlarmTimer;
import com.android.imsstack.core.agents.IWifiState;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
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
    @Mock private IWifiState mMockWifiState;
    @Mock private SimInterface mMockSimInterface;

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
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN });
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_USE_CSFB_ON_XCAP_OVER_UT_FAILURE_BOOL))
                .thenReturn(true);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Assets.KEY_UT_TEMPORARY_BLOCK_TIMER_WITH_ANY_REASON_SEC_INT))
                .thenReturn(mBlockTimer);
        when(mMockCarrierConfig.getInt(
                CarrierConfig.Assets.KEY_UT_TEMPORARY_BLOCK_TIMER_MIN_INT)).thenReturn(mBlockTimer);

        when(mMockWifiState.isWifiConnected()).thenReturn(false);
        when(mMockDcNetWatcher.isRoaming()).thenReturn(false);
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_LTE);
        when(mMockAlarmTimer.getTimerId()).thenReturn(mTimerId);
        when(mMockAlarmTimer.startTimer(anyLong(), anyLong())).thenReturn(true);

        AgentFactory.getInstance().setAgent(SimInterface.class, mMockSimInterface, SLOT_0);
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
        AgentFactory.getInstance().setAgent(SimInterface.class, null, SLOT_0);
        mSscServiceState.deInit();
        mLooper.destroy();
    }

    @Test
    public void testInit() {
        // init() is called in setup()
        verify(mMockWifiState).registerForWifiStateChanged(mSscServiceState.mHandler,
                SscServiceState.EVENT_WIFI_STATE_CHANGED, null);
        verify(mMockDcNetWatcher).registerForAirplaneModeChanged(mSscServiceState.mHandler,
                SscServiceState.EVENT_AIRPLANE_MODE_CHANGED, null);
        verify(mMockDcNetWatcher).registerForRatChanged(mSscServiceState.mHandler,
                SscServiceState.EVENT_DATA_RAT_CHANGED, null);
        verify(mMockDcNetWatcher).registerForRoamingStateChanged(mSscServiceState.mHandler,
                SscServiceState.EVENT_DATA_ROAMING_STATE_CHANGED, null);
        verify(mMockSimInterface).addListener(mSscServiceState.mSimStateListener);
        verify(mMockAosService).addListener(mSscServiceState.mRegiStateListener);
    }

    @Test
    public void testDeInit() {
        ISscAuthAgent authAgent = SscAuthAgent.getInstance(SLOT_0);
        authAgent.setIsCredentialInfoUpdated(true);
        authAgent.setETag("etag");
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtAvailability = false;

        mSscServiceState.deInit();

        verify(mMockWifiState).unregisterForWifiStateChanged(mSscServiceState.mHandler);
        verify(mMockDcNetWatcher).unregisterForAirplaneModeChanged(mSscServiceState.mHandler);
        verify(mMockDcNetWatcher).unregisterForRatChanged(mSscServiceState.mHandler);
        verify(mMockDcNetWatcher).unregisterForRoamingStateChanged(mSscServiceState.mHandler);
        verify(mMockSimInterface).removeListener(mSscServiceState.mSimStateListener);
        verify(mMockAosService).removeListener(mSscServiceState.mRegiStateListener);

        // verifying deInit() calls resetAllUtStatus()
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        assertEquals(false, authAgent.isCredentialInfoUpdated());
        assertEquals("", authAgent.getETag());
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testIsUtAvailable() {
        mSscServiceState.mUtAvailability = false;
        assertEquals(false, mSscServiceState.isUtAvailable());

        mSscServiceState.mUtAvailability = true;
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testIsUtAvailable_utDisabledByCarrierConfig() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.KEY_CARRIER_SUPPORTS_SS_OVER_UT_BOOL)).thenReturn(false);
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.init();
        processDelayedMessage();

        assertEquals(false, mSscServiceState.isUtAvailable());
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
    }

    @Test
    public void testSetDnsQueryFailed() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setDnsQueryFailed(true);

        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 1000);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.getDnsQueryFailed());

        mSscServiceState.setDnsQueryFailed(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.getDnsQueryFailed());
    }

    @Test
    public void testSetGbaRequestFailed() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setGbaRequestFailed(true);

        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 1000);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.getGbaRequestFailed());

        mSscServiceState.setGbaRequestFailed(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.getGbaRequestFailed());
    }

    @Test
    public void testSetPdnConnectionTimeout() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setPdnConnectionTimeout(true);

        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 1000);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.getPdnConnectionTimeout());

        mSscServiceState.setPdnConnectionTimeout(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.getPdnConnectionTimeout());
    }

    @Test
    public void testSetSocketConnectionExpired() {
        assertEquals(true, mSscServiceState.isUtAvailable());

        mSscServiceState.setSocketConnectionExpired(true);

        verify(mMockAlarmTimer).startTimer((long) mTimerId, (long) mBlockTimer * 1000);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.getSocketConnectionExpired());

        mSscServiceState.setSocketConnectionExpired(false);

        verify(mMockUtInterface, times(3)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.getSocketConnectionExpired());
    }

    @Test
    public void testSscServiceStateHandler_utBlockTimerExpired() {
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_DNS_QUERY_FAILURE
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_TIMEOUT
                | SscConstant.BLOCK_REASON_SOCKET_CONNECTION_TIMEOUT
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_GBA_FAILURE
                | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_TEMP;
        mSscServiceState.mUtAvailability = false;

        sendMessage(SscServiceState.EVENT_UT_BLOCK_TIMER_EXPIRED);

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_airplaneModeOn() {
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.isAirplaneMode()).thenReturn(true);

        sendMessage(SscServiceState.EVENT_AIRPLANE_MODE_CHANGED);
        processDelayedMessage();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_airplaneModeOff() {
        int permanentBlockReasons = SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_PERM
                | SscConstant.BLOCK_REASON_PDN_CONNECTION_FAILURE_TEMP
                | SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtBlockReason = permanentBlockReasons;
        mSscServiceState.mUtAvailability = false;
        when(mMockDcNetWatcher.isAirplaneMode()).thenReturn(false);

        sendMessage(SscServiceState.EVENT_AIRPLANE_MODE_CHANGED);
        processDelayedMessage();

        assertEquals(permanentBlockReasons, mSscServiceState.mUtBlockReason);
        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_wifiStateChangedConnectedWhenUtOverWifiSupported() {
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.IWLAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mMockWifiState.isWifiConnected()).thenReturn(true);

        sendMessage(SscServiceState.EVENT_WIFI_STATE_CHANGED);
        processDelayedMessage();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_wifiStateChangedToDisconnectedWhenUtOverWifiSupported() {
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.IWLAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mMockWifiState.isWifiConnected()).thenReturn(false);

        sendMessage(SscServiceState.EVENT_WIFI_STATE_CHANGED);
        processDelayedMessage();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_wifiStateChangedToConnectedWhenUtOverWifiNotSupported() {
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.EUTRAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        when(mMockWifiState.isWifiConnected()).thenReturn(true);

        sendMessage(SscServiceState.EVENT_WIFI_STATE_CHANGED);
        processDelayedMessage();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataRatChangedToValidRat() {
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.NGRAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_NR);

        sendMessage(SscServiceState.EVENT_DATA_RAT_CHANGED);
        processDelayedMessage();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataRatChangedToInvalidRat() {
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getIntArray(
                CarrierConfigManager.ImsSs.KEY_XCAP_OVER_UT_SUPPORTED_RATS_INT_ARRAY))
                .thenReturn(new int[] { AccessNetworkType.UTRAN });
        when(mMockDcNetWatcher.getNetworkType()).thenReturn(TelephonyManager.NETWORK_TYPE_NR);

        sendMessage(SscServiceState.EVENT_DATA_RAT_CHANGED);
        processDelayedMessage();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataRoamingStateChangedWhenUtNotSupportedWhenRoaming() {
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL)).thenReturn(false);
        when(mMockDcNetWatcher.isRoaming()).thenReturn(true);

        sendMessage(SscServiceState.EVENT_DATA_ROAMING_STATE_CHANGED);
        processDelayedMessage();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscServiceStateHandler_dataRoamingStateChangedWhenUtSupportedWhenRoaming() {
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_SUPPORTED_WHEN_ROAMING_BOOL)).thenReturn(true);
        when(mMockDcNetWatcher.isRoaming()).thenReturn(true);

        sendMessage(SscServiceState.EVENT_DATA_ROAMING_STATE_CHANGED);
        processDelayedMessage();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testSscSimStateListener_onSimCardStateChangedWhenSimAbsent() {
        ISscAuthAgent authAgent = SscAuthAgent.getInstance(SLOT_0);
        authAgent.setIsCredentialInfoUpdated(true);
        authAgent.setETag("etag");
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_BY_RESPONSE_CODE_PERM;
        mSscServiceState.mUtAvailability = false;
        when(mMockSimInterface.getSimCardState()).thenReturn(Sim.STATE_ABSENT);

        ((Sim.Listener) mSscServiceState.mSimStateListener).onSimCardStateChanged();

        assertEquals(SscConstant.BLOCK_REASON_NONE, mSscServiceState.mUtBlockReason);
        assertEquals(false, authAgent.isCredentialInfoUpdated());
        assertEquals("", authAgent.getETag());
        verify(mMockUtInterface, times(2)).onServiceStateChanged();
    }

    @Test
    public void testSscSimStateListener_onSimCardStateChangedWhenSimLoaded() {
        ISscAuthAgent authAgent = SscAuthAgent.getInstance(SLOT_0);
        authAgent.setIsCredentialInfoUpdated(true);
        authAgent.setETag("etag");
        mSscServiceState.mUtBlockReason = SscConstant.BLOCK_REASON_GBA_FAILURE;
        mSscServiceState.mUtAvailability = false;
        when(mMockSimInterface.getSimCardState()).thenReturn(Sim.STATE_LOADED);

        ((Sim.Listener) mSscServiceState.mSimStateListener).onSimCardStateChanged();

        assertEquals(SscConstant.BLOCK_REASON_GBA_FAILURE, mSscServiceState.mUtBlockReason);
        assertEquals(true, authAgent.isCredentialInfoUpdated());
        assertEquals("etag", authAgent.getETag());
        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyRegisteredWhenNotRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyRegistered(0, 0, null);
        processDelayedMessage();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyRegisteredWhenRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyRegistered(0, 0, null);
        processDelayedMessage();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(true, mSscServiceState.isUtAvailable());
        assertEquals(true, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyRegistering() {
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyRegistering(0, 0, null);

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
    }

    @Test
    public void testRegistrationListener_notifyDeregisteredWhenNotRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = false;
        mSscServiceState.mUtAvailability = false;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyDeregistered(0, 0);
        processDelayedMessage();

        verifyNoMoreInteractions(mMockUtInterface);
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyDeregisteredWhenRegistered() {
        mSscServiceState.mRegiStateListener.mImsRegistrationState = true;
        mSscServiceState.mUtAvailability = true;
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyDeregistered(0, 0);
        processDelayedMessage();

        verify(mMockUtInterface, times(2)).onServiceStateChanged();
        assertEquals(false, mSscServiceState.isUtAvailable());
        assertEquals(false, mSscServiceState.mRegiStateListener.mImsRegistrationState);
    }

    @Test
    public void testRegistrationListener_notifyTechnologyChangeFailed() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyTechnologyChangeFailed(0, 0);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyAssociatedUriChanged() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyAssociatedUriChanged(null);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    @Test
    public void testRegistrationListener_notifyCapabilitiesUpdateFailed() {
        when(mMockCarrierConfig.getBoolean(
                CarrierConfigManager.ImsSs.KEY_UT_REQUIRES_IMS_REGISTRATION_BOOL)).thenReturn(true);

        mSscServiceState.mRegiStateListener.notifyCapabilitiesUpdateFailed(0, 0, 0);

        verifyNoMoreInteractions(mMockUtInterface);
    }

    private void sendMessage(int what) {
        ((Handler) mSscServiceState.mHandler)
                .handleMessage(Message.obtain(mSscServiceState.mHandler, what));
    }

    private void processDelayedMessage() {
        mLooper.moveTimeForward(1000);
        mLooper.processAllMessages();
    }

    private class FakeSscServiceState extends SscServiceState {
        FakeSscServiceState(int slotId, Looper looper) {
            super(slotId, looper);
        }

        @Override
        protected IAlarmTimer getTimerAgent() {
            super.getTimerAgent();
            return mMockAlarmTimer;
        }

        @Override
        protected IWifiState getWifiStateAgent() {
            super.getWifiStateAgent();
            return mMockWifiState;
        }

        @Override
        protected IDcNetWatcher getDcNetWatcher() {
            super.getDcNetWatcher();
            return mMockDcNetWatcher;
        }
    }
}
