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
package com.android.imsstack.core.agents;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;
import android.testing.TestableLooper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.SimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Arrays;
import java.util.List;

@RunWith(JUnit4.class)
public class SimAgentTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1, 2 };
    private static final String UST = "86EF112C27FE01744200FF040100001E01";
    private static final byte[] UST_BYTES = SimUtils.hexStringToBytes(UST);
    private static final String IST = "E200";
    private static final byte[] IST_BYTES = SimUtils.hexStringToBytes(IST);
    private static final String IMPI = "1234@test.ims.com";
    private static final String DOMAIN = "test.ims.com";
    private static final String[] IMPU_ARRAY = { "sip:1234@test.ims.com", "tel:1234" };
    private static final List<Uri> IMPU_LIST =
            List.of(Uri.parse("sip:1234@test.ims.com"), Uri.parse("tel:1234"));

    @Mock private Sim.Listener mSimListener;
    @Mock private Sim.IsimListener mIsimListener;
    @Mock private TelephonyManager mTelephonyManager;
    @Mock private ISystem mSystem;
    @Mock private SystemInterface mSystemInterface;
    @Mock private NativeStateInterface mNativeStateInterface;

    private Context mContext;
    private ContextFixture mContextFixture;
    private TestableLooper mTestableLooper;
    private SimAgent mSimAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        mContext = mContextFixture.getTestDouble();
        AppContext.init(mContext);

        SubscriptionManager sm = mContext.getSystemService(SubscriptionManager.class);
        when(sm.getSubscriptionIds(anyInt())).thenReturn(SUB_ID);

        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        when(tm.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(1);
        when(mTelephonyManager.getSupportedModemCount()).thenReturn(1);
        when(mTelephonyManager.getSimServiceTable(eq(TelephonyManager.APPTYPE_USIM)))
                .thenReturn(UST);
        when(mTelephonyManager.getSimServiceTable(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(IST);
        when(mTelephonyManager.getImsPrivateUserIdentity()).thenReturn(IMPI);
        when(mTelephonyManager.getIsimDomain()).thenReturn(DOMAIN);
        when(mTelephonyManager.getImsPublicUserIdentities()).thenReturn(IMPU_LIST);
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);

        when(mSystemInterface.getSystem(eq(SLOT0))).thenReturn(mSystem);
        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mNativeStateInterface, SLOT0);

        mTestableLooper = new TestableLooper(Looper.getMainLooper());
        mSimAgent = new SimAgent(SLOT0, Looper.getMainLooper());
        mSimAgent.init(mContext);
    }

    @After
    public void tearDown() throws Exception {
        if (mSimAgent != null) {
            mSimAgent.cleanup();
            mSimAgent = null;
        }

        if (mTestableLooper != null) {
            mTestableLooper.destroy();
            mTestableLooper = null;
        }

        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, SLOT0);
        SystemInterface.setSystemInterface(null);
        mSimListener = null;
        mIsimListener = null;
        mTelephonyManager = null;
        mSystem = null;
        mSystemInterface = null;
        mNativeStateInterface = null;
        mContextFixture = null;
        mContext = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testInit() {
        assertEquals(SLOT0, mSimAgent.getSlotId());
        assertEquals(SUB_ID[0], mSimAgent.getSubId());
        assertNotNull(mSimAgent.getUsatInterface());
    }

    @Test
    @SmallTest
    public void testGetSmscAddress() {
        mContextFixture.setSystemService(Context.SMS_SERVICE, null);
        assertNull(mSimAgent.getSmscAddress());
    }

    @Test
    @SmallTest
    public void testGetSimCardState() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED, TelephonyManager.SIM_STATE_ABSENT);

        // SIM_STATE_PRESENT
        mSimAgent.updateSimState();

        assertEquals(Sim.STATE_PRESENT, mSimAgent.getSimCardState());

        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertEquals(Sim.STATE_ABSENT, mSimAgent.getSimCardState());
    }

    @Test
    @SmallTest
    public void testGetSimState() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT, TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_NOT_READY,
                        TelephonyManager.SIM_STATE_PIN_REQUIRED,
                        TelephonyManager.SIM_STATE_LOADED);

        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertEquals(Sim.STATE_ABSENT, mSimAgent.getSimState());
        assertFalse(mSimAgent.isSimLoaded());
        assertTrue(mSimAgent.isSimLoadCompleted());

        // SIM_STATE_NOT_READY
        mSimAgent.updateSimState();

        assertEquals(Sim.STATE_NOT_READY, mSimAgent.getSimState());
        assertFalse(mSimAgent.isSimLoaded());
        assertFalse(mSimAgent.isSimLoadCompleted());

        // SIM_STATE_PIN_REQUIRED
        mSimAgent.updateSimState();

        assertEquals(Sim.STATE_LOCKED, mSimAgent.getSimState());
        assertFalse(mSimAgent.isSimLoaded());
        assertTrue(mSimAgent.isSimLoadCompleted()); // LOCKED state

        // SIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(Sim.STATE_LOADED, mSimAgent.getSimState());
        assertTrue(mSimAgent.isSimLoaded());
        assertTrue(mSimAgent.isSimLoadCompleted());
    }

    @Test
    @SmallTest
    public void testGetSimRecords() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED, TelephonyManager.SIM_STATE_ABSENT);

        // SIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertTrue(Arrays.equals(UST_BYTES, mSimAgent.getUsimServiceTable()));

        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertNull(mSimAgent.getUsimServiceTable());
    }

    @Test
    @SmallTest
    public void testGetIsimRecords() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_LOADED);

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertEquals(DOMAIN, mSimAgent.getIsimDomain());
        assertTrue(Arrays.equals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0])));
        assertTrue(Arrays.equals(IST_BYTES, mSimAgent.getIsimServiceTable()));

        // ISIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertNull(mSimAgent.getIsimImpi());
        assertNull(mSimAgent.getIsimDomain());
        assertTrue(mSimAgent.getIsimImpu().isEmpty());
        assertNull(mSimAgent.getIsimServiceTable());

        // ISIM_STATE_LOADED and throw RuntimeException for IMPI/IMPU
        doThrow(new RuntimeException("IMPI not loaded."))
                .when(mTelephonyManager).getImsPrivateUserIdentity();
        doThrow(new RuntimeException("IMPU not loaded."))
                .when(mTelephonyManager).getImsPublicUserIdentities();
        mSimAgent.updateSimState();

        assertNull(mSimAgent.getIsimImpi());
        assertEquals(DOMAIN, mSimAgent.getIsimDomain());
        assertTrue(mSimAgent.getIsimImpu().isEmpty());
        assertTrue(Arrays.equals(IST_BYTES, mSimAgent.getIsimServiceTable()));
    }

    @Test
    @SmallTest
    public void testGetIsimState() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT, TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_NOT_READY,
                        TelephonyManager.SIM_STATE_PIN_REQUIRED,
                        TelephonyManager.SIM_STATE_LOADED);

        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_PRESENT, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());
        assertEquals("NOT_PRESENT", mSimAgent.getIsimStateString());

        // SIM_STATE_NOT_READY
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_READY, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());
        assertEquals("NOT_READY", mSimAgent.getIsimStateString());

        // SIM_STATE_PIN_REQUIRED
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_READY, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());
        assertEquals("NOT_READY", mSimAgent.getIsimStateString());

        // SIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_LOADED, mSimAgent.getIsimState());
        assertTrue(mSimAgent.isIsimLoaded());
        assertEquals("LOADED", mSimAgent.getIsimStateString());
        verify(mSystem, times(3))
                .notifyIsimState(eq(SimAgent.NOTIFICATION_ISIM_STATE_CHANGED), anyString());
    }

    @Test
    @SmallTest
    public void testGetIsimState_noIsimApplication() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT, TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_NOT_READY,
                        TelephonyManager.SIM_STATE_PIN_REQUIRED,
                        TelephonyManager.SIM_STATE_LOADED);
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(false);

        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_PRESENT, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());

        // SIM_STATE_NOT_READY
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_READY, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());

        // SIM_STATE_PIN_REQUIRED
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_READY, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());

        // SIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(Sim.ISIM_STATE_NOT_PRESENT, mSimAgent.getIsimState());
        assertFalse(mSimAgent.isIsimLoaded());
    }

    @Test
    @SmallTest
    public void testIsGbaAvailable() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED, TelephonyManager.SIM_STATE_ABSENT);

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertTrue(mSimAgent.isGbaAvailable());

        // ISIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertFalse(mSimAgent.isGbaAvailable());
    }

    @Test
    @SmallTest
    public void testAddListener() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_LOADED);

        mSimAgent.addListener(mSimListener);
        // SIM_STATE_LOADED
        mSimAgent.updateSimState();
        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        verify(mSimListener, times(2)).onSimCardStateChanged();
        verify(mSimListener, times(2)).onSimStateChanged();

        // SIM_STATE_ABSENT: same state
        mSimAgent.updateSimState();

        verifyNoMoreInteractions(mSimListener);

        mSimAgent.removeListener(mSimListener);
        // SIM_STATE_LOADED
        mSimAgent.updateSimState();

        verifyNoMoreInteractions(mSimListener);
    }

    @Test
    @SmallTest
    public void testAddIsimListener() {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_LOADED);

        mSimAgent.addIsimListener(mIsimListener);
        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();
        // ISIM_STATE_REFRESH_STARTED
        mSimAgent.updateSimState();

        verify(mIsimListener, times(2)).onIsimStateChanged();

        // ISIM_STATE_REFRESH_STARTED: same state
        mSimAgent.updateSimState();

        verifyNoMoreInteractions(mIsimListener);

        mSimAgent.removeIsimListener(mIsimListener);
        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        verifyNoMoreInteractions(mIsimListener);
    }

    @Test
    @SmallTest
    public void testSimCardStateChanged() {
        mSimAgent.addListener(mSimListener);
        BroadcastReceiver receiver = getBroadcastReceiver();
        receiver.onReceive(mContext, createSimCardStateChangedIntent(Sim.STATE_PRESENT));

        assertEquals(SUB_ID[0], mSimAgent.getSubId());
        assertEquals(Sim.STATE_PRESENT, mSimAgent.getSimCardState());
        verify(mSimListener).onSimCardStateChanged();
    }

    @Test
    @SmallTest
    public void testSimApplicationStateChanged() {
        mSimAgent.addListener(mSimListener);
        BroadcastReceiver receiver = getBroadcastReceiver();
        receiver.onReceive(mContext,
                createSimApplicationStateChangedIntent(Sim.STATE_LOADED, SUB_ID[1]));

        assertEquals(SUB_ID[1], mSimAgent.getSubId());
        assertEquals(Sim.STATE_LOADED, mSimAgent.getSimState());
        assertEquals(Sim.ISIM_STATE_LOADED, mSimAgent.getIsimState());
        verify(mSimListener).onSimStateChanged();
    }

    @Test
    @SmallTest
    public void testReadIsimFileAttributes() throws Exception {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED);

        mSimAgent.updateSimState();
        mSimAgent.readIsimFileAttributes(Sim.ISIM_FILE_ID_DOMAIN);
        processAllMessages();

        verify(mSystem).notifyIsimFileAttributesResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE),
                eq(Sim.ISIM_FILE_ID_DOMAIN), anyInt(), any());

        mSimAgent.readIsimFileAttributes(Sim.ISIM_FILE_ID_IMPI);
        processAllMessages();

        verify(mSystem).notifyIsimFileAttributesResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE),
                eq(Sim.ISIM_FILE_ID_IMPI), anyInt(), any());

        mSimAgent.readIsimFileAttributes(Sim.ISIM_FILE_ID_IMPU);
        processAllMessages();

        verify(mSystem).notifyIsimFileAttributesResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE),
                eq(Sim.ISIM_FILE_ID_IMPU), anyInt(), any());

        int unknownFileId = 0x6FFF;
        mSimAgent.readIsimFileAttributes(unknownFileId);
        processAllMessages();

        verify(mSystem).notifyIsimFileAttributesResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE),
                eq(unknownFileId), anyInt(), any());
    }

    @Test
    @SmallTest
    public void testReadIsimRecord() throws Exception {
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED);
        int index = 0;

        mSimAgent.updateSimState();
        mSimAgent.readIsimRecord(Sim.ISIM_FILE_ID_DOMAIN, index);
        processAllMessages();

        verify(mSystem).notifyIsimRecordResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_RECORD),
                eq(Sim.ISIM_FILE_ID_DOMAIN), eq(index), any());

        mSimAgent.readIsimRecord(Sim.ISIM_FILE_ID_IMPI, index);
        processAllMessages();

        verify(mSystem).notifyIsimRecordResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_RECORD),
                eq(Sim.ISIM_FILE_ID_IMPI), eq(index), any());

        mSimAgent.readIsimRecord(Sim.ISIM_FILE_ID_IMPU, index);
        processAllMessages();

        verify(mSystem).notifyIsimRecordResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_RECORD),
                eq(Sim.ISIM_FILE_ID_IMPU), eq(index), any());

        int unknownFileId = 0x6FFF;
        mSimAgent.readIsimRecord(unknownFileId, index);
        processAllMessages();

        verify(mSystem).notifyIsimRecordResponse(
                eq(SimAgent.NOTIFICATION_ISIM_READ_RECORD),
                eq(unknownFileId), eq(index), any());
    }

    @Test
    @SmallTest
    public void testRequestSimAuthenticationForIsim() throws Exception {
        String response = "2wgB9X9Haqnj4xA5E6y5r44SxQ7tVvwkeadzEKX0wenwECw/EUHzIDCq5P8A";
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        when(mTelephonyManager.getIccAuthentication(
                eq(Sim.APP_TYPE_ISIM), eq(TelephonyManager.AUTHTYPE_EAP_AKA), eq(nonce)))
                .thenReturn(response);
        mSimAgent.requestSimAuthentication(Sim.APP_TYPE_ISIM, nonce, owner);
        processAllMessages();

        verify(mSystem).notifyIsimAuthenticationResponse(
                eq(SimAgent.NOTIFICATION_ISIM_AUTH), eq(response), eq(owner));
    }

    @Test
    @SmallTest
    public void testRequestSimAuthenticationForUsim() throws Exception {
        String response = "2wgB9X9Haqnj4xA5E6y5r44SxQ7tVvwkeadzEKX0wenwECw/EUHzIDCq5P8A";
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        when(mTelephonyManager.getIccAuthentication(
                eq(Sim.APP_TYPE_USIM), eq(TelephonyManager.AUTHTYPE_EAP_AKA), eq(nonce)))
                .thenReturn(response);
        mSimAgent.requestSimAuthentication(Sim.APP_TYPE_USIM, nonce, owner);
        processAllMessages();

        verify(mSystem).notifyUsimAuthenticationResponse(
                eq(SimAgent.NOTIFICATION_USIM_AUTH), eq(response), eq(owner));
    }

    @Test
    @SmallTest
    public void testOnNativeServiceReady() {
        TelephonyManager tm = mContext.getSystemService(TelephonyManager.class);
        TelephonyManager telephonyManager = mock(TelephonyManager.class);
        when(tm.createForSubscriptionId(anyInt())).thenReturn(telephonyManager);

        ArgumentCaptor<NativeStateInterface.Listener> captor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mNativeStateInterface).addListener(captor.capture());
        NativeStateInterface.Listener listener = captor.getValue();

        assertNotNull(listener);

        listener.onNativeServiceReady();

        verify(telephonyManager).getSimCardState();
        verify(telephonyManager).getSimApplicationState();

        when(tm.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManager.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED);
        mSimAgent.updateSimState();

        assertTrue(mSimAgent.isSimLoaded());

        when(tm.createForSubscriptionId(anyInt())).thenReturn(telephonyManager);
        listener.onNativeServiceReady();

        verifyNoMoreInteractions(telephonyManager);
    }

    private Intent createSimApplicationStateChangedIntent(int state, int subId) {
        return new Intent(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)
                .putExtra(TelephonyManager.EXTRA_SIM_STATE, state)
                .putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, subId)
                .putExtra(SubscriptionManager.EXTRA_SLOT_INDEX, SLOT0);
    }

    private Intent createSimCardStateChangedIntent(int state) {
        return new Intent(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)
                .putExtra(TelephonyManager.EXTRA_SIM_STATE, state)
                .putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, SUB_ID[0])
                .putExtra(SubscriptionManager.EXTRA_SLOT_INDEX, SLOT0);
    }

    private BroadcastReceiver getBroadcastReceiver() {
        ArgumentCaptor<BroadcastReceiver> receiverCaptor =
                ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mContext).registerReceiver(receiverCaptor.capture(),
                any(IntentFilter.class), any(), any(Handler.class), anyInt());
        return receiverCaptor.getValue();
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
