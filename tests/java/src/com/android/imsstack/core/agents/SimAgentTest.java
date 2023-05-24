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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;
import static org.mockito.Mockito.when;

import android.content.Context;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.SimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.Arrays;

@RunWith(JUnit4.class)
public class SimAgentTest {
    private static final int SLOT0 = 0;
    private static final int[] SUB_ID = { 1 };
    private static final String UST = "86EF112C27FE01744200FF040100001E01";
    private static final byte[] UST_BYTES = SimUtils.hexStringToBytes(UST);
    private static final String IST = "E200";
    private static final byte[] IST_BYTES = SimUtils.hexStringToBytes(IST);
    private static final String IMPI = "1234@test.ims.com";
    private static final String DOMAIN = "test.ims.com";
    private static final String[] IMPU_ARRAY = { "sip:1234@test.ims.com", "tel:1234" };

    @Mock private Sim.Listener mSimListener;
    @Mock private Sim.IsimListener mIsimListener;
    @Mock private TelephonyManager mTelephonyManager;

    private ContextFixture mContextFixture;
    private SimAgent mSimAgent;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mContextFixture = new ContextFixture();
        Context context = mContextFixture.getTestDouble();
        AppContext.init(context);

        SubscriptionManager sm = context.getSystemService(SubscriptionManager.class);
        when(sm.getSubscriptionIds(anyInt())).thenReturn(SUB_ID);

        TelephonyManager tm = context.getSystemService(TelephonyManager.class);
        when(tm.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getActiveModemCount()).thenReturn(1);
        when(mTelephonyManager.getSupportedModemCount()).thenReturn(1);
        when(mTelephonyManager.getSimServiceTable(eq(TelephonyManager.APPTYPE_USIM)))
                .thenReturn(UST);
        when(mTelephonyManager.getSimServiceTable(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(IST);
        when(mTelephonyManager.getIsimImpi()).thenReturn(IMPI);
        when(mTelephonyManager.getIsimDomain()).thenReturn(DOMAIN);
        when(mTelephonyManager.getIsimImpu()).thenReturn(IMPU_ARRAY);

        mSimAgent = new SimAgent(SLOT0);
        mSimAgent.init(context);
    }

    @After
    public void tearDown() throws Exception {
        if (mSimAgent != null) {
            mSimAgent.cleanup();
            mSimAgent = null;
        }
        mTelephonyManager = null;
        mContextFixture = null;
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void testGetSlotId() {
        assertEquals(SLOT0, mSimAgent.getSlotId());
    }

    @Test
    @SmallTest
    public void testGetSubId() {
        assertEquals(SUB_ID[0], mSimAgent.getSubId());
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
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);


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

        // ISIM_STATE_LOADED
        mSimAgent.updateSimState();

        assertEquals(IMPI, mSimAgent.getIsimImpi());
        assertEquals(DOMAIN, mSimAgent.getIsimDomain());
        assertTrue(Arrays.equals(IMPU_ARRAY, mSimAgent.getIsimImpu().toArray(new String[0])));
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
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);

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

        assertEquals(Sim.ISIM_STATE_LOADED, mSimAgent.getIsimState());
        assertTrue(mSimAgent.isIsimLoaded());
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
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);

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
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);

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
        when(mTelephonyManager.isApplicationOnUicc(eq(TelephonyManager.APPTYPE_ISIM)))
                .thenReturn(true);

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
}
