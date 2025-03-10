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

import static org.junit.Assert.assertArrayEquals;
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
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Handler;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.testing.AndroidTestingRunner;
import android.testing.TestableLooper;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.SystemServiceProxy.SmsManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;

@RunWith(AndroidTestingRunner.class)
@TestableLooper.RunWithLooper
public class SimAgentTest {
    private static final String UST = "86EF112C27FE01744200FF040100001E01";
    private static final byte[] UST_BYTES = ImsUtils.hexStringToBytes(UST);
    private static final String IST = "E300";
    private static final byte[] IST_BYTES = ImsUtils.hexStringToBytes(IST);
    private static final byte[] EMPTY_BYTE_ARRAY = new byte[0];
    private static final String IMPI = "1234@test.ims.com";
    private static final String DOMAIN = "test.ims.com";
    private static final String[] IMPU_ARRAY = { "sip:1234@test.ims.com", "tel:1234" };
    private static final List<Uri> IMPU_LIST =
            List.of(Uri.parse("sip:1234@test.ims.com"), Uri.parse("tel:1234"));
    private static final String[] PCSCF_ARRAY = { "test.pcscf.com", "11.22.33.44" };
    private static final List<String> PCSCF_LIST = List.of("test.pcscf.com", "11.22.33.44");
    private static final String ISIM_LOADED = "LOADED";

    @Mock private Sim.Listener mSimListener;
    @Mock private Sim.IsimListener mIsimListener;
    @Mock private TelephonyManagerProxy mTelephonyManagerProxy;
    @Mock private ISystem mSystem;
    @Mock private SystemInterface mSystemInterface;
    @Mock private NativeStateInterface mNativeStateInterface;

