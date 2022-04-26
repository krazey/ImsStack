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
import static org.junit.Assert.assertNull;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;

import android.os.Looper;
import android.testing.TestableLooper;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class SscServiceStateAgentTest {
    private final static int SLOT_0 = 0;
    private SscServiceStateAgent mSscServiceStateAgent;

    @Mock private SscServiceState mockSscServiceState;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        mSscServiceStateAgent = SscServiceStateAgent.getInstance();
        mSscServiceStateAgent.init(SLOT_0);

        mSscServiceStateAgent.setSscServiceState(SLOT_0, mockSscServiceState);
    }

    @After
    public void tearDown() {
        mSscServiceStateAgent.deInit(SLOT_0);

        Looper.myLooper().quit();
    }

    @Test
    public void testInit() {
        assertEquals(mockSscServiceState, mSscServiceStateAgent.getSscServiceState(SLOT_0));
    }

    @Test
    public void testInit_invalidParam() {
        final int invalidSlotId = -1;
        mSscServiceStateAgent.init(invalidSlotId);

        assertNull(mSscServiceStateAgent.getSscServiceState(invalidSlotId));
    }

    @Test
    public void testDeInit() {
        mSscServiceStateAgent.deInit(SLOT_0);

        verify(mockSscServiceState).deInit();
        assertNull(mSscServiceStateAgent.getSscServiceState(SLOT_0));
    }

    @Test
    public void testIsUtAvailable() {
        mSscServiceStateAgent.isUtAvailable(SLOT_0);

        verify(mockSscServiceState).isUtAvailable();
    }

    @Test
    public void testSetErrorResponseCode() {
        mSscServiceStateAgent.setErrorResponseCode(SLOT_0, SscConstant.HTTP_CONFLICT);

        verify(mockSscServiceState).setErrorResponseCode(eq(SscConstant.HTTP_CONFLICT));
    }

    @Test
    public void testSetDnsQueryFailed() {
        final boolean input = true;
        mSscServiceStateAgent.setDnsQueryFailed(SLOT_0, input);

        verify(mockSscServiceState).setDnsQueryFailed(eq(input));
    }

    @Test
    public void testSetGbaRequestFailed() {
        final boolean input = true;
        mSscServiceStateAgent.setGbaRequestFailed(SLOT_0, input);

        verify(mockSscServiceState).setGbaRequestFailed(eq(input));
    }

    @Test
    public void testSetPdnConnectionTimerExpired() {
        final boolean input = false;
        mSscServiceStateAgent.setPdnConnectionTimerExpired(SLOT_0, input);

        verify(mockSscServiceState).setPdnConnectionTimerExpired(eq(input));
    }

    @Test
    public void testSetSocketConnectionExpired() {
        final boolean input = true;
        mSscServiceStateAgent.setSocketConnectionExpired(SLOT_0, input);

        verify(mockSscServiceState).setSocketConnectionExpired(eq(input));
    }

    @Test
    public void testSetAllSrvAddrTried() {
        final boolean input = false;
        mSscServiceStateAgent.setAllSrvAddrTried(SLOT_0, input);

        verify(mockSscServiceState).setAllSrvAddrTried(eq(input));
    }

    @Test
    public void testGetDnsQueryFailed() {
        mSscServiceStateAgent.getDnsQueryFailed(SLOT_0);

        verify(mockSscServiceState).getDnsQueryFailed();
    }

    @Test
    public void testGetGbaRequestFailed() {
        mSscServiceStateAgent.getGbaRequestFailed(SLOT_0);

        verify(mockSscServiceState).getGbaRequestFailed();
    }

    @Test
    public void testGetPdnConnectionTimerExpired() {
        mSscServiceStateAgent.getPdnConnectionTimerExpired(SLOT_0);

        verify(mockSscServiceState).getPdnConnectionTimerExpired();
    }

    @Test
    public void testGetSocketConnectionExpired() {
        mSscServiceStateAgent.getSocketConnectionExpired(SLOT_0);

        verify(mockSscServiceState).getSocketConnectionExpired();
    }

    @Test
    public void testGetAllSrvAddrTried() {
        mSscServiceStateAgent.getAllSrvAddrTried(SLOT_0);

        verify(mockSscServiceState).getAllSrvAddrTried();
    }

    @Test
    public void testGetPdnConnectionFailed() {
        mSscServiceStateAgent.getPdnConnectionFailed(SLOT_0);

        verify(mockSscServiceState).getPdnConnectionFailed();
    }
}