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
package com.android.imsstack.internal.imsservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoMoreInteractions;

import android.content.Context;
import android.telecom.TelecomManager;
import android.test.suitebuilder.annotation.SmallTest;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MmTelFeatureRegistryTest {
    private static final int SLOT0 = 0;

    @Mock Context mContext;
    @Mock MmTelFeatureRegistry.Listener mListener;
    private MmTelFeatureRegistry mMmTelFeatureRegistry;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mMmTelFeatureRegistry = ImsServiceRegistry.getInstance(SLOT0).getMmTelFeatureRegistry();
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_NONE);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    @SmallTest
    public void testSetTerminalBasedCallWaitingStatus() {
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(true);
        assertTrue(mMmTelFeatureRegistry.isTerminalBasedCallWaitingEnabled());

        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        assertFalse(mMmTelFeatureRegistry.isTerminalBasedCallWaitingEnabled());
    }

    @Test
    @SmallTest
    public void testSetSrvccState() {
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_STARTED,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_CANCELED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_FAILED);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());

        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_NONE);
        assertEquals(MmTelFeatureRegistry.SRVCC_STATE_NONE,
                mMmTelFeatureRegistry.getSrvccState());
    }

    @Test
    @SmallTest
    public void testSetTtyMode() {
        assertEquals(TelecomManager.TTY_MODE_OFF, mMmTelFeatureRegistry.getTtyMode());

        mMmTelFeatureRegistry.setTtyMode(TelecomManager.TTY_MODE_FULL);
        assertEquals(TelecomManager.TTY_MODE_FULL, mMmTelFeatureRegistry.getTtyMode());

        mMmTelFeatureRegistry.setTtyMode(TelecomManager.TTY_MODE_HCO);
        assertEquals(TelecomManager.TTY_MODE_HCO, mMmTelFeatureRegistry.getTtyMode());

        mMmTelFeatureRegistry.setTtyMode(TelecomManager.TTY_MODE_VCO);
        assertEquals(TelecomManager.TTY_MODE_VCO, mMmTelFeatureRegistry.getTtyMode());
    }

    @Test
    @SmallTest
    public void testAddListener() {
        verify(mListener, never()).onTerminalBasedCallWaitingStatusChanged();
        verify(mListener, never()).onSrvccStateChanged(anyInt());

        mMmTelFeatureRegistry.addListener(mListener);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(true);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);

        verify(mListener, times(2)).onTerminalBasedCallWaitingStatusChanged();
        verify(mListener, times(2)).onSrvccStateChanged(anyInt());

        mMmTelFeatureRegistry.removeListener(mListener);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(true);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setTerminalBasedCallWaitingStatus(false);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);

        verifyNoMoreInteractions(mListener);
    }

    @Test
    @SmallTest
    public void testAddListenerWhenSrvccStateChanged() {
        verify(mListener, never()).onSrvccStateChanged(anyInt());

        mMmTelFeatureRegistry.addListener(mListener);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_CANCELED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_FAILED);

        verify(mListener, times(3))
                .onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_STARTED));
        verify(mListener).onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED));
        verify(mListener).onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_CANCELED));
        verify(mListener).onSrvccStateChanged(eq(MmTelFeatureRegistry.SRVCC_STATE_FAILED));

        mMmTelFeatureRegistry.removeListener(mListener);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_STARTED);
        mMmTelFeatureRegistry.setSrvccState(MmTelFeatureRegistry.SRVCC_STATE_COMPLETED);

        verifyNoMoreInteractions(mListener);
    }
}