    private TestAppContext mTestAppContext;
    private TestableLooper mTestableLooper;
    private SimAgent mSimAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestableLooper = TestableLooper.get(this);
        mTestAppContext = new TestAppContext();
        mTestAppContext.setUpWithLooper(mTestableLooper.getLooper());

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.getSimServiceTable(eq(TelephonyManager.APPTYPE_USIM)))
                .thenReturn(UST_BYTES);
        when(mTelephonyManagerProxy.getSimServiceTable(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(IST_BYTES);
        when(mTelephonyManagerProxy.getImsPrivateUserIdentity()).thenReturn(IMPI);
        when(mTelephonyManagerProxy.getIsimDomain()).thenReturn(DOMAIN);
        when(mTelephonyManagerProxy.getImsPublicUserIdentities()).thenReturn(IMPU_LIST);
        when(mTelephonyManagerProxy.getImsPcscfAddresses()).thenReturn(PCSCF_LIST);
        when(mTelephonyManagerProxy.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);

        when(mSystemInterface.getSystem(eq(TestAppContext.SLOT0))).thenReturn(mSystem);
        SystemInterface.setSystemInterface(mSystemInterface);
        AgentFactory.getInstance().setAgent(
                NativeStateInterface.class, mNativeStateInterface, TestAppContext.SLOT0);

        mSimAgent = new SimAgent(TestAppContext.SLOT0, mTestableLooper.getLooper());
        mSimAgent.init(mTestAppContext.getContext());
    }

    @After
    public void tearDown() throws Exception {
        if (mSimAgent != null) {
            mSimAgent.cleanup();
            mSimAgent = null;
        }

        AgentFactory.getInstance().setAgent(NativeStateInterface.class, null, TestAppContext.SLOT0);
        SystemInterface.setSystemInterface(null);
        mSimListener = null;
        mIsimListener = null;
        mTelephonyManagerProxy = null;
        mSystem = null;
        mSystemInterface = null;
        mNativeStateInterface = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
        mTestableLooper = null;
    }

    @Test
    @SmallTest
    public void testInit() {
        assertEquals(TestAppContext.SLOT0, mSimAgent.getSlotId());
        assertEquals(TestAppContext.SUB_ID_1, mSimAgent.getSubId());
        assertNotNull(mSimAgent.getUsatInterface());
    }

    @Test
    @SmallTest
    public void testGetSmscAddress() {
        final String smscAddress = "1234";
        SmsManagerProxy smsManagerProxy =
                mTestAppContext.getSystemServiceProxy(SmsManagerProxy.class);
        when(smsManagerProxy.getSmscAddress()).thenReturn(smscAddress);

        assertEquals(smscAddress, mSimAgent.getSmscAddress());

        doThrow(new RuntimeException("getSmscAddress fails."))
                .when(smsManagerProxy).getSmscAddress();

        assertNull(mSimAgent.getSmscAddress());
    }

    @Test
    @SmallTest
    public void testGetSimCardState() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
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
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT, TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
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
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED, TelephonyManager.SIM_STATE_ABSENT);

        // SIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertArrayEquals(UST_BYTES, mSimAgent.getUsimServiceTable());

        // SIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertArrayEquals(EMPTY_BYTE_ARRAY, mSimAgent.getUsimServiceTable());
    }

    @Test
    @SmallTest
    public void testGetIsimRecords() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_LOADED);

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertEquals(DOMAIN, mSimAgent.getIsimDomain());
        assertArrayEquals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0]));
        assertArrayEquals(PCSCF_ARRAY, mSimAgent.getIsimPcscf().toArray(new String[0]));
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());

        // ISIM_STATE_ABSENT
        mSimAgent.updateSimState();

        assertNull(mSimAgent.getIsimImpi());
        assertNull(mSimAgent.getIsimDomain());
        assertTrue(mSimAgent.getIsimImpu().isEmpty());
        assertTrue(mSimAgent.getIsimPcscf().isEmpty());
        assertArrayEquals(EMPTY_BYTE_ARRAY, mSimAgent.getIsimServiceTable());

        // ISIM_STATE_LOADED and throw RuntimeException for IMPI/IMPU/P-CSCF
        doThrow(new RuntimeException("IMPI not loaded."))
                .when(mTelephonyManagerProxy).getImsPrivateUserIdentity();
        doThrow(new RuntimeException("IMPU not loaded."))
                .when(mTelephonyManagerProxy).getImsPublicUserIdentities();
        doThrow(new RuntimeException("P-CSCF not loaded."))
                .when(mTelephonyManagerProxy).getImsPcscfAddresses();
        mSimAgent.updateSimState();

        assertNull(mSimAgent.getIsimImpi());
        assertEquals(DOMAIN, mSimAgent.getIsimDomain());
        assertTrue(mSimAgent.getIsimImpu().isEmpty());
        assertTrue(mSimAgent.getIsimPcscf().isEmpty());
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());
    }

    @Test
    @SmallTest
    public void testIsimLoadedWithRetry() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED);
        // "domain" record is null
        when(mTelephonyManagerProxy.getIsimDomain()).thenReturn(null);

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertNull(mSimAgent.getIsimDomain());
        assertArrayEquals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0]));
        assertArrayEquals(PCSCF_ARRAY, mSimAgent.getIsimPcscf().toArray(new String[0]));
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());

        // Wait for "domain" record.
        when(mTelephonyManagerProxy.getIsimDomain()).thenReturn(DOMAIN);
        processAllMessages(SimAgent.IsimLoadedState.MAX_DELAY_MS);

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertEquals(DOMAIN, mSimAgent.getIsimDomain());
        assertArrayEquals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0]));
        assertArrayEquals(PCSCF_ARRAY, mSimAgent.getIsimPcscf().toArray(new String[0]));
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());
        verify(mSystem)
                .notifyIsimState(eq(SimAgent.NOTIFICATION_ISIM_STATE_CHANGED), eq(ISIM_LOADED));
    }

    @Test
    @SmallTest
    public void testIsimLoadedWithMaxRetryCounter() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED);
        // "domain" record is null
        when(mTelephonyManagerProxy.getIsimDomain()).thenReturn(null);

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertNull(mSimAgent.getIsimDomain());
        assertArrayEquals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0]));
        assertArrayEquals(PCSCF_ARRAY, mSimAgent.getIsimPcscf().toArray(new String[0]));
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());

        for (int i = 0; i < SimAgent.IsimLoadedState.MAX_RETRY_COUNTER; ++i) {
            processAllMessages(SimAgent.IsimLoadedState.MAX_DELAY_MS + 100);
        }

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        // Domain is still null and notify ISIM LOADED state as is.
        assertNull(mSimAgent.getIsimDomain());
        assertArrayEquals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0]));
        assertArrayEquals(PCSCF_ARRAY, mSimAgent.getIsimPcscf().toArray(new String[0]));
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());
        verify(mSystem)
                .notifyIsimState(eq(SimAgent.NOTIFICATION_ISIM_STATE_CHANGED), eq(ISIM_LOADED));
    }

    @Test
    @SmallTest
    public void testIsimLoadedWithPendingStateProcessing() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED, TelephonyManager.SIM_STATE_ABSENT);
        // "domain" record is null
        when(mTelephonyManagerProxy.getIsimDomain()).thenReturn(null);

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertNull(mSimAgent.getIsimDomain());
        assertArrayEquals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0]));
        assertArrayEquals(PCSCF_ARRAY, mSimAgent.getIsimPcscf().toArray(new String[0]));
        assertArrayEquals(IST_BYTES, mSimAgent.getIsimServiceTable());

        // ISIM_STATE_ABSENT
        mSimAgent.updateSimState();

        verify(mSystem)
                .notifyIsimState(eq(SimAgent.NOTIFICATION_ISIM_STATE_CHANGED), eq(ISIM_LOADED));
    }

    @Test
    @SmallTest
    public void testGetIsimState() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT, TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
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
        assertEquals(ISIM_LOADED, mSimAgent.getIsimStateString());
        verify(mSystem, times(3))
                .notifyIsimState(eq(SimAgent.NOTIFICATION_ISIM_STATE_CHANGED), anyString());
    }

    @Test
    @SmallTest
    public void testGetIsimState_noIsimApplication() {
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT, TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_NOT_READY,
                        TelephonyManager.SIM_STATE_PIN_REQUIRED,
                        TelephonyManager.SIM_STATE_LOADED);
        when(mTelephonyManagerProxy.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
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
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT, TelephonyManager.SIM_STATE_ABSENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
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
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
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
        when(mTelephonyManagerProxy.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_ABSENT,
                        TelephonyManager.SIM_STATE_PRESENT);
        when(mTelephonyManagerProxy.getSimApplicationState())
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
        receiver.onReceive(mTestAppContext.getContext(),
                createSimCardStateChangedIntent(Sim.STATE_PRESENT));

        assertEquals(TestAppContext.SUB_ID_1, mSimAgent.getSubId());
        assertEquals(Sim.STATE_PRESENT, mSimAgent.getSimCardState());
        verify(mSimListener).onSimCardStateChanged();
    }

    @Test
    @SmallTest
    public void testSimApplicationStateChanged() {
        mSimAgent.addListener(mSimListener);
        BroadcastReceiver receiver = getBroadcastReceiver();
        receiver.onReceive(mTestAppContext.getContext(),
                createSimApplicationStateChangedIntent(Sim.STATE_LOADED, TestAppContext.SUB_ID_2));

        assertEquals(TestAppContext.SUB_ID_2, mSimAgent.getSubId());
        assertEquals(Sim.STATE_LOADED, mSimAgent.getSimState());
        assertEquals(Sim.ISIM_STATE_LOADED, mSimAgent.getIsimState());
        verify(mSimListener).onSimStateChanged();
    }

    @Test
    @SmallTest
    public void testRequestSimAuthenticationForIsim() throws Exception {
        String response = "2wgB9X9Haqnj4xA5E6y5r44SxQ7tVvwkeadzEKX0wenwECw/EUHzIDCq5P8A";
        String nonce = "EMQHeGstjjt2pkTck3aM95AQtArxaBmBAADaolOr+yoMYw==";
        long owner = 1L;
        when(mTelephonyManagerProxy.getIccAuthentication(
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
        when(mTelephonyManagerProxy.getIccAuthentication(
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
        TelephonyManagerProxy telephonyManagerProxy = mock(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.createForSubscriptionId(anyInt()))
                .thenReturn(telephonyManagerProxy);

        ArgumentCaptor<NativeStateInterface.Listener> captor =
                ArgumentCaptor.forClass(NativeStateInterface.Listener.class);
        verify(mNativeStateInterface).addListener(captor.capture());
        NativeStateInterface.Listener listener = captor.getValue();

        assertNotNull(listener);

        listener.onNativeServiceReady();

        verify(telephonyManagerProxy).getSimCardState();
        verify(telephonyManagerProxy).getSimApplicationState();

        TelephonyManagerProxy telephonyManagerProxy2 = mock(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.createForSubscriptionId(anyInt()))
                .thenReturn(telephonyManagerProxy2);
        when(telephonyManagerProxy2.getSimCardState())
                .thenReturn(TelephonyManager.SIM_STATE_PRESENT);
        when(telephonyManagerProxy2.getSimApplicationState())
                .thenReturn(TelephonyManager.SIM_STATE_LOADED);
        mSimAgent.updateSimState();

        assertTrue(mSimAgent.isSimLoaded());

        when(mTelephonyManagerProxy.createForSubscriptionId(anyInt()))
                .thenReturn(telephonyManagerProxy);
        listener.onNativeServiceReady();

        verifyNoMoreInteractions(telephonyManagerProxy);
    }

    private Intent createSimApplicationStateChangedIntent(int state, int subId) {
        return new Intent(TelephonyManager.ACTION_SIM_APPLICATION_STATE_CHANGED)
                .putExtra(TelephonyManager.EXTRA_SIM_STATE, state)
                .putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, subId)
                .putExtra(SubscriptionManager.EXTRA_SLOT_INDEX, TestAppContext.SLOT0);
    }

    private Intent createSimCardStateChangedIntent(int state) {
        return new Intent(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)
                .putExtra(TelephonyManager.EXTRA_SIM_STATE, state)
                .putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, TestAppContext.SUB_ID_1)
                .putExtra(SubscriptionManager.EXTRA_SLOT_INDEX, TestAppContext.SLOT0);
    }

    private BroadcastReceiver getBroadcastReceiver() {
        ArgumentCaptor<BroadcastReceiver> receiverCaptor =
                ArgumentCaptor.forClass(BroadcastReceiver.class);
        verify(mTestAppContext.getBroadcastReceiverProxy())
                .registerReceiver(receiverCaptor.capture(), any(IntentFilter.class),
                        any(Handler.class));
        return receiverCaptor.getValue();
    }

    private void processAllMessages() {
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }

    private void processAllMessages(long moveTimeMillis) {
        if (moveTimeMillis > 0) {
            mTestableLooper.moveTimeForward(moveTimeMillis);
        }
        while (!mTestableLooper.getLooper().getQueue().isIdle()) {
            mTestableLooper.processAllMessages();
        }
    }
}
