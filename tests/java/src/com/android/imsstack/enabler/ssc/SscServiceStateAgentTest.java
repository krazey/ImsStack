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

import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.verify;

import android.os.Looper;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.ArrayList;

@RunWith(JUnit4.class)
public class SscServiceStateAgentTest {
    private final static int SLOT_0 = 0;
    private SscServiceStateAgent mSscServiceStateAgent;

    @Mock private SscServiceState mockSscServiceState;

    @Before
    public void setup() {
        MockitoAnnotations.initMocks(this);

        AppContext.init(new ContextFixture().getTestDouble());

        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        mSscServiceStateAgent = SscServiceStateAgent.getInstance();
        mSscServiceStateAgent.init(SLOT_0, Looper.myLooper());

        mSscServiceStateAgent.setSscServiceState(SLOT_0, mockSscServiceState);
    }

    @After
    public void tearDown() {
        mSscServiceStateAgent.deInit(SLOT_0);

        AppContext.deinit();
    }

    @Test
    public void testInit() {
        assertEquals(mockSscServiceState, mSscServiceStateAgent.getSscServiceState(SLOT_0));
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
    public void testIsUtAvailable_whenServiceStateIsNull() {
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        boolean isAvailable = mSscServiceStateAgent.isUtAvailable(SLOT_0);

        assertEquals(false, isAvailable);
        verify(mockSscServiceState, never()).isUtAvailable();
    }

    @Test
    public void testChangeCapabilities() {
        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();

        mSscServiceStateAgent.changeCapabilities(SLOT_0, enabledCaps, disabledCaps);

        verify(mockSscServiceState).changeCapabilities(enabledCaps, disabledCaps);
    }

    @Test
    public void testChangeCapabilities_whenServiceStateIsNull() {
        ArrayList<CapabilityPair> enabledCaps = new ArrayList<CapabilityPair>();
        ArrayList<CapabilityPair> disabledCaps = new ArrayList<CapabilityPair>();
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.changeCapabilities(SLOT_0, enabledCaps, disabledCaps);

        verify(mockSscServiceState, never()).changeCapabilities(enabledCaps, disabledCaps);
    }

    @Test
    public void testSetErrorResponseCode() {
        mSscServiceStateAgent.setErrorResponseCode(SLOT_0, SscConstant.HTTP_CONFLICT);

        verify(mockSscServiceState).setErrorResponseCode(SscConstant.HTTP_CONFLICT);
    }

    @Test
    public void testSetErrorResponseCode_whenServiceStateIsNull() {
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.setErrorResponseCode(SLOT_0, SscConstant.HTTP_CONFLICT);

        verify(mockSscServiceState, never()).setErrorResponseCode(SscConstant.HTTP_CONFLICT);
    }

    @Test
    public void testSetPdnConnectionFailed() {
        final int smCause = 27;

        mSscServiceStateAgent.setPdnConnectionFailed(SLOT_0, smCause);

        verify(mockSscServiceState).setPdnConnectionFailed(smCause);
    }

    @Test
    public void testSetPdnConnectionFailed_whenServiceStateIsNull() {
        final int smCause = 27;
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.setPdnConnectionFailed(SLOT_0, smCause);

        verify(mockSscServiceState, never()).setPdnConnectionFailed(smCause);
    }

    @Test
    public void testSetDnsQueryFailed() {
        final boolean input = true;

        mSscServiceStateAgent.setDnsQueryFailed(SLOT_0, input);

        verify(mockSscServiceState).setDnsQueryFailed(input);
    }

    @Test
    public void testSetDnsQueryFailed_whenServiceStateIsNull() {
        final boolean input = true;
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.setDnsQueryFailed(SLOT_0, input);

        verify(mockSscServiceState, never()).setDnsQueryFailed(input);
    }

    @Test
    public void testSetGbaRequestFailed() {
        final boolean input = true;

        mSscServiceStateAgent.setGbaRequestFailed(SLOT_0, input);

        verify(mockSscServiceState).setGbaRequestFailed(input);
    }

    @Test
    public void testSetGbaRequestFailed_whenServiceStateIsNull() {
        final boolean input = true;
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.setGbaRequestFailed(SLOT_0, input);

        verify(mockSscServiceState, never()).setGbaRequestFailed(input);
    }

    @Test
    public void testSetPdnConnectionTimeout() {
        final boolean input = false;

        mSscServiceStateAgent.setPdnConnectionTimeout(SLOT_0, input);

        verify(mockSscServiceState).setPdnConnectionTimeout(input);
    }

    @Test
    public void testSetPdnConnectionTimeout_whenServiceStateIsNull() {
        final boolean input = false;
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.setPdnConnectionTimeout(SLOT_0, input);

        verify(mockSscServiceState, never()).setPdnConnectionTimeout(input);
    }

    @Test
    public void testSetSocketConnectionExpired() {
        final boolean input = true;

        mSscServiceStateAgent.setSocketConnectionExpired(SLOT_0, input);

        verify(mockSscServiceState).setSocketConnectionExpired(input);
    }

    @Test
    public void testSetSocketConnectionExpired_whenServiceStateIsNull() {
        final boolean input = true;
        mSscServiceStateAgent.removeSscServiceState(SLOT_0);

        mSscServiceStateAgent.setSocketConnectionExpired(SLOT_0, input);

        verify(mockSscServiceState, never()).setSocketConnectionExpired(input);
    }
}